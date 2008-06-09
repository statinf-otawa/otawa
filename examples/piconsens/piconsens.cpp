/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/oipet/piconsens.cpp -- pipeline context-sensitivity experimentation.
 */

#include <errno.h>
#include <stdlib.h>
#include <elm/io.h>
#include <elm/io/OutFileStream.h>
#include <elm/options.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/ilp.h>
#include <elm/system/StopWatch.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/BBTimeSimulator.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/exegraph/LiExeGraphBBTime.h>
#include <otawa/exegraph/Microprocessor.h>
#include <otawa/ipet/TimeDeltaObjectFunctionModifier.h>

using namespace elm;
using namespace elm::option;
using namespace otawa;
using namespace otawa::ipet;
using namespace otawa::hard;
using namespace elm::option;
using namespace otawa::gensim;

Identifier<int> DELTA_MAX("delta_max", 0, otawa::NS);

typedef struct node_stat_t {
	struct node_stat_t *children, *sibling;
	int min, max;
	BasicBlock *bb;
	Vector<int> vals;
	inline node_stat_t(BasicBlock *_bb, int cost)
		: bb(_bb), children(0), sibling(0), min(cost), max(cost) { }
} node_stat_t;

Identifier<node_stat_t *> STAT("piconsens_stat", 0, otawa::NS);
Identifier<bool> MARK("piconsens_mark", false, otawa::NS);


// Command
class Command: public elm::option::Manager {
	String file;
	genstruct::Vector<String> funs;
	otawa::Manager manager;
	WorkSpace *fw;
	PropList stats;
	ilp::Constraint *node_cons;
	bool cons_used;
	
	void computeDeltaMax(TreePath< BasicBlock *, BBPath * > *tree, int parent_time);
	void computeMax(TreePath< BasicBlock *, BBPath * > *tree, int parent_time);
	void buildTrees(WorkSpace* fw, CFG* cfg);

	// Suffix methods
	typedef struct context_t {
		BasicBlock *bb;
		struct context_t *prev;
		inline context_t(BasicBlock *_bb, context_t *_prev)
		: bb(_bb), prev(_prev) { }
	} context_t;
	void computeMinTime(
		TreePath< BasicBlock *, BBPath * > *tree,
		int parent_time,
		context_t *pctx);
	void addSuffixConstraints(
		TreePath< BasicBlock *, BBPath * > *tree,
		int parent_time,
		context_t *pctx);
	void buildBoundConstraint(context_t *ctx, node_stat_t *node, int level, int min);

	// Statistics
	typedef struct stat_t {
		double total_span_sum;
		double total_vals_sum;
		int total_max_span;
		int total_max_vals;
		int bb_cnt;
		int bb_span_sum;
		int bb_vals_sum;
		int seq_cnt;
		inline stat_t(void): total_span_sum(0), total_vals_sum(0),
		bb_cnt(0),bb_span_sum(0), bb_vals_sum(0), seq_cnt(0),
		total_max_span(0), total_max_vals(0) { };
	} stat_t;
	Vector<stat_t> exe_stats;
	void recordSuffixStats(
		context_t *ctx,
		node_stat_t *node,
		int cost);
	void collectSuffixStats(node_stat_t *node, int depth = 0);

public:
	Command(void);
	void compute(String fun);
	void run(void);
	
	// Manager overload
	virtual void process (String arg);
};
Command command;


// Options
BoolOption dump_constraints(command, 'u', "dump-cons",
	"dump lp_solve constraints", false);
BoolOption verbose(command, 'v', "verbose", "verbose mode", false);
BoolOption exegraph(command, 'E', "exegraph", "use exegraph method", false);
IntOption delta(command, 'D', "delta", "use delta method with given sequence length", "length", 0);
IntOption suffix(command, 'S', "suffix", "use suffix method with given sequence length", "bound length", 0);
IntOption bound(command, 'b', "bound", "bound the suffix method to given length", "suffix length", 0);
IntOption degree(command, 'd', "degree", "superscalar degree power (real degree = 2^power)", "degree", 1);
StringOption proc(command, 'p', "processor", "used processor", "processor", "deg1.xml");
BoolOption do_stats(command, 's', "stats", "display statistics", false);
BoolOption do_time(command, 't', "time", "display basic block times", false);
BoolOption do_context(command, 'c', "context", "use context to improve accuracy of ExeGraph", false);
BoolOption deep_context(command, 'C', "deep-context", "use deep context to improve accuracy of ExeGraph", false);
BoolOption do_max(command, 'm', "max-context", "use max context in suffixes", false);


