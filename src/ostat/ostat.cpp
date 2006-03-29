/*
 * $Id$
 * Copyright (c) 2006, IRIT-UPS
 *
 * ostat.cpp -- ostat main entry
 */
 
#include <otawa/otawa.h>
#include <otawa/cfg.h>
#include <elm/options.h>
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/HashTable.h>
#include <elm/genstruct/VectorQueue.h>

using namespace otawa;
using namespace elm;
using namespace elm::genstruct;

// Command class
class Statistics;
class Command: public option::Manager {
	String file;
	otawa::Manager manager;
	FrameWork *fw;
	CFGInfo *info;
	genstruct::Vector<String> funs;
	genstruct::Vector<Statistics *> stats;
public:
	Command(void);
	~Command(void);
	void run(int argc, char **argv);
	
	// Manager overload
	virtual void process(String arg);
};


/* Options */
static Command command;
static option::BoolOption tree_option(command, 't', "tree",
	"scan the whole calling tree.", false);
static option::BoolOption short_option(command, 's', "short",
	"perform short display.", false);


// Statistics class
class Statistics {
protected:
	long long bb_cnt;
	long long inst_cnt;
	long long inst_max;
	long long mem_cnt;
	long long mem_max;
	long long bra_cnt;
	long long bra_max;
public:
	Statistics(void);
	void addBB(BasicBlock *bb);
	inline long long bbCount(void) const { return bb_cnt; };
	virtual void print(elm::io::Output& out = cout);
	
	inline int instCount(void) const { return inst_cnt; };
	inline double averageInstCount(void) const { return (double)inst_cnt / bb_cnt; };
	inline long long maxInstCount(void) const { return inst_max; };
	
	inline long long memAccessCount(void) const { return mem_cnt; };
	inline double averageMemAccessCount(void) const { return (double)mem_cnt / bb_cnt; };
	inline long long maxMemAccessCount(void) const { return mem_max; };
	
	inline long long branchCount(void) const { return bra_cnt; };
	inline double averageBranchCount(void) const { return (double)bra_cnt / bb_cnt; };
	inline long long maxBranchCount(void) const { return bra_max; };
};


/**
 * Print the statistics.
 * @param out	Output to use (default cout).
 */
void Statistics::addBB(BasicBlock *bb) {
	
	// do not process virtual
	if(bb->isEnd())
		return;
	
	// initialize statistics
	long long insts = 0;
	long long mems = 0;
	long long bras = 0;
	bb_cnt++;
	
	// Count instructions
	for(BasicBlock::InstIterator inst(bb); inst; inst++)
		if(!inst->isPseudo()) {
			insts++;
			if(inst->isMem())
				mems++;
			if(inst->isControl())
				bras++;
		}
	
	// Record computations
	if(insts > inst_max)
		inst_max = insts;
	if(mems > mem_max)
		mem_max = mems;
	if(bras > bra_max)
		bra_max = bras;
	inst_cnt += insts;
	mem_cnt += mems;
	bra_cnt += bras;
}


void Statistics::print(elm::io::Output& out) {
	if(short_option) {
		out << bbCount() << '\t'
			<< instCount() << ','
			<< averageInstCount() << ','
			<< maxInstCount() << '\t'
			<< memAccessCount() << ','
			<< averageMemAccessCount() << ','
			<< maxMemAccessCount() << '\t'
			<< branchCount() << ','
			<< averageBranchCount() << ','
			<< maxBranchCount() << '\n';
	}
	else {
		out << "BB count = " << bbCount() << "\n";
		out << "type = total count, average/bb, max/bb\n";
		out << "instructions = "
			 << instCount() << ", "
			 << averageInstCount() << ", "
			 << maxInstCount() << "\n";
		out << "memory instructions = "
			 << memAccessCount() << ", "
			 << averageMemAccessCount() << ", "
			 << maxMemAccessCount() << "\n";
		out << "branch instructions = "
			 << branchCount() << ", "
			 << averageBranchCount() << ", "
			 << maxBranchCount() << "\n";
	}
}


