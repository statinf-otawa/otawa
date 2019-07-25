/*
 *	display::DotDisplayer class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005, IRIT UPS.
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

#include <elm/io.h>
#include <elm/data/HashMap.h>
#include <elm/options.h>

#include <otawa/cfg/features.h>
#include <otawa/prog/Process.h>

#include "DotDisplayer.h"

using namespace elm;
using namespace otawa;

/**
 */
DotDisplayer::DotDisplayer(void): Displayer("DotDisplayer", Version(2, 0, 0)) {
}


/**
 * Generate unique name for a block.
 * @param g		Owner CFG.
 * @param v		Current block.
 */
void DotDisplayer::displayName(CFG *g, otawa::Block *v) {
	cout << '"' << g->name() << "_" << g->index() << "_" << v->index() << '"';
}


/**
 */
void DotDisplayer::processWorkSpace(WorkSpace *ws) {
	const CFGCollection& coll = **otawa::INVOLVED_CFGS(ws);
	cout << "digraph " << coll[0]->label() << " {\n"
	 	 << "node [shape=Mrecord, labeljust=l, fontsize=10];\n";

	// traverse CFGs
	for(int i = 0; i < coll.count(); i++) {

		// get next CFG
		CFG *cfg = coll[i];
		if(i > 0 && !display_all)
			break;

		// traverse blocks
		for(CFG::BlockIter v = cfg->blocks(); v(); v++) {
			if(display_all && v->isSynth())
				continue;

			// display block header
			cout << "\t";
			displayName(cfg, *v);
			cout << " [label=\"";
			displayLabel(*v);
			cout << "\"];\n";

			// display edges
			for(Block::EdgeIter e = v->outs(); e(); e++) {

				// case of a call with display of all
				if(e->sink()->isSynth() && display_all && e->sink()->toSynth()->callee()) {
					SynthBlock *sb = e->sink()->toSynth();

					// call edge
					cout << "\t";
					displayName(cfg, *v);
					cout << " -> ";
					displayName(sb->callee(), sb->callee()->entry());
					cout << " [label=\"call\", style=dashed, weight=1];\n";

					// return edge
					cout << "\t";
					displayName(sb->callee(), sb->callee()->exit());
					cout << " -> ";
					displayName(cfg, sb->outs()->sink());
					cout << " [label=\"return\", style=dashed, weight=1];\n";
				}

				// usual display of blocks
				else {

					// display edge header
					cout << "\t";
					displayName(cfg, *v);
					cout << " -> ";
					displayName(cfg, e->sink());

					// display properties
					cout << " [ ";
					if(v->isEntry() || e->sink()->isExit())
						cout << "style=dashed, weight=1";
					else if(v->isSynth())
						cout << "label=\"return\", style=dashed, weight=1";
					else if(e->sink()->isSynth())
						cout << "label=\"call\", style=dashed, weight=1";
					else if(e->isNotTaken())
						cout << "weight=4";
					else
						cout << "label=\"taken\", weight=3";
					cout << "];\n";
					/* TODO
					 case Edge::VIRTUAL_CALL:
						cout << "label=\"call\", style=dashed, ";
						weight = 2;
						break;
					case Edge::VIRTUAL_RETURN:
						cout << "label=\"return\", style=dashed, ";
						weight = 2;*/

				}
			}
		}
	}

	cout << "}\n";
}

/**
 * Display the name of a block.
 * @param v	Displayed block.
 */
void DotDisplayer::displayLabel(Block *v) {
	cstring file;
	int line = 0;

	if(v->isEntry())
		cout <<  "ENTRY";
	else if(v->isExit())
		cout << "EXIT";
	else if(v->isUnknown())
		cout << "unknown";
	else if(v->isSynth()) {
		SynthBlock *sb = v->toSynth();
		if(sb->callee())
			cout << sb->callee()->name();
		else
			cout << "unknown";
	}
	else if(v->isBasic()) {
		BasicBlock *bb = v->toBasic();
		if(display_assembly)
			cout << "{";
		cout << "BB " << v->index() << " (" << ot::address(bb->address()) << ")";
		if(display_assembly) {
			cout << " | ";
			bool first = true;
			for(BasicBlock::InstIter inst(bb); inst(); inst++) {
				if(first)
					first = false;
				else
					cout << "\\l";

				// Display labels
				for(Identifier<String>::Getter label(*inst, FUNCTION_LABEL);
				label(); label++)
					cout << *label << ":\\l";
				for(Identifier<String>::Getter label(*inst, LABEL);
				label(); label++)
					cout << *label << ":\\l";

				// display the debug line
				if(source_info) {
					Option<Pair<cstring, int> > info = workspace()->process()->getSourceLine(inst->address());
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

