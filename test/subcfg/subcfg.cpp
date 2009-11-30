/*
 * subcfg.cpp
 *
 *  Created on: 29 nov. 2009
 *      Author: casse
 */

#include <otawa/otawa.h>
#include <otawa/cfg/SubCFGBuilder.h>

using namespace elm;
using namespace otawa;

int main(void) {
	PropList props;
	Processor::VERBOSE(props) = true;
	WorkSpace *ws = MANAGER.load("/home/casse/Benchs/snu-rt.old/bs/bs", props);

	CFG_START(props) = 0x50140;
	CFG_STOP(props) = 0x50150;
	SubCFGBuilder builder;
	builder.process(ws, props);

	delete ws;
	return 0;
}
