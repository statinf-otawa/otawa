/*
 *	$Id$
 *	Copyright (c) 2008, IRIT UPS.
 *
 *	Test for PFG.
 */

#include <otawa/otawa.h>
#include <otawa/cfg/features.h>
#include <elm/options.h>
#include <otawa/cfg/CFGBuilder.h>

using namespace elm;
using namespace elm::option;
using namespace otawa;

class Command: public option::Manager {
public:
	
	Command(void): ws(0) {
		description = "CFG BB outputter";
		free_argument_description = "program_to_process";
	}
	
	~Command(void) {
		if(ws)
			delete ws;			// build the PFG
	}
	
	int run(int argc, char **argv) {
		try {
			
			// open the file
			parse(argc, argv);
			if(!file)
				throw OptionException("binary to process required");
			PropList props;
			Processor::VERBOSE(props) = true;
			ws = MANAGER.load(file, props);
			ASSERT(ws);
			
			// build the CFG
			ws->require(CFG_INFO_FEATURE);
			
			// look the different CFG
			CFGInfo *info = CFGInfo::ID(ws);
			for(CFGInfo::Iter cfg(info); cfg; cfg++)
				for(CFG::BBIterator bb(cfg); bb; bb++)
					if(!bb->isEnd())
						cout << bb->address() << '\t' << bb->topAddress()
							 << '\t' << bb->size() << io::endl;
			
			// success
			cerr << "SUCCESS: " << file << " processed !\n";			
			return 0;
		}
		catch(OptionException& e) {
			displayHelp();
			cerr << "ERROR: " << e.message() << io::endl;
			return 1;
		}
		catch(elm::Exception& e) {
			cerr << "ERROR: " << e.message() << '\n';
			return 2;
		}
	}

private:
	virtual void process(string arg) {
		if(file)
			throw OptionException("bad syntax");
		file = arg;
	}

	string file;
	WorkSpace *ws;
};

int main(int argc, char **argv) {
	Command command;
	return command.run(argc, argv);
}

