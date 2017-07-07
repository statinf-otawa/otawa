/*
 *	cfgio::Input class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <elm/avl/Map.h>
#include <elm/io/BlockInStream.h>
#include <elm/genstruct/HashTable.h>
#include <elm/util/UniquePtr.h>
#include <elm/xom.h>

#include <otawa/cfg/features.h>
#include <otawa/cfgio/features.h>
#include <otawa/cfgio/Input.h>
#include <otawa/proc/Processor.h>
#include <otawa/prog/Process.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/WorkSpace.h>


namespace elm {

namespace dtd {

using namespace elm;

class Exception: public elm::Exception {
public:
	Exception(xom::Element *element, const string& message): elt(element), msg(message) { }
	virtual string message(void)
		{ return _ << elt->getDocument()->getBaseURI() << ':' << elt->line() << ": " << msg; }
private:
	xom::Element *elt;
	const string& msg;
};

class Element;
class Factory;

class Parser: public PreIterator<Parser, xom::Node *> {
public:
	Parser(Factory *factory, Element& element, xom::Element *parent): fact(factory) {
		cur.elt = &element;
		cur.xelt = parent;
		cur.i = 0;
	}

	inline Factory *factory(void) const { return fact; }
	inline Element& element(void) const { return *cur.elt; }
	inline Option<xom::String> get(xom::String name) const { return cur.xelt->getAttributeValue(name); }
	inline void raise(const string& msg) const throw(Exception) { throw Exception(cur.xelt, msg); }

	inline bool ended(void) const { return cur.i >= cur.xelt->getChildCount(); }
	xom::Node *item(void) const {  return cur.xelt->getChild(cur.i); }
	inline void next(void) { cur.i++; }

	typedef int mark_t;
	inline mark_t mark(void) { return cur.i; }
	inline bool backtrack(mark_t m) { cur.i = m; return false; }

	void push(Element& element) {
		stack.push(cur);
		cur.i = 0;
		cur.elt = &element;
		cur.xelt = static_cast<xom::Element *>(item());
	}

	inline void pop(void) { cur = stack.pop(); }

private:
	Factory *fact;
	typedef struct {
		int i;
		Element *elt;
		xom::Element *xelt;
	} context_t;
	context_t cur;
	genstruct::Vector<context_t> stack;
};


class Factory {
public:
	virtual ~Factory(void) { }
	virtual void begin(Element& element) = 0;
	virtual void end(Element& element) = 0;
	virtual void failed(Element& element) = 0;
	virtual bool hasID(Element& element, xom::String id) = 0;
	virtual void *getID(Element& element, xom::String id) = 0;
	virtual void setID(Element& element, xom::String id) = 0;
};

class Attribute {
public:
	static const t::uint32
		REQUIRED = 0x01,	// attribute is required
		STRICT = 0x02;		// bad parsing causes an exception
	Attribute(xom::String name, t::uint32 flags = 0): _name(name), _flags(flags) { }
	virtual ~Attribute(void) { }
	inline xom::String name(void) const { return _name; }
	inline bool isRequired(void) const { return _flags & REQUIRED; }
	inline bool isStrict(void) const { return _flags & STRICT; }

	bool parse(Parser& parser) throw(Exception) {
		Option<xom::String> val = parser.get(_name);
		if(val)
			return process(parser, val);
		else {
			reset();
			return !isRequired();
		}
	}

	virtual bool process(Parser& parser, xom::String value) throw(Exception) = 0;
	virtual void reset(void) { }
private:
	xom::String _name;
	t::uint32 _flags;
};


class Content {
public:

	virtual ~Content(void) { }
	virtual bool parse(Parser& parser) throw(Exception) = 0;

	static bool isEmpty(xom::Node *node) {
		if(node->kind() != xom::Node::TEXT)
			return false;
		else {
			xom::String t = static_cast<xom::Text *>(node)->getText();
			for(int i = 0; i < t.length(); i++)
				switch(t[i]) {
				case ' ':
				case '\t':
				case '\v':
				case '\n':
					continue;
				default:
					return false;
				}
			return true;
		}
	}
};

class EmptyContent: public Content {
public:
	virtual bool parse(Parser& parser) throw(Exception) {
		Parser::mark_t m = parser.mark();
		for(; parser; parser++)
			if(!isEmpty(*parser)) {
				parser.backtrack(m);
				return false;
			}
		return true;
	}
};

static EmptyContent _empty;
Content& empty = _empty;

class Element: public Content {
public:
	class Make {
		friend class Element;
	public:
		inline Make(xom::String name, int kind = 0): _name(name), _kind(kind), _content(&empty) { }
		inline Make& attr(Attribute& attr) { attrs.add(&attr); return *this; }
		inline Make& kind(int kind) { _kind = kind; return *this; }
		inline Make& content(Content& content) { _content = &content; return *this; }
	private:
		xom::String _name;
		int _kind;
		genstruct::Vector<Attribute *> attrs;
		Content *_content;
	};

	Element(xom::String name, int kind = 0): _name(name), _kind(kind), _content(empty) { }
	Element(const Make& m): _name(m._name), _kind(m._kind), attrs(m.attrs), _content(*m._content) { }

	virtual bool parse(Parser& parser) throw(Exception) {
		if(!parser)
			return false;

		// check element
		xom::Node *node = *parser;
		if(node->kind() != xom::Node::ELEMENT)
			return false;
		xom::Element *element = static_cast<xom::Element *>(node);
		if(element->getLocalName() != name())
			return false;
		parser.push(*this);
		parser.factory()->begin(*this);

		// parse attributes
		for(int j = 0; j < attrs.length(); j++)
			if(!attrs[j]->parse(parser)) {
				parser.factory()->failed(*this);
				return false;
			}

		// parse content
		bool success = _content.parse(parser);

		// terminate the parsing
		if(!success)
			parser.factory()->failed(*this);
		else
			parser.factory()->end(*this);
		parser.pop();
		if(success)
			parser++;
		return success;
	}

	inline xom::String name(void) const { return _name; }
	inline int kind(void) const { return _kind; }
	inline Content& content(void) const { return _content; }

private:
	xom::String _name;
	int _kind;
	genstruct::Vector<Attribute *> attrs;
	Content& _content;
};


/**
 * Represent an optional content: error causes no match.
 */
