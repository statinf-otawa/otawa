/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/dumpcfg/dumpcfg.cpp -- dumpcfg utility source.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <elm/io.h>
#include <elm/genstruct/DLList.h>
#include <elm/genstruct/SortedBinMap.h>
#include <elm/genstruct/DLList.h>
#include <elm/options.h>
#include <otawa/manager.h>

#include "SimpleDisplayer.h"
#include "DisassemblerDisplayer.h"
#include "DotDisplayer.h"

using namespace elm;
using namespace otawa;


/**
 * @page dumpcfg dumpcfg Command
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
 * dumpcfg accepts other options like:
 * @li -i: inline the functions calls (recursive calls are reduced to loops),
 * @li -d: disassemble the machine code contained in each basic block,
 * @li -a: dump all functions.
 */

// Command class
class Command: public option::Manager {
	bool one;
	otawa::Manager manager;
	FrameWork *fw;
	CFGInfo *info;
	void dump(CFG *cfg);
	void dump(CString name);
public:
	Command(void);
	~Command(void);
	void run(int argc, char **argv);
	
	// Manager overload
	virtual void 	process (String arg);
};


// Displayers
static SimpleDisplayer simple_displayer;
static DisassemblerDisplayer disassembler_displayer;
static DotDisplayer dot_displayer;
static Displayer *displayer = &simple_displayer;


// Options
static Command command;
static option::BoolOption all_functions(command, 'a', "all",
	"Dump all functions.", false);
static option::BoolOption inline_calls(command, 'i', "inline",
	"Inline the function calls.", false);
option::BoolOption display_assembly(command, 'd', "display assembly instructions",
	"Display assembly.", false);


// Simple output option
class SimpleOption: public option::ActionOption {
public:
	inline SimpleOption(Command& command): option::ActionOption(command,
	'S', "simple", "Select simple output (default).") {
	}
	virtual void perform(void) {
		displayer = &simple_displayer;
	};
};
static SimpleOption simple(command);


// Disassemble output option
class DisassembleOption: public option::ActionOption {
public:
	inline DisassembleOption(Command& command): option::ActionOption(command,
	'L', "list", "Select listing output.") {
	};
	virtual void perform(void) {
		displayer = &disassembler_displayer;
	};
};
static DisassembleOption disassemble(command);


// Dot output option
class DotOption: public option::ActionOption {
public:
	inline DotOption(Command& command): option::ActionOption(command,
	'D', "dot", "Select DOT output.") {
	};
	virtual void perform(void) {
		displayer = &dot_displayer;
	};
};
static DotOption dot(command);


/**
 * Dump the CFG.
 * @param cfg	CFG to dump.
 */
void Command::dump(CFG *cfg) {
	CFG *current_inline = 0;
	
	// Get the virtual CFG
	VirtualCFG vcfg(cfg, inline_calls);
	
	// Dump the CFG
	displayer->onCFGBegin(cfg);
	for(CFG::BBIterator bb(&vcfg); bb; bb++) {
		
		// Looking for start of inline
		for(BasicBlock::InIterator edge(bb); edge; edge++)
			if(edge->kind() == Edge::VIRTUAL_CALL) {
				if(current_inline)
					displayer->onInlineEnd(current_inline);
				current_inline = edge->get<CFG *>(VirtualCFG::ID_CalledCFG, 0);
				displayer->onInlineBegin(current_inline);
			}
		
		// BB begin
		int index = bb->use<int>(CFG::ID_Index);
		displayer->onBBBegin(bb, index);
		
		// Look out edges
		for(BasicBlock::OutIterator edge(bb); edge; edge++) {
			int target_index = -1;
			if(edge->target() && edge->kind() != Edge::CALL)
				target_index = edge->target()->use<int>(CFG::ID_Index);
			displayer->onEdge(0, bb, index, edge->kind(), edge->target(),
				target_index);
		}
		
		// BB end
		displayer->onBBEnd(bb, index);
	}
	
	// Perform end
	if(current_inline)
		displayer->onInlineEnd(current_inline);
	displayer->onCFGEnd(cfg);
}


/**
 * Process the given CFG, that is, build the sorted list of BB in the CFG and then display it.
 * @param fw		Framework to use.
 * @param name	Name of the function to process.
 */
void Command::dump(CString name) {
	
	// Get the CFG information
	CFGInfo *info = fw->getCFGInfo();
	
	// Find label address
	address_t addr = 0;
	for(Iterator<File *> file(*fw->files()); file; file++) {
		addr = file->findLabel(name);
		if(addr)
			break;
	}
	if(!addr) {
		cerr << "ERROR: cannot find the label \"" << name << "\".\n";
		return;
	}
		
	// Find the matching CFG
	Inst *inst = fw->findInstAt(addr);
	if(!inst) {
		cerr << "ERROR: label \"" << name << "\" does not match code.\n";
		return;
	}
	CFG *cfg = info->findCFG(inst);
	if(!cfg) {
		cerr << "ERROR: label \"" << name
			 << "\" does not match sub-program entry.\n";
		return;
	}
	
	// Output the CFG
	dump(cfg);
}



/**
 * Process the free arguments.
 * @param arg	Free param value.
 */
void Command::process (String arg) {

	// First free argument is binary path
	if(!fw) {
		PropList props;
		props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
		fw = manager.load(arg.toCString(), props);
		info = fw->getCFGInfo();
	}
	
	// Process function names
	else {
		one = true;
		dump(arg.toCString());
	}
}


/**
 * Build the command.
 */
Command::Command(void): one(false), fw(0) {
	program = "DumpCFG";
	version = "0.2";
	author = "Hugues Cass�";
	copyright = "Copyright (c) 2004, IRIT-UPS France";
	description = "Dump to the standard output the CFG of functions."
		"If no function name is given, the main function is dumped.";
	free_argument_description = "[function names...]";
}


/**
 * Run the command.
 * @param argc	Argument count.
 * @param argv	Argument vector.
 */
void Command::run(int argc, char **argv) {
	parse(argc, argv);
	if(!fw) {
		displayHelp();
		throw option::OptionException();
	}
	if(all_functions) {
		CFGInfo *info = fw->getCFGInfo();
		for(Iterator<CFG *> cfg(info->cfgs()); cfg; cfg++)
			if(cfg->label())
				dump(cfg);
	}
	else if(!one)
		process("main");
}


/**
 * Release all command ressources.
 */
Command::~Command(void) {
	delete fw;
}


/**
 * "dumpcfg" entry point.
 * @param argc		Argument count.
 * @param argv		Argument list.
 * @return		0 for success, >0 for error.
 */
int main(int argc, char **argv) {
	try {
		command.run(argc, argv);
		return 0;
	}
	catch(option::OptionException _) {
		return 1;
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
		return 2;
	}
}
