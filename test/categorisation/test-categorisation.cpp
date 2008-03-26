#include <stdlib.h>
#include <elm/io.h>
#include <otawa/ilp.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/cache/categorisation/CATConstraintBuilder.h>
#include <otawa/cache/categorisation/CATBuilder.h>
#include <otawa/cache/LBlockSet.h>


using namespace otawa;
using namespace elm;
using namespace otawa::ipet;
using namespace otawa::hard;

int main(int argc, char **argv) {
	
	Manager manager;
	PropList props;
	CACHE_CONFIG_PATH(props) = "icache.xml";
	try {
		WorkSpace *fw = manager.load(argv[1], props);
		
		// Find main CFG
		CFG *cfg = fw->getCFGInfo()->findCFG("main");
		if(cfg == 0) {
			cerr << "ERROR: cannot find main !\n";
			return 1;
		}
		
		// Removing __eabi call if available
		for(CFG::BBIterator bb(cfg); bb; bb++)
			for(Iterator<Edge *> edge(bb->outEdges()); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					break;
				}
		
		// Processors configuration
		//PROC_VERBOSE(props) = true;
		//EXPLICIT(props) = true;
		
		// Get external constraints
		ipet::FlowFactLoader ffl;
		ffl.process(fw, props);
			
		// Build CAT contraint
		CATConstraintBuilder decomp;
		decomp.process(fw, props);
					
		// Resolve the system
		WCETComputation wcomp;
		wcomp.process(fw, props);
		
		// Display the result
		//ilp::System *sys = SYSTEM(fw);
		/*sys->dump();
		cout << sys->countVars() << " variables and "
			 << sys->countConstraints() << " constraints.\n";*/
		cout << "WCET = " << WCET(fw) << '\n';
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

