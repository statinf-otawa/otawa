/*
 *	CFGProvider class implementation.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2022, IRIT UPS.
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

#include <otawa/cfg/CFGProvider.h>
#include <otawa/prog/Process.h>
#include <otawa/prog/Symbol.h>

namespace otawa {

///
class DisassemblyView: public view::View {
	friend class CFGProvider;
public:
	DisassemblyView(WorkSpace& ws)
		: View("disassembly", "Disassembly", "Machine instructions composing the program."),
		_ws(ws)
		{}
	
	void start(otawa::BasicBlock *b) override {
		i = b->begin();
		if(!i.ended())
			g = p::id<Symbol *>::Getter(*i, SYMBOL);	
	}
	
	Inst *item() const override { return *i; }
	
	void next() override {
		if(!g.ended())
			g.next();
		else {
			i.next();
			if(!i.ended())
				g = p::id<Symbol *>::Getter(*i, SYMBOL);
		}
	}
	
	bool ended() const override { return i.ended(); }
	
	void print(io::Output& out) override {
		if(!g.ended())
			out << g->name() << ':';
		else
			i->dump(out);
	}
	
private:
	WorkSpace& _ws;
	BasicBlock::InstIter i;
	p::id<Symbol *>::Getter g;
};


///
class SourceView: public view::View {
public:
	SourceView(WorkSpace& ws):
		view::View("source", "Source"),
		_ws(ws)
	{ }
 
	void start(BasicBlock *b) override
		{ i = b->begin(); file = ""; line = 0; lookup(); }
	bool ended() const override { return i.ended(); }
	void next() override { i.next(); lookup(); }
	Inst * item() const override { return *i; }
	void print(io::Output& out) override { out << file << ':' << line; }
	
private:
	
	void lookup() {
		while(!i.ended()) {
			auto r = _ws.process()->getSourceLine(i->address());
			if(r && ((*r).fst != file || (*r).snd != line)) {
				file = (*r).fst;
				line = (*r).snd;
				break;
			}
			i.next();
		}
	}
	
	WorkSpace& _ws;
	cstring file;
	int line;
	BasicBlock::InstIter i;
};

	
/**
 * @class CFGProvider
 * Base class of all code processor producing CFGs. It provides common services
 * as availability of views like disassembly or source;
 * 
 * @par Provided features
 * * @ref COLLECTED_CFG_FEATURE
 * 
 * @par Required features
 * * view::BASE_FEATURE
 * * LABEL_FEATURE
 * 
 * @ingroup cfg
 */

p::declare CFGProvider::reg
	= p::init("otawa::CFGProvider", Version(1, 0, 0))
	.provide(COLLECTED_CFG_FEATURE)
	.require(view::BASE_FEATURE)
	.require(LABEL_FEATURE);

///
CFGProvider::CFGProvider(p::declare& r):
	Processor(r),
	coll(nullptr),
	dview(nullptr),
	sview(nullptr)
	{}

///	
void *CFGProvider::interfaceFor(const AbstractFeature& feature) {
	if(&feature == &COLLECTED_CFG_FEATURE)
		return coll;
	else
		return nullptr;
}

///
void CFGProvider::commit(WorkSpace *ws) {
	INVOLVED_CFGS(workspace()) = coll;
	ENTRY_CFG(workspace()) = coll->entry();
	auto base = view::BASE_FEATURE.get(ws);
	dview = new DisassemblyView(*ws);
	base->add(dview);
	sview = new SourceView(*ws);
	base->add(sview);
}

///
void CFGProvider::destroy(WorkSpace *ws) {
	if(dview != nullptr) {
		auto base = view::BASE_FEATURE.get(ws);
		base->remove(dview);
		delete dview;
		base->remove(sview);
		delete sview;
	}
	if(coll != nullptr) {
		delete coll;
		INVOLVED_CFGS(ws).remove();
		ENTRY_CFG(ws).remove();
		coll = nullptr;
	}
}

/**
 * @fn inline CFGCollection *CFGProvider::collection();
 * Get the collection.
 * @return CFG collection.
 */

/**
 * Set the current CFG collection.
 * @param coll	Collection to set.
 */
void CFGProvider::setCollection(CFGCollection *collection) {
	this->coll = collection;
}

}	// otawa

