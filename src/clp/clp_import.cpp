/*
 *	clp::Input class implementation
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
#include <elm/io/OutFileStream.h>
#include <elm/xom/Serializer.h>

#include <elm/avl/Set.h>
#include <elm/xom.h>
#include <otawa/cfg.h>
#include <otawa/cfgio/features.h>
#include <otawa/proc/BBProcessor.h>

#include <otawa/data/clp/features.h>
#include <otawa/dfa/State.h>

#include <otawa/ipet/features.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/prog/Process.h>
#include <otawa/prop/DynIdentifier.h>

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
	inline void raise(const string& msg) const { throw Exception(cur.xelt, msg); }

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
	Vector<context_t> stack;
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

	bool parse(Parser& parser) {
		Option<xom::String> val = parser.get(_name);
		if(val)
			return process(parser, val);
		else {
			reset();
			return !isRequired();
		}
	}

	virtual bool process(Parser& parser, xom::String value) = 0;
	virtual void reset(void) { }
private:
	xom::String _name;
	t::uint32 _flags;
};


class Content {
public:

	virtual ~Content(void) { }
	virtual bool parse(Parser& parser) = 0;

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
	virtual bool parse(Parser& parser) {
		Parser::mark_t m = parser.mark();
		for(; parser(); parser++)
			if(!isEmpty(*parser)) {
				parser.backtrack(m);
				return false;
			}
		return true;
	}
};

static EmptyContent _empty;
Content& EMPTY = _empty;

class Element: public Content {
public:
	class Make {
		friend class Element;
	public:
		inline Make(xom::String name, int kind = 0): _name(name), _kind(kind), _content(&EMPTY) { }
		inline Make& attr(Attribute& attr) { attrs.add(&attr); return *this; }
		inline Make& kind(int kind) { _kind = kind; return *this; }
		inline Make& content(Content& content) { _content = &content; return *this; }
	private:
		xom::String _name;
		int _kind;
		Vector<Attribute *> attrs;
		Content *_content;
	};

	Element(xom::String name, int kind = 0): _name(name), _kind(kind), _content(EMPTY) { }
	Element(const Make& m): _name(m._name), _kind(m._kind), attrs(m.attrs), _content(*m._content) { }

	virtual bool parse(Parser& parser) {
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
	Vector<Attribute *> attrs;
	Content& _content;
};


/**
 * Represent an optional content: error causes no match.
 */
class Optional: public Content {
public:
	Optional(Content& content): con(content) { }

