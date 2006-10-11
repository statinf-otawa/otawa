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

using namespace elm;
using namespace elm::option;
using namespace otawa;
using namespace otawa::ipet;
using namespace otawa::hard;
using namespace elm::option;
using namespace otawa::gensim;


// Command
class Command: public elm::option::Manager {
	String file;
	genstruct::Vector<String> funs;
	otawa::Manager manager;
	FrameWork *fw;
	
	void initExeGraph(Microprocessor *processor);
public:
	Command(void);
	void compute(String fun);
	void run(void);
	
	// Manager overload
	virtual void process (String arg);
};
Command command;


// Options
BoolOption dump_constraints(command, 'c', "dump-cons",
	"dump lp_solve constraints", false);
BoolOption verbose(command, 'v', "verbose", "verbose mode", false);
BoolOption exegraph(command, 'E', "exegraph", "use exegraph method", false);
IntOption delta(command, 'D', "delta", "use delta method with given sequence length", "length", 0);
IntOption degree(command, 'd', "degree", "superscalar degree power (real degree = 2^power)", "degree", 1);
 

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
		
	//
	
	// Compute BB time
	if(exegraph && !delta) {
		Microprocessor processor;
		initExeGraph(&processor);
		ExeGraphBBTime::PROCESSOR(props) = &processor;
		ExeGraphBBTime tbt(props);
		tbt.process(fw);
	}
	else {
		BBTimeSimulator bbts(props);
		bbts.process(fw);
	}
	
	// Assign variables
	VarAssignment assign(props);
	assign.process(fw);
		
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
	
	// Resolve the system
	WCETComputation wcomp(props);
	wcomp.process(fw);

	// Get the result
	ilp::System *sys = vcfg.use<ilp::System *>(SYSTEM);
	cout /*<< "WCET [" << file << ":" << fun << "] = "*/
		 << vcfg.use<int>(WCET) << io::endl;	

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
//	LOADER(props) = &Loader::LOADER_Gliss_PowerPC;
	SIMULATOR(props) = &sim;
	DEGREE(props) = degree;
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
 * Initialize the given microprocessor.
 * @param processor	Processor to initialize.
 */
void Command::initExeGraph(Microprocessor *processor) {
	Queue *fetchQueue = processor->addQueue("FetchQueue", 1 << (degree + 1));
	Queue *ROB = processor->addQueue("ROB", 1 << (degree + 3));
	PipelineStage::pipeline_info_t pipeline_info;
	pipeline_info.stage_name = "FetchStage";
	pipeline_info.stage_short_name = "IF";
	pipeline_info.stage_category = PipelineStage::FETCH;
	pipeline_info.order_policy = PipelineStage::IN_ORDER;
	pipeline_info.stage_width = 1 << degree;
	pipeline_info.min_latency = 1;
	pipeline_info.max_latency = 1;
	pipeline_info.source_queue = NULL;
	pipeline_info.destination_queue = fetchQueue;
	processor->addPipelineStage(pipeline_info);
	pipeline_info.stage_name = "DecodeStage";
	pipeline_info.stage_short_name = "ID";
	pipeline_info.stage_category = PipelineStage::DECODE;
	pipeline_info.order_policy = PipelineStage::IN_ORDER;
	pipeline_info.stage_width = 1 << degree;
	pipeline_info.min_latency = 1;
	pipeline_info.max_latency = 1;
	pipeline_info.source_queue = fetchQueue;
	pipeline_info.destination_queue = ROB;
	processor->addPipelineStage(pipeline_info);
	pipeline_info.stage_name = "ExecutionStage";
	pipeline_info.stage_short_name = "EX";
	pipeline_info.stage_category = PipelineStage::EXECUTE;
	pipeline_info.order_policy = PipelineStage::OUT_OF_ORDER;
	pipeline_info.stage_width = 1 << degree;
	pipeline_info.min_latency = 1;
	pipeline_info.max_latency = 1;
	pipeline_info.source_queue = NULL;
	pipeline_info.destination_queue = NULL;
	PipelineStage *execute_stage = processor->addPipelineStage(pipeline_info);
	processor->setOperandReadingStage(execute_stage);
	processor->setOperandProducingStage(execute_stage);

	// Memory FU
	PipelineStage::FunctionalUnit::fu_info_t mem_info = {
		"MemUnit",
		"MEM",
		true,
		5,
		5,
		1,
		PipelineStage::IN_ORDER
	};
	PipelineStage::FunctionalUnit *memunit = execute_stage->addFunctionalUnit(mem_info);
	execute_stage->bindFunctionalUnit(memunit, MEMORY);

	// ALU FU
	PipelineStage::FunctionalUnit::fu_info_t alu_info = {
		"IALU",
		"IALU",
		false,
		1,
		1,
		2,
		PipelineStage::OUT_OF_ORDER
	};
	PipelineStage::FunctionalUnit *alu = execute_stage->addFunctionalUnit(alu_info);
	execute_stage->bindFunctionalUnit(alu, IALU);
	execute_stage->bindFunctionalUnit(alu,CONTROL);

	// FALU FU
	PipelineStage::FunctionalUnit::fu_info_t falu_info = {
		"FALU",
		"FALU",
		true,
		3,
		3,
		1,
		PipelineStage::OUT_OF_ORDER
	};
	PipelineStage::FunctionalUnit *falu = execute_stage->addFunctionalUnit(falu_info);
	execute_stage->bindFunctionalUnit(falu, FALU);
	
	// MUL FU
	PipelineStage::FunctionalUnit::fu_info_t mul_info = {
		"MUL",
		"MUL",
		true,
		6,
		6,
		1,
		PipelineStage::OUT_OF_ORDER
	};
	PipelineStage::FunctionalUnit *mul = execute_stage->addFunctionalUnit(mul_info);
	execute_stage->bindFunctionalUnit(mul, MUL);

	// DIV FU
	PipelineStage::FunctionalUnit::fu_info_t div_info = {
		"DIV",
		"DIV",
		false,
		15,
		15,
		1,
		PipelineStage::OUT_OF_ORDER
	};
	PipelineStage::FunctionalUnit *div = execute_stage->addFunctionalUnit(div_info);
	execute_stage->bindFunctionalUnit(div, DIV);

	// Commit stage
	pipeline_info.stage_name = "CommitStage";
	pipeline_info.stage_short_name = "CM";
	pipeline_info.stage_category = PipelineStage::COMMIT;
	pipeline_info.order_policy = PipelineStage::IN_ORDER;
	pipeline_info.stage_width = 1 << degree;
	pipeline_info.min_latency = 1;
	pipeline_info.max_latency = 1;
	pipeline_info.source_queue = ROB;
	pipeline_info.destination_queue = NULL;
	processor->addPipelineStage(pipeline_info);
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