/**
 * Build the command manager.
 */
Command::Command(void) {
	program = "piconsens";
	version = "1.0.0";
	author = "H. Cassé";
	copyright = "Copyright (c) 2006, IRIT-UPS";
	description = "pipeline context-sensitivity experimentation.";
	free_argument_description = "binary_file function1 function2 ...";
}


/**
 * Process the free arguments.
 */
void Command::process (String arg) {
	if(!file)
		file = arg;
	else
		funs.add(arg);
}


/**
 * Compute the WCET for the given function.
 */
void Command::compute(String fun) {
	
	// Get the VCFG
	CFG *cfg = fw->getCFGInfo()->findCFG(fun);
	if(!cfg) {
		cerr << "ERROR: binary file does not contain the function \""
			 << fun << "\".\n";
		return;
	}
	VirtualCFG vcfg(cfg);
	ENTRY_CFG(fw) = &vcfg;
		
	// Prepare processor configuration
	PropList props;
	if(verbose) {
		otawa::Processor::VERBOSE(props) = true;
		cerr << "verbose !\n";
	}
	if(dump_constraints)
		props.set(EXPLICIT, true);
	if(do_stats)
		otawa::Processor::STATS(props) = &stats;
	/*if(deep_context)
		LiExeGraphBBTime::CONTEXT(props) = true;
	if(do_context)
		LiExeGraphBBTime::DELTA(props) = true;*/
	
	// Assign variables
	VarAssignment assign;
	assign.process(fw, props);
		
	// Compute BB time
	if(exegraph) {
		LiExeGraphBBTime tbt;
		tbt.process(fw, props);
	}
	else {
		BBTimeSimulator bbts;
		bbts.process(fw, props);
		
		// Compute min times
		if(suffix) {
			buildTrees(fw, &vcfg);
			for(CFG::BBIterator bb(&vcfg); bb; bb++)
				if(Delta::TREE(bb))
					computeMinTime(Delta::TREE(bb), 0, 0);
			for(CFG::BBIterator bb(&vcfg); bb; bb++)
				if(STAT(bb)) {
					if(do_max)
						TIME(bb) = STAT(bb)->max;
					else
						TIME(bb) = STAT(bb)->min;
				}
		}
	}

	// Build the system
	BasicConstraintsBuilder builder;
	builder.process(fw, props);
		
	// Load flow facts
	ipet::FlowFactLoader loader;
	loader.process(fw, props);

	// Build the object function to maximize
	BasicObjectFunctionBuilder fun_builder;
	fun_builder.process(fw);	

	// Use delta approach
	if(delta) {
		Delta::LEVELS(props) = delta;
		Delta delta;
		delta.process(fw, props);
	}
	
	// Add constraints for suffix approach
	if(suffix && !do_max) {
		if(bound) 
			for(CFG::BBIterator bb(&vcfg); bb; bb++) {
				if(STAT(bb)) {
					ilp::System *system = SYSTEM(fw);
					node_cons = system->newConstraint(ilp::Constraint::EQ);
					node_cons->addRight(1, VAR(bb));
					cons_used = false;
					buildBoundConstraint(0, STAT(bb), 0, TIME(bb));
					if(!cons_used)
						delete node_cons;
					else
						for(BasicBlock::InIterator edge(bb); edge; edge++)
							if(!MARK(edge))
								node_cons->addLeft(1, VAR(edge));
				}
			}
		else 
			for(CFG::BBIterator bb(&vcfg); bb; bb++) {
				if(Delta::TREE(bb)) {
					
					addSuffixConstraints(Delta::TREE(bb), 0, 0 /*&ctx*/);
				}
			}
	}
	
	// Time delta modifier
	if(exegraph && !delta && do_context) {
		TimeDeltaObjectFunctionModifier tdom;
		tdom.process(fw, props);
	}
	
	// Resolve the system
	WCETComputation wcomp;
	wcomp.process(fw, props);

	// Get the results
	ilp::System *sys = vcfg.use<ilp::System *>(SYSTEM);
	if(!do_stats && !do_time) 
		cout << WCET(vcfg);
	
	// Get statistics
	else if(!do_time) {
		/*if(exegraph) {
			const Vector<ExeGraphBBTime::stat_t>& prefs =
				*ExeGraphBBTime::PREFIX_STATS(stats);
			for(int i = 0; i < prefs.length(); i++)
				cout << i << '\t'
					<< prefs[i].total_span_sum << '\t'
				 	<< prefs[i].total_vals_sum << '\t'
				 	<< prefs[i].bb_cnt << '\t'
					 << ((double)prefs[i].total_span_sum / prefs[i].bb_cnt) << '\t'
					 << ((double)prefs[i].total_vals_sum / prefs[i].bb_cnt) << io::endl;
		}
		else*/ if(!exegraph && suffix) {
			for(CFG::BBIterator bb(&vcfg); bb; bb++)
				collectSuffixStats(STAT(bb));
			for(int i = 0; i < exe_stats.length(); i++)
				cout << i << '\t'
					 << ((double)exe_stats[i].total_span_sum / exe_stats[i].bb_cnt) << '\t'
					 << exe_stats[i].total_max_span << '\t'
					 << ((double)exe_stats[i].total_vals_sum / exe_stats[i].bb_cnt) << '\t'
					 << exe_stats[i].total_max_vals << io::endl;
		}
	}
	
	// Get the times
	else {
		if(exegraph)
			for(CFG::BBIterator bb(&vcfg); bb; bb++)
				cout << bb->number() << '\t' << ipet::TIME(bb) << io::endl;
		else
			for(CFG::BBIterator bb(&vcfg); bb; bb++)
				cout << bb->number() << '\t' << DELTA_MAX(bb) << io::endl;			
	}

	// Dump the ILP system
	if(dump_constraints) {
		String out_file = fun + ".lp";
		io::OutFileStream stream(&out_file);
		if(!stream.isReady())
			throw MessageException(_ << "cannot create file \"" << out_file << "\".");
		sys->dump(stream);
	}
}


