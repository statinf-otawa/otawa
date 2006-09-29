#include <stdlib.h>
#include <otawa/otawa.h>
#include <otawa/prog/Loader.h>
#include <otawa/util/Dominance.h>
#include <elm/io.h>
#include <otawa/display/CFGDrawer.h>


using namespace elm;
using namespace otawa;


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
		LOADER(props) = &Loader::LOADER_Gliss_PowerPC;
		FrameWork *fw = manager.load(file, props);
		
		// Find main
		CFG *cfg = fw->getCFGInfo()->findCFG("main");
		if(cfg == 0) {
			cerr << "ERROR: cannot find main !\n";
			exit(3);
		}
		
		cfg = new VirtualCFG(cfg);
		Dominance::ensure(cfg);
		Dominance::markLoopHeaders(cfg);

		props.clearProps();
		display::EXCLUDE(props) += &CALLED_CFG;
		display::EXCLUDE(props) += &Dominance::ID_RevDom;
		display::CFGDrawer drawer(cfg, props);
		// DEFAULT(drawer.all()) = INCLUDE;
		// DEFAULT(drawer.graph()) = EXCELUDE;
		drawer.display();
	}
	catch(LoadException e){
		cerr << "ERROR: " << e.message() << '\n';
		exit(2);
	}
}
