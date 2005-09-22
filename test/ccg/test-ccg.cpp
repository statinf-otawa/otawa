#include <stdlib.h>
#include <elm/io.h>
#include <otawa/ilp.h>
#include <otawa/otawa.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/TrivialBBTime.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/ipet/BasicConstraintsBuilder.h>
#include <otawa/ipet/WCETComputation.h>
#include <otawa/cache/ccg/CCGConstraintBuilder.h>
#include <otawa/cache/ccg/CCGBuilder.h>
#include <otawa/cache/ccg/CCGObjectFunction.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/hardware/Cache.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/hardware/CacheConfiguration.h>

using namespace otawa;
using namespace elm;
using namespace otawa::ipet;

int main(int argc, char **argv) {
	  // Construction de la hiérarchie de cache
 		Cache::info_t info;
 		info.block_bits = 3;  // 2^3 octets par bloc
 		info.line_bits = 3;   // 2^3 lignes
 		info.set_bits = 0;    // 2^0 élément par ensemble (cache direct)
 		info.replace = Cache::NONE;
 		info.write = Cache::WRITE_THROUGH;
 		info.access_time = 0;
 		info.miss_penalty = 10;
 		info.allocate = false;
 		Cache *level1 = new Cache(info);
 		CacheConfiguration *caches = new CacheConfiguration(level1);
				
			
		Manager manager;
		PropList props;
		props.set<const CacheConfiguration *>(Platform::ID_Cache, caches);
		props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
	try {
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
			for(Iterator<Edge *> edge(bb->outEdges()); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					break;
				}
		
		// Now, use a VCFG
		VirtualCFG vcfg(cfg);
		
				
		// assigne variable to CFG
		cout << "Numbering the main\n";
		VarAssignment assign;
		assign.processCFG(fw, &vcfg);
		
		// Build the system
		cout << "Building the ILP system\n";
		BasicConstraintsBuilder builder;
		builder.processCFG(fw, &vcfg);
		
		// Get external constraints
		cout << "Loading external constraints\n";
		ipet::FlowFactLoader ffl;
		ffl.processCFG(fw, &vcfg);

		// Build the CCG
		cout << "Building the CCG Contraints\n";
			
		// build ccg graph
		CCGBuilder ccgbuilder(fw);
		ccgbuilder.processCFG(fw, &vcfg );
			
		// Build ccg contraint
		CCGConstraintBuilder decomp(fw);
		decomp.processCFG(fw, &vcfg );
			
		//Build the objectfunction
		CCGObjectFunction ofunction(fw);
		ofunction.processCFG(fw, &vcfg );
		
		// Resolve the system
		cout << "Resolve the system\n";
		WCETComputation wcomp;
		wcomp.processCFG(fw, &vcfg);
		
		// Display the result
		ilp::System *sys = vcfg.use<ilp::System *>(IPET::ID_System);
		vcfg.use<ilp::System *>(IPET::ID_System)->dump();
		cout << sys->countVars() << " variables and "
			 << sys->countConstraints() << " constraints.\n";
		cout << "SUCCESS\nWCET = " << vcfg.use<int>(IPET::ID_WCET) << '\n';
		cout << "dernier version";
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}