/**
 * Launch the work.
 */
void Command::run(void) {
	
	// Any file
	if(!file)
		throw OptionException("binary file path required !");
	
	// Load the file
	GenericSimulator sim;
	PropList props;
	SIMULATOR(props) = &sim;
	DEGREE(props) = degree;
	PROCESSOR_PATH(props) = proc.value();
	fw = manager.load(&file, props);
	
	// Removing __eabi call if available (should move in a file configuration)
	CFG *cfg = fw->getCFGInfo()->findCFG("main");
	if(cfg != 0)
		for(CFG::BBIterator bb(cfg); bb; bb++)
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					break;
				}
	
	// Now process the functions
	if(!funs)
		compute("main");
	else
		for(int i = 0; i < funs.length(); i++)
			compute(funs[i]);
}


/**
 * Compute the delta max for the given tree.
 * @param tree			Tree to explore.
 * @param parent_time	Execution time of the parent tree.
 */
void Command::computeDeltaMax(
	TreePath< BasicBlock *, BBPath * > *tree,
	int parent_time)
{
	// Compute max
	int path_time = tree->rootData()->time(fw); 
	if(parent_time) {
		int time = path_time - parent_time;
		if(time > DELTA_MAX(tree->rootLabel()))
			DELTA_MAX(tree->rootLabel()) = time;
	}
	
	// Traverse children
	for(TreePath< BasicBlock *, BBPath * >::Iterator child(tree); child; child++)
		computeDeltaMax(child, path_time);
}


/**
 * Compute the delta max for the given tree.
 * @param tree			Tree to explore.
 * @param parent_time	Execution time of the parent tree.
 */
void Command::computeMax(
	TreePath< BasicBlock *, BBPath * > *tree,
	int parent_time)
{
	// Compute max
	int path_time = tree->rootData()->time(fw); 
	if(parent_time) {
		int time = path_time - parent_time;
		if(time > TIME(tree->rootLabel()))
			TIME(tree->rootLabel()) = time;
	}

	// Traverse children
	for(TreePath< BasicBlock *, BBPath * >::Iterator child(tree); child; child++)
		computeDeltaMax(child, path_time);
}


/**
 * Compute the delta min for the given tree.
 * @param tree			Tree to explore.
 * @param parent_time	Execution time of the parent tree.
 */
