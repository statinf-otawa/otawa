#include <stdlib.h>
#include <elm/io.h>
#include <otawa/ilp.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/cache/categorisation/CATConstraintBuilder.h>
#include <otawa/cache/categorisation/CATBuilder.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/hard/Cache.h>
#include <otawa/hard/CacheConfiguration.h>


using namespace otawa;
using namespace elm;
using namespace otawa::ipet;
using namespace otawa::hard;

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
	CACHE_CONFIG(props) = caches;
//	LOADER(props) = &Loader::LOADER_Gliss_PowerPC;
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
		ENTRY_CFG(fw) = &vcfg;
		
		// Compute BB times
		cout << "Timing the BB\n";
		TrivialBBTime tbt;
		ipet::PIPELINE_DEPTH(props) = 5;
		tbt.process(fw, props);
				
		// assigne variable to CFG
		cout << "Numbering the main\n";
		VarAssignment assign;
		assign.process(fw);
		
		// Build the system
		cout << "Building the ILP system\n";
		BasicConstraintsBuilder builder;
		builder.process(fw);
		
		// Build the object function to maximize
		cout << "Building the ILP object function\n";
		BasicObjectFunctionBuilder fun_builder;
		fun_builder.process(fw);
		
		// Get external constraints
		cout << "Loading external constraints\n";
		ipet::FlowFactLoader ffl;
		ffl.process(fw);
		// Build the CCG
		/*cout << "Building the Categories Contraints\n";
		for (int i=0; i < level1->lineCount(); i++){
			cout << "construire les lblocks pour la ligne"<<i <<"\n";	
			LBlockSet *idccg = new LBlockSet();
			vcfg.addDeletable<LBlockSet *>(LBlockSet::ID_LBlockSet, idccg);*/
			//CCGDFA dfa(idccg,cfg,  3);
			
			// build Cat lblocks
			CATBuilder catbuilder;
			catbuilder.process(fw);
			
			// Build CAT contraint
			CATConstraintBuilder decomp;
			decomp.process(fw);
					
		///}
		// Resolve the system
		cout << "Resolve the system\n";
		WCETComputation wcomp;
		wcomp.process(fw);
		
		// Display the result
		ilp::System *sys = vcfg.use<ilp::System *>(SYSTEM);
		vcfg.use<ilp::System *>(SYSTEM)->dump();
		cout << sys->countVars() << " variables and "
			 << sys->countConstraints() << " constraints.\n";
		cout << "SUCCESS\nWCET = " << vcfg.use<int>(WCET) << '\n';
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

