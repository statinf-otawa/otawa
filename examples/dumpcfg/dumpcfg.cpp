/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	examples/dumpcfg/dumpcfg.cpp -- example dumping the description of a CFG.
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

using namespace elm;
using namespace otawa;


// Types
typedef genstruct::SortedBinMap<BasicBlock *, int> *call_map_t;


// Command class
class Command: public option::Manager {
	bool one;
	otawa::Manager manager;
	FrameWork *fw;
	CFGInfo *info;
	void dump(CString name);
public:
	Command(void);
	~Command(void);
	void run(int argc, char **argv);
	
	// Manager overload
	virtual void 	process (String arg);
};


// Options
static Command command;
static option::BoolOption inline_calls(command, 'i', "inline", "Inline the function calls.", false);
static option::BoolOption link_rec(command, 'r', "recursive", "Replace recursive calls by CFG loop links.", false);


// BasicBlock comparator
class BasicBlockComparator: public elm::Comparator<BasicBlock *> {
public:
	virtual int compare(BasicBlock *v1, BasicBlock *v2) {
		return v1->address() - v2->address();
	}
	static BasicBlockComparator comp;
};
BasicBlockComparator BasicBlockComparator::comp;
Comparator<BasicBlock *>& Comparator<BasicBlock *>::def
	= BasicBlockComparator::comp;
	

// CountVisitor class
class CountVisitor
: public genstruct::SortedBinMap<BasicBlock *, int >::Visitor {
	int cnt;
public:
	inline CountVisitor(void): cnt(0) { };
	virtual void process(BasicBlock *bb, int& index) {
		index = cnt;
		cnt++;
	}
};


/**
 * This class is used for storing information about each call to inline.
 */
class Call: public genstruct::SortedBinMap<BasicBlock *, int>::Visitor {
	CFGInfo *info;
	int usage;
	CFG *_cfg;
	call_map_t map;
	int base, ret;
	Call *back;
	static genstruct::DLList<Call *> calls;
	static int top;
	static otawa::id_t ID_Map;

	void build_map(CFG *cfg);
	void process(void);
	~Call(void);
public:
	Call(Call *back, CFGInfo *info, CFG *cfg, int _ret);
	void lock(void);
	void unlock(void);
	inline void put(void);
	inline int getBase(void) const { return base; };
	inline int getReturn(void) const { return ret; };
	inline Call *getBack(void) const { return back; };
	inline CFG *getCFG(void) const { return _cfg; };
	static void output(CFGInfo *info, CFG *cfg);
	
	// Visistor overload
	virtual void process(BasicBlock *bb, int& index);
};


// CircularityException class
class CircularityException {
	Call *call;
public:
	inline CircularityException(Call *_call): call(_call) { };
	void display(elm::io::Output& out);
};



/**
 * Display the error message when a circularity is found in function calls.
 * @param out	Output stream.
 */
void CircularityException::display(elm::io::Output& out) {
	Call *cur;
	out << "ERROR: circularity found in function calls.\nERROR: ";
	out << "(" << call->getCFG()->label() << " <- ";
	for(cur = call->getBack(); cur->getCFG() != call->getCFG(); cur = cur->getBack()) {
		assert(cur);
		out << " <- " << cur->getCFG()->label();
	}
	out << cur->getCFG()->label() << ") ";
	for(cur = cur->getBack(); cur; cur = cur->getBack())
		out << " <- " << cur->getCFG()->label();
	out << '\n';
}


/**
 * Queue of all function call remaining to process.
 */
genstruct::DLList<Call *> Call::calls;


/**
 * Top number for the basic blocks.
 */
int Call::top = 0;


/**
 * Map property identifier.
 */
otawa::id_t Call::ID_Map = Property::getID("Call.Map");


/**
 * Delete the call.
 */
Call::~Call(void) {
	if(back)
		back->unlock();
}


/**
 * Lock the call.
 */
void Call::lock(void) {
	usage++;
}


/**
 * Unlock the call.
 */
void Call::unlock(void) {
	usage--;
	/*if(!usage)
		delete this;*/
}


/**
 * Build a new inlined function call.
 * @param _back		Calling call.
 * @param _info		CFG information data structure.
 * @param cfg		CFG to process.
 * @param _ret		Index of the BB to return to.
 */