class Optional: public Content {
public:
	Optional(Content& content): con(content) { }

	virtual bool parse(Parser& parser) throw(Exception) {
		if(parser)
			con.parse(parser);
		return true;
	}

private:
	Content& con;
};


/**
 * Represent an alternative between two contents.
 */
class Alt: public Content {
public:
	Alt(Content& content1, Content& content2): con1(content1), con2(content2) { }

	virtual bool parse(Parser& parser) throw(Exception) {
		if(!parser)
			return false;
		if(con1.parse(parser))
			return true;
		else if(con2.parse(parser))
			return true;
		else
			return false;
	}

private:
	Content &con1, &con2;
};

class Seq: public Content {
public:
	Seq(Content& content1, Content& content2, bool crop = true): con1(content1), con2(content2), _crop(crop) { }

	virtual bool parse(Parser& parser) throw(Exception) {
		if(!parser)
			return false;
		Parser::mark_t m = parser.mark();

		// crop spaces
		if(_crop) {
			for(; parser && isEmpty(*parser); parser++);
			if(!parser)
				return parser.backtrack(m);
		}

		// look for first content
		if(!con1.parse(parser))
			return parser.backtrack(m);

		// crop spaces
		if(_crop) {
			for(; parser && isEmpty(*parser); parser++);
			if(!parser)
				return parser.backtrack(m);
		}

		// look for second content
		if(!con2.parse(parser))
			return parser.backtrack(m);

		// crop last spaces
		if(_crop)
			for(; parser && isEmpty(*parser); parser++);

		// step on
		return true;
	}

private:
	Content &con1, &con2;
	bool _crop;
};


class Repeat: public Content {
public:
	Repeat(Content& content, bool crop = true): _crop(crop), con(content) { }

	virtual bool parse(Parser& parser) throw(Exception) {
		while(1) {

			// crop spaces
			if(_crop) {
				for(; parser && isEmpty(*parser); parser++);
				if(!parser)
					break;
			}

			// look for first content
			if(con.parse(parser))
				break;
		}
		return true;
	}

private:
	bool _crop;
	Content& con;
};

class TextAttr: public Attribute {
public:
	TextAttr(xom::String name, xom::String init = "", t::uint32 flags = 0): Attribute(name, flags), s(init), i(init) { }
	xom::String& operator*(void) { return s; }
	virtual bool process(Parser& parser, xom::String value) throw(Exception) { s = value; return true; }
	virtual void reset(void) { s = i; }
private:
	xom::String s, i;
};

