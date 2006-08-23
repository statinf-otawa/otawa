/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test/ipet/exegraph.cpp -- test for Execution Graph modeling feature.
 */

#include <systemc>
#include <stdlib.h>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/ilp.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/sim/BasicBlockDriver.h>


using namespace elm;
using namespace elm::io;
using namespace otawa;
using namespace otawa::ipet;
using namespace std;
using namespace sc_core;
using namespace sc_dt;

// void sc_main
int sc_main(int argc, char *argv[]) {
	return 0;
}



int main(int argc, char **argv) {
	
	try {
		
	elm::io::OutFileStream logStream("log.txt");
	if(!logStream.isReady())
		throw IOException("cannot open log file !");
	elm::io::Output logFile(logStream);
	
	
	
	Manager manager;
	PropList loader_props;
	loader_props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
		
		// Load program
		if(argc < 2) {
			elm::cerr << "ERROR: no argument.\n"
				 << "Syntax is : exegraph <executable>\n";
			exit(2);
		}
		FrameWork *fw = manager.load(argv[1], loader_props);
		
		
		// Find main CFG
		elm::cout << "Looking for the main CFG\n";
		CFG *cfg = fw->getCFGInfo()->findCFG("main");
		if(cfg == 0) {
			elm::cerr << "ERROR: cannot find main !\n";
			return 1;
		}
		else
			elm::cout << "main found at 0x" << cfg->address() << '\n';
		
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
		
		// Prepare processor configuration
		PropList props;
		props.set(EXPLICIT, true);
		
		GenericSimulator simulator;
		GenericState *simulator_state = (GenericState *) simulator.instantiate(fw,props);
		simulator_state->init();
		sc_core::sc_elab_and_sim(argc, argv);
		
		
		// Compute BB times
		//elm::cout << "Timing the BB\n";
		//TrivialBBTime tbt(5, props);
		//tbt.processCFG(fw, &vcfg);
		
		// Compute BB times with execution graphs
		logFile << "\n----------------------------------------------\n";
		logFile << "Timing the BB with execution graphs\n";
		logFile << "----------------------------------------------\n";
		logFile << "CFG: \n";
		for(CFG::BBIterator bb(&vcfg); bb; bb++) {
			if (bb->countInstructions() != 0) {
				elm::cout << "**** GOING TO MEASURE BLOCK " << bb->number() << "(at " << bb->address() << ")\n\t";
				for (BasicBlock::InstIterator inst(bb) ; inst ; inst++) {
					elm::cout << inst->address() << ", ";
				}
				elm::cout << "\n\tuntil: " << bb->address() << " + (" << bb->size() << "-4)" << " = " << bb->address() + (bb->size()-4);
				elm::cout << "\n\n";
				
				int start_cycle = simulator_state->cycle();
				sim::BasicBlockDriver driver(bb);
				simulator_state->run(driver);
				int finish_cycle = simulator_state->cycle();
				logFile << "block b" << bb->number() << ":";
				logFile << finish_cycle - start_cycle << " cycles\n";
			}
		}
		
//		ExeGraphBBTime tbt(props, &processor, logFile);
//		tbt.processCFG(fw, &vcfg);
		
	}
	catch(elm::Exception& e) {
		elm::cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;

}

