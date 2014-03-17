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
void DisassemblerDisplayer::onCall(Edge *edge) {
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
				cout << ot::address(cfg->address());
		}
		cout << ")";
		break;

	case Edge::VIRTUAL_CALL:
		cout << " VC(" << target_index << ")";
		break;

	case Edge::VIRTUAL_RETURN:
		cout << " VR(" << target_index << ")";
		break;

	case Edge::VIRTUAL:
		cout << " V(" << target_index << ")";
		break;
	}
}


/**
 */
void DisassemblerDisplayer::onBBEnd(BasicBlock *bb, int index) {
	cout << '\n';
	for(BasicBlock::InstIter inst(bb); inst; inst++) {

		// Put the label
		for(Identifier<String>::Getter label(inst, FUNCTION_LABEL); label; label++)
			cout << '\t' << *label << ":\n";
		for(Identifier<String>::Getter label(inst, LABEL); label; label++)
			cout << '\t' << *label << ":\n";

		// Disassemble the instruction
		cout << "\t\t" << ot::address(inst->address()) << ' ';
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