void Command::computeMinTime(
	TreePath< BasicBlock *, BBPath * > *tree,
	int parent_time,
	context_t *pctx)
{
	context_t ctx(tree->rootLabel(), pctx);
	
	// Traverse children
	int path_time = tree->rootData()->time(fw); 
	bool one = false;
	for(TreePath< BasicBlock *, BBPath * >::Iterator child(tree); child; child++) {
		computeMinTime(child, path_time, &ctx);
		one = true;
	}

	// Compute max
	if(!one) {
		int time = path_time - parent_time;
		/*if(time < TIME(tree->rootLabel()))
			TIME(tree->rootLabel()) = time;*/
		node_stat_t *node = STAT(tree->rootLabel());
		if(!node) {
			node = new node_stat_t(tree->rootLabel(), time);
			STAT(tree->rootLabel()) = node;
		}
		recordSuffixStats(&ctx, node, time);
		//cout << "COST = " << time << io::endl;
	}
}


/**
 * Build the trees.
 */
void Command::buildTrees(WorkSpace* fw, CFG* cfg) {
	assert(fw);
	assert(cfg);
	int levels = suffix;
	
	Vector<BBPath*> bbPathVector(4*cfg->countBB());
	for(CFG::BBIterator bb(cfg) ; bb ; bb++){
		if(!bb->isEntry() && !bb->isExit()){
			bbPathVector.add(BBPath::getBBPath(bb));
		}
	}
	
	// from 1 to nlevels+1 <=> from 0 to nlevels
	for(int i = 0 ;
		(levels && i < levels) || (bbPathVector.length() > 0 && !levels)  ;
		i++)
	{
		Vector<BBPath*> bbPathToProcess; // BBPaths that have to be processed
		
		// one search all length+1 sequences from sequences in bbPathVector
		// and one put all these in bbPathToProcess
		for(int j=0 ; j < bbPathVector.length() ; j++){
			Vector<BBPath*> *toInsert = bbPathVector[j]->nexts();
			int l2 = toInsert->length();
			for(int k = 0 ; k < l2 ; k++)
				bbPathToProcess.add(toInsert->get(k));
			delete toInsert;
		}

		// BBPaths processing
		bbPathVector.clear();
		for(int j=0 ; j < bbPathToProcess.length() ; j++){
			BBPath *bbPathPtr = bbPathToProcess[j];
			BBPath &bbPath = *bbPathPtr;
			int l = bbPath.length();

			bbPathVector.add(bbPathPtr);	
		}
	}	
}

/**
 * Add constraints for the suffix.
 */
void Command::addSuffixConstraints(
	TreePath< BasicBlock *, BBPath * > *tree,
	int parent_time,
	context_t *pctx)
{
	// traverse children
	context_t ctx(tree->rootLabel(), pctx);
	bool one = false;
	int path_time = tree->rootData()->time(fw);
	for(TreePath< BasicBlock *, BBPath * >::Iterator child(tree); child; child++) {
		addSuffixConstraints(child, path_time, &ctx);
		one = true;
	}
	
	// leaf -> add constraint
	if(!one) {
		int time = path_time - parent_time;
		if(time > TIME(tree->rootLabel())) {
			
			// add extra object function factor
			int delta = time - TIME(tree->rootLabel());
			ilp::System *system = SYSTEM(fw);
			ilp::Var *var = system->newVar();
			system->addObjectFunction(delta, var);
			 
			// Add edge constraints 
			for(context_t *cur = pctx, *prev = &ctx; cur; prev = cur, cur = cur->prev) {
		 	
		 		// find edge
		 		Edge *edge = 0;
		 		/*cout << "Looking for " << cur->bb->number()
		 			 << " to " << prev->bb->number() << io::endl;*/
		 		for(BasicBlock::OutIterator iter(cur->bb); iter; iter++) {
		 			if(iter->target() == prev->bb) {
		 				edge = iter;
		 				break;
		 			}
		 		}
		 		assert(edge);
		 	
		 		// add constraint
		 		ilp::Constraint *cons = system->newConstraint(ilp::Constraint::LE);
		 		cons->addLeft(1, var);
		 		cons->addRight(1, VAR(edge));
		 	}
		}
	}
}


/**
 */