	virtual bool parse(Parser& parser) {
		if(parser())
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

	virtual bool parse(Parser& parser) {
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

	virtual bool parse(Parser& parser) {
		if(!parser)
			return false;
		Parser::mark_t m = parser.mark();

		// crop spaces
		if(_crop) {
			for(; parser() && isEmpty(*parser); parser++);
			if(!parser)
				return parser.backtrack(m);
		}

		// look for first content
		if(!con1.parse(parser))
			return parser.backtrack(m);

		// crop spaces
		if(_crop) {
			for(; parser() && isEmpty(*parser); parser++);
			if(!parser)
				return parser.backtrack(m);
		}

		// look for second content
		if(!con2.parse(parser))
			return parser.backtrack(m);

		// crop last spaces
		if(_crop)
			for(; parser() && isEmpty(*parser); parser++);

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

	virtual bool parse(Parser& parser) {
		while(1) {

			// crop spaces
			if(_crop) {
				for(; parser() && isEmpty(*parser); parser++);
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
	virtual bool process(Parser& parser, xom::String value) { s = value; return true; }
	virtual void reset(void) { s = i; }
private:
	xom::String s, i;
};

class IntAttr: public Attribute {
public:
	IntAttr(xom::String name, int init = 0, t::uint32 flags = 0): Attribute(name, flags), v(init), i(init) { }
	int& operator*(void) { return v; }

	virtual bool process(Parser& parser, xom::String value) {
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
	virtual bool process(Parser& parser, xom::String value) {
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
	virtual bool process(Parser& parser, xom::String value) {
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
		for(List<Content *>::Iter con; con(); con++)
			delete *con;
	}
	inline Content& add(Content *c) { to_free.add(c); return *c; }
private:
	List<Content *> to_free;
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

namespace clp {

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
 *
 * @ingroup clp
 */
Identifier<Path> IMPORT_PATH("otawa::clp::IMPORT_PATH", "pqr");


class CLPImport: public Processor {
	typedef avl::Map<xom::String, Block *> bb_map_t;
	typedef avl::Map<xom::String, CFGMaker *> cfg_map_t;
	typedef Vector<bb_map_t*> bb_map_table_t;
	typedef Vector<Pair<xom::String, xom::String> > edge_list_t;
	typedef Vector<edge_list_t*> edge_list_table_t;

public:
	// inner class
	class CafeBabeInst: public otawa::Inst {
	public:
		virtual kind_t kind(void) { return 0; }
		virtual Address address(void) const { return Address(0xCAFEBABE); }
		virtual t::uint32 size(void) const { return 4; }
	} *cafebabeInst;

	CLPImport(p::declare& r = reg);
	static p::declare reg;
protected:
	virtual void configure(const PropList& props);
	virtual void setup(WorkSpace *ws);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);

private:
	void processState(xom::Element *parent, clp::State& clpState);


	void reset(void);
	void clear(void);
	void raiseError(xom::Element *elt, const string& msg);
	void raiseError(const string& msg);

	Path path;
	CFGCollection *coll;
	cfg_map_t cfg_map;
	bb_map_table_t bb_map_table;
	edge_list_table_t edge_list_table;
	Vector<CFGMaker*> cfgMakers;
};


/**
 * @addtogroup clp
 *
 * @section clp-input Input
 * The clp::Input provides the processor to construct CFGCollection (which is stored to INVOLVED_CFG(props)) from an XML file.
 * The XML file can be generated by using the clp::Output processor, or written by hand.
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
 * otawa::clp::FROM can be used to specified the PATH of the XML file to be read (the defualt is main.xml).
 * @code
 * your_program --add-prop otawa::clp::FROM=PATH_TO_YOUR_XML_FILE
 * @endcode
 *
 * The feature of clp::Input can be used as with the feature otawa::clp::CFG_FILE_INPUT_FEATURE.
 * @code
 * workspace()->require(DynFeature("otawa::clp::CFG_FILE_INPUT_FEATURE"), props);
 * @endcode
 */

/**
 * @class CLPImport
 * Create CFGCollection from an XML file matching the DTD ${OTAWA_HOME}/share/Otawa/dtd/cfg.dtd .
 * @ingroup clp
 */
CLPImport::CLPImport(p::declare& r): Processor(r), coll(0) {
	cafebabeInst = new CafeBabeInst();
}


/**
 *
 */
void CLPImport::configure(const PropList& props) {
	Processor::configure(props);
	path = IMPORT_PATH(props);
	ASSERTP(path, "clp::IMPORT_PATH is not provided!\n");
}


/**
 *
 */
void CLPImport::setup(WorkSpace *ws) {
//	reset();
}

void CLPImport::processState(xom::Element *parent, clp::State& clpState) {
	clpState = clp::State::FULL;
	Option<xom::String> topXOM = parent->getAttributeValue("top");
	if(!topXOM) raiseError(parent, "no 'top' attribute");
	bool top = false;
	*topXOM >> top;
	if(top) {
	}
	else
	{
		// get the register values
		UniquePtr<xom::Elements> reg_elts(parent->getChildElements("reg"));
		for(int j = 0; j < reg_elts->size(); j++) {
			xom::Element *reg_elt = reg_elts->get(j);
			Option<xom::String> regIDXOM = reg_elt->getAttributeValue("id");
			if(!regIDXOM) raiseError(reg_elt, "no 'ID' attribute");
			long regID = 0;
			*regIDXOM >> regID;

			Option<xom::String> regBaseXOM = reg_elt->getAttributeValue("base");
			if(!regBaseXOM) raiseError(reg_elt, "no 'base' attribute");
			long regBase = 0; // <bb id="_0-4" number="4" address="0x80002174" size="22"> r15 = (-0x80000000, 0x1, inf)
			elm::cout << *regBaseXOM << endl;
			*regBaseXOM >> regBase;

			Option<xom::String> regDeltaXOM = reg_elt->getAttributeValue("delta");
			if(!regDeltaXOM) raiseError(reg_elt, "no 'delta' attribute");
			long regDelta = 0;
			*regDeltaXOM >> regDelta;

			Option<xom::String> regMtimesXOM = reg_elt->getAttributeValue("mtimes");
			if(!regMtimesXOM) raiseError(reg_elt, "no 'mtimes' attribute");
			long regMtimes = 0;
			*regMtimesXOM >> regMtimes;

			clp::Value addr(clp::REG, regID);
			clp::Value v = clp::Value(clp::VAL, regBase, regDelta, regMtimes);
			clpState.set(addr, v);
		} // end of getting register values

		// get the memory values
		UniquePtr<xom::Elements> memory_elts(parent->getChildElements("mem"));
		for(int j = 0; j < memory_elts->size(); j++) {
			xom::Element *memory_elt = memory_elts->get(j);
			Option<xom::String> memoryIDXOM = memory_elt->getAttributeValue("addr");
			if(!memoryIDXOM) raiseError(memory_elt, "no 'ADDRESS' attribute");
			long memoryID = 0;
			*memoryIDXOM >> memoryID;

			Option<xom::String> memoryBaseXOM = memory_elt->getAttributeValue("base");
			if(!memoryBaseXOM) raiseError(memory_elt, "no 'base' attribute");
			long memoryBase = 0;
			*memoryBaseXOM >> memoryBase;

			Option<xom::String> memoryDeltaXOM = memory_elt->getAttributeValue("delta");
			if(!memoryDeltaXOM) raiseError(memory_elt, "no 'delta' attribute");
			long memoryDelta = 0;
			*memoryDeltaXOM >> memoryDelta;

			Option<xom::String> memoryMtimesXOM = memory_elt->getAttributeValue("mtimes");
			if(!memoryMtimesXOM) raiseError(memory_elt, "no 'mtimes' attribute");
			long memoryMtimes = 0;\
			*memoryMtimesXOM >> memoryMtimes;

			clp::Value addr(clp::VAL, memoryID);
			clp::Value v = clp::Value(clp::VAL, memoryBase, memoryDelta, memoryMtimes);
			clpState.set(addr, v);
		} // end of getting memory values
	}
}


/**
 *
 */
void CLPImport::processWorkSpace(WorkSpace *ws) {
	const CFGCollection *cfgc = INVOLVED_CFGS(ws);
	const CFG* currentCFG = nullptr;
	BasicBlock* currentBB = nullptr;

	// open the document
	xom::Builder builder;
	xom::Document *doc = builder.build(path.toString().toCString());
	if(!doc)
		raiseError(_ << " cannot open " << path);

	// get the top element
	xom::Element *top = doc->getRootElement();
	if(top->getLocalName() != "clp-state-collection")
		raiseError(top, "bad top level element");
	UniquePtr<xom::Elements> cfg_elts(top->getChildElements("cfg"));
	if(cfg_elts->size() == 0)
		raiseError(top, "no CFG");

	// prepare the CFGs and the BBs
	for(int i = 0; i < cfg_elts->size(); i++) { // for each CFG
		// get information
		xom::Element *celt = cfg_elts->get(i);
		int cfgIndex = 0;
		Option<xom::String> cfgID = celt->getAttributeValue("id");
		if(!cfgID)
			raiseError(celt, "no 'id' attribute");
		(*cfgID).substring(1) >> cfgIndex;


		Option<xom::String> cfgAddress = celt->getAttributeValue("address");
		t::uint32 cfgAddr = 0;
		if(!cfgAddress)
			raiseError(celt, "no 'address' attribute");
		*cfgAddress >> cfgAddr;

		// checking if the CFG matches the address
		// elm::cout << "CFG " << cfgIndex << " with address 0x" << hex(cfgAddr) << endl;
		for(CFGCollection::Iter cfg(cfgc); cfg(); cfg++) {
			if(cfg->index() == cfgIndex) {
				ASSERTP(Address(cfgAddr) == cfg->address(), "CFG " << cfgIndex << "does not match with address 0x" << hex(cfgAddr));
				currentCFG = *cfg;
				break;
			}
		}

		// build the BBs
		UniquePtr<xom::Elements> bb_elts(celt->getChildElements("bb"));
		if(bb_elts->size() == 0)
			raiseError(celt, _ << "no BB in CFG " << *cfgID);

		for(int j = 0; j < bb_elts->size(); j++) {
			// get information, e.g.
			// <bb id="_0-1" number="1" address="0x00008d0c" size="24">
			xom::Element *bb_elt = bb_elts->get(j); // BB element
			Option<xom::String> blockID = bb_elt->getAttributeValue("number");
			if(!blockID)
				raiseError(bb_elt, "no 'number' attribute");
			int bbIndex = 0;
			*blockID >> bbIndex;

			Option<xom::String> bbAddress = bb_elt->getAttributeValue("address");
			if(!bbAddress)
				continue; // skip the non-BB

			t::uint32 bbAddr = 0;
			if(bbAddress)
				*bbAddress >> bbAddr;

			for(CFG::BlockIter cfgbi = currentCFG->blocks(); cfgbi(); cfgbi++) {
				if(cfgbi->index() == bbIndex) {
					currentBB = cfgbi->toBasic();
					ASSERTP(Address(bbAddr) == cfgbi->address(), "BB " << bbIndex << " does not match with address 0x" << hex(bbAddr));
					break;
				}
			}

			// extract state in
			UniquePtr<xom::Elements> state_in_elts(bb_elt->getChildElements("clp_state_in"));
			if(state_in_elts->size() == 0) raiseError(bb_elt, _ << "no CLP_STATE_IN in BB " << *blockID);
			xom::Element *state_in_elt = state_in_elts->get(0);
			clp::State clpState;
			processState(state_in_elt, clpState);
			clp::STATE_IN(currentBB) = clpState;

			// extract state out
			UniquePtr<xom::Elements> state_out_elts(bb_elt->getChildElements("clp_state_out"));
			if(state_out_elts->size() == 0) raiseError(bb_elt, _ << "no CLP_STATE_OUT in BB " << *blockID);
			xom::Element *state_out_elt = state_out_elts->get(0);
			processState(state_out_elt, clpState);
			clp::STATE_OUT(currentBB) = clpState;


		} // end of each BB entity
	} // end of each CFG
}

/**
 *
 */
void CLPImport::cleanup(WorkSpace *ws) {
}


/**
 *
 */
void CLPImport::reset(void) {
}


/**
 *
 */
void CLPImport::clear(void) {
}


/**
 *
 */
void CLPImport::raiseError(xom::Element *elt, const string& msg) {
	raiseError(_ << msg << " at " << path << ": " << elt->line());
}


/**
 *
 */
void CLPImport::raiseError(const string& msg) {
	clear();
	throw ProcessorException(*this, msg);
}

p::feature CLP_IMPORT_FEATURE("otawa::clp::CLP_IMPORT_FEATURE", new Maker<CLPImport>());

p::declare CLPImport::reg = p::init("otawa::clp::CLPImport", Version(1, 0, 0))
	.maker<CLPImport>()
	.require(COLLECTED_CFG_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(dfa::INITIAL_STATE_FEATURE)
	.provide(clp::CLP_IMPORT_FEATURE)
	.provide(clp::CLP_ANALYSIS_FEATURE);



} }	// otawa::clp

