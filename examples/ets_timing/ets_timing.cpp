/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	examples/ets_timing/ets_timing.cpp -- ETS example measuring the computation time.
 */

#include <elm/debug.h>
#include <otawa/ets.h>
#include <otawa/ast.h>
#include <otawa/hard/Cache.h>
#include <otawa/ast/ASTLoader.h>
#include <otawa/proc/ProcessorException.h>
#include <elm/system/StopWatch.h>

using namespace otawa;
using namespace elm::io;
using namespace otawa::ets;
using namespace otawa::hard;

int main(int argc, char **argv) {
	
	// Instruction cache configuration
	Cache::info_t info;
 	info.block_bits = 3;  // 2^3 octets par bloc
 	info.line_bits = 2;   // 2^3 lignes
 	info.set_bits = 0;    // 2^0 �l�ment par ensemble 
 	info.replace = Cache::NONE;
 	info.write = Cache::WRITE_THROUGH;
 	info.access_time = 0;
 	info.miss_penalty = 10;
 	info.allocate = false;
 	Cache *level1 = new Cache(info);
 	CacheConfiguration *caches = new CacheConfiguration(level1, 0);
 
 	// Configuration
 	Manager manager;
	PropList props;
	FrameWork *fw;
	CACHE_CONFIG(props) = caches;
//	LOADER(props) = &Loader::LOADER_Gliss_PowerPC;
	
	try { 

		// Start timer
		elm::system::StopWatch main_sw;
		main_sw.start();
		
		// Loading the file
		fw = manager.load(argv[1], props);
		
		// Loading the AST
		ASTLoader loader;
		loader.processFrameWork(fw);
		ASTInfo *info = fw->getASTInfo();
		
		// Find main AST
		Option< FunAST *> result = info->map().get("main");
		if(!result) {
			TRACE;
			throw IOException("Cannot find main !");
		}
		AST *ast = (*result)->ast();
		ets::WCET(ast) = -1;
		
		// Compute each BB times
		TrivialAstBlockTime tabt(5);
		tabt.processAST(fw, ast);
		
		// assignment for each loop
		ets::FlowFactLoader ffl(fw);
		ffl.processAST(fw, ast);
		
		// compute wcet
		WCETComputation wcomp;
		wcomp.processAST(fw, ast);
		
		// simulate cache
		ACSComputation ac(fw);
		ac.processAST(fw, ast);
		
		// compute first miss
		CacheFirstMissComputation cfmc;
		cfmc.processAST(fw, ast);
		
		// compute hit
		CacheHitComputation chc;
		chc.processAST(fw, ast);
		
		// compute miss
		CacheMissComputation cmc;
		cmc.processAST(fw, ast);
		
		// Output result
		main_sw.stop();
		cout << argv[1] << '\t'
			 << ets::WCET(ast) << '\t'
			 << ets::MISSES(ast) << '\t'
			 << (int)(main_sw.delay() / 1000) << '\n';
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
	}
	catch(IOException e){
		cerr << "ERROR : " << e.message() << '\n';
	}
	catch(ProcessorException e){
		cerr << "ERROR : " << e.message() << '\n';
	}
	return 0;
}
