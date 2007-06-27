#include <stdlib.h>
#include <otawa/otawa.h>
#include <otawa/prog/Loader.h>
#include <otawa/util/Dominance.h>
#include <elm/io.h>
#include <otawa/display/CFGDrawer.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/ipet.h>
#include <otawa/ipet/BBTimeSimulator.h>
#include <otawa/gensim/GenericSimulator.h>

using namespace elm;
using namespace otawa;
using namespace otawa::ipet;


void usage_error(){
	cerr << "error\n";
	cerr << "Usage: test_display file\n";
	exit(1);
}



int main(int argc, char **argv){
	CString file = argv[1];
	if(argc < 2){
		usage_error();
	}
	
	Manager manager;
	try{
		PropList props;
//		LOADER(props) = &Loader::LOADER_Gliss_PowerPC;
		PROCESSOR_PATH(props) = "../../data/procs/op1.xml";
		SIMULATOR(props) = &gensim_simulator;
		ipet::EXPLICIT(props) = true;
		WorkSpace *fw = manager.load(file, props);
		
		// Find main
		CFG *cfg = fw->getCFGInfo()->findCFG("main");
		if(cfg == 0) {
			cerr << "ERROR: cannot find main !\n";
			exit(3);
		}
		// Removing __eabi call if available
        for(CFG::BBIterator bb(cfg); bb; bb++)
                        for(BasicBlock::OutIterator edge(bb); edge; edge++)
                                if(edge->kind() == Edge::CALL
                                && edge->target()
                                && edge->calledCFG()->label() == "__eabi") {
                                        delete(*edge);
                                        break;
                                }		CFG *vcfg = new VirtualCFG(cfg);
		ENTRY_CFG(props) = vcfg;
		
		Dominance dom;
		dom.process(fw, props);
		VarAssignment vars;
		vars.process(fw, props);
		BBTimeSimulator bbts;
        bbts.process(fw, props);
		
		props.clearProps();
		/*display::EXCLUDE(props).add(&CALLED_CFG);
		display::EXCLUDE(props).add(&REVERSE_DOM);*/
		display::CFGDrawer drawer(vcfg, props);
		// DEFAULT(drawer.all()) = INCLUDE;
		// DEFAULT(drawer.graph()) = EXCELUDE;
		drawer.display();
	}
	catch(elm::Exception& e){
		cerr << "ERROR: " << e.message() << '\n';
		exit(2);
	}
}
