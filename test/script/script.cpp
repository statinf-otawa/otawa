/*
 * script.cpp
 *
 *  Created on: 3 juin 2009
 *      Author: casse
 */

#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/script/Script.h>
#include <otawa/display/ILPSystemDisplayer.h>

using namespace otawa;
using namespace elm;

int main(int argc, char **argv) {
	try {
		PropList props;
		//Processor::VERBOSE(props) = true;
		script::PARAM(props) = pair(string("virtual"), string("0"));
		script::PARAM(props) = pair(string("stages"), string("3"));
		script::PATH(props) = argv[2];
		WorkSpace *ws = MANAGER.load(argv[1], props);

		script::Script sc;
		sc.process(ws, props);

		display::ILPSystemDisplayer disp;
		disp.process(ws, props);

		delete ws;
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << io::endl;
	}
	return 0;
}

