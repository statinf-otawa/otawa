/*
 * $Id$
 * Copyright (c) 2003, IRIT UPS.
 *
 * src/dumpcfg/DisassemblerDisplayer.cpp -- DisassemblerDisplayer class implementation.
 */

#include <elm/io.h>
#include "DisassemblerDisplayer.h"

using namespace elm;
using namespace otawa;

/**
 */
void DisassemblerDisplayer::onCFGBegin(CFG *cfg) {
	cout << "# Function " << cfg->label() << '\n';
}


/**
 */
void DisassemblerDisplayer::onCFGEnd(CFG *cfg) {
}


/**
 */
void DisassemblerDisplayer::onBBBegin(BasicBlock *bb, int index) {
	cout << "BB " << index << ": ";
}

/**
 */
void DisassemblerDisplayer::onEdge(CFGInfo *info, BasicBlock *source,
int source_index, edge_kind_t kind, BasicBlock *target, int target_index) {
	switch(kind) {
		
	case EDGE_Null:
		if(target_index >= 0)
			cout << " R(" << target_index << ")";
		else
			cout << " R";
		break;
		
	case EDGE_NotTaken:
		cout << " NT(" << target_index << ')';
		break;
		
	case EDGE_Taken:
		cout << " T(";
		if(target_index >= 0)
			cout << target_index;
		else
			cout << '?';
		cout << ")";
		break;
		
	case EDGE_Call:
		cout << " C(";
		if(target_index >= 0)
			cout << target_index;
		else if(!target)
			cout << '?';
		else {
			CFG *cfg = info->findCFG(target);
			if(!cfg)
				cout << '?';
			else if(cfg->label())
				cout << cfg->label();
			else
				cout << fmt::address(cfg->address());
		}
		cout << ")";
		break;
		
	}
}


/**
 */
void DisassemblerDisplayer::onBBEnd(BasicBlock *bb, int index) {
	cout << '\n';
	PseudoInst *pseudo;
	for(Iterator<Inst *> inst(*bb); inst; inst++) {
			
		// Put the label
		for(PropList::Getter<String> label(inst, FUNCTION_LABEL); label; label++)
			cout << '\t' << *label << ":\n";
		for(PropList::Getter<String> label(inst, LABEL); label; label++)
			cout << '\t' << *label << ":\n";
			
		// Disassemble the instruction
		cout << "\t\t" << fmt::address(inst->address()) << ' ';
		inst->dump(cout);
		cout << '\n';
	}
}


/**
 * Handle an inline of a program call.
 */
void DisassemblerDisplayer::onInlineBegin(CFG *cfg) {
	cout << "# Inlining " << cfg->label() << '\n';
}


/**
 * Handle an end of inline.
 */
void DisassemblerDisplayer::onInlineEnd(CFG *cfg) {
}

