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

#include <otawa/proc/Processor.h>
#include <otawa/cfg/features.h>
#include <otawa/cfgio/features.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/cfg/CFGCollector.h>
#include <elm/xom.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/cfg/VirtualCFG.h>
#include <elm/util/AutoDestructor.h>
#include <elm/io/BlockInStream.h>

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


/**
 * This processor allows to read-back CFG produced by cfgio::Output
 * or in XML CFG format described in ${OTAWA_HOME}/share/Otawa/dtd/cfg.dtd .
 *
 * @par Configuration
 * @li @ref otawa::cfgio::FROM
 *
 * @par Required Features
 * @li @ref otawa::DECODED_TEXT
 *
 * @par Provided Features
 * @li @ref otawa::COLLECTED_CFG_FEATURE
 *
 * @ingroup cfgio
 */
class Input: public Processor {
public:
	static p::declare reg;
	Input(p::declare& r = reg): Processor(r) { }

protected:

	virtual void configure(const PropList& props) {
		Processor::configure(props);
		path = FROM(props);
		if(!path) {
			string task = TASK_ENTRY(props);
			if(task)
				path = task + ".xml";
		}
	}

	virtual void setup(WorkSpace *ws) {
		reset();
	}

	virtual void process(WorkSpace *ws) {

		// open the document
		xom::Builder builder;
		xom::Document *doc = builder.build(path.toString().toCString());
		if(!doc)
			raiseError(_ << " cannot open " << path);

		// get the top element
		xom::Element *top = doc->getRootElement();
		if(top->getLocalName() != "cfg-collection")
			raiseError(top, "bad top level element");
		AutoDestructor<xom::Elements> cfg_elts = top->getChildElements("cfg");
		if(cfg_elts->size() == 0)
			raiseError(top, "no CFG");

		// prepare the CFGs and the BBs
		coll = new CFGCollection();
		for(int i = 0; i < cfg_elts->size(); i++) {

			// get information
			xom::Element *celt = cfg_elts->get(i);
			Option<xom::String> id = celt->getAttributeValue("id");
			if(!id)
				raiseError(celt, "no 'id' attribute");
			if(cfg_map.hasKey(*id))
				raiseError(celt, _ << "id " << *id << " at least used two times.");

			// build the CFG
			VirtualCFG *cfg = new VirtualCFG();
			cfg_map.put(*id, cfg);
			coll->add(cfg);

			// get entry
			xom::Element *eelt = celt->getFirstChildElement("entry");
			if(!eelt)
				raiseError(celt, "no entry element");
			Option<xom::String> eid = eelt->getAttributeValue("id");
			if(!eid)
				raiseError(eelt, "no ID");
			if(bb_map.hasKey(*eid))
				raiseError(eelt, _ << "ID " << *id << " used at least two times.");
			bb_map.put(*eid, cfg->entry());

			// get exit
			eelt = celt->getFirstChildElement("exit");
			if(!eelt)
				raiseError(celt, "no exit element");
			eid = eelt->getAttributeValue("id");
			if(!eid)
				raiseError(eelt, "no ID");
			if(bb_map.hasKey(*eid))
				raiseError(eelt, _ << "ID " << *id << " used at least two times.");
			bb_map.put(*eid, cfg->exit());

			// build the BBs
			AutoDestructor<xom::Elements> bb_elts = celt->getChildElements("bb");
			if(bb_elts->size() == 0)
				raiseError(celt, _ << "no BB in CFG " << *id);
			for(int j = 0; j < bb_elts->size(); j++) {

				// get information


				// build the basic block

			}
		}

		// build the edges

		// cleanup
	}

private:
	Path path;
	CFGCollection *coll;
	genstruct::HashTable<xom::String, VirtualCFG *> cfg_map;
	genstruct::HashTable<xom::String, BasicBlock *> bb_map;

	void reset(void) {
		coll = 0;
	}

	void clear(void) {
		if(coll)
			delete coll;
		reset();
	}

	void raiseError(xom::Element *elt, const string& msg) {
		raiseError(_ << msg << " at " << path << ": " << elt->line());
	}

	void raiseError(const string& msg) {
		clear();
		throw ProcessorException(*this, msg);
	}
};

p::declare Input::reg = p::init("otawa::cfgio::Input", Version(1, 0, 0))
	.maker<Input>()
	.require(otawa::DECODED_TEXT)
	.provide(otawa::COLLECTED_CFG_FEATURE);

/**
 *
 * @ingroup cfgio
 */
Identifier<Path> FROM("otawa::cfgio::FROM");

} }	// otawa::cfgio

