/*
 *	$Id$
 *	dumpcfg command implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2004-08, IRIT UPS.
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
#include <elm/genstruct/DLList.h>
#include <elm/genstruct/SortedBinMap.h>
#include <elm/genstruct/DLList.h>
#include <elm/options.h>

#include <otawa/app/Application.h>
#include <otawa/manager.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/Virtualizer.h>
#include <otawa/cfg/CFGBuilder.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/proc/DynProcessor.h>
#include <otawa/prog/features.h>

#include "SimpleDisplayer.h"
#include "DisassemblerDisplayer.h"
#include "DotDisplayer.h"

using namespace elm;
using namespace otawa;


/**
 * @addtogroup commands
 * @section dumpcfg dumpcfg Command
 *
 * This command is used to output the CFG of a binary program using different
 * kind of output.
 * @par SYNTAX
 * @code
 * > dumpcfg options binary_file functions1 function2 ...
 * @endcode
 * dumpcfg first loads the binary file then compute and display the CFG of the
 * functions whose name is given. If there is no function, the @e main functions
 * is dumped.
 *
 * Currently, dumpcfg provides the following outputs:
 *
 * @li -S (simple output): the basic blocks are displayed, one by one, with
 * their number, their address and the -1-ended list of their successors.
 * @code
 * !icrc1
 * # Inlining icrc1
 * 0 10000368 100003a4 1 -1
 * 1 100003a8 100003b0 3 2 -1
 * 2 100003b4 100003b4 4 -1
 * 3 100003b8 100003c8 6 5 -1
 * 4 1000040c 10000418 7 -1
 * 5 100003cc 100003e8 8 -1
 * 6 100003ec 100003f8 8 -1
 * 7 1000041c 10000428 9 -1
 * 8 100003fc 10000408 1 -1
 * @endcode
 *
 * @li -L (listing output): each basic block is displayed starting by "BB",
 * followed by its number, a colon and the list of its successors. Its
 * successors may be T(number) for a taken edge, NT(number) for a not-taken edge
 * and C(function) for a function call.
 * @code
 * # Function main
 * # Inlining main
 * BB 1:  C(icrc) NT(2)
 *        main:
 *               10000754 stwu r1,-32(r1)
 *               10000758 mfspr r0,256
 *               1000075c stw r31,28(r1)
 *               10000760 stw r0,36(r1)
 *					...
 * BB 2:  C(icrc) NT(3)
 *               1000079c or r0,r3,r3
 *               100007a0 or r9,r0,r0
 *               100007a4 sth r9,8(r31)
 *               100007a8 addis r9,r0,4097
 *               ...
 *  BB 3:  T(4)
 *               10000808 or r0,r3,r3
 *               1000080c or r9,r0,r0
 *               10000810 sth r9,10(r31)
 *               10000814 addi r3,r0,0
 *               10000818 b 1
 * BB 4:
 *               1000081c lwz r11,0(r1)
 *               10000820 lwz r0,4(r11)
 *               10000824 mtspr 256,r0
 *               10000828 lwz r31,-4(r11)
 *               1000082c or r1,r11,r11
 *               10000830 bclr 20,0
 *
 * @endcode
 *
 * @li -D (dot output): the CFG is output as a DOT graph description.
 * @image html dot.png
 *
 * @li -X or --xml (XML output): output a CFG as an XML file satisfying the DTD
 * from $(OTAWA_HOME/share/Otawa/data/cfg.dtd .
 *
 * dumpcfg accepts other options like:
 * @li -a -- dump all functions.
 * @li -d -- disassemble the machine code contained in each basic block,
 * @li -i -- inline the functions calls (recursive calls are reduced to loops),
 * @li -v -- verbose information about the work.
 */

// Displayers
SimpleDisplayer simple_displayer;
DisassemblerDisplayer disassembler_displayer;
DotDisplayer dot_displayer;


// DumpCFG class
class DumpCFG: public Application {
public:

	DumpCFG(void);

	// options
	option::BoolOption remove_eabi;
	option::BoolOption all_functions;
	option::BoolOption inline_calls;
	option::BoolOption display_assembly;
	option::BoolOption simple;
	option::BoolOption disassemble;
	option::BoolOption dot;
	option::SwitchOption source;
	option::SwitchOption xml;
	Displayer *displayer;

protected:
	virtual void work(const string& entry, PropList &props) throw(elm::Exception) { dump(entry, props); }
	virtual void prepare(PropList &props);

private:
	void dump(CFG *cfg, PropList& props);
	void dump(const string& name, PropList& props);

};


/**
 */
