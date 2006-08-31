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
#include <otawa/cache/ccg/CCGObjectFunction.h>
#include <otawa/cache/categorisation/CATConstraintBuilder.h>
#include <otawa/cache/categorisation/CATBuilder.h>
#include <otawa/ipet/BBTimeSimulator.h>
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

	// Cache configuration	
	Cache::info_t inst_cache_info = {
		1,
		10,
		4,
		6,
		0,
		Cache::LRU,
		Cache::WRITE_THROUGH,
		false
	};
	Cache::info_t data_cache_info = {
		1,
		10,
		4,
		6,
		2,
		Cache::LRU,
		Cache::WRITE_THROUGH,
		false
	};
	Cache inst_cache(inst_cache_info);
	Cache data_cache(data_cache_info);
	CacheConfiguration cache_conf(&inst_cache, &data_cache);
	
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
	Manager manager;
	PropList props;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
	props.set<CacheConfiguration *>(Platform::ID_Cache, &cache_conf);
	try {
		FrameWork *fw = manager.load(file, props);
		
		// Find main CFG
		CFG *cfg = fw->getCFGInfo()->findCFG("main");
		if(cfg == 0) {
			cerr << "ERROR: cannot find main !\n";
			return 1;
		}
		
		// Removing __eabi call if available
		for(CFG::BBIterator bb(cfg); bb; bb++)
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					break;
				}
		
		// Now, use an inlined VCFG
		VirtualCFG vcfg(cfg);
		
		// Prepare processor configuration
		PropList props;
		props.set(EXPLICIT, true);
		
		// Compute BB times
		BBTimeSimulator bbts(props);
		bbts.processCFG(fw, &vcfg);
		
		// Trivial data cache
		TrivialDataCacheManager dcache(props);
		dcache.processCFG(fw, &vcfg);
		
		// Assign variables
		VarAssignment assign(props);
		assign.processCFG(fw, &vcfg);
		
		// Build the system
		BasicConstraintsBuilder builder(props);
		builder.processCFG(fw, &vcfg);
		
		// Process the instruction cache
		if(method == CCG) {
			
			// build ccg graph
			CCGBuilder ccgbuilder;
			ccgbuilder.processCFG(fw, &vcfg );
			
			// Build ccg contraint
			CCGConstraintBuilder decomp(fw);
			decomp.processCFG(fw, &vcfg );
			
			//Build the objectfunction
			CCGObjectFunction ofunction(fw);
			ofunction.processCFG(fw, &vcfg );
		}
		else {
			if(method == CAT) {
				
				// build Cat lblocks
				CATBuilder catbuilder;
				catbuilder.processCFG(fw, &vcfg);
			
				// Build CAT contraint
				CATConstraintBuilder decomp;
				decomp.processCFG(fw, &vcfg);
			}
			
			// Build the object function to maximize
			BasicObjectFunctionBuilder fun_builder;
			fun_builder.processCFG(fw, &vcfg);	

		}

		// Load flow facts
		ipet::FlowFactLoader loader(props);
		loader.processCFG(fw, &vcfg);
		
		// Calculate deltas
		//cout << "Computing deltas... ";
		elm::system::StopWatch delta_sw;
		delta_sw.start();
		//props.set<int>(Delta::ID_Levels,deltaLevels);
		if(deltaLevels)
			Delta::LEVELS(props) = *deltaLevels;
		Delta delta(props);
		delta.processCFG(fw, &vcfg);
		delta_sw.stop();
		//cout << "OK in " << delta_sw.delay()/1000 << " ms\n";
		if(infos){
			cout << "CFG have " << vcfg.bbs().count() << " nodes\n";
			cout << BBPath::instructions_simulated << " instructions have been simulated\n";
			cout << "The longer path have " << delta.max_length << " basic blocks\n";
			cout << "The average length of paths is " << delta.mean_length << '\n';
		}
		
		// Resolve the system
		elm::system::StopWatch ilp_sw;
		ilp_sw.start();
		WCETComputation wcomp(props);
		wcomp.processCFG(fw, &vcfg);
		ilp_sw.stop();
		main_sw.stop();

		// Find the code size
		int size = 0;
		for(Iterator<File *> ffile(*fw->files()); ffile; ffile++)
			for(Iterator<Segment *> seg(ffile->segments()); seg; seg++)
				if(seg->flags() & Segment::EXECUTABLE)
					size += seg->size();
		
		// Get the result
		ilp::System *sys = vcfg.use<ilp::System *>(SYSTEM);
		
		if(print){
			PropList display_props;
			display::GRAPHVIZ_FILE(display_props) = "cfg.ps";
			display::EXCLUDE(display_props) += &VirtualCFG::ID_CalledCFG;
			display::DeltaCFGDrawer drawer(&vcfg, display_props);
			drawer.display();
		}
		
		if(dump)
			sys->dump();
		cout << file << '\t'
			 << sys->countVars() << '\t'
			 << sys->countConstraints() << '\t'
			 << (int)(main_sw.delay() / 1000) << '\t'
			 << (int)(ilp_sw.delay() / 1000) << '\t'
			 << vcfg.use<int>(WCET) << '\t'
			 << size << '\n';
	}
	catch(LoadException e) {
		elm::cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	catch(ProcessorException e) {
		elm::cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

