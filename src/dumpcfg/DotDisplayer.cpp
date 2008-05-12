/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/dumpcfg/DotDisplayer.cpp -- DotDisplayer class implementation.
 */

#include <elm/io.h>
#include <elm/options.h>
#include "DotDisplayer.h"

using namespace elm;
using namespace otawa;

// Externals
extern option::BoolOption display_assembly;

/**
 * Display the name of a basic block.
 * @param bb	Displayed basic block.
 */
void DotDisplayer::displayLabel(BasicBlock *bb, int index) {
	if(bb) {
		if(bb->isEntry())
			cout <<  "ENTRY";
		else if(bb->isExit())
			cout << "EXIT";
		else {
			if(display_assembly)
				cout << "{";
			cout << index << " (" << fmt::address(bb->address()) << ")";
			if(display_assembly) {
				cout << " | ";
				bool first = true;
				for(BasicBlock::InstIterator inst(bb); inst; inst++) {
				//for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
					if(first)
						first = false;
					else 
						cout << "\\l";
					for(PropList::Getter<String> label(inst, FUNCTION_LABEL);
					label; label++)
						cout << *label << ":\\l";
					for(PropList::Getter<String> label(inst, LABEL);
					label; label++)
						cout << *label << ":\\l";
					cout << fmt::address(inst->address()) << "    ";
					StringBuffer buf;
					inst->dump(buf);
					String dis = buf.toString();
					for(int i = 0; i < dis.length(); i++) {
						if(dis[i] == '{'
						|| dis[i] == '|'
						|| dis[i] == '}')
							cout << '\\';
						cout << dis[i];
					}
				}
				cout << "\\l }";
			}
		}
	}
	else
		cout << "unknow";
}


/**
 */
void DotDisplayer::onCFGBegin(CFG *cfg) {
	cout << "digraph " << cfg->label() << "{\n"
		 << "node [shape=Mrecord, labeljust=l, fontsize=10];\n";
}


/**
 */
void DotDisplayer::onCFGEnd(CFG *cfg) {
	cout << "}\n";
}


/**
 */
void DotDisplayer::onBBBegin(BasicBlock *bb, int index) {
	cout << "\t\"" << index << "\"";
	cout << " [label=\"";
	displayLabel(bb, index);
	cout << "\"]\n";
}


/**
 */
void DotDisplayer::onEdge(CFGInfo *info, BasicBlock *source, int source_index,
edge_kind_t kind, BasicBlock *target, int target_index) {
	if(target_index >= 0) {
		int weight = 1;
		cout << "\t\"" << source_index
			 << "\" -> \"" << target_index << "\" [";
		switch(kind) {
		case Edge::VIRTUAL:
			cout << "style=dashed, ";
			weight = 1;
			break;
		case Edge::CALL:
			cout << "label=\"call\", style=dashed, ";
			weight = 1;
			break;
		case Edge::VIRTUAL_CALL:
			cout << "label=\"call\", style=dashed, ";
			weight = 2;
			break;
		case Edge::VIRTUAL_RETURN:
			cout << "label=\"return\", style=dashed, ";
			weight = 2;
			break;
		case Edge::NOT_TAKEN:
			weight = 4;
			break;
		case Edge::TAKEN:
			cout << "label=\"taken\", ";
			weight = 3;
			break;
		default:
			ASSERT(false);
		}
		cout << "weight=" << weight << "];\n";
	}
}


/**
 */
void DotDisplayer::onBBEnd(BasicBlock *bb, int index) {
}


/**
 * Handle an inline of a program call.
 */
void DotDisplayer::onInlineBegin(CFG *cfg) {
}


/**
 * Handle an end of inline.
 */
void DotDisplayer::onInlineEnd(CFG *cfg) {
}