void DumpCFG::prepare(PropList &props) {
	if(simple)
		displayer = &simple_displayer;
	else if(disassemble)
		displayer = &disassembler_displayer;
	else if(dot)
		displayer = &dot_displayer;
}


/**
 * Build the command.
 */
DumpCFG::DumpCFG(void):
	Application(
		"DumpCFG",
		Version(0, 3),
		"Dump to the standard output the CFG of functions."
			"If no function name is given, the main function is dumped.",
		"Hugues Casse <casse@irit.fr",
		"Copyright (c) 2004-08, IRIT-UPS France"
	),

	remove_eabi(*this, 'r', "remove", "Remove __eabi function call, if available.", false),
	all_functions(*this, 'a', "all", "Dump all functions.", false),
	inline_calls(*this, 'i', "inline", "Inline the function calls.", false),
	display_assembly(*this, 'd', "display", "Display assembly instructions.", false),
	//verbose(*this, 'v', "verbose", "activate verbose mode", false),
	simple(*this, 'S', "simple", "Select simple output (default).", false),
	disassemble(*this, 'L', "list", "Select listing output.", false),
	dot(*this, 'D', "dot", "Select DOT output.", false),
	source(*this, option::short_cmd, 's', option::cmd, "--source", option::description, "enable source debugging information output", option::def, false, option::end),
	xml(option::SwitchOption::Make(*this).cmd("-x").cmd("--xml").description("output the CFG as an XML file")),

	displayer(&simple_displayer)
{
}


/**
 * Dump the CFG.
 * @param cfg	CFG to dump.
 */
void DumpCFG::dump(CFG *cfg, PropList& props) {
	CFG *current_inline = 0;
	WorkSpace *my_ws = new WorkSpace(workspace());
	ENTRY_CFG(my_ws) = cfg;

	// if required, build the delayed
	if(workspace()->isProvided(DELAYED_FEATURE)
	|| workspace()->isProvided(DELAYED2_FEATURE))
		my_ws->require(DELAYED_CFG_FEATURE, props);

	// if required, virtualize
	if(inline_calls)
		my_ws->require(VIRTUALIZED_CFG_FEATURE, props);

	// get the CFG
	my_ws->require(COLLECTED_CFG_FEATURE);
	const CFGCollection *coll = INVOLVED_CFGS(my_ws);
	CFG *vcfg = (*coll)[0];

	// set options
	displayer->display_assembly = display_assembly;
	displayer->source_info = source;

	// XML case (will become the generic case)
	if(xml) {
		//XMLDisplayer dis;
		DynProcessor dis("otawa::cfgio::Output");
		dis.process(my_ws);
	}

	// Dump the CFG
	else {
		displayer->onProgramBegin(my_ws);
		displayer->onCFGBegin(vcfg);
		for(CFG::BBIterator bb(vcfg); bb; bb++) {

			// Looking for start of inline
			for(BasicBlock::InIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::VIRTUAL_CALL) {
					if(current_inline)
						displayer->onInlineEnd(current_inline);
					current_inline = CALLED_CFG(edge);
					displayer->onInlineBegin(current_inline);
				}

			// BB begin
			int index = bb->number();
			displayer->onBBBegin(bb, index);

			// Look out edges
			for(BasicBlock::OutIterator edge(bb); edge; edge++) {
				int target_index = -1;
				if(edge->target() && edge->kind() != Edge::CALL) {
					target_index = edge->target()->number();
					displayer->onEdge(0, bb, index, edge->kind(), edge->target(),
						target_index);
				} else {
					displayer->onCall(edge);
				}
			}

			// BB end
			displayer->onBBEnd(bb, index);
		}

		// Perform end
		if(current_inline)
			displayer->onInlineEnd(current_inline);
		displayer->onCFGEnd(vcfg);
		displayer->onProgramEnd(my_ws);
	}
}


/**
 * Process the given CFG, that is, build the sorted list of BB in the CFG and then display it.
 * @param name	Name of the function to process.
 */
void DumpCFG::dump(const string& name, PropList& props) {
	require(CFG_INFO_FEATURE);
	CFGInfo *info = CFGInfo::ID(workspace());
	ASSERT(info);
	CFG *cfg = info->findCFG(name);
	if(!cfg)
		throw elm::MessageException(_ << "cannot find function named \"" << name << '"');
	dump(cfg, props);
}


/**
 * "dumpcfg" entry point.
 * @param argc		Argument count.
 * @param argv		Argument list.
 * @return		0 for success, >0 for error.
 */
int main(int argc, char **argv) {
	DumpCFG dump;
	return dump.run(argc, argv);
}