Call::Call(Call *_back, CFGInfo *_info, CFG *cfg, int _ret)
: info(_info), base(top), ret(_ret), _cfg(cfg), back(_back), usage(1) {
	
	// Lock the back
	if(back)
		back->lock();

	// Build the CFG if not already built
	map = cfg->get<call_map_t>(ID_Map, 0);
	if(!map) {
		map = new genstruct::SortedBinMap<BasicBlock *, int>;
		build_map(cfg);
 		cfg->set<call_map_t>(ID_Map, map);
	}

	// Allocate BB numbers
	top += map->count();
}


/**
 * Put a call in the call queue.
 * @return	True for success, false if there a recursivity.
 */
void Call::put(void) {
	calls.addLast(this);
}


/**
 * Build the CFG representation.
 * @param cfg	CFG to use.
 * @param bbs	Ordered list of BB in the CFG.
 */
void Call::build_map(CFG *cfg) {
	
	/* Initialize list of BB to process */
	genstruct::DLList<BasicBlock *> remain;
	remain.addLast(cfg->entry());	
	
	// Find all basic blocks
	while(!remain.isEmpty()) {
		
		// Get the current BB
		BasicBlock *bb = remain.first();
		remain.removeFirst();
		if(map->exists(bb))
			continue;
		map->put(bb, 0);
		
		// Process not-taken
		BasicBlock *target = bb->getNotTaken();
		if(target && !map->exists(target))
			remain.addLast(target);

		// Process taken
		if(!bb->isCall()) {
			target = bb->getTaken();
			if(target && !map->exists(target))
				remain.addLast(target);
		}
	}

	// Give number to basic blocks
	CountVisitor count_visitor;
	map->visit(count_visitor);	
}


/**
 * Process this call, that is, output the matching CFG.
 */
void Call::process(void) {
	map->visit(*this);
}


/**
 * Output the given CFG.
 * @param info	CFG information.
 * @param cfg	CFG to output.
 */
void Call::output(CFGInfo *info, CFG *cfg) {
	Call *call = new Call(0, info, cfg, -1);
	call->put();
	try {
		while(!calls.isEmpty()) {
			call = calls.first();
			calls.removeFirst();
			cout << "# " << call->_cfg->label() << '\n';
			/*for(Call *cur = call->getBack(); cur; cur = cur->getBack())
				cout << "<-" << cur->getCFG()->label();
			cout << '\n';*/
			call->process();
			//call->unlock();	!!DEBUG!!
		}
	}
	catch(CircularityException exn) {
		cerr << '\n';
		exn.display(cerr);
	}
}


/**
 * Process each BB in the given list for displaying one line by BB.
 * @param bb		Basic block to process.
 * @param index	Basic block index.
 */
void Call::process(BasicBlock *bb, int& index) {
	
	// Display header
	cout << (index + base)
		<< ' ' << bb->address()
		<< ' ' << (bb->address() + bb->getBlockSize() - 4);
	
	// Is it a return ?
	if(bb->isReturn()) {
		if(ret != -1)
			cout << ' ' << ret;
	}
	
	// Display the following BBs for an inlined call
	else if(bb->isCall() && inline_calls) {
		
		// Look not-taken tagey
		BasicBlock *target = bb->getNotTaken();
		assert(target);
		int next = map->get(target, -1);
		assert(next >= 0);
		next += base;
		
		// Look taken target
		target = bb->getTaken();
		assert(target);
		
		// Check circularity
		CFG *called_cfg = info->findCFG(target);
		Call *call;
		for(call = this; call; call = call->getBack())
			if(call->_cfg == called_cfg) {
				if(link_rec)
					break;
				else
					throw CircularityException(new Call(this, info, called_cfg, next));
			}

		// Add the function call
		if(!call) {
			call = new Call(this, info, called_cfg, next);
			call->put();
		}
				
		// Put the target
		cout << ' ' << call->getBase();
	}
	
	// Display following BBs
	else {

		// Not taken
		BasicBlock *target = bb->getNotTaken();
		if(target) {
			int next = map->get(target, -1);
			assert(next >= 0);
			next += base;
			cout << ' ' << next;
		}
		
		// Taken
		target = bb->getNotTaken();
		if(target) {
			int next = map->get(target, -1);
			assert(next >= 0);
			next += base;
			cout << ' ' << next;
		}
	}
	
	// End of line
	cout << " -1\n";
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
			 << "\" does not match sub-programm entry.\n";
		return;
	}
	
	// Output the CFG
	Call::output(info, cfg);
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
	if(!one)
		process("main");
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
		cout << '!' << arg << '\n';
		dump(arg.toCString());
		cout << '\n';
	}
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
