/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cache/CCG/CCGBuilder.h -- CCGConstraintsBuilder class implementation.
 */
#include<stdio.h>
#include <elm/io.h>
#include <otawa/cache/ccg/CCGBuilder.h>
#include <otawa/cfg.h>
#include <otawa/instruction.h>
#include <otawa/cache/LBlock.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/util/DFABitSet.h>
#include <elm/Collection.h>
#include <otawa/hardware/Cache.h>
#include <otawa/cfg.h>
#include <string>
#include <otawa/hardware/CacheConfiguration.h>

using namespace otawa::ilp;
using namespace otawa;
using namespace elm::genstruct;
using namespace otawa::ipet;

namespace otawa {
	
// DFA Properties
static Identifier ID_In("ipet.ccg.dfain");
static Identifier ID_Out("ipet.ccg.dfaout");

/**
 */
void CCGBuilder::processLBlockSet(FrameWork *fw, CFG *cfg, LBlockSet *lbset) {
	assert(fw);
	assert(cfg);
	assert(lbset);

	// Initialization
	System *system = cfg->get<System *>(IPET::ID_System, 0);
	assert (system);
	const Cache *cach = fw->platform()->cache().instCache();
	int dec = cach->blockBits();	
	CCGDFA dfa(lbset, cfg, cach);
	new LBlock(lbset, 0, 0, 0, 0, 0, "ccg");
	
	// Build the l-blocks
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++)
		if (!bb->isEntry() && !bb->isExit()) {
			
			ilp::Var *bbv = bb->use<ilp::Var *>(IPET::ID_Var);
			Inst *inst;
			bool find = false;
			PseudoInst *pseudo;
			
			for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {				
				pseudo = inst->toPseudo();
				address_t address = inst->address();

				// Do not process pseudo or stop on BB start pseudo
				if(pseudo) {
					if(pseudo->id() == bb->ID)
						break;
				}
				
				// Process the instruction
				else {
					
					if(cach->line(address) != lbset->line())
						find = false;
						
					if(!find && cach->line(address) == lbset->line()) {
						StringBuffer buf;
						buf << "xhit" << address << "(" << *bb << ")";
						String namex = buf.toString();
						ilp::Var *vhit = system->newVar(namex);
						StringBuffer buf1;
						buf1 << "xmiss" << address << "(" << *bb << ")";
						String name1 = buf1.toString();
						ilp::Var *miss = system->newVar(name1);
						new LBlock(lbset, address ,bb, vhit, miss, bbv, "ccg");
						find = true;
					}
				}
			}
		}
	
	// Node 'END' of the CCG	
	new LBlock(lbset, 0, 0, 0, 0, 0, "ccg");
	int length = lbset->count();	
	
	// DFA processing
	dfa.DFA::resolve(cfg, &ID_In, &ID_Out);

	// Detecting the non conflict state of each lblock
	BasicBlock *BB;
	LBlock *line;
	for (Iterator<LBlock *> lbloc(lbset->visit()); lbloc; lbloc++)
		if(lbloc->id() != 0 && lbloc->id() != length - 1) {
			BB = lbloc->bb();
			DFABitSet *inid = BB->use<DFABitSet *>(ID_In);
			for (int i = 0; i < inid->size(); i++)
				if(inid->contains(i)){
					line = lbset->lblock(i);
					unsigned long tagline = ((unsigned long)line->address()) >> dec;
					unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
					if(tagline == taglbloc && BB != line->bb())
						lbloc->setNonConflictState(true);
				}
		}
	
	// Building the ccg edges using DFA
	dfa.addCCGEDGES(cfg ,&ID_In, &ID_Out);
}

/**
 */
void CCGBuilder::processCFG(FrameWork *fw, CFG *cfg) {
	assert(fw);
	assert(cfg);
	
	// Create the LBlock set array
	const Cache *cache = fw->platform()->cache().instCache();
	if(!cache)
		return;
	LBlockSet **lbsets = new LBlockSet *[cache->lineCount()];
	for(int i = 0; i < cache->lineCount(); i++)
		lbsets[i] = new LBlockSet(i);
	cfg->set(LBlockSet::ID_LBlockSet, lbsets);
		
	// Process the l-block sets
	for(int i = 0; i < cache->lineCount(); i++)
		processLBlockSet(fw, cfg, lbsets[i]);
}

} //otawa




