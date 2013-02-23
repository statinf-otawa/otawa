/*
 *	$Id$
 *	PFGBuilder analyzer implementation
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#include <otawa/cfg/PFGBuilder.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/prog/Inst.h>
#include <otawa/cfg/PFG.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/prog/File.h>

namespace otawa {

#ifdef NDEBUG
#	define TRACE(x)
#else
#	define TRACE(x) cerr << x << io::endl
#endif


/**
 * Constructor of PFG builder.
 */
PFGBuilder::PFGBuilder(void): Processor("otawa::PFGBuilder", Version(1, 0, 0)) {
	provide(PFG_FEATURE);
	require(DECODED_TEXT);
}


/**
 */
void PFGBuilder::processWorkSpace(WorkSpace *ws) {
	ASSERT(ws);

	// build the PFG
	pfg = new pfg::PFG();
	PFG(ws) = pfg;

	// find and mark the start
	Inst *start = ws->start();
	if(start)
		addFunction(ws, start);

	// find BB for each function of each file
	for(Process::FileIter file(ws->process()); file; file++)
		addFile(ws, file);

	// finalize BB
	for(pfg::PFG::Iterator bb(pfg); bb; bb++)
		if(bb != &pfg->ret && bb != &pfg->unknown) {
			if(logFor(LOG_BB)) {
				cerr << "\tBB at " << bb->address() << io::endl;
			}
			finalizeBB(bb);
		}
}


/**
 * Add the given code file to the PFG.
 * @param ws	Current workspace.
 * @param file	Added file.
 */
void PFGBuilder::addFile(WorkSpace *ws, File *file) {
	ASSERT(ws);
	ASSERT(file);
	for(File::SymIter sym(file); sym; sym++)
		if(sym->kind() == Symbol::FUNCTION) {
			Inst *inst = sym->findInst();
			if(inst)
				addFunction(ws, inst);
	}
}


/**
 * Record basic block of a function.
 * @param ws	Current workspace.
 * @param inst	First instruction fo the function.
 */
void PFGBuilder::addFunction(WorkSpace *ws, Inst *inst) {
	ASSERT(ws);
	ASSERT(inst);
	if(logFor(LOG_CFG))
		log << "\tfunction entry at " << inst->address() << " (" << FUNCTION_LABEL(inst) << ")\n";

	// prepare to-do list
	genstruct::Vector<Inst *> todo;
	todo.push(inst);

	// follow allow paths
	while(todo) {

		// make the BB
		Inst *inst = todo.pop();
		TRACE("\t\tprocessing " << inst->address());
		if(PFG_BB(inst))
			continue;
		pfg::BB *bb = new pfg::BB(inst, 0);
		pfg->add(bb);
		PFG_BB(inst) = bb;

		// find next branch
		while(inst && !inst->isControl())
			inst = inst->nextInst();
		if(inst) {

			if(inst->isConditional() || inst->isCall()) {
				TRACE("\t\tpushing " << inst->nextInst()->address());
				todo.push(inst->nextInst());
			}
			if(!inst->isReturn()) {
				if(inst->target()) {
					TRACE("\t\tpushing " << inst->target()->address());
					todo.push(inst->target());
				}
				else
					for(Identifier<Address>::Getter target(inst, BRANCH_TARGET);
					target;
					target++) {
						TRACE("\t\tpushing " << ws->process()->findInstAt(target));
						todo.push(ws->process()->findInstAt(target));
					}
			}
		}
	}
}


/**
 * Finalize a basic block.
 * @param bb	Basic block to work on.
 */
void PFGBuilder::finalizeBB(pfg::BB *bb) {
	ASSERT(bb);

	for(Inst *inst = bb->first(); inst; inst = inst->nextInst()) {

		// instruction sequence cut as branch target
		if(PFG_BB(inst) && inst != bb->first()) {
			new pfg::Edge(bb, PFG_BB(inst), pfg::Edge::SEQ);
			bb->setSize(inst->address() - bb->address());
			return;
		}

		// branch at end of a BB
		else if(inst->isControl()) {

			// build the edges
			if(inst->isCall() || inst->isConditional()) {
				ASSERTP(PFG_BB(inst->nextInst()),
					"call/conditional without next at " << inst->address() << " from " << bb->address());
				new pfg::Edge(bb, PFG_BB(inst->nextInst()), pfg::Edge::SEQ);
			}
			if(inst->isReturn())
				new pfg::Edge(bb, &pfg->ret,
					inst->isConditional() ? pfg::Edge::COND_RETURN : pfg::Edge::RETURN);
			else {
				pfg::Edge::kind_t kind;
				if(inst->isCall())
					kind = inst->isConditional() ? pfg::Edge::COND_CALL : pfg::Edge::CALL;
				else
					kind = inst->isConditional() ? pfg::Edge::COND_BRANCH : pfg::Edge::BRANCH;
				if(inst->target())
					new pfg::Edge(bb, PFG_BB(inst->target()), kind);
				else if(inst->hasProp(BRANCH_TARGET))
					for(Identifier<Address>::Getter target(inst, BRANCH_TARGET);
					target;
					target++)
						new pfg::Edge(bb, PFG_BB(workspace()->process()->findInstAt(target)), kind);
				else
					new pfg::Edge(bb, &pfg->unknown, kind);
			}

			// compute the size
			bb->setSize(inst->topAddress() - bb->address());
			return;
		}
	}

	// unended BB ???
	throw ProcessorException(*this, _ << "unended BB at " << bb->address());
}

} //otawa
