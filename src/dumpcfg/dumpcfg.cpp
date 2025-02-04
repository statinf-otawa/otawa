/*
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
#include <elm/data/BiDiList.h>
#include <elm/data/SortedList.h>
#include <elm/options.h>

#include <otawa/app/CFGApplication.h>
#include <otawa/manager.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/proc/DynProcessor.h>
#include <otawa/prog/features.h>
#include "../../include/otawa/flowfact/FlowFactLoader.h"

#include "SimpleDisplayer.h"
#include "DisassemblerDisplayer.h"
#include "DotDisplayer.h"
#include "MultipleDotDisplayer.h"

using namespace elm;
using namespace otawa;
namespace opt = elm::option;


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


/**
 */
Displayer::Displayer(cstring name, const Version version):
	Processor(name, version),
	display_assembly(false),
	source_info(false),
	display_all(false),
	perform_view(nullptr),
	display_sem(false),
	display_kind(false),
	display_regs(false),
	display_target(false),
	display_bytes(false)
{
	require(otawa::COLLECTED_CFG_FEATURE);
}


/**
 * Select disassembling option.
 */
Identifier<bool> Displayer::DISASSEMBLE("", false);

/**
 * Select source information display.
 */
Identifier<bool> Displayer::SOURCE("", false);

/**
 * Display all CFGs.
 */
Identifier<bool> Displayer::ALL("", false);

/**
 * If possible, launch a viewer of the produced output.
 */
Identifier<bool> Displayer::VIEW("");

/**
 */
void Displayer::configure(const PropList& props) {
	Processor::configure(props);
	display_assembly = DISASSEMBLE(props);
	source_info = SOURCE(props);
	display_all = ALL(props);
	perform_view = VIEW(props);
}


// Displayers
/*SimpleDisplayer simple_displayer;
DisassemblerDisplayer disassembler_displayer;
DotDisplayer dot_displayer;
MultipleDotDisplayer mult_displayer;*/


// DumpCFG class
class DumpCFG: public CFGApplication {
public:

	DumpCFG(void);

	// options
	opt::Switch	remove_eabi,
				all_functions,
				display_assembly,
				simple,
				disassemble,
				dot,
				source,
				xml,
				mult,
				display_sem,
				display_kind,
				display_regs,
				display_target,
				display_bytes;

	Displayer *displayer;

protected:
	void processTask(const CFGCollection& coll, PropList &props) override {
		dump(props);
	}
	void prepare(PropList &props) override;

private:
	void dump(CFG *cfg, PropList& props);
	void dump(PropList& props);

};


/**
 */
void DumpCFG::prepare(PropList &props) {
	if(simple)
		displayer = new SimpleDisplayer();
	else if(disassemble)
		displayer = new DisassemblerDisplayer();
	else if(dot)
		displayer = new DotDisplayer();
	else if(mult)
		displayer = new MultipleDotDisplayer();
	else
		displayer = new SimpleDisplayer();
}


/**
 * Build the command.
 */
DumpCFG::DumpCFG(void):
	CFGApplication(Make("dumpcfg", Version(2, 2, 0))
		.description(
			"Dump to the standard output the CFG of functions.\n"
			"If no function name is given, the main function is dumped.")
		.copyright("Copyright (c) 2016, IRIT - UPS")
		.author("H. Cassé <casse@irit.fr>")
		.free_argument("PROGRAM TASK*")
	),

	remove_eabi		(make_switch()			.cmd("-r").cmd("--remove")		.description("Remove __eabi function call, if available.")),
	all_functions	(make_switch()			.cmd("-a").cmd("--all")			.description("Dump all functions.")),
	display_assembly(make_switch()			.cmd("-d").cmd("--display")		.description("Display assembly instructions.")),
	simple			(make_switch()			.cmd("-S").cmd("--simple")		.description("Select simple output (default).")),
	disassemble		(make_switch()			.cmd("-L").cmd("--list")		.description("Select listing output.")),
	dot				(make_switch()			.cmd("-D").cmd("--dot")			.description("Select DOT output.")),
	source			(make_switch()			.cmd("-s").cmd("--source")		.description("enable source debugging information output")),
	xml				(make_switch()			.cmd("-x").cmd("--xml")			.description("output the CFG as an XML file")),
	mult			(make_switch()			.cmd("-M").cmd("--multiple-dot").description("output multiple .dot file (one for each CFG) linked with URLs")),
	display_sem		(make_switch()			.cmd("--display-sem")			.help("display semantic instructions")),
	display_kind	(make_switch()			.cmd("--display-kind")			.help("display instruction kind")),
	display_regs	(make_switch()			.cmd("--display-regs")			.help("display used registers")),
	display_target	(make_switch()			.cmd("--display-target")		.help("display branch instruction targets")),
	display_bytes	(make_switch()			.cmd("--display-bytes")			.help("display instruction bytes")),

	displayer(nullptr)
{
}


/**
 * Process the given CFG, that is, build the sorted list of BB in the CFG and then display it.
 * @param name	Name of the function to process.
 */
void DumpCFG::dump(PropList& props) {

	// get the CFG
	require(COLLECTED_CFG_FEATURE);

	// XML case
	if(xml) {
		DynProcessor dis("otawa::cfgio::Output");
		dis.process(workspace());
	}

	// view used
	else if(view) {
		workspace()->require(CFG_DUMP_FEATURE, props);
	}
	
	// Dump the CFG
	else {

		// set options
		if(display_assembly)
			Displayer::DISASSEMBLE(props) = display_assembly;
		if(source)
			Displayer::SOURCE(props) = source;
		if(cfg_virtualize)
			Displayer::ALL(props) = true;
		/*if(view)
			Displayer::VIEW(props) = view;*/
		displayer->display_sem = display_sem;
		displayer->display_kind = display_kind;
		displayer->display_regs = display_regs;
		displayer->display_target = display_target;
		displayer->display_bytes = display_bytes;
		run(displayer);
	}
}


/**
 * "dumpcfg" entry point.
 * @param argc		Argument count.
 * @param argv		Argument list.
 * @return		0 for success, >0 for error.
 */
/*int main(int argc, char **argv) {
	DumpCFG dump;
	return dump.run(argc, argv);
}*/
//ELM_RUN(DumpCFG);
OTAWA_RUN(DumpCFG);

