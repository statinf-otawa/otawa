/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test/ipet/exegraph.cpp -- test for Execution Graph modeling feature.
 */

#include <cstdlib>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/gensim/GenericSimulator.h>
#include "FullSimulationDriver.h"


using namespace elm;
using namespace elm::io;
using namespace otawa;
using namespace otawa::ipet;
using namespace std;


int main(int argc, char **argv, char **envp) {
	
	try {
		
		// Prepare the log output
		elm::io::OutFileStream logStream("log.txt");
		if(!logStream.isReady())
			throw IOException("cannot open log file !");
		elm::io::Output logFile(logStream);
	
		// Check arguments
		if(argc < 2) {
			elm::cerr << "ERROR: no argument.\n"
				 << "Syntax is : " << argv[0] << " <executable>\n";
			exit(2);
		}
			
		// Load the file
		Manager manager;
		PropList loader_props;
		char **new_argv = new char *[argc];
		for(int i = 1; i < argc; i++)
			new_argv[i - 1] = argv[i];
		new_argv[argc - 1] = 0;
		ARGC(loader_props) = argc - 1;
		ARGV(loader_props) = new_argv;
		ENVP(loader_props) = envp;
		//TODO: move this parameter on command line
		elm::system::Path xml_processor_file("../../examples/piconsens/deg4.xml");
		PROCESSOR_PATH(loader_props) = xml_processor_file;
		WorkSpace *ws = manager.load(argv[1], loader_props);
		
		// Find _start CFG
		elm::cout << "Looking for the _start CFG\n";
		CFG *cfg = ws->getCFGInfo()->findCFG("_start");
		if(cfg == 0) {
			elm::cerr << "ERROR: cannot find _start !\n";
			return 1;
		}
		else
			elm::cout << "_start found at 0x" << cfg->address() << '\n';
		otawa::address_t addr_start = cfg->address();
		
		// Prepare processor configuration
		PropList props;
		EXPLICIT(props) = true;
		
		// instantiate generic simulator
		gensim::GenericSimulator simulator;
		sim::State *simulator_state = simulator.instantiate(ws, props);

		int start_cycle = simulator_state->cycle();
		// create the driver which will go through all the program to simulate
		sim::FullSimulationDriver driver(ws, ws->findInstAt(addr_start));
		// start and run the simulation until its end
		simulator_state->run(driver);
		int finish_cycle = simulator_state->cycle();
		logFile << "cycle number: " << finish_cycle - start_cycle << "\n";

		
	}
	catch(elm::Exception& e) {
		elm::cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;

}

