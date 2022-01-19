/*
 *	CFGAnalyzer class implementation
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

#include <elm/data/ListQueue.h>
#include <otawa/ai/CFGAnalyzer.h>
#include <otawa/ai/RankedQueue.h>

namespace otawa { namespace ai {

class Restructurer: public io::OutStream {
public:
	Restructurer(io::StructuredOutput& o): out(o) {}
	int write(const char *buffer, int size) override {
		for(int i = 0; i < size; i++) {
			if(buffer[i] != '\n')
				buf << buffer[i];
			else
				flush();
		}
		return size;
	}
	
	int flush(void) override {
		if(buf.length() != 0) {
			auto x = buf.toCString();
			out.write(x);
			buf.reset();
		}
		return 0;
	}

private:
	io::StructuredOutput& out;
	StringBuffer buf;
};

	
/**
 * @class State
 * This class is the opaque representation of a state in the abstract
 * interpretation. Any state involved in abstract interpretation
 * must be extend this class to be used in standatd abstract interpreters.
 * @ingroup ai
 */

/*
 * @class Domain
 * This class represents a domain in an abstract interpretation. It provides
 * all algorithm used in the analysis: bottom state, top state, update and
 * join. Moreover, it provides display and memory management facilities.
 *
 * The memory management has to be performed by the domain. However, an abstract
 * interpretation analyzer provides a function dedicated to collecting active
 * states at any moment.
 *
 * @ingroup ai
 */

///
Domain::~Domain() {
}

/**
 * @fn State *Domain::bot();
 * Get the bottom state of the domain. The bottom state does not need to be
 * unlocked.
 * @return	Bottom state.
 */

/**
 * @fn State *Domain::top();
 * Get the top state of the domain. The top state does not need to be
 * unlocked.
 */

/**
 * @fn State *entry();
 * Get the default entry state for an analysis in this domain.
 * @return	Default entry state.
 */

/**
 * @fn State *Domain::update(Edge *e, State *s);
 * Update the passed state according the passed edge.
 * @param e		Edge to update with.
 * @param s		State to update.
 */

/**
 * @fn State *Domain::join(State *s1, State *s2);
 * Join the given states.
 * @param s1		First state.
 * @param s2		Second state.
 * @return			State result of the join.
 */

/**
 * Join the two states coming from the given edges. The default implementation
 * just the join without edge parameters.
 * @param s1		First state.
 * @param s2		Second state.
 * @param e			Edge for the second state.
 * @return			State Result of the join.
 */
State *Domain::join(State *s1, State *s2, Edge *e) {
	return join(s1, s2);
}

/**
 * Update the the state s according to the block v.
 * The default implementation does nothing: it returns the input state s.
 * @param v		Block to update with.
 * @param s		Input state.
 * @return		Output state.
 */
State *Domain::update(Block *v, State *s) {
	return s;
}

/**
 * @fn bool Domain::equals(State *s1, State *s2);
 */

/**
 * Print the passed state. The default implementation of this function is
 * to display the address of the state.
 * @param s		State to display.
 * @param out	Output stream.
 */
void Domain::print(State *s, io::Output& out) {
	out << s;
}

/**
 * Return true if the print function is impplemented.
 * @return	True if the print is implemented.
 */
bool Domain::implementsPrinting() {
	return false;
}

/**
 * Return true if input-output operations are implemented on the states.
 * @return	True if input-output are implemented.
 */
bool Domain::implementsIO() {
	return false;
}

/**
 * Save the state in the given stream.
 * @param s					State to save.
 * @param out				Output stream.
 * @throw io::IOException	In case of error.
 */
void Domain::save(State *s, io::OutStream *out) {
	ASSERTP(false, "Domain::save() not implemented");
}

/**
 * Read a state from the given stream;
 * @param in				Input stream.
 * @return					Read state.
 * @throw io::IOException	In case of error.
 */
State *Domain::load(io::InStream *in) {
	ASSERTP(false, "Domain::load() not implemented");
	return nullptr;
}

/**
 * Test if the current domain supports the printing of the underlying code.
 * @return	True if the domain prints code (default to false).
 */
bool Domain::implementsCodePrinting() {
	return false;
}

/**
 * Print the code for the given block. Default implementation prints nothing.
 * @param b		Block to print code for.
 * @param out	Output stream to print to.
 */
void Domain::printCode(Block *b, io::Output& out) {
}

/**
 * Print the code for the given edge (if any). Default implementation prints
 * nothing.
 * @param e		Edge to print code for.
 * @param out	Output stream to print to.
 */
void Domain::printCode(Edge *e, io::Output& out) {
}

