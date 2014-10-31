/*
 *	$Id$
 *	CFGBuilder processor implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-09, IRIT UPS.
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

#include <otawa/cfg/CFGBuilder.h>
#include <otawa/cfg.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/loader/new_gliss/BranchInst.h>
#include <otawa/util/FlowFactLoader.h>

namespace otawa {

// Useful
inline bool isReturn(Inst *inst) {
	return inst->isReturn() || IS_RETURN(inst);
}


/**
 */
static Identifier<bool> IS_ENTRY("", false);
static Identifier<CodeBasicBlock *> BB("", 0);



/**
 * @class CFGBuilder
 * This processor is used for building the CFG from the binary program
 * representation. Found CFG are linked the framework to the framework thanks
 * a CFGInfo object stored in a property.
 *
 * @par Provided Features
 * @li @ref CFG_INFO_FEATURE
 *
 * @par Required Features
 * @li @ref DECODED_TEXT
 *
 * @ingroup cfg
 */


/**
 * CFG builder constructor.
 */
CFGBuilder::CFGBuilder(void)
: Processor("otawa::CFGBuilder", Version(1, 0, 0)) {
	provide(CFG_INFO_FEATURE);
	require(DECODED_TEXT);
}


/**
 */
void CFGBuilder::processWorkSpace(WorkSpace *fw) {
	ASSERT(fw);
	buildAll(fw);
}


/**
 * Find the basic block starting on the next instruction. If none exists, create
 * it.
 * @param inst		Instruction to find the BB after.
 * @return			Found or created BB or null if end-of-code is reached.
 */
CodeBasicBlock *CFGBuilder::nextBB(Inst *inst) {
	ASSERT(inst);

	// get next instruction
	Inst *next_inst = inst->nextInst();
	if(!next_inst)
		return 0;

	// is there a BB ?
	CodeBasicBlock *next_bb = BB(next_inst);
	if(next_bb)
		return next_bb;

	// create it
	next_bb = new CodeBasicBlock(next_inst);
	BB(next_inst) = next_bb;
	if(logFor(LOG_BB))
		log << "\tfound BB at " << next_bb->address() << io::endl;
	return next_bb;
}


/**
 * Get or create the basic block on the given instruction.
 * @param inst		Instruction starting the BB.
 * @return				Found or created BB.
 */
CodeBasicBlock *CFGBuilder::thisBB(Inst *inst) {
	ASSERT(inst);

	// find container BB
	CodeBasicBlock *bb = BB(inst);
	if(bb)
		return bb;

	// at start, create it
	bb = new CodeBasicBlock(inst);
	BB(inst) = bb;
	if(logFor(LOG_BB))
		log << "\tfound BB at " << bb->address() << io::endl;
	return bb;
}


/**
 * Record the given instruction as the startup of a sub-program.
 * @param inst	Subprogram startup.
 */
void CFGBuilder::addSubProgram(Inst *inst) {
	ASSERT(inst);
	if(logFor(LOG_CFG))
		log << "\tadd subprogram " << inst->address() << " (" << FUNCTION_LABEL(inst) << ")\n";
	BasicBlock *bb = thisBB(inst);
	IS_ENTRY(bb) = true;
}


/**
 * Build the CFG for the given code.
 * @param code	Code to build the CFG for.
 */
