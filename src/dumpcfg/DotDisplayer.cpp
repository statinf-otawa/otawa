/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/dumpcfg/DotDisplayer.cpp -- DotDisplayer class implementation.
 */

#include <elm/io.h>
#include <elm/options.h>
#include <elm/genstruct/HashTable.h>
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
	cstring file;
	int line = 0;

	if(bb) {
		if(bb->isEntry())
			cout <<  "ENTRY";
		else if(bb->isExit())
			cout << "EXIT";
		else {
			if(display_assembly)
				cout << "{";
			cout << "BB " << index << " (" << ot::address(bb->address()) << ")";
			if(display_assembly) {
				cout << " | ";
				bool first = true;
				for(BasicBlock::InstIter inst(bb); inst; inst++) {
				//for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
					if(first)
						first = false;
					else
						cout << "\\l";

					// Display labels
					for(Identifier<String>::Getter label(inst, FUNCTION_LABEL);
					label; label++)
						cout << *label << ":\\l";
					for(Identifier<String>::Getter label(inst, LABEL);
					label; label++)
						cout << *label << ":\\l";

					// display the debug line
					if(source_info) {
						Option<Pair<cstring, int> > info = ws->process()->getSourceLine(inst->address());
						if(info) {
							if((*info).fst != file || (*info).snd != line) {
								file = (*info).fst;
								line = (*info).snd;
								cout << file << ":" << line << "\\l";
							}
						}
						else {
							file = "";
							line = 0;
						}
					}

					// Display the instruction
					cout << ot::address(inst->address()) << "    ";
					StringBuffer buf;
					inst->dump(buf);
					String dis = buf.toString();
					for(int i = 0; i < dis.length(); i++) {
						switch(dis[i]) {
						case '{':
						case '}':
						case '<':
						case '>':
						case '|':
						case '\\':
						case '"':
							cout << '\\';
							break;
						}
						cout << dis[i];
					}

					// Add called label for branch
					if(inst->isControl() && !inst->isReturn()) {
						cout << "    # ";
						Inst *target = inst->target();
						if(!target)
							cout << "unknown";
						else {
							string label = FUNCTION_LABEL(target);
							if(!label)
								label = LABEL(target);
							if(label)
								cout << label;
							else
								cout << target->address();
						}
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
void DotDisplayer::onCall(Edge *edge) {
  BasicBlock *bb = edge->source();
  CFG *called = edge->calledCFG();
  string label = called->label();
  cout << "\t\"" << label << "\" [label=\"" << label << "\"]\n";
  cout << "\t\"" << bb->number() << "\" -> \"" << label << "\" [label=\"call\", style=dashed, weight=1]\n";
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
			 << "\" -> \"";
		//if(kind == Edge::CALL)
		//	cout << target->cfg()->label();
		//else
			cout << target_index;
		cout << "\" [";
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

