/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test/ipet/exegraph.cpp -- test for Execution Graph modeling feature.
 */

#include <stdlib.h>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/ilp.h>
#include <otawa/exegraph/ExeGraphBBTime.h>
#include <otawa/exegraph/Microprocessor.h>


using namespace elm;
using namespace elm::io;
using namespace otawa;
using namespace otawa::ipet;
using namespace std;


int main(int argc, char **argv) {
	
	elm::io::OutFileStream *logStream = new elm::io::OutFileStream("log.txt");
	if(!logStream->isReady())
		throw IOException(logStream->lastErrorMessage());
	elm::io::Output logFile(*logStream);
	
	Microprocessor processor;
	Queue *fetchQueue = processor.addQueue("FetchQueue", 2);
	Queue *ROB = processor.addQueue("ROB",2);
	PipelineStage::pipeline_info_t pipeline_info;
	pipeline_info.stage_name = "FetchStage";
	pipeline_info.stage_short_name = "IF";
	pipeline_info.stage_category = PipelineStage::FETCH;
	pipeline_info.order_policy = PipelineStage::IN_ORDER;
	pipeline_info.stage_width = 2;
	pipeline_info.min_latency = 1;
	pipeline_info.max_latency = 1;
	pipeline_info.source_queue = NULL;
	pipeline_info.destination_queue = fetchQueue;
	processor.addPipelineStage(pipeline_info);
	pipeline_info.stage_name = "DecodeStage";
	pipeline_info.stage_short_name = "ID";
	pipeline_info.stage_category = PipelineStage::DECODE;
	pipeline_info.order_policy = PipelineStage::IN_ORDER;
	pipeline_info.stage_width = 2;
	pipeline_info.min_latency = 1;
	pipeline_info.max_latency = 1;
	pipeline_info.source_queue = fetchQueue;
	pipeline_info.destination_queue = ROB;
	processor.addPipelineStage(pipeline_info);
	pipeline_info.stage_name = "ExecutionStage";
	pipeline_info.stage_short_name = "EX";
	pipeline_info.stage_category = PipelineStage::EXECUTE;
	pipeline_info.order_policy = PipelineStage::OUT_OF_ORDER;
	pipeline_info.stage_width = 2;
	pipeline_info.min_latency = 1;
	pipeline_info.max_latency = 1;
	pipeline_info.source_queue = NULL;
	pipeline_info.destination_queue = NULL;
	PipelineStage *execute_stage = processor.addPipelineStage(pipeline_info);
	processor.setOperandReadingStage(execute_stage);
	processor.setOperandProducingStage(execute_stage);
	PipelineStage::FunctionalUnit::fu_info_t fu_info = {
		"ALU",
		"ALU",
		false,
		1,
		1,
		2 };
	PipelineStage::FunctionalUnit *alu = execute_stage->addFunctionalUnit(fu_info);
//	execute_stage->bindFunctionalUnit(alu,PipelineStage::IALU);
//	execute_stage->bindFunctionalUnit(alu,PipelineStage::FALU);
	execute_stage->bindFunctionalUnit(alu,IALU);
	execute_stage->bindFunctionalUnit(alu,FALU);
	execute_stage->bindFunctionalUnit(alu,CONTROL);
	fu_info.name = "MemUnit";
	fu_info.short_name = "MEM";
	fu_info.is_pipelined = true;
	fu_info.min_latency = 2;
	fu_info.max_latency = 10;
	fu_info.width = 1;
	PipelineStage::FunctionalUnit *memunit = execute_stage->addFunctionalUnit(fu_info);
//	execute_stage->bindFunctionalUnit(memunit,PipelineStage::MEMORY);
	execute_stage->bindFunctionalUnit(memunit,MEMORY);
	pipeline_info.stage_name = "WriteBackStage";
	pipeline_info.stage_short_name = "WB";
	pipeline_info.stage_category = PipelineStage::WRITE;
	pipeline_info.order_policy = PipelineStage::OUT_OF_ORDER;
	pipeline_info.stage_width = 2;
	pipeline_info.min_latency = 1;
	pipeline_info.max_latency = 1;
	pipeline_info.source_queue = NULL;
	pipeline_info.destination_queue = NULL;
	processor.addPipelineStage(pipeline_info);
	pipeline_info.stage_name = "CommitStage";
	pipeline_info.stage_short_name = "CM";
	pipeline_info.stage_category = PipelineStage::COMMIT;
	pipeline_info.order_policy = PipelineStage::IN_ORDER;
	pipeline_info.stage_width = 2;
	pipeline_info.min_latency = 1;
	pipeline_info.max_latency = 1;
	pipeline_info.source_queue = ROB;
	pipeline_info.destination_queue = NULL;
	processor.addPipelineStage(pipeline_info);
	processor.dump(logFile);
	
	Manager manager;
	PropList props;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
	
	try {
		
		// Load program
		if(argc < 2) {
			cerr << "ERROR: no argument.\n"
				 << "Syntax is : exegraph <executable>\n";
			exit(2);
		}
		FrameWork *fw = manager.load(argv[1], props);
		
		// Find main CFG
		cout << "Looking for the main CFG\n";
		CFG *cfg = fw->getCFGInfo()->findCFG("main");
		if(cfg == 0) {
			cerr << "ERROR: cannot find main !\n";
			return 1;
		}
		else
			cout << "main found at 0x" << cfg->address() << '\n';
		
		// Removing __eabi call if available
		for(CFG::BBIterator bb(cfg); bb; bb++)
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == otawa::Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					break;
				}
		
		// Now, use a VCFG
		VirtualCFG vcfg(cfg);
		ENTRY_CFG(fw) = &vcfg;
		
		// Prepare processor configuration
		PropList props;
		props.set(EXPLICIT, true);
		
		// Compute BB times
		//cout << "Timing the BB\n";
		//TrivialBBTime tbt(5, props);
		//tbt.processCFG(fw, &vcfg);
		
		// Compute BB times with execution graphs
		cout << "Timing the BB with execution graphs\n";
		logFile << "\n----------------------------------------------\n";
		logFile << "Timing the BB with execution graphs\n";
		logFile << "----------------------------------------------\n";
//		logFile << "CFG: \n";
//		for(CFG::BBIterator bb(cfg); bb; bb++) {
//			logFile << "block b" << bb->number() << "\n";
//			logFile << "\tpredecessors: ";
//			for(BasicBlock::InIterator edgein(bb); edgein; edgein++) {
//				logFile << "b" << edgein->source()->number() << ", ";
//			}
//			logFile << "\n\tsuccessors: ";
//			for(BasicBlock::OutIterator edgeout(bb); edgeout; edgeout++) {
//				logFile << "b" << edgeout->target()->number() << ", ";
//			}
//			logFile << "\n";
//		}
		
		ExeGraphBBTime tbt(props, &processor, logFile);
		tbt.process(fw);
		
		
		// Assign variables
		cout << "Numbering the main\n";
		VarAssignment assign(props);
		assign.process(fw);
		
		// Build the system
		cout << "Building the ILP system\n";
		BasicConstraintsBuilder builder(props);
		builder.process(fw);
		
		// Build the object function to maximize
		cout << "Building the ILP object function\n";
		BasicObjectFunctionBuilder fun_builder(props);
		fun_builder.process(fw);
		
		// Load flow facts
		cout << "Loading flow facts\n";
		ipet::FlowFactLoader loader(props);
		loader.process(fw);
		
		// Resolve the system
		cout << "Resolve the system\n";
		WCETComputation wcomp(props);
		wcomp.process(fw);
		
		// Display the result
		ilp::System *sys = vcfg.use<ilp::System *>(SYSTEM);
		sys->dump();
		cout << sys->countVars() << " variables and "
			 << sys->countConstraints() << " constraints.\n";
		cout << "SUCCESS\nWCET = " << vcfg.use<int>(WCET) << '\n';
	}
	catch(elm::Exception e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

