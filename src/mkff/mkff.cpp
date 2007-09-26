/*
 * $Id$
 * Copyright (c) 2005-07, IRIT-UPS <casse@irit.fr>
 *
 * mkff utility entry point
 */

#include <elm/io.h>
#include <elm/options.h>
#include <elm/genstruct/Vector.h>
#include <otawa/cfg.h>
#include <otawa/otawa.h>
#include <otawa/util/ContextTree.h>
#include <elm/checksum/Fletcher.h>
#include <elm/io/InFileStream.h>
#include <elm/system/Path.h>

using namespace elm;
using namespace otawa;


/**
 * @page mkff mkff Command
 * 
 * This command is used to generate a @e .ff file template to pass flow facts
 * to OTAWA. Currently, only constant loop bounds are supported as flow facts.
 * 
 * @par SYNTAX
 * @code
 * $ mkff binary_file function1 function2 ...
 * @endcode
 * 
 * mkff builds the .ff loop statements for each function calling sub-tree for
 * the given binary file. If no function name is given, only the main()
 * function is processed.
 * 
 * The loop statement are indented according their depth in the context tree
 * and displayed with the current syntax:
 * @code
 * // Function function_name
 * loop loop1_address ?;
 *  loop loop1_1_address ?;
 *    loop loop_1_1_address ?;
 * loop loop2_address ?;
 * @endcode
 * The "?" question marks must be replaced by the maximum loop bound in order
 * to get valid .ff files. A good way to achieve this task is to use the
 * @ref dumpcfg command to get  a graphical display of the CFG.
 * 
 * @par Example
 * @code
 * $ mkff fft1
 * // Function main
 * loop 0x100006c0 ?;
 * 
 * // Function fft1
 * loop 0x10000860 ?;
 * loop 0x10000920 ?;
 *  loop 0x10000994 ?;
 *    loop 0x10000a20 ?;
 * loop 0x10000bfc ?;
 *  loop 0x10000d18 ?;
 * loop 0x10000dc0 ?;
 *
 * // Function sin
 * loop 0x10000428 ?;
 * loop 0x1000045c ?;
 * loop 0x10000540 ?;
 * @endcode
 * 
 * @par Future
 * mkff is temporary solution to the problem of passing flow fact information
 * to the WCET computation. We hope in a closer future to provide an improved
 * tool:
 * @li allowing to put annotation in source files,
 * @li allowing to have more accurate loop bound description. 
 */

// Marker for processed sub-programs
//static Identifier ID_Processed("mkff.processed");

// Command class
class Command: public option::Manager {
	bool one;
	otawa::Manager manager;
	WorkSpace *fw;
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
			cout << "loop 0x" << fmt::address(child->bb()->address()) << " ?;\n"; 
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

	// Configuration	
	PropList props;
	//Processor::VERBOSE(props) = true;
	TASK_ENTRY(props) = &name;

	// Build the context tree
	ContextTreeBuilder builder;
	builder.process(fw, props);
	funs.add(CONTEXT_TREE(fw));
	
	// Build the checksums of the binary files
	for(Process::FileIter file(fw->process()); file; file++) {
		checksum::Fletcher sum;
		io::InFileStream stream(file->name());
		sum.put(stream);
		elm::system::Path path = file->name();
		cout << "checksum \"" << path.namePart()
			 << "\" 0x" << io::hex(sum.sum()) << ";\n";
	}
	cout << io::endl;

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
		NO_SYSTEM(props) = true;
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
	version = "1.0";
	author = "Hugues Cassé";
	copyright = "Copyright (c) 2005-07, IRIT-UPS France";
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
		throw option::OptionException("no binary file to process");
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
	catch(option::OptionException& e) {
		cerr << "ERROR: " << e.message() << io::endl;
		command.displayHelp();
		return 1;
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << '\n';
		return 2;
	}
}
