/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	examples/dumpcfg/dumpcfg.cpp -- example dumping the description of a CFG.
 */

#include <elm/io.h>
#include <elm/genstruct/DLList.h>
#include <elm/genstruct/SortedBinTree.h>
#include <otawa/manager.h>

using namespace elm;
using namespace otawa;


// Globals
static id_t ID_Number = Property::getID("dumpcfg.number");


// BasicBlock comparator
class BasicBlockComparator: public elm::Comparator< AutoPtr<BasicBlock> > {
public:
	virtual int compare(AutoPtr<BasicBlock> v1, AutoPtr<BasicBlock> v2) {
		return v1->address() - v2->address();
	}
	static BasicBlockComparator comp;
};
BasicBlockComparator BasicBlockComparator::comp;
Comparator< AutoPtr<BasicBlock> >& Comparator< AutoPtr<BasicBlock> >::def
	= BasicBlockComparator::comp;
	

// BasicBlockVisitor class
class BasicBlockVisitor
: public genstruct::SortedBinTree< AutoPtr<BasicBlock> >::Visitor {
	int cnt;
public:
	inline BasicBlockVisitor(void): cnt(0) { };
	int process(AutoPtr<BasicBlock> bb) {
		bb->set<int>(ID_Number, cnt++);
		return 1;
	}
};


// CFG Visitor
class ListVisitor
: public genstruct::SortedBinTree< AutoPtr<BasicBlock> >::Visitor {
	io::Output& out;
public:
	inline ListVisitor(io::Output& _out): out(_out) { };
	int process(AutoPtr<BasicBlock> bb);
};


/**
 * Process each BB in the given list for displaying one line by BB.
 * @param bb	BasicBlock to process.
 * @return	Not used.
 */
int ListVisitor::process(AutoPtr<BasicBlock> bb) {
	
	// Display header
	out << bb->use<int>(ID_Number)
		<< ' ' << bb->address()
		<< ' ' << (bb->address() + bb->getBlockSize() - 4);
	
	// Display the following BBs
	int cnt = 0;
	AutoPtr<BasicBlock> target = bb->getNotTaken();
	if(target) {
		cnt++;
		out << ' ' << target->use<int>(ID_Number);
	}
	target = bb->getTaken();
	if(target && !bb->isCall()) {
		cnt++;
		out << ' ' << target->use<int>(ID_Number);
	}
	if(cnt < 2)
		out << " -1";
	
	// End of line
	out << '\n';
	return 1;
}


/**
 * Build the CFG representation.
 * @param cfg	CFG to use.
 * @param bbs	Ordered list of BB in the CFG.
 */
static void build(CFG *cfg,
genstruct::SortedBinTree< AutoPtr<BasicBlock> >& bbs) {
	
	/* Initialize list of BB to process */
	genstruct::DLList< AutoPtr<BasicBlock> > remain;
	remain.addLast(cfg->entry());	
	
	// Find all basic blocks
	for(int num = 0; !remain.isEmpty(); num++) {
		
		// Get the current BB
		AutoPtr<BasicBlock> bb = remain.first();
		remain.removeFirst();
		if(bbs.contains(bb))
			continue;
		bbs.insert(bb);
		
		// Process not-taken
		AutoPtr<BasicBlock> target = bb->getNotTaken();
		if(target && !bbs.contains(target))
			remain.addLast(target);

		// Process taken
		if(!bb->isCall()) {
			target = bb->getTaken();
			if(target && !bbs.contains(target))
				remain.addLast(target);
		}
	}
}


/**
 * Process the given CFG, that is, build the sorted list of BB in the CFG and then display it.
* @param fw		Framework to use.
 * @param name	Name of the function to process.
 */
static void process(FrameWork *fw, CString name) {
	
	// Get the CFG information
	AutoPtr<CFGInfo> info = fw->getCFGInfo();
	
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
		
	// Build list of the CFG
	genstruct::SortedBinTree< AutoPtr<BasicBlock> > bbs;
	build(cfg, bbs);

	// Give number to basic blocks
	BasicBlockVisitor bb_visitor;
	bbs.visit(&bb_visitor);
	
	// Display the basic blocks
	ListVisitor list_visitor(cout);
	bbs.visit(&list_visitor);
}


/**
 * "dumpcfg" entry point.
 * @param argc		Argument count.
 * @param argv		Argument list.
 * @return		0 for success, >0 for error.
 */
int main(int argc, char **argv) {
	Manager manager;
	
	// Scan the arguments
	if(argc < 2) {
		cerr << "dumpcfg\nSYNTAX: dumpcfg <file> (<function> ...)?\n";
		return 1;
	}
	
	// Load the file
	PropList props;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
	FrameWork *fw;
	try {
		fw = manager.load(argv[1], props);
	} catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
		return 2;
	}
	
	// Compute the CFG
	AutoPtr<CFGInfo> info = fw->getCFGInfo();
	
	// Process only the main
	if(argc == 2)
		process(fw, "main");
	
	// Process the arguments
	else
		for(int i = 2; i < argc; i++) {
			cout << argv[i] << '\n';
			process(fw, argv[i]);
			cout << '\n';
		}
	
	// All is fine
	return 0;	
}