/**
 * Called to test if the domain suports tracing.
 * The default implementation returns false.
 * @return	True if it support tracing, false else.
 */
bool Domain::implementsTracing() {
	return false;
}

/**
 * Called to trace the given state on the given structured output.
 * The default implementation trace "0".
 * @param s		State to trace.
 * @param out	To output to.
 */
void Domain::printTrace(State *s, io::StructuredOutput& out) {
	out.write(0);
}


/**
 * @class CFGAnalyzer
 * This class implements an abstract interpretation applied to the
 * representation of the program as a set of CFG.
 * 
 * This analyzer works on a CFG G = <V, E, a> and computes the state s[v] for
 * each block of G:
 * * s[a] = s_init
 * * foreach v in V \ { a }, s[v] = U(v, J_{w->v in E} U(w->v, s[w]))
 * 
 * The following functions are provided by the domain:
 * * s_init -- initial value of the state before analysis,
 * * J(s1,s2) -- joins two abstract states s1 and s2,
 * * U(w->v, s) -- update the given state with the execution of edge w->v in E,
 * * U(v, s) -- update the given state with the execution of block v.
 * 
 * Therefore, the analyzer provides the following results:
 * * s^before_v  = J_{w->v in E} s[w]
 * * s^after_v = s[v]
 * * s^before_w->v = s[w]
 * * s^after_w->v = U(w->v, s[w])
 * 
 * @ingroup ai
 */

/**
 * Build the analyzer.
 * @param workspace		Current workspace.
 * @param domain		Current domain.
 * @param entry			Entry state.
 */
CFGAnalyzer::CFGAnalyzer(Monitor& monitor, Domain& domain, State *entry):
	AbstractInterpreter(domain),
	mon(monitor),
	cfgs(nullptr),
	is(dom.bot()),
	es(dom.bot()),
	s0(entry == nullptr ? dom.entry() : entry),
	verbose(false),
	verbose_inst(false),
	trace(nullptr)
{
}

///
CFGAnalyzer::~CFGAnalyzer() {
}


/**
 * Enable tracing of the computation on the given structured output.
 * Notice that tracing will only enabled if the current domain supports it.
 * @param t	Structured output to trace to.
 */
void CFGAnalyzer::setTrace(io::StructuredOutput& t) {
	if(dom.implementsTracing())
		trace = &t;
}


/**
 * Perform the analysis.
 */
void CFGAnalyzer::process() {
	if(mon.logFor(Monitor::LOG_BLOCK) && dom.implementsPrinting())
		verbose = true;
	if(mon.logFor(Monitor::LOG_INST) && dom.implementsCodePrinting())
		verbose_inst = true;

	// initialize
	cfgs = otawa::COLLECTED_CFG_FEATURE.get(mon.workspace());
	ASSERTP(cfgs, "otawa::COLLECTED_CFG_FEATURE must be required first!");
	if(trace != nullptr)
		beginTrace();
	State **buf = new State *[cfgs->countBlocks()];
	states.set(cfgs->countBlocks(), buf);
	if(verbose) {
		mon.log << "\ts0 = ";
		dom.print(s0, mon.log);
		mon.log << io::endl;
	}
	for(int i = 0; i < states.length(); i++)
		states[i] = bot;

	// prepare the queue
	//ListQueue<Block *> todo;
	Queue todo(cfgs);
	todo.put(cfgs->entry()->entry());
	while(todo) {
		auto v = todo.get();
		if(verbose) {
			mon.log << "\tprocessing " << v << " (" << v->cfg()->label() << ")\n";
			if(verbose_inst)
				dom.printCode(v, mon.log);
		}

		// synthetic block
		if(v->isSynth()) {
			auto c = v->toSynth();
			if(c->callee() == nullptr)
				is = top;
			else {
				todo.put(c->callee()->entry());
				continue;
			}
		}

		// entry block
		else if(v->isEntry()) {
			if(v->cfg()->isTop())
				is = s0;
			else {
				is = bot;
				for(auto c: v->cfg()->callers())
					for(auto e: c->inEdges()) {
						es = dom.update(e, states[e->source()->id()]);	// let the domain to account for calls
						is = dom.join(is, es, e);
					}
			}
		}

		// common processing
		else {
			is = bot;
			for(auto e: v->inEdges()) {
				if(verbose) {
					mon.log << "\t\tbefore " << e << ": ";
					dom.print(states[e->source()->id()], mon.log);
					mon.log << io::endl;
					if(verbose_inst)
						dom.printCode(e, mon.log);
				}
				es = dom.update(e, states[e->source()->id()]);
				if(verbose) {
					mon.log << "\t\tafter " << e << ": ";
					dom.print(es, mon.log);
					mon.log << io::endl;
				}
				is = dom.join(is, es, e);
			}
			if(verbose) {
				mon.log << "\t\tbefore " << v << ": ";
				dom.print(is, mon.log);
				mon.log << io::endl;
			}
			auto isp = is;
			is = dom.update(v, is);
			if(trace != nullptr) {
				doTrace(v, "in", isp);
				doTrace(v, "out", is);
			}
		}

		// record the new value
		if(verbose) {
			mon.log << "\t\tafter " << v << ": ";
			dom.print(is, mon.log);
			mon.log << io::endl;
		}
		if(is == states[v->id()] || dom.equals(is, states[v->id()]))
			/* nothing to push */;
		else {
			states[v->id()] = is;
			if(v->isExit())
				for(auto c: v->cfg()->callers()) {
					states[c->id()] = is;
					for(auto e: c->outEdges()) {
						todo.put(e->sink());
						if(verbose)
							mon.log << "\t\tput " << e->sink() << io::endl;
					}
				}
			else
				for(auto e: v->outEdges()) {
					todo.put(e->sink());
					if(verbose)
						mon.log << "\t\tput " << e->sink() << io::endl;
				}
		}
	}
	
	if(trace != nullptr) {
		endTrace();
		trace = nullptr;
	}
}