class IntAttr: public Attribute {
public:
	IntAttr(xom::String name, int init = 0, t::uint32 flags = 0): Attribute(name, flags), v(init), i(init) { }
	int& operator*(void) { return v; }

	virtual bool process(Parser& parser, xom::String value) throw(Exception) {
		static elm::io::Input in;
		io::BlockInStream stream(value);
		in.setStream(stream);
		try {
			in >> v;
			if(stream.read() == elm::io::InStream::ENDED)
				return true;
			else if(isStrict())
				parser.raise(_ << "garbage after integer in " << name());
		}
		catch(elm::io::IOException& e) {
			if(isStrict())
				parser.raise(_ << "bad formatted integer in " << name());
		}
		return false;
	}

	virtual void reset(void) { v = i; }
private:
	int v, i;
};


class IDAttr: public Attribute {
public:
	IDAttr(xom::String name, t::uint32 flags = 0): Attribute(name, flags) { }
	virtual bool process(Parser& parser, xom::String value) throw(Exception) {
		if(parser.factory()->hasID(parser.element(), value)) {
			if(isStrict())
				parser.raise(_ << "already used identifier \"" << value << "\"in " << name());
			else
				return false;
		}
		parser.factory()->setID(parser.element(), value);
		return true;
	}
};


template <class T>
class RefAttr: public Attribute {
public:
	RefAttr(xom::String name, t::uint32 flags = 0): Attribute(name, flags), ref(0) { }
	inline T *reference(void) const { return ref; }
	virtual bool process(Parser& parser, xom::String value) throw(Exception) {
		if(!parser.factory()->hasID(parser.element(), value)) {
			if(isStrict())
				parser.raise(_ << "undefined reference \"" << value << "\" in " << name());
			else
				return false;
		}
		ref = static_cast<T *>(parser.factory()->getID(parser.element(), value));
		return true;
	}
	virtual void reset(void) { ref = 0; }
private:
	T *ref;
};

typedef Element::Make make;

class GC {
public:
	~GC(void) {
		for(genstruct::SLList<Content *>::Iterator con; con; con++)
			delete *con;
	}
	inline Content& add(Content *c) { to_free.add(c); return *c; }
private:
	genstruct::SLList<Content *> to_free;
};
GC _gc;

inline Content& operator*(Content& c)
	{ return _gc.add(new Repeat(c)); }
inline Content& operator+(Content& c1, Content& c2) { return _gc.add(new Alt(c1, c2)); }
inline Content& operator|(Content& c1, Content& c2) { return _gc.add(new Alt(c1, c2)); }
inline Content& operator,(Content& c1, Content& c2) { return _gc.add(new Seq(c1, c2)); }
inline Content& operator&(Content& c1, Content& c2) { return _gc.add(new Seq(c1, c2)); }

const t::uint32 STRICT = Attribute::STRICT;
const t::uint32 REQUIRED = Attribute::REQUIRED;

} }		// dtd::elm

