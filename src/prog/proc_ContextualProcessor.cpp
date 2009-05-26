/*
 *	$Id$
 *	ContextualProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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

#include <otawa/proc/ContextualProcessor.h>
#include <elm/genstruct/Vector.h>
#include <otawa/cfg.h>

namespace otawa {

using namespace elm::genstruct;

/**
 * @class ContextualProcessor
 * A contextual processor allows to process basic block in a call-aware way
 * even with a virtual CFG (where called CFG are inlined).
 *
 * First, the basic
 * blocks of the top are processed. When a virtual call is found, enteringCall()
 * method is called and the basic blocks of the called CFG are processed until
 * end of the call (leavingCall() is called at this moment). Then the traversal
 * of the caller CFG restarts. Note that thus behaviour is recursive and that
 * a call in the callee is processed in the same way.
 *
 * This processor may be used to build @ref ContextPath.
 *
 * As an example is often better than any discussion, let three functions :
 * <code>
 *	function f
 *		basic block f1
 *		basic block f2 calling h
 * 		basic block f3 calling g
 *	function g
 *		basic block g1
 *		basic block g2 calling h
 *		basic block g3
 *	function h
 *		basic block h1
 * </code>
 *
 * First the function f is traversed: enteringCall(f), processBB(f1) and processBB(f2).
 * As f2 calls h, we get enteringCall(h), processBB(h1), leavingCall(h).
 * We continue with f3: processBB(f3).
 * And we process the call to g: enteringCall(g), processBB(g1) and processBB(g2).
 * We call another time h: enteringCall(h), processBB(h1) and leavingCall(h).
 * Finally, we leaving g then g: processBB(g3), leavingCall(g), leavingCall(f).
 * @ingroup proc
 */


/**
 * Build a contextual processor.
 * @param name		Name of the implemented actual processor.
 * @param version	Version of the implemented atucal processor.
 *
 */
ContextualProcessor::ContextualProcessor(cstring name, const Version& version)
:	CFGProcessor(name, version) {
}


/**
 */
void ContextualProcessor::processCFG (WorkSpace *ws, CFG *cfg) {
	static Identifier<BasicBlock *> MARK("", 0);
	Vector<BasicBlock *> todo;
	Vector<BasicBlock *> calls;
	Vector<BasicBlock *> returns;
	int level = 0;
	todo.push(cfg->entry());
	calls.push(cfg->entry());
	MARK(cfg->entry()) = calls.top();

	// traverse the BB
	while(todo) {
		BasicBlock *bb = todo.pop();

		// is it a return ?
		if(!bb) {

			// verbosity and control
			if(isVerbose())
				log << "\tleaving\n";
			if(!calls)
				throw ProcessorException(*this, _ << "unstructured CFG " + cfg->label());

			// pop call
			calls.pop();
			BasicBlock *to = returns.pop();
			ASSERT(to);
			level--;
			leavingCall(ws, cfg, to);
			continue;
		}

		// process the current BB
		if(isVerbose())
			log << "\tprocessing " << level << ": BB" << bb->number() << " (" << bb->address() << ")\n";
		processBB(ws, cfg, bb);

		// look outing edge
		for(BasicBlock::OutIterator edge(bb); edge; edge++)
			switch(edge->kind()) {

			case Edge::NONE:
				ASSERT(false);

			case Edge::CALL:
				break;

			case Edge::VIRTUAL_RETURN: {
					BasicBlock *to = edge->target();
					ASSERT(to);
					ASSERT(!returns[returns.length() - 1] || returns[returns.length() - 1] == to);
					returns[returns.length() - 1] = to;
				}
				break;

			case Edge::TAKEN:
			case Edge::NOT_TAKEN:
				if(edge->target() && MARK(edge->target()) != calls.top()) {
					todo.push(edge->target());
					MARK(edge->target()) = calls.top();
				}
				break;

			case Edge::VIRTUAL:
				ASSERT(edge->target());
				if(edge->target()->isExit()) {
					BasicBlock *to = edge->target();
					ASSERT(to);
					ASSERT(!returns[returns.length() - 1] || returns[returns.length() - 1] == to);
					returns[returns.length() - 1] = to;
					break;
				}
				// !!WORKAROUND!!
				CALLED_CFG(edge) = cfg;

			case Edge::VIRTUAL_CALL:

				// check recursivity
				for(int i = 0; i < calls.length(); i++)
					if(calls[i] == edge->target()) {
						if(isVerbose()) {
							log << "\trecursive call";
							CFG *cfg = CALLED_CFG(edge);
							if(cfg)
								log << " to " << cfg->label();
							log << io::endl;
						}
						avoidingRecursive(ws, cfg, bb, edge->target());
						continue;
					}

				// do the call
				if(isVerbose()) {
					log << "\tentering call";
					level++;
					CFG *cfg = CALLED_CFG(edge);
					if(cfg)
						log << " to " << cfg->label();
					log << io::endl;
				}
				calls.push(edge->target());
				todo.push(0);
				todo.push(edge->target());
				returns.push(0);
				MARK(edge->target()) = calls.top();
				enteringCall(ws, cfg, bb, edge->target());
				break;
			}
	}

	// clean up
	for(CFG::BBIterator bb(cfg); bb; bb++)
		MARK(bb).remove();
}


/**
 * This function is called each time a function call is traversed.
 * @param ws		Current workspace.
 * @param cfg		Current top CFG (not the inlined one).
 * @param caller	Caller basic block.
 * @param callee	Callee basic block.
 */
void ContextualProcessor::enteringCall(
	WorkSpace *ws,
	CFG *cfg,
	BasicBlock *caller,
	BasicBlock *callee)
{
}


/**
 * This function is called each time a function return is traversed.
 * @param ws		Current workspace.
 * @param cfg		Current top CFG (not the inlined one).
 * @param bb		Basic block target of a returning call.
 */
void ContextualProcessor::leavingCall(WorkSpace *ws, CFG *cfg, BasicBlock *bb)
{
}


/**
 * This function is called each time a recursive function call is found.
 * @param ws		Current workspace.
 * @param cfg		Current top CFG (not the inlined one).
 * @param caller	Caller basic block.
 * @param callee	Callee basic block.
 * @warning			The recursive call is ignored in the processing.
 */
void ContextualProcessor::avoidingRecursive(
	WorkSpace *ws,
	CFG *cfg,
	BasicBlock *caller,
	BasicBlock *callee)
{
}


/**
 * @fn void ContextualProcessor::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
 * This method is called each time a basic block is found.
 * @param ws		Current workspace.
 * @param cfg		Current top CFG.
 * @param bb		Current basic block.
 */

} // otawa
