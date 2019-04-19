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

#include <elm/data/HashMap.h>
#include <elm/io.h>
#include <elm/io/OutFileStream.h>
#include <elm/options.h>

#include <otawa/cfg/features.h>
#include <otawa/oslice/DotDisplayer.h>
#include <otawa/oslice/oslice.h>
#include <otawa/prog/Process.h>

using namespace elm;
using namespace otawa;

namespace otawa { namespace oslice {



/**
 */
DotDisplayer::DotDisplayer(WorkSpace* ws, String path, int showSlicing) {
	display_all = true;
	display_assembly = true;
	source_info = true;
	_show_slicing = showSlicing;
	_workspace = ws;

	if(!path.isEmpty()) {
		OutStream * _output_stream = new elm::io::OutFileStream(path);
		_output = Output(*_output_stream);
		_skip = 0;
	}
	else
		_skip = 1;

}


/**
 * Generate unique name for a block.
 * @param g		Owner CFG.
 * @param v		Current block.
 */
void DotDisplayer::displayName(CFG *g, otawa::Block *v, otawa::Block *u) {
	//_output << '"' << g->name() << "_" << v->index() << '"';
	_output << '"' << g->name() << "_" << g->index() << "_" << v->index() ;
	if(u != 0)
		_output << "_" << u->index();

	_output << '"';
}


/**
 */
void DotDisplayer::display(const CFGCollection& coll) {

	if(_skip == 1) return;

	_output << "digraph " << coll[0]->label() << " {\n"
	 	 << "node [shape=Mrecord, labeljust=l, fontsize=10];\n";

	// traverse CFGs
	for(int i = 0; i < coll.count(); i++) {

		// get next CFG
		CFG *cfg = coll[i];

//		for(CFG::CallerIter cci=cfg->callers(); cci; cci++) {
//			elm::cout << "Caller " << cci->index() << ":" << cci->cfg()->name() << " calls " << cfg->name() << io::endl;
//		}
//		elm::cout << "==== end callers ====" << io::endl;

		if(i > 0 && !display_all)
			break;

		// traverse blocks
		for(CFG::BlockIter v = cfg->blocks(); v(); v++) {
			if(display_all && v->isSynth()) {

				if(!v->toSynth()->callee()) {
					elm::cout << __SOURCE_INFO__ << "sliced block " << v->index() << io::endl;
					for(Block::EdgeIter e = v->outs(); e(); e++) {
						elm::cout << __SOURCE_INFO__ << "\tto " << e->sink()->index() << io::endl;
					}

					// display block header
					_output << "\t";
					displayName(cfg, *v);
					_output << " [label=";
					//displayLabel(v);
					_output << "Sliced_BB_" << v->index();
					_output << "];\n";

					for(Block::EdgeIter e = v->outs(); e(); e++) {
						// display edge header
						_output << "\t";
						displayName(cfg, *v);
						_output << " -> ";
						displayName(cfg, e->sink());
						// display properties
						_output << " [ ";
						_output << "weight=4";
						_output << "];\n";
					}

				} // end of checking sliced (synth block has no callee)

				// if the synth block has callee
				// then the exit block of the callee should be linked with the target, either the normal block or entry block of a call

				else {
					// locate the exit block
					// Block* exitBlock = v->toSynth()->callee()->exit();
					// locate the out going block
					if(v->toSynth()->outs()->sink()->isSynth()) {
						SynthBlock* sb = v->toSynth()->outs()->sink()->toSynth();
						if(sb->callee()) {
							_output << "\t";
							displayName(v->toSynth()->callee(), v->toSynth()->callee()->exit(), *v);
							_output << " -> ";
							displayName(sb->callee(), sb->callee()->entry(), sb);
							_output << " [label=\"return then call\", style=dashed, weight=1];\n";
						}
						else
							ASSERT(0);
					}
					else {
						// normal block here
						CFG::CallerIter cci = cfg->callers().begin();
						do {
							SynthBlock* xyz = 0;
							if(cci())
								xyz = *cci;

							_output << "\t";
							displayName(v->toSynth()->callee(), v->toSynth()->callee()->exit(), *v);
							_output << " -> ";
							// to the output node of the synth node
							displayName(cfg, v->toSynth()->outs()->sink(), xyz);
							_output << " [label=\"return\", style=dashed, weight=1];\n";

							if(cci())
								cci++;
						} while(cci());
					}

				}



				continue;
			}

			// display block header
			if(!v->isEntry() && !v->isExit()) {

			}

			// for each call, create a node
			CFG::CallerIter cci = cfg->callers().begin();
			do {

				SynthBlock* xyz = 0;
				if(cci())
					xyz = *cci;
				_output << "\t";
				displayName(cfg, *v, xyz);
				_output << " [label=";
				displayLabel(*v);
				_output << "];\n";



//			for(CFG::CallerIter cci=cfg->callers(); cci; cci++) {
//				_output << "\t";
//				displayName(cfg, v, cci);
//				_output << " [label=";
//				displayLabel(v);
//				_output << "];\n";
//			}
//
//			if(!cfg->callers()) {
//				_output << "\t";
//				displayName(cfg, v);
//				_output << " [label=";
//				displayLabel(v);
//				_output << "];\n";
//			}

			// display edges
			for(Block::EdgeIter e = v->outs(); e(); e++) {
				// case of a call with display of all
				if(e->sink()->isSynth() && display_all && e->sink()->toSynth()->callee()) {
					SynthBlock *sb = e->sink()->toSynth();

					// call edge
					_output << "\t";
					displayName(cfg, *v, xyz);
					_output << " -> ";
					displayName(sb->callee(), sb->callee()->entry(), sb);
					_output << " [label=\"call\", style=dashed, weight=1];\n";

					// already handled by the synthblock
//					// return edge
//					// from EXIT block
//					_output << "\t";
//					displayName(sb->callee(), sb->callee()->exit());
//					_output << " -> ";
//					// to the output node of the synth node
//					elm::cout << "Synthblock " << sb->index() << " has output to BB " << sb->outs()->sink()->index() << io::endl;
//					displayName(cfg, sb->outs()->sink());
//					_output << " [label=\"return\", style=dashed, weight=1];\n";
				}

				else if(e->sink()->isSynth() && display_all) { // no callee .... sliced?
					// display edge header
					_output << "\t";
					displayName(cfg, *v);
					_output << " -> ";
					displayName(cfg, e->sink());
					// display properties
					_output << " [ ";
					_output << "weight=4";
					_output << "];\n";
				}

				// usual display of blocks
				else {

					// display edge header
					_output << "\t";
					displayName(cfg, *v, xyz);
					_output << " -> ";
					displayName(cfg, e->sink(), xyz);

					// display properties
					_output << " [ ";
					if(v->isEntry() || e->sink()->isExit())
						_output << "style=dashed, weight=1";
					else if(v->isSynth())
						_output << "label=\"return\", style=dashed, weight=1";
					else if(e->sink()->isSynth())
						_output << "label=\"call\", style=dashed, weight=1";
					else if(e->isNotTaken())
						_output << "weight=4";
					else
						_output << "label=\"taken\", weight=3";
					_output << "];\n";
					/* TODO
					 case Edge::VIRTUAL_CALL:
						_output << "label=\"call\", style=dashed, ";
						weight = 2;
						break;
					case Edge::VIRTUAL_RETURN:
						_output << "label=\"return\", style=dashed, ";
						weight = 2;*/

				}
			}

			if(cci())
				cci++;
		} while(cci());

		} // for each block
	}

	_output << "}\n";
}

/**
 * Display the name of a block.
 * @param v	Displayed block.
 */
void DotDisplayer::displayLabel(Block *v) {
	cstring file;
	int line = 0;

	if(v->isEntry()) {
		_output <<  "\"ENTRY\\n" << v->cfg()->name() << "\"";
	}
	else if(v->isExit()) {
		_output << "\"EXIT\\n" << v->cfg()->name() << "\"";
	}
	else if(v->isUnknown())
		_output << "unknown";
	else if(v->isSynth()) {
		SynthBlock *sb = v->toSynth();
		if(sb->callee())
			_output << sb->callee()->name();
		else
			_output << "unknown";
	}
	else if(v->isBasic()) {
		BasicBlock *bb = v->toBasic();
		if(display_assembly)
			_output << "<{";
		_output << "BB " << v->index() << " (" << ot::address(bb->address()) << ")";
		if(display_assembly) {
			_output << " | ";
			bool first = true;

			InstSet* setInst = SET_OF_REMAINED_INSTRUCTIONS(bb);
			for(BasicBlock::InstIter inst(bb); inst(); inst++) {


				if(first)
					first = false;
				else
					_output << "<br ALIGN=\"LEFT\"/>";

				// Display labels
				for(Identifier<String>::Getter label(*inst, FUNCTION_LABEL);
				label(); label++)
					_output << *label << ":<br ALIGN=\"LEFT\"/>";
				for(Identifier<String>::Getter label(*inst, LABEL);
				label(); label++)
					_output << *label << ":<br ALIGN=\"LEFT\"/>";

				// display the debug line
				if(source_info) {
					Option<Pair<cstring, int> > info = _workspace->process()->getSourceLine(inst->address());
					if(info) {
						if((*info).fst != file || (*info).snd != line) {

							if(_show_slicing) {
								if(!setInst->contains(*inst)) {
									_output << "<Font face=\"monospace\" color=\"red\">";
								}
								else
									_output << "<Font face=\"monospace\">";
							}
							else
								_output << "<Font face=\"monospace\">";

							file = (*info).fst;
							line = (*info).snd;
							_output << file << ":" << line << "</Font><br ALIGN=\"LEFT\"/>";
						}
					}
					else {
						file = "";
						line = 0;
					}
				}

				// Display the instruction
				// Display the instruction
				if(_show_slicing) {
					if(!setInst->contains(*inst)) {
						_output << "<Font face=\"monospace\" color=\"red\">";
					}
					else
						_output << "<Font face=\"monospace\">";
				}
				else
					_output << "<Font face=\"monospace\">";

				_output << ot::address(inst->address()) << "    ";
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
						_output << '\\';
						break;
					}
					_output << dis[i];
				}

				// Add called label for branch
				if(inst->isControl() && !inst->isReturn()) {
					_output << "    # ";
					Inst *target = inst->target();
					if(!target)
						_output << "unknown";
					else {
						string label = FUNCTION_LABEL(target);
						if(!label)
							label = LABEL(target);
						if(label)
							_output << label;
						else
							_output << target->address();
					}
				}

				_output << "</Font>";
			}
			_output << "<BR ALIGN=\"LEFT\"/>}>";
		}
	}
}

}}