namespace otawa {

namespace cfgio {

using namespace elm;

typedef enum {
	_NONE,
	_COLL,
	_CFG,
	_ENTRY,
	_BB,
	_EXIT,
	_EDGE
} entity_t;

dtd::IDAttr id("id", dtd::STRICT | dtd::REQUIRED);
dtd::IntAttr address("address", dtd::STRICT | dtd::REQUIRED);
dtd::IntAttr size("size", dtd::STRICT | dtd::REQUIRED);
dtd::RefAttr<BasicBlock *> source("source", dtd::STRICT | dtd::REQUIRED);
dtd::RefAttr<BasicBlock *> target("target", dtd::STRICT | dtd::REQUIRED);
dtd::RefAttr<CFG *> called("called", dtd::STRICT);

dtd::Element entry(dtd::make("entry", _ENTRY).attr(id));
dtd::Element bb(dtd::make("bb", _BB).attr(id).attr(address).attr(size));
dtd::Element exit(dtd::make("exit", _EXIT).attr(id));
dtd::Element edge(dtd::make("edge", _EDGE).attr(source).attr(target).attr(called));
dtd::Element cfg(dtd::make("cfg", _CFG).attr(id).content((entry, *bb, exit, *edge)));
dtd::Element cfg_collection(dtd::make("cfg-collection", _COLL).content((cfg, *cfg)));

static Identifier<Option<xom::String> > SYNTH_TARGET("");


/**
 * @addtogroup cfgio
 *
 * @section cfgio-input Input
 * The cfgio::Input provides the processor to construct CFGCollection (which is stored to INVOLVED_CFG(props)) from an XML file.
 * The XML file can be generated by using the cfgio::Output processor, or written by hand.
 * Users can use the manually created XML file to create CFGs of the desired topologies.
 * To create BasicBlock which does not exist in the program, one has to include the instruction with the address at 0xCAFEBABE to indicate
 * that such BasicBlock is created from scratch.
 * @code
 * 	<?xml version="1.0" encoding="UTF-8"?>
 * 	<cfg-collection>
 * 		<cfg id="test1">
 * 			<entry id="0"/>
 * 			<bb id="1"><inst address="0xCAFEBABE"/></bb>
 * 			<bb id="2"><inst address="0xCAFEBABE"/></bb>
 * 			<bb id="3"><inst address="0xCAFEBABE"/></bb>
 * 			<exit id="4" />
 * 			<edge source="0" target="1"/>
 * 			<edge source="1" target="2"/>
 * 			<edge source="1" target="3"/>
 * 			<edge source="2" target="3"/>
 * 			<edge source="3" target="4"/>
 * 		</cfg>
 * 	</cfg-collection>
 * @endcode
 *
 * otawa::cfgio::FROM can be used to specified the PATH of the XML file to be read (the defualt is main.xml).
 * @code
 * your_program --add-prop otawa::cfgio::FROM=PATH_TO_YOUR_XML_FILE
 * @endcode
 *
 * The feature of cfgio::Input can be used as with the feature otawa::cfgio::CFG_FILE_INPUT_FEATURE.
 * @code
 * workspace()->require(DynFeature("otawa::cfgio::CFG_FILE_INPUT_FEATURE"), props);
 * @endcode
 */

/**
 * @class Input
 * Create CFGCollection from an XML file matching the DTD ${OTAWA_HOME}/share/Otawa/dtd/cfg.dtd .
 * @ingroup cfgio
 */
Input::Input(p::declare& r): Processor(r), coll(0) {
	cafebabeInst = new CafeBabeInst();
}


/**
 *
 */
void Input::configure(const PropList& props) {
	Processor::configure(props);
	path = FROM(props);
	if(!path) {
		string task = TASK_ENTRY(props);
		if(task)
			path = task + ".xml";
	}
}


/**
 *
 */
void Input::setup(WorkSpace *ws) {
	reset();
}


/**
 *
 */
void Input::processWorkSpace(WorkSpace *ws) {
	// open the document
	xom::Builder builder;
	xom::Document *doc = builder.build(path.toString().toCString());
	if(!doc)
		raiseError(_ << " cannot open " << path);

	// get the top element
	xom::Element *top = doc->getRootElement();
	if(top->getLocalName() != "cfg-collection")
		raiseError(top, "bad top level element");
	UniquePtr<xom::Elements> cfg_elts(top->getChildElements("cfg"));
	if(cfg_elts->size() == 0)
		raiseError(top, "no CFG");

	// prepare the CFGs and the BBs
	for(int i = 0; i < cfg_elts->size(); i++) {
		CFGMaker *maker = 0; // the CFG maker
		bb_map_t* bb_map = new bb_map_t;
		bb_map_table.add(bb_map);

		// get information
		xom::Element *celt = cfg_elts->get(i);
		Option<xom::String> cfgID = celt->getAttributeValue("id");
		if(!cfgID)
			raiseError(celt, "no 'id' attribute");
		if(cfg_map.hasKey(*cfgID))
			raiseError(celt, _ << "id " << *cfgID << " at least used two times.");


		// get entry
		xom::Element *eelt = celt->getFirstChildElement("entry");
		if(!eelt)
			raiseError(celt, "no entry element");
		Option<xom::String> entryID = eelt->getAttributeValue("id");
		if(!entryID)
			raiseError(eelt, "no entryID");
		if(bb_map->hasKey(*entryID))
			raiseError(eelt, _ << "entryID " << *entryID << " used at least two times.");

		// get exit
		eelt = celt->getFirstChildElement("exit");
		if(!eelt)
			raiseError(celt, "no exit element");
		Option<xom::String> exitID = eelt->getAttributeValue("id");
		if(!exitID)
			raiseError(eelt, "no exitID");
		if(bb_map->hasKey(*exitID))
			raiseError(eelt, _ << "exitID " << *exitID << " used at least two times.");

		// build the BBs
		UniquePtr<xom::Elements> bb_elts(celt->getChildElements("bb"));
		if(bb_elts->size() == 0)
			raiseError(celt, _ << "no BB in CFG " << *cfgID);

		genstruct::Vector<Block*> basicBlocksInOrder;

		Inst* firstInst = &otawa::Inst::null;
		for(int j = 0; j < bb_elts->size(); j++) {
			// get information, e.g.
			// <bb id="_0-1" number="1" address="0x00008d0c" size="24">
			xom::Element *bb_elt = bb_elts->get(j); // BB element
			Option<xom::String> blockID = bb_elt->getAttributeValue("id");
			if(!blockID)
				raiseError(eelt, "no blockID");
			if(bb_map->hasKey(*blockID))
				raiseError(eelt, _ << "blockID " << *blockID << " used at least two times.");

			// build the basic block
			// first we collect the instructions
			UniquePtr<xom::Elements> inst_elts(bb_elt->getChildElements("inst"));
			Vector<Inst *> insts(inst_elts->size()!=0?inst_elts->size():1); // we need size of 1 to have empty BB
			for(int k = 0; k < inst_elts->size(); k++) {
				// <inst address="0x0020099c" file="cover.c" line="232"/>
				xom::Element *inst_elt = inst_elts->get(k); // current instruction
				Option<xom::String> bbAddress = inst_elt->getAttributeValue("address");
				t::uint32 address = 0;
				if(bbAddress)
					*bbAddress >> address;
				Inst* currentInst = ws->process()->findInstAt(Address(address));

				if(!currentInst && address == 0xCAFEBABE) {
					currentInst = cafebabeInst;
				}
				else // instruction not found
					ASSERT(currentInst);

				insts.add(currentInst);
				if((firstInst == &otawa::Inst::null) && currentInst) {
					firstInst = currentInst;
				}
			}

			// now we create the block
			Block *nbb;
			if(insts.count()) { // if there are some instructions inside the block, then it is a normal BB
				nbb = new BasicBlock(insts.detach());
				basicBlocksInOrder.push(nbb);
			}
			else { // otherwise, we treat the block as Synth Block
				nbb = new SynthBlock(); // a basic block without instruction
				Option<xom::String> callID = bb_elt->getAttributeValue("call");
				if(callID) { // if the block has callID, which is the target of the call, then we mark it
					SYNTH_TARGET(nbb) = callID;
				}
			}

			// since the edges uses the BB id to connect, we need to use a map of id and the BBs
			bb_map->put(*blockID, nbb);
		}

		// build the CFG
		maker = new CFGMaker(firstInst);
		bb_map->put(*entryID, maker->entry()); // put the entry in the map
		cfg_map.put(*cfgID, maker);
		cfgMakers.add(maker);

		// add basic block in order ...
		// we don't add the Synth Blocks now because the id of the Synth Blocks are after the Basic Blocks
		// to have the same fashion of id numbering of the original and reconstructed CFG, we add the BB first.
		for(genstruct::Vector<Block*>::Iterator vbbi(basicBlocksInOrder); vbbi; vbbi++)
			maker->add(*vbbi);

		bb_map->put(*exitID, maker->exit()); // put the exit in the map

		// collect the Edges
		UniquePtr<xom::Elements> edge_elts(celt->getChildElements("edge"));
		if(edge_elts->size() == 0)
			raiseError(celt, _ << "no Edge in CFG " << *cfgID);

		edge_list_t* edge_list = new edge_list_t(edge_elts->size());
		edge_list_table.add(edge_list);
		for(int j = 0; j < edge_elts->size(); j++) {
			// get information, e.g.
			// <edge source="_0-5" target="_0-8"/>
			xom::Element *edge_elt = edge_elts->get(j); // BB element
			Option<xom::String> sourceID = edge_elt->getAttributeValue("source");
			if(!sourceID)
				raiseError(eelt, "no sourceID");
			Option<xom::String> targetID = edge_elt->getAttributeValue("target");
			if(!targetID)
				raiseError(eelt, "no targetID");
			edge_list->add(pair(*sourceID, *targetID));
		} // end of each edge
	} // end of each CFG

	// now we have all the CFGs, we can fill the info of CFGs to the Synth Blocks
	int cfgIndex = 0;
	for(bb_map_table_t::Iter bbmtti(bb_map_table); bbmtti; bbmtti++, cfgIndex++) { // for each CFG
		CFGMaker* cfgMaker = cfgMakers[cfgIndex];
		for(bb_map_t::Iterator bbmti(**bbmtti); bbmti; bbmti++) { // for each entry in the bb_map
			if(bbmti->isSynth()) { // now process the synth block. basic block were processed previously
				if(*SYNTH_TARGET(*bbmti)) { // if there is an ID of the caller
					cfgMaker->call(*bbmti->toSynth(),**cfg_map.get(**SYNTH_TARGET(*bbmti))); // get the map ID from the cfg_map
					SYNTH_TARGET(*bbmti).remove(); // clear the property
				}
				else
					cfgMaker->add(*bbmti);
			}

		} // for each Block
	} // for each CFG

	// build the edges
	cfgIndex = 0;
	for(edge_list_table_t::Iter eltti(edge_list_table); eltti; eltti++, cfgIndex++) { // for each CFG
		CFGMaker* cfgMaker = cfgMakers[cfgIndex];
		bb_map_t* bb_map = bb_map_table[cfgIndex];
		for(edge_list_t::Iter elti(**eltti); elti; elti++) {
			Block* sourceBlock = 0;
			if(bb_map->hasKey((*elti).fst))
				sourceBlock = bb_map->get((*elti).fst);
			else {
				// sourceBlock = cfgMaker->unknown();
				sourceBlock = new SynthBlock();
				cfgMaker->add(sourceBlock);
			}

			Block* targetBlock = 0;
			if(bb_map->hasKey((*elti).snd))
				targetBlock = bb_map->get((*elti).snd);
			else {
				//targetBlock = cfgMaker->unknown();
				targetBlock = new SynthBlock();
				cfgMaker->add(targetBlock);
			}

			// build the edge
			if(	sourceBlock->isEntry() || targetBlock->isExit() ||
			(sourceBlock->isBasic() && sourceBlock->toBasic()->first() == cafebabeInst) ||
			(targetBlock->isBasic() && targetBlock->toBasic()->first() == cafebabeInst) ||
			(sourceBlock->isBasic() && targetBlock->isBasic() && sourceBlock->toBasic()->last()->nextInst() == targetBlock->toBasic()->first())) {
				cfgMaker->add(sourceBlock, targetBlock, new Edge(Edge::NOT_TAKEN));
			}
			else
				cfgMaker->add(sourceBlock, targetBlock, new Edge(Edge::TAKEN));
		} // for each Edge
	} // For each CFG

	// cleanup
	for(bb_map_table_t::Iter i(bb_map_table); i; i++)
		delete *i;
	for(edge_list_table_t::Iter i(edge_list_table); i; i++)
		delete *i;
}

/**
 *
 */
void Input::cleanup(WorkSpace *ws) {
	CFGCollection *new_coll = new CFGCollection();
	for(int i = 0; i < cfgMakers.count(); i++) {
		CFG *cfg = cfgMakers[i]->build();
		new_coll->add(cfg);
		if(i == 0)
			addRemover(COLLECTED_CFG_FEATURE, ENTRY_CFG(ws) = cfg);
		delete cfgMakers[i];
	}
	track(COLLECTED_CFG_FEATURE, INVOLVED_CFGS(ws) = new_coll);
}


/**
 *
 */
void Input::reset(void) {
	coll = 0;
}


/**
 *
 */
void Input::clear(void) {
	if(coll)
		delete coll;
	reset();
}


/**
 *
 */
void Input::raiseError(xom::Element *elt, const string& msg) {
	raiseError(_ << msg << " at " << path << ": " << elt->line());
}


/**
 *
 */
void Input::raiseError(const string& msg) {
	clear();
	throw ProcessorException(*this, msg);
}




p::feature CFG_FILE_INPUT_FEATURE("otawa::cfgio::CFG_FILE_INPUT_FEATURE", new Maker<Input>());

p::declare Input::reg = p::init("otawa::cfgio::Input", Version(1, 0, 0))
	.maker<Input>()
	.require(otawa::DECODED_TEXT)
	.provide(otawa::COLLECTED_CFG_FEATURE)
	.provide(otawa::cfgio::CFG_FILE_INPUT_FEATURE);

/**
 *
 * @ingroup cfgio
 */
Identifier<Path> FROM("otawa::cfgio::FROM");

} }	// otawa::cfgio

