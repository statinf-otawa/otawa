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
#include <otawa/cfg/features.h>

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


MetaRegistration ContextualProcessor::reg(
	"otawa::ContextualProcessor", Version(1, 0, 0),
	p::require, &CHECKED_CFG_FEATURE,
	p::end);


/**
 * Build a contextual processor.
 * @param name		Name of the implemented actual processor.
 * @param version	Version of the implemented atucal processor.
 *
 */
ContextualProcessor::ContextualProcessor(cstring name, const Version& version)
:	CFGProcessor(name, version) {
	require(CHECKED_CFG_FEATURE);
}

/**
 * Build a contextual processor.
 * @param reg	Registration of the actual processor.
 */
ContextualProcessor::ContextualProcessor(AbstractRegistration & reg)
: CFGProcessor(reg) {
}

/**
 */
void ContextualProcessor::processCFG (WorkSpace *ws, CFG *cfg) {
	static Identifier<BasicBlock *> MARK("", 0);
	typedef Pair<CFG *, Edge *> call_t;
	Vector<Edge *> todo;
	Vector<call_t> calls;
	int level = 0;

	// initialization
	calls.push(call_t(cfg, 0));
	for(BasicBlock::OutIterator edge(cfg->entry()); edge; edge++)
		todo.push(edge);

	// traverse until the end
	while(todo) {
		Edge *edge = todo.pop();
		BasicBlock *bb;

		// null edge -> leaving function
		if(!edge) {
			edge = calls.top().snd;
			calls.pop();
			this->leavingCall(ws, cfg, edge);
			if(logFor(LOG_CFG))
				log << "\t\t[" << level << "] leaving call\n";
			level--;
			bb = edge->target();
		}

		// a virtual call ?
		else switch(edge->kind()) {

		case Edge::NONE:
			ASSERT(false);

		case Edge::VIRTUAL_RETURN:
		case Edge::CALL:
			bb = 0;
			break;

		case Edge::TAKEN:
		case Edge::NOT_TAKEN:
			if(!MARK(edge->target()))
				bb = edge->target();
			else
				bb = 0;
			break;

		case Edge::VIRTUAL:
			if(edge->target() == cfg->exit()) {
				bb = 0;
				break;
			}

		case Edge::VIRTUAL_CALL: {
				bb = edge->target();

				// recursive call
				if(MARK(bb)) {
					avoidingRecursive(ws, cfg, edge);
					if(logFor(LOG_CFG))
						log << "\t\t[" << level << "] avoiding recursive call from " << edge->source() << " to " << bb << io::endl;
					bb = 0;
				}

				// simple call
				else {
					enteringCall(ws, cfg, edge);
					if(logFor(LOG_CFG))
						log << "\t\t[" << level << "] entering call to " << bb << " from " << edge->source() << io::endl;
					BasicBlock *ret = VIRTUAL_RETURN_BLOCK(edge->source());
					if(!ret)
						ret = cfg->exit();
					CFG *called_cfg = CALLED_CFG(edge);
					if(!called_cfg)
						called_cfg = cfg;
					if(!BasicBlock::InIterator(ret))
						throw ProcessorException(*this, _ << "unconnected CFG for function " << called_cfg->label());
					calls.push(call_t(called_cfg, BasicBlock::InIterator(ret)));
					todo.push(0);
					level++;
				}
			}
			break;
		}

		// process basic block
		if(bb) {
			processBB(ws, cfg, bb);
			if(logFor(LOG_BB))
				log << "\t\t[" << level << "] processing " << bb << io::endl;
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				todo.push(edge);
			MARK(bb) = calls.top().fst->entry();
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


/**
 * Called when the processor enters an inlined function.
 * As a default, call enteringCall(ws, cfg, source, target).
 * @param ws	Current workspace.
 * @param cfg	Current CFG.
 * @param edge	Edge causing the call.
 */
void ContextualProcessor::enteringCall(WorkSpace *ws, CFG *cfg, Edge *edge) {
	enteringCall(ws, cfg, edge->source(), edge->target());
}

/**
 * Called when the processor leaves an inlined function.
 * As a default, call leavingCall(ws, cfg, source, target).
 * @param ws	Current workspace.
 * @param cfg	Current CFG.
 * @param edge	Edge causing the call.
 */
void ContextualProcessor::leavingCall(WorkSpace *ws, CFG *cfg, Edge *edge) {
	leavingCall(ws, cfg, edge->target());
}


/**
 * Called to avoid a recursive call. As a default, call avoidingRecursive(ws, cfg, source, target).
 * @param cfg	Current CFG.
 * @param edge	Edge causing the call.
 */
void ContextualProcessor::avoidingRecursive(WorkSpace *ws, CFG *cfg, Edge *edge) {
	avoidingRecursive(ws, cfg, edge->source(), edge->target());
}

} // otawa
