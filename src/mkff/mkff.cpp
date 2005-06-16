/*
 * $Id$
 * Copyright (c) 2005, IRIT-UPS <casse@irit.fr>
 *
 * src/mkff/mkff.cpp -- entry point of mkff utility.
 */

#include <elm/io.h>
#include <elm/options.h>
#include <elm/genstruct/Vector.h>
#include <otawa/otawa.h>
#include <otawa/util/ContextTree.h>

using namespace elm;
using namespace otawa;

// Marker for processed sub-programs
//static Identifier ID_Processed("mkff.processed");

// Command class
class Command: public option::Manager {
	bool one;
	otawa::Manager manager;
	FrameWork *fw;
	CFGInfo *info;
	genstruct::Vector<ContextTree *> funs;
	void perform(String name);
	void scanFun(ContextTree *ctree);
	void scanLoop(ContextTree *ctree, int indent);
public:
	Command(void);
	~Command(void);
	void run(int argc, char **argv);
	
	// Manager overload
	virtual void process (String arg);
};


// Options
static Command command;


/**
 * Process a function context tree node.
 * @param ctree	Function context tree node.
 */
void Command::scanFun(ContextTree *ctree) {
	assert(ctree);
	
	// Display header
	bool display = false;
	for(Iterator<ContextTree *> child(ctree->children()); child; child++)
		if(child->kind() == ContextTree::LOOP) {
			display = true;
			break;
		}
	if(display)
		cout << "// Function " << ctree->cfg()->label() << "\n";

	// Scan the content
	scanLoop(ctree, 0);		
	
	// Display footer
	if(display)
		cout << "\n";
}


/**
 * Scan a context tree for displaying its loop flow facts.
 * @param ctree		Current context tree.
 * @param indent	Current indentation.
 */
void Command::scanLoop(ContextTree *ctree, int indent) {
	assert(ctree);
	
	for(Iterator<ContextTree *> child(ctree->children()); child; child++) {
		
		// Process loop
		if(child->kind() == ContextTree::LOOP) {
			for(int i = 0; i < indent; i++)
				cout << "  ";
			cout << "loop 0x" << child->bb()->address() << " ?;\n"; 
			scanLoop(child, indent + 1);
		}
		
		// Process function
		else {
			bool found = false;
			for(int i = 0; i < funs.length(); i++)
				if(funs[i]->cfg() == child->cfg()) {
					found = true;
					break;
				}
			if(!found)
				funs.add(child);
		}
	}
}


/**
 * Perform the work, get the loop and outputting the flow fact file.
 * @param name	Name of the function to process.
 */
void Command::perform(String name) {
	assert(name);
	
	// Find the function
	CFGInfo *info = fw->getCFGInfo();
	CFG *cfg = info->findCFG(name);
	if(!cfg) {
		cerr << "ERROR: label \"" << name
			 << "\" does not match sub-programm entry.\n";
		return;
	}
	
	// Build the context tree
	ContextTree ctree(cfg);
	funs.add(&ctree);
	
	// Display the context tree
	for(int i = 0; i < funs.length(); i++)
		scanFun(funs[i]);
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
		perform(arg);
	}
}


/**
 * Build the command.
 */
Command::Command(void): one(false), fw(0) {
	program = "mkff";
	version = "0.1";
	author = "Hugues Cassé";
	copyright = "Copyright (c) 2005, IRIT-UPS France";
	description = "Generate a flow fact file for an application.";
	free_argument_description = "program [function names...]";
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
