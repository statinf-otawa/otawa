/*
 *	CFGAnalyzer class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2020, IRIT UPS.
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
#ifndef OTAWA_AI_CFGANALYZER_H_
#define OTAWA_AI_CFGANALYZER_H_

#include <elm/io/StructuredOutput.h>
#include "Domain.h"

namespace otawa { namespace ai {

class AbstractInterpreter {
public:
protected:
	inline AbstractInterpreter(Domain& domain):
		dom(domain), bot(dom.bot()), top(dom.top()) { }
	Domain& dom;
	State *bot, *top;
};

typedef std::function<void(State *)> state_collector_t;

class CFGAnalyzer: public AbstractInterpreter {
public:
	CFGAnalyzer(Monitor& monitor, Domain& domain, State *entry = nullptr);
	~CFGAnalyzer();

	void process();

	inline State *before(Edge *e) { return states[e->source()->id()]; }
	State *after(Edge *e);
	State *before(Block *v);
	inline State *after(Block *v)  { return states[v->id()]; }
	inline void release(State *s) { in_use.remove(s); }
	inline void use(State *s) { in_use.add(s); }

	inline void collect(state_collector_t f) {
		f(s0); f(bot); f(top);
		if(is != nullptr) f(is);
		if(es != nullptr) f(es);
		for(auto s: states) if(s != nullptr) f(s);
		for(auto s: in_use) if(s != nullptr) f(s);
	}

	void setTrace(io::StructuredOutput& t);

private:
	
	void beginTrace();
	void endTrace();
	void doTrace(Block *v, cstring type, State *s);
	
	Monitor& mon;
	const CFGCollection *cfgs;
	AllocArray<State *> states;
	State *is, *es, *s0;
	bool verbose, verbose_inst;
	List<State *> in_use;
	io::StructuredOutput *trace;
};

} }	// otawa::ai

#endif /* OTAWA_AI_CFGANALYZER_H_ */
