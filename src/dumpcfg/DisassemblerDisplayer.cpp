/*
 * $Id$
 * Copyright (c) 2003, IRIT UPS.
 *
 * src/dumpcfg/DisassemblerDisplayer.cpp -- DisassemblerDisplayer class implementation.
 */

#include <elm/io.h>
#include <otawa/cfg/features.h>
#include "DisassemblerDisplayer.h"

using namespace elm;
using namespace otawa;

/**
 */
DisassemblerDisplayer::DisassemblerDisplayer(void): Displayer("DisassemblerDisplayer", Version(2, 0, 0)) {
}


/**
 */
void DisassemblerDisplayer::processWorkSpace(WorkSpace *ws) {
	const CFGCollection& coll = **otawa::INVOLVED_CFGS(ws);

	// process CFGs
	for(int i = 0; i < coll.count(); i++) {
		if(!display_all && i > 0)
			break;

		// display function head
		CFG *cfg = coll[i];
		cout << "# Function " << cfg->label() << '\n';

		// display blocks
		for(CFG::BlockIter v = cfg->blocks(); v(); v++) {

			// display header
			cout << "BB " << v->index() << ": ";
			for(Block::EdgeIter e = v->outs(); e(); e++) {
				if(e->sink()->isSynth())
					cout << " C(" << e->sink()->toSynth()->callee()->label() << ")";
				else {
					if(v->isEntry() || e->sink()->isExit())
						cout << " V(";
					else if(e->isNotTaken())
						cout << " NT(";
					else
						cout << " T(";
					cout << e->sink()->index() << ")";
				}
			}
			cout << io::endl;

			// if needed, display code
			if(v->isBasic()) {
				BasicBlock *bb = **v;
				for(BasicBlock::InstIter i(bb); i(); i++) {

					// Put the label
					for(Identifier<String>::Getter label(*i, FUNCTION_LABEL); label(); label++)
						cout << '\t' << *label << ":\n";
					for(Identifier<String>::Getter label(*i, LABEL); label(); label++)
						cout << '\t' << *label << ":\n";

					// Disassemble the instruction
					cout << "\t\t" << ot::address(i->address()) << ' ';
					i->dump(cout);
					cout << '\n';

				}
			}
		}
	}

}
