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
option::BoolOption display_assembly(command, 'A', "assembly",
	"Display assembly.", false);


// Simple output option
class SimpleOption: public option::ActionOption {
public:
	inline SimpleOption(Command& command): option::ActionOption(command,
	's', "simple", "Perform simple output.") {
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
	'd', "disassemble", "Perform output with disassembling the basic blocks.") {
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
	'D', "dot", "Perform output usable by dot utility.") {
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
	author = "Hugues Cassé";
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
