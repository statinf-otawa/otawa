/*
 * subcfg.cpp
 *
 *  Created on: 29 nov. 2009
 *      Author: casse
 */

#include <otawa/otawa.h>
#include <otawa/cfg/SubCFGBuilder.h>
#include <otawa/display/CFGOutput.h>

using namespace elm;
using namespace otawa;

int main(void) {
	PropList props;
	Processor::VERBOSE(props) = true;
	WorkSpace *ws = MANAGER.load("main_prodcons.elf", props);
    /*display::CFGOutput::KIND(props) = display::OUTPUT_PDF;
    display::CFGOutput::PATH(props) = "out.pdf";*/


    TASK_ENTRY(props) = "consumer";
	CFG_START(props) = 0xa0003e10;
	CFG_STOP(props) = 0xa0003ebc;
	SubCFGBuilder builder;
	builder.process(ws, props);



	delete ws;
	return 0;
}
