/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	test/delta/test_delta.cpp -- test for DELTA feature.
 */

#include <stdlib.h>
#include <stdio.h>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/ilp.h>
#include <elm/system/StopWatch.h>
#include <otawa/cache/ccg/CCGConstraintBuilder.h>
#include <otawa/cache/ccg/CCGBuilder.h>
#include <otawa/cache/categorisation/CATConstraintBuilder.h>
#include <otawa/cache/categorisation/CATBuilder.h>
#include <otawa/ipet/BBTimeSimulator.h>
#include <otawa/gensim/GenericSimulator.h>
#include "DeltaCFGDrawer.h"


using namespace elm;
using namespace otawa;
using namespace otawa::ipet;
using namespace otawa::hard;


// Option string
CString ccg_option = "-ccg";
CString cat_option = "-cat";
CString dump_option = "-dump";
CString infos_option = "-infos";
CString print_option = "-print";
CString verbose_option = "-v";

/**
 * Display help message and exit the program.
 */
void help(void) {
	cerr << "ERROR: bad arguments.\n";
	cerr << "SYNTAX: test_delta [-ccg|-cat] [-dump] program\n";
	cerr << "    -dump  dumps to the standard output the ILP system\n";
	cerr << "    -infos prints some informations\n";
	cerr << "    -print prints the CFG to cfg.ps\n";
	cerr << "    -D#    forces the depth of the delta algorithm\n";
	cerr << "    -ccg   uses ccg method\n";
	cerr << "    -cat   uses categorisation methos\n";
	cerr << "    -v     verbose display\n";
	cerr << "  If neither -ccg nor -cat is specified, the program does NOT perform\n";
	cerr << "  the Instruction Cache Processor\n";
	exit(1);
}

/**
 * Program entry point.
 * @return Error code.
 */
int main(int argc, char **argv) {
	// Options
	CString file;
	enum {
		NONE = 0,
		CCG,
		CAT
	} method = NONE; //CCG;
	
	elm::Option<int> deltaLevels;
	bool dump = false;
	bool infos = false;
	bool print = false;
	bool verbose = false;

	// Processing the arguments
	for(int i = 1; i < argc; i++) {
		if(argv[i][0] != '-')
			file = argv[i];
		else if(ccg_option == argv[i])
			method = CCG;
		else if(cat_option == argv[i])
			method = CAT;
		else if(dump_option == argv[i])
			dump = true;
		else if(infos_option == argv[i])
			infos = true;
		else if(print_option == argv[i])
			print = true;
		else if(verbose_option == argv[i])
			verbose = true;
		else if(argv[i][0] == '-' && argv[i][1] == 'D'){
			int d;
			sscanf(&argv[i][2],"%d",&d);
			deltaLevels = d;
		}
		else
			help();
	}
	if(!file)
		help();

	// Start timer
	elm::system::StopWatch main_sw;
	main_sw.start();
	
	// Load the file
	gensim::GenericSimulator sim;
	Manager manager;
	PropList props;
	PROCESSOR_PATH(props) = "../../data/procs/op1.xml";
	SIMULATOR(props) = &gensim_simulator;
	CACHE_CONFIG_PATH(props) = "../ccg/icache.xml";	
	PropList stats;
	
	//cout << "config = " << props << io::endl;
	try {
		WorkSpace *fw = manager.load(file, props);
		
		// Find main CFG
		CFG *cfg = fw->getCFGInfo()->findCFG("main");
		if(cfg == 0) {
			cerr << "ERROR: cannot find main !\n";
			return 1;
		}
		
		//cout << "fw = " << fw << io::endl;
		//cout << "process = " << fw->process() << io::endl;
		
		// Removing __eabi call if available
		for(CFG::BBIterator bb(cfg); bb; bb++)
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					break;
				}
		
		// Prepare processor configuration
		//PropList props;
		//EXPLICIT(props) = true;
		if(verbose)
			otawa::Processor::VERBOSE(props) = true;
		VirtualCFG vcfg(cfg);
		ENTRY_CFG(props) = &vcfg;
		
		// Calculate deltas
		//cout << "Computing deltas... ";
		elm::system::StopWatch delta_sw;
		delta_sw.start();
		if(deltaLevels)
			Delta::LEVELS(props) = *deltaLevels;
		Delta delta;
		otawa::Processor::STATS(props) = &stats;
		delta.process(fw, props);
		delta_sw.stop();
		//cout << "OK in " << delta_sw.delay()/1000 << " ms\n";
		/*if(infos){
			cout << "CFG have " << vcfg.bbs().count() << " nodes\n";
			cout << BBPath::instructions_simulated << " instructions have been simulated\n";
			cout << "The longer path have " << Delta::MAX_LENGTH(stats) << " basic blocks\n";
			cout << "The average length of paths is " << Delta::MEAN_LENGTH(stats) << '\n';
		}*/
		
		// Trivial data cache
		TrivialDataCacheManager dcache;
		dcache.process(fw, props);
		
		// Process the instruction cache
		if(method == CCG) {
			
			// build ccg graph
			CCGBuilder ccgbuilder;
			ccgbuilder.process(fw, props);
			
			// Build ccg contraint
			CCGConstraintBuilder decomp;
			decomp.process(fw, props);
		}
		else if(method == CAT) {
				
			// build Cat lblocks
			CATBuilder catbuilder;
			catbuilder.process(fw, props);
			
			// Build CAT contraint
			CATConstraintBuilder decomp;
			decomp.process(fw, props);
		}

		// Load flow facts
		ipet::FlowFactLoader loader;
		loader.process(fw, props);
		
		// Resolve the system
		elm::system::StopWatch ilp_sw;
		ilp_sw.start();
		WCETComputation wcomp;
		wcomp.process(fw, props);
		ilp_sw.stop();
		main_sw.stop();
		WCETCountRecorder recorder;
		recorder.process(fw, props);

		// Find the code size
		int size = 0;
		for(Process::FileIter ffile(fw->process()); ffile; ffile++)
			for(File::SegIter seg(ffile); seg; seg++)
				if(seg->flags() & Segment::EXECUTABLE)
					size += seg->size();
		
		// Get the result
		ilp::System *sys = SYSTEM(fw);
		
		if(print){
			PropList display_props;
			display::OUTPUT_PATH(display_props) = "cfg.ps";
			
			display::INCLUDE(display_props).add(&TIME);
			display::INCLUDE(display_props).add(&Delta::DELTA);
			display::INCLUDE(display_props).add(&ipet::LOOP_COUNT);
			display::INCLUDE(display_props).add(&ipet::COUNT);
			
			display::EXCLUDE(display_props).add(&CALLED_CFG);
			
			display::DEFAULT(display_props) = &display::EXCLUDE;
			
			display::DeltaCFGDrawer drawer(&vcfg, display_props);
			drawer.display();
		}
		
		if(dump)
			sys->dump();
		cout << "WCET[" << file << "] = " << WCET(fw) << io::endl;
	}
	catch(LoadException& e) {
		elm::cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	catch(elm::Exception& e) {
		elm::cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