void Command::recordSuffixStats(
	context_t *ctx,
	node_stat_t *node,
	int cost)
{
	//cout << ctx->bb->number() << " - ";
		
	// Record stats
	if(cost < node->min)
		node->min = cost;
	if(cost > node->max)
		node->max = cost;
	bool found = false;
	for(int i = 0; i < node->vals.length(); i++)
		if(cost == node->vals[i]) {
			found = true;
			break;
		}
	if(!found)
		node->vals.add(cost);	
	
	// Traverse backward
	if(ctx->prev) {
		
		// Find the matching child
		node_stat_t *child;
		for(child = node->children; child; child = child->sibling)
			if(child->bb == ctx->prev->bb)
				break;
		if(!child) {
			child = new node_stat_t(ctx->prev->bb, cost);
			child->sibling = node->children;
			node->children = child;
		}
		
		// Record this node
		recordSuffixStats(ctx->prev, child, cost);
	}
	
}


/**
 * Call to end the statistics of the current basic block.
 */
void Command::collectSuffixStats(node_stat_t *node, int depth) {
	assert(depth >= 0);

	// Do nothing if no node
	//cout << "!! " << depth << io::endl;
	if(!node)
		return;
	
	// Record local information
	while(depth >= exe_stats.length())
		exe_stats.add();
	exe_stats[depth].seq_cnt++;
	int span = node->max - node->min;
	int vals = node->vals.length();
	exe_stats[depth].bb_span_sum += span;
	exe_stats[depth].bb_vals_sum += vals;
	for(node_stat_t *child = node->children; child; child = child->sibling)
		collectSuffixStats(child, depth + 1);
	if(span > exe_stats[depth].total_max_span)
		exe_stats[depth].total_max_span = span; 
	if(vals > exe_stats[depth].total_max_vals)
		exe_stats[depth].total_max_vals = vals; 
	
	// Totalize
	if(depth == 0)
		for(int i = 0; i < exe_stats.length(); i++)
			if(exe_stats[i].seq_cnt) {
				exe_stats[i].bb_cnt++;
				exe_stats[i].total_span_sum +=
					(double)exe_stats[i].bb_span_sum / exe_stats[i].seq_cnt;
				exe_stats[i].total_vals_sum +=
					(double)exe_stats[i].bb_vals_sum / exe_stats[i].seq_cnt;
				exe_stats[i].seq_cnt = 0;
				exe_stats[i].bb_span_sum = 0;
				exe_stats[i].bb_vals_sum = 0;
			}
}


void Command::buildBoundConstraint(context_t *pctx, node_stat_t *node,
int level, int min) {
	//cout << "level = " << level << ", bound = " << bound << ", min = " << min << io::endl;
	
	context_t ctx(node->bb, pctx); 
	if(level < bound) {
		assert(node->children);
		for(node_stat_t *child = node->children; child; child = child->sibling)
			buildBoundConstraint(&ctx, child, level + 1, min);
	}
	else /* if(min < node->max) */ {
		
		// Build var name
		StringBuffer buf;
		buf << "ctx_" << node->bb->number();
		for(context_t *cur = pctx; cur; cur = cur->prev)
			buf << "_" << cur->bb->number();
		
		// add extra object function factor
		ilp::System *system = SYSTEM(fw);
		ilp::Var *var = system->newVar(buf.toString());
		node_cons->addLeft(1, var);
		cons_used = true;
		system->addObjectFunction(node->max - min, var);
			 
		// Add edge constraints 
		for(context_t *cur = pctx, *prev = &ctx; cur; prev = cur, cur = cur->prev) {
		 	
		 	// find edge
		 	Edge *edge = 0;
		 	for(BasicBlock::OutIterator iter(prev->bb); iter; iter++) {
		 		if(iter->target() == cur->bb) {
		 			edge = iter;
		 			break;
		 		}
		 	}
		 	assert(edge);
		 	if(!cur->prev)
		 		MARK(edge) = true;
		 	
		 	// add constraint
		 	ilp::Constraint *cons = system->newConstraint(ilp::Constraint::LE);
		 	cons->addLeft(1, var);
		 	cons->addRight(1, VAR(edge));
		}
	}
}


/**
 * Program entry point.
 */
int main(int argc, char **argv) {
	try {
		command.parse(argc, argv);
		command.run();
		return 0;
	}
	catch(OptionException& e) {
		cerr << "ERROR: " << e.message() << io::endl;
		command.displayHelp();
		return 1;
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << io::endl;
		cerr << strerror(errno) << io::endl;
		return 2;
	}
}
