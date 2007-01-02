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
#include <otawa/sim/BasicBlockDriver.h>
#include <ARMSimulator.h>

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
	
	elm::io::OutFileStream logStream("/home/rochange/ECLIPSE/otawa/otawa/test/simulator/log");
	elm::io::Output logFile(logStream);
	
	sc_core::sc_elab_and_sim(argc, argv);
	
	
	Manager manager;
	PropList props;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
	
	
	try {
		
		// Load program
		if(argc < 2) {
			elm::cerr << "ERROR: no argument.\n"
				 << "Syntax is : exegraph <executable>\n";
			exit(2);
		}
		FrameWork *fw = manager.load(argv[1], props);
		
		
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
		/*PropList props;
		props.set(IPET::ID_Explicit, true);*/
		
		ARMSimulator simulator;
		ARMState *simulator_state = (ARMState *) simulator.instantiate(fw,props);
		simulator_state->init();
		
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
				logFile << "block b" << bb->number() << ":";
				int start_cycle = simulator_state->cycle();
				//simulator_state->setPC(bb->address());
				//simulator_state->runUntil(bb->address() + 4*bb->size());
				sim::BasicBlockDriver driver(bb);
				simulator_state->run(driver);
				int finish_cycle = simulator_state->cycle();
				logFile << finish_cycle - start_cycle << " cycles\n";
			}
		}
		
//		ExeGraphBBTime tbt(props, &processor, logFile);
//		tbt.processCFG(fw, &vcfg);
		
	}
	catch(LoadException e) {
//		elm::cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	catch(ProcessorException e) {
//		elm::cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;

}