/**
 * Perform initial actions for beginning a trace: mainly generate the CFGs
 * involved in this analysis.
 */
void CFGAnalyzer::beginTrace() {
	Restructurer res(*trace);
	io::Output out(res);
	trace->beginMap();
	
	// output CFGs
	trace->key("program");
	trace->beginList();
	for(auto g: *cfgs) {
		trace->beginMap();
		trace->key("label"); trace->write(g->name());
		trace->key("id"); trace->write(g->index());
		
		// output blocks
		trace->key("blocks");
		trace->beginList();
		for(auto b: *g) {
			trace->beginMap();
			trace->key("id"); trace->write(b->id());
 			trace->key("title"); trace->write(_ << b);
			if(b->isBasic() && dom.implementsCodePrinting()) {
				trace->key("code");
				trace->beginList();
				dom.printCode(b, out);
				trace->endList();
			}
			else if(b->isSynth() && b->toSynth()->callee() != nullptr) {
				trace->key("to"); trace->write(b->toSynth()->callee()->index());
			}
			trace->endMap();
		}
		trace->endList();
		
		// output edges
		trace->key("edges");
		trace->beginList();
		for(auto v: *g)
			for(auto e: v->outEdges()) {
				trace->beginMap();
				trace->key("src"); trace->write(v->id());
				trace->key("snk"); trace->write(e->sink()->id());
				trace->key("taken"); trace->write(e->isTaken());
				trace->endMap();
			}
		trace->endList();
		trace->endMap();
	}
	trace->endList();
	
	// start outputting the analysis
	trace->key("analysis");
	trace->beginList();
}


/**
 * Perform final actions to complete a trace.
 */
void CFGAnalyzer::endTrace() {
	trace->endList();
	trace->endMap();
}


/**
 * Generate the given state in the trace.
 * @param v		Current block.
 * @param t		Type of the trace (usually one of "in" or "out").
 * @param s		State to generate.
 */
void CFGAnalyzer::doTrace(Block *v, cstring t, State *s) {
	trace->beginMap();
	trace->key("id"); trace->write(v->id());
	trace->key("type"); trace->write(t);
	trace->key("state"); dom.printTrace(s, *trace);
	trace->endMap();
}


/**
 * @fn State *CFGAnalyzer::before(Edge *e);
 * Get the state before the given edge.
 * The returned state must be released when it is no more used.
 * @param	Looked edge.
 * @return	Corresponding state.
 */

/**
 * Get the state after the given edge.
 * The returned state must be released when it is no more used.
 * @param	Looked edge.
 * @return	Corresponding state.
 */
State *CFGAnalyzer::after(Edge *e) {
	is = dom.update(e, states[e->source()->id()]);
	in_use.add(is);
	return is;
}

/**
 * @fn State *CFGAnalyzer::after(Block *b);
 * Get the state after the given block.
 * @param b		Looked block.
 * @return		Corresponding state.
 */

/**
 * Get the state before the given block.
 * @param b		Looked block.
 * @return		Corresponding state.
 */
State *CFGAnalyzer::before(Block *b) {
	is = dom.bot();
	for(auto e: b->inEdges()) {
		es = dom.update(e, states[e->source()->id()]);
		is = dom.join(is, es);
	}
	in_use.add(is);
	return is;
}

} }	// otawa::ai