// CFGStatistics class
class CFGStatistics: public Statistics {
	CFG *_cfg;
public:
	CFGStatistics(CFG *cfg);
	inline CFG *cfg(void) { return _cfg; };
	virtual void print(elm::io::Output& out = cout);
};


/**
 * Initialize a statistics for a CFG.
 */
CFGStatistics::CFGStatistics(CFG *cfg): _cfg(cfg) {
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++)
		addBB(bb);
};


/**
 * Initialize the statistics.
 */
Statistics::Statistics(void): bb_cnt(0), inst_cnt(0), mem_cnt(0), bra_cnt(0) {
}


/**
 * Count a new basic block.
 */
/**
 */
void CFGStatistics::print(elm::io::Output& out) {
	if(short_option)
		out << _cfg->label() << '\t';
	else
		out << "FUNCTION " << _cfg->label() << "\n";
	Statistics::print(out);
}


// TreeStatistics class
class TreeStatistics: public Statistics {
	HashTable<void *, CFGStatistics *> stats;
	VectorQueue<CFG *> todo;
public:
	TreeStatistics(CFG *cfg);
	virtual void print(elm::io::Output& out = cout);
};


/**
 * Build a new tree statistics and collect statistics for sub-functions.
 * @param cfg	Root CFG of the CFG tree to explore.
 */
TreeStatistics::TreeStatistics(CFG *cfg) {
	todo.put(cfg);
	while(!todo.isEmpty()) {
		
		// Compute stats for the head CFG
		CFG *cfg = todo.get();
		CFGStatistics *stat = new CFGStatistics(cfg);
		stats.put(cfg, stat);

		// Collect global statistics
		bb_cnt += stat->bbCount();
		inst_cnt += stat->instCount();
		mem_cnt += stat->memAccessCount();
		bra_cnt += stat->branchCount();
		
		// Look for called CFG
		for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++)
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->calledCFG()
				&& !stats.hasKey(edge->calledCFG())) {
					todo.put(edge->calledCFG());
					stats.put(edge->calledCFG(), 0);
				}
	}
}


/**
 */
void TreeStatistics::print(elm::io::Output& out) {
	
	// Print content
	for(HashTable<void *, CFGStatistics *>::ItemIterator stat(stats); stat; stat++) {
		stat->print(out);
		if(!short_option)
			out << "\n";
	}
	
	// Print total
	if(short_option)
		out << "TOTAL\t";
	else
		out << "TOTAL\n";
	Statistics::print(out);
}



/**
 * Initialize the command.
 */
Command::Command(void) {
	program = "OStat";
	version = "0.1";
	author = "Hugues Cassé";
	copyright = "Copyright (c) 2006, IRIT-UPS France";
	description =
		"Compute statistics on a binary file.\n"
		"If no function name is given, the main() function is used.";
	free_argument_description = "file_name [function names...]";
}


/**
 * Cleanup.
 */
Command::~Command(void) {
}


/**
 * Process a free argument.
 * @param arg	Free argument.
 */
void Command::process(String arg) {
	if(! file)
		file = arg;
	else
		funs.add(arg);
}


/**
 * run the program.
 */
void Command::run(int argc, char **argv) {
	parse(argc, argv);
	
	// Check file presence
	if(!file) {
		displayHelp();
		throw option::OptionException();
	}
	
	// Add main if no argument
	if(!funs)
		funs.add("main");
	
	// Load the file
	PropList props;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
	fw = manager.load(file.toCString(), props);
	info = fw->getCFGInfo();

	// Process arguments
	for(int i = 0; i < funs.length(); i++) {
		CFG *cfg = info->findCFG(funs[i]);
		if(!cfg) {
			cerr << "ERROR: \"" << file
				 << "\" does not contain a function named \"" << funs[i]
				 << "\".\n";
			throw option::OptionException();
		}
		if(tree_option)
			stats.add(new TreeStatistics(cfg));
		else
			stats.add(new CFGStatistics(cfg));
	}
	
	// Display statistics
	for(int i = 0; i < stats.length(); i++)
		stats[i]->print();
}


/* Startup */
int main(int argc, char **argv) {
	command.run(argc, argv);
}
