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
#include <otawa/exegraph/ExeGraphBBTime.h>
#include <otawa/exegraph/Microprocessor.h>
#include <otawa/ipet/TimeDeltaObjectFunctionModifier.h>

using namespace elm;
using namespace elm::option;
using namespace otawa;
using namespace otawa::ipet;
using namespace otawa::hard;
using namespace elm::option;
using namespace otawa::gensim;

GenericIdentifier<int> DELTA_MAX("delta_max", 0, OTAWA_NS);

// Command
class Command: public elm::option::Manager {
	String file;
	genstruct::Vector<String> funs;
	otawa::Manager manager;
	FrameWork *fw;
	PropList stats;
	
	void computeDeltaMax(TreePath< BasicBlock *, BBPath * > *tree, int parent_time);
	void computeMinTime(TreePath< BasicBlock *, BBPath * > *tree, int parent_time);
	void buildTrees(FrameWork* fw, CFG* cfg);

	typedef struct context_t {
		BasicBlock *bb;
		struct context_t *prev;
		inline context_t(BasicBlock *_bb, context_t *_prev)
		: bb(_bb), prev(_prev) { }
	} context_t;
	void addSuffixConstraints(
		TreePath< BasicBlock *, BBPath * > *tree,
		int parent_time,
		context_t *pctx);

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
BoolOption suffix(command, 'S', "suffix", "use suffix method with given sequence length", false);
IntOption degree(command, 'd', "degree", "superscalar degree power (real degree = 2^power)", "degree", 1);
StringOption proc(command, 'p', "processor", "used processor", "processor", "deg1.xml");
BoolOption do_stats(command, 's', "stats", "display statistics", false);
BoolOption do_time(command, 't', "time", "display basic block times", false);
BoolOption do_context(command, 'c', "context", "use context to improve accuracy of ExeGraph", false);
BoolOption deep_context(command, 'C', "deep-context", "use deep context to improve accuracy of ExeGraph", false);
 

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
		PROC_VERBOSE(props) = true;
		cerr << "verbose !\n";
	}
	if(dump_constraints)
		props.set(EXPLICIT, true);
	if(do_stats)
		PROC_STATS(props) = &stats;
	if(deep_context)
		EXEGRAPH_CONTEXT(props) = true;
	if(do_context)
		EXEGRAPH_DELTA(props) = true;
	
	// Assign variables
	VarAssignment assign(props);
	assign.process(fw);
		
	// Compute BB time
	if(exegraph) {
		ExeGraphBBTime tbt(props);
		tbt.process(fw);
	}
	else {
		BBTimeSimulator bbts(props);
		bbts.process(fw);
		
		// Compute min times
		if(suffix) {
			buildTrees(fw, cfg);
			for(CFG::BBIterator bb(&vcfg); bb; bb++)
				if(BBPath::TREE(bb)) 
					computeMinTime(BBPath::TREE(bb), 0);
		}
	}

	// Build the system
	BasicConstraintsBuilder builder(props);
	builder.process(fw);
		
	// Load flow facts
	ipet::FlowFactLoader loader(props);
	loader.process(fw);

	// Build the object function to maximize
	BasicObjectFunctionBuilder fun_builder;
	fun_builder.process(fw);	

	// Use delta approach
	if(delta) {
		Delta::LEVELS(props) = delta;
		Delta delta(props);
		delta.process(fw);
	}
	
	// Add constraints for suffix approach
	if(suffix) {
		for(CFG::BBIterator bb(&vcfg); bb; bb++) {
			context_t ctx(bb, 0);
			if(BBPath::TREE(bb)) 
				addSuffixConstraints(BBPath::TREE(bb), 0, &ctx);
		}
	}
	
	// Time delta modifier
	if(exegraph && !delta && do_context) {
		TimeDeltaObjectFunctionModifier tdom(props);
		tdom.process(fw);
	}
	
	// Resolve the system
	WCETComputation wcomp(props);
	wcomp.process(fw);

	// Get the results
	ilp::System *sys = vcfg.use<ilp::System *>(SYSTEM);
	if(!do_stats && !do_time) 
		cout << WCET(vcfg);
	
	// Get statistics
	else if(!do_time) {
		if(exegraph) {
			const Vector<ExeGraphBBTime::stat_t>& prefs = *EXEGRAPH_PREFIX_STATS(stats);
			for(int i = 0; i < prefs.length(); i++)
				cout << i << '\t'
					/*<< prefs[i].total_span_sum << '\t'
				 	<< prefs[i].total_vals_sum << '\t'
				 	<< prefs[i].bb_cnt << '\t'*/
					 << ((double)prefs[i].total_span_sum / prefs[i].bb_cnt) << '\t'
					 << ((double)prefs[i].total_vals_sum / prefs[i].bb_cnt) << io::endl;
		}
	}
	
	// Get the times
	else {
		if(exegraph)
			for(CFG::BBIterator bb(&vcfg); bb; bb++)
				cout << bb->number() << '\t' << ipet::TIME(bb) << io::endl;
		else {
			for(CFG::BBIterator bb(&vcfg); bb; bb++)
				if(BBPath::TREE(bb)) 
					computeDeltaMax(BBPath::TREE(bb), 0);
			for(CFG::BBIterator bb(&vcfg); bb; bb++)
				cout << bb->number() << '\t' << DELTA_MAX(bb) << io::endl;			
		}
	}

	// Dump the ILP system
	if(dump_constraints) {
		String out_file = fun + ".lp";
		io::OutFileStream stream(&out_file);
		if(!stream.isReady())
			throw MessageException("cannot create file \"%s\".", &out_file);
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
 * Compute the delta min for the given tree.
 * @param tree			Tree to explore.
 * @param parent_time	Execution time of the parent tree.
 */
void Command::computeMinTime(
	TreePath< BasicBlock *, BBPath * > *tree,
	int parent_time)
{
	// Traverse children
	int path_time = tree->rootData()->time(fw); 
	bool one = false;
	for(TreePath< BasicBlock *, BBPath * >::Iterator child(tree); child; child++) {
		computeMinTime(child, path_time);
		one = true;
	}

	// Compute max
	if(!one) {
		int time = path_time - parent_time;
		if(time < TIME(tree->rootLabel()))
			TIME(tree->rootLabel()) = time;
	}
}


/**
 * Build the trees.
 */
void Command::buildTrees(FrameWork* fw, CFG* cfg) {
	assert(fw);
	assert(cfg);
	int levels = 4;
	
	Vector<BBPath*> bbPathVector(4*cfg->bbs().count());
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
			ilp::System *system = getSystem(fw, ENTRY_CFG(fw));
			ilp::Var *var = system->newVar();
			system->addObjectFunction(delta, var);
			 
			// Add edge constraints 
			for(context_t *cur = pctx, *prev = &ctx; cur; prev = cur, cur = cur->prev) {
		 	
		 		// find edge
		 		Edge *edge = 0;
		 		for(BasicBlock::OutIterator iter(prev->bb); iter; iter++)
		 			if(iter->target() == cur->bb) {
		 				edge = iter;
		 				break;
		 			}
		 		assert(edge);
		 	
		 		// add constraint
		 		ilp::Constraint *cons = system->newConstraint(ilp::Constraint::LE);
		 		cons->addLeft(1, var);
		 		cons->addRight(1, getVar(system, edge));
		 	}
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