void CFGBuilder::buildCFG(WorkSpace *ws, Segment *seg) {
	ASSERT(seg);

	// Add the initial basic block
	Segment::ItemIter item(seg);
	while(item && !item->toInst())
		item++;
	if(!item)
		return;
	CodeBasicBlock *bb = thisBB(item->toInst());

	// Find the basic blocks
	for(; item; item++) {
		Inst *inst = item->toInst();
		if(inst && inst->isControl() && !IGNORE_CONTROL(inst)) {

			// found BB starting on target instruction
			Inst *target = inst->target();
			if(target) {
				BasicBlock *bb = thisBB(target);
				ASSERT(bb);
				if(inst->isCall())
					IS_ENTRY(bb) = true;
			}

			// verbose message for non-return
			else if(!isReturn(inst) && logFor(LOG_INST))
				warn( _ << "unresolved indirect control at 0x" << inst->address()
					<< "\n\t" << inst->address() << '\t' << inst);

			// look for target properties
			else {

				// simple branches
				for(Identifier<Address>::Getter target(inst, BRANCH_TARGET); target; target++) {
					Inst *target_inst = ws->findInstAt(target);
					if(target_inst) {
						BasicBlock *bb = thisBB(target_inst);
						ASSERT(bb);
						if(inst->isCall())
							IS_ENTRY(bb) = true;
					}
				}

				// simple branches
				for(Identifier<Address>::Getter target(inst, CALL_TARGET); target; target++) {
					Inst *target_inst = ws->findInstAt(target);
					if(target_inst) {
						BasicBlock *bb = thisBB(target_inst);
						ASSERT(bb);
						IS_ENTRY(bb) = true;
					}
				}
			}


			// Found BB starting on next instruction
			nextBB(inst);
		}
	}

	// Build the graph
	genstruct::Vector<BasicBlock *> entries;
	bb = 0;
	bool follow = true;
	for(Segment::ItemIter item(seg); item; item++) {

		// an instruction ?
		Inst *inst = item->toInst();
		if(!inst)
			continue;

		// start of block found
		CodeBasicBlock *next_bb = BB(inst);
		if(next_bb) {

			// no current BB
			if(!bb)
				bb = next_bb;

			// end of current BB
			else {
				if(!bb->size()) {
					bb->setSize(inst->address() - bb->address());
					info->add(bb);
				}

				// Record not-taken edge
				if(bb && follow)
					new Edge(bb, next_bb, EDGE_NotTaken);

				// Initialize new BB
				bb = next_bb;
				follow = true;
			}

			// record entries
			if(IS_ENTRY(bb)) {
				ASSERT(!entries.contains(bb));
				entries.add(bb);
				IS_ENTRY(bb).remove();
			}
		}

		// end of block
		if(IS_RETURN(inst) || (inst->target() && NO_RETURN(inst->target()))) {
			bb->setSize(inst->topAddress() - bb->address());
			info->add(bb);
			bb->flags |= BasicBlock::FLAG_Return;
			follow = false;
		}

		else if(inst->isControl() && !IGNORE_CONTROL(inst)) {
			ASSERTP(bb, "no BB at " << inst->address() << io::endl);
			bb->setSize(inst->topAddress() - bb->address());
			info->add(bb);

			// record the taken edge
			Inst *target = inst->target();
			if(target && (!inst->isCall() || !NO_CALL(target))) {
				BasicBlock *target_bb = thisBB(target);
				ASSERT(target_bb);
				Edge *edge = new Edge(bb, target_bb, inst->isCall() ? EDGE_Call : EDGE_Taken);
			}

			// look for target properties
			bool perform_call = false;
			if(!target) {

				// look for simple branches
				for(Identifier<Address>::Getter addr(inst, BRANCH_TARGET); addr; addr++) {
					target = ws->findInstAt(addr);
					if(target) {
						BasicBlock *target_bb = thisBB(target);
						ASSERT(target_bb);
						new Edge(bb, target_bb, inst->isCall() ? Edge::CALL : Edge::TAKEN);
					}
					else
						warn(_ << "branch target to " << *target << " at " << inst->address()
							<< " does not match an instruction.");
				}

				// look for calls
				for(Identifier<Address>::Getter addr(inst, CALL_TARGET); addr; addr++) {
					target = ws->findInstAt(addr);
					if(target) {
						BasicBlock *target_bb = thisBB(target);
						ASSERT(target_bb);
						new Edge(bb, target_bb, Edge::CALL);
						perform_call = true;
					}
					else
						warn(_ << "call target to " << *target << " at " << inst->address()
							<< " does not match an instruction.");
				}

			}

			// record BB flags
			if(perform_call)
				bb->flags |= BasicBlock::FLAG_Call;
			else if(isReturn(inst))
				bb->flags |= BasicBlock::FLAG_Return;
			else if(inst->isCall())
				bb->flags |= BasicBlock::FLAG_Call;
			if(inst->isConditional())
				bb->flags |= BasicBlock::FLAG_Cond;
			if(!target && !isReturn(inst))
				bb->flags |= BasicBlock::FLAG_Unknown;
			follow = (perform_call || inst->isCall() || inst->isConditional()) && !IGNORE_SEQ(inst);
		}
	}


	// last size
	if(bb && !bb->size()) {
		bb->setSize(seg->topAddress() - bb->address());
		info->add(bb);
	}

	// Recording the CFG
	for(int i = 0; i < entries.length(); i++) {
		CFG *cfg = new CFG(seg, entries[i]);
		_cfgs.add(cfg);
		ENTRY(entries[i]) = cfg;
		if(logFor(LOG_CFG))
			log << "\tadded CFG " << cfg->label() << " at " << cfg->address() << io::endl;
	}
}


/**
 * Add a file to the builder.
 * @param ws	Current workspace.
 * @param file	Added file.
 */
void CFGBuilder::addFile(WorkSpace *ws, File *file) {
	ASSERT(file);

	// Scan file symbols
	for(File::SymIter sym(file); sym; sym++)
		if(sym->kind() == Symbol::FUNCTION) {
			Inst *inst = sym->findInst();
			if(inst)
				addSubProgram(inst);
			else
				warn(_ << "function symbol \"" << sym->name() << "\" does not match any instruction");
	}

	// Scan file segments
	for(File::SegIter seg(file); seg; seg++)
		if(seg->isExecutable())
			buildCFG(ws, seg);


	// clean instructions
	for(File::SegIter seg(file); seg; seg++)
		for(Segment::ItemIter item(seg); item; item++)
			BB(item).remove();
}


/**
 * Build the CFG of the program.
 * @param fw	Current framework.
 */
void CFGBuilder::buildAll(WorkSpace *fw) {
	ASSERT(fw);
	_cfgs.clear();

	// create CFG info
	info = new CFGInfo(fw);
	//this->addCleaner(CFG_INFO_FEATURE, deletor(CFGInfo::ID, fw, info));
	track(CFG_INFO_FEATURE, CFGInfo::ID(fw) = info);

	// Find and mark the start
	Inst *start = fw->start();
	if(start)
		addSubProgram(start);

	// Compute CFG for each code piece
	for(Process::FileIter file(fw->process()); file; file++)
		addFile(fw, file);

	// Build the CFGInfo
	for(genstruct::Vector<CFG *>::Iterator cfg(_cfgs); cfg; cfg++)
		info->add(cfg);
}


/**
 */
void CFGBuilder::configure(const PropList& props) {
	Processor::configure(props);
}

static SilentFeature::Maker<CFGBuilder> CFG_INFO_MAKER;
/**
 * Feature asserting that the CFG has been scanned in the program. The result
 * is put the @ref CFGInfo::ID.
 *
 * @Properties
 * @li @ref CFGInfo::ID (FrameWork)
 *
 * @ingroup cfg
 */
SilentFeature CFG_INFO_FEATURE("otawa::CFG_INFO_FEATURE", CFG_INFO_MAKER);

} // otawa
