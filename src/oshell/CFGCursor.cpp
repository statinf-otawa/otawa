/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/CFGCursor.cpp -- implementation of CFGCursor class.
 */

#include "CFGCursor.h"

namespace otawa {

/**
 * @class CFGCursor
 * Cursor for exploring CFG.
 */

	
/**
 * Build a new CFG cursor.
 * @param back	Back cursor.
 * @param _cfg	Used CFG.
 */
CFGCursor::CFGCursor(Cursor *back, CFG *_cfg)
: Cursor(back), cfg(_cfg), built(false) {
}


// Cursor overload
void CFGCursor::path(Output& out) {
	back()->path(out);
	String name = cfg->entry()->get<String>(File::ID_Label, "");
	if(name)
		out << '/' << name;
	else
		out << "/CFG" << cfg->entry()->address();
}


// Cursor overload
void CFGCursor::list(Output& out) {
	
	// Build
	if(!built)
		build();
	
	// Visit the bbs
	ListVisitor visitor(out);
	bbs.visit(&visitor);
}

/**
 * Build the CFG representation.
 */
void CFGCursor::build(void) {
	built = true;
	genstruct::DLList<BasicBlock *> remain;
	remain.addLast(cfg->entry());	
	
	// Find all basic blocks
	for(int num = 0; !remain.isEmpty(); num++) {
		
		// Get the current BB
		BasicBlock *bb = remain.first();
		remain.removeFirst();
		if(bbs.contains(bb))
			continue;
		bbs.insert(bb);
		
		// Display not-taken
		BasicBlock *target = bb->getNotTaken();
		if(target && !bbs.contains(target))
			remain.addLast(target);

		// Display taken
		if(!bb->isCall()) {
			target = bb->getTaken();
			if(target && !bbs.contains(target))
				remain.addLast(target);
		}
	}

	// Give number to basic blocks
	BasicBlockVisitor visitor;
	bbs.visit(&visitor);
}


/**
 * Used for storing numbers in basic blocks.
 */
id_t CFGCursor::ID_Number = Property::getID("OShell.Number");


/**
 * @class CFGCursor::ListVisitor
 * This class is used for visiting the basic block of the CFG and displaying each absic bloc.
 */


/**
 * Display the graph of the CFG.
 * @param bb	Basic bloc to display.
 */
int CFGCursor::ListVisitor::process(BasicBlock *bb) {
	
	// Display header
	out << "BB " << bb->use<int>(ID_Number) << ": ";
	BasicBlock *target = bb->getNotTaken();
	if(target)
		out << " NT(" << target->use<int>(ID_Number) << ')';
	if(bb->isReturn())
		out << " R";
	else {
		target = bb->getTaken();
		if(target) {
			if(!bb->isCall())
				out << " T(" << target->use<int>(ID_Number);
			else
				out << " C(" << target->address();
			Option<String> label = target->get<String>(File::ID_Label);
			if(label)
				out << '[' << *label << ']';
			out << ')';
		}
		else if(bb->isTargetUnknown())
			out << (bb->isCall() ? " C(?)" : " T(?)");
	}
	out << '\n';

	// Disassemble basic block
	PseudoInst *pseudo;
	for(Iterator<Inst *> inst(*bb); inst; inst++) {
			
		// Put the label
		Option<String> label = inst->get<String>(File::ID_Label);
		if(label)
			out << '\t' << *label << ":\n";
			
		// Disassemble the instruction
		out << "\t\t" << inst->address() << ' ';
		inst->dump(out);
		out << '\n';
	}
}

} // otawa
