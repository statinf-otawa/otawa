/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test/ipet/test_ipet.cpp -- test for IPET feature.
 */

#include <stdlib.h>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet/TrivialBBTime.h>

using namespace otawa;
using namespace elm;

int main(int argc, char **argv) {

	Manager manager;
	PropList props;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
	try {
		FrameWork *fw = manager.load(argv[1], props);
		TrivialBBTime tbt(5);
		tbt.processFrameWork(fw);
		cout << "SUCCESS\n";
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

