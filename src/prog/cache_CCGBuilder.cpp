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
#include <otawa/hardware/CacheConfiguration.h>
#include <otawa/hardware/Platform.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/ipet/IPET.h>

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
Identifier CCGBuilder::ID_NonConflict("ipet.ccg.nonconflict");


/**
 */
Identifier CCGBuilder::ID_Node("ipet.ccg.node");


/**
 */
Identifier CCGBuilder::ID_HitVar("ipet.ccg.hit_var");


/**
 */
Identifier CCGBuilder::ID_MissVar("ipet.ccg.miss_var");


/**
 */
Identifier CCGBuilder::ID_BBVar("ipet.ccg.bb_var");


/**
 */
void CCGBuilder::processLBlockSet(FrameWork *fw, CFG *cfg, LBlockSet *lbset) {
	assert(fw);
	assert(cfg);
	assert(lbset);

	// Get some information
	System *system = IPET::getSystem(fw, cfg);
	const Cache *cache = fw->platform()->cache().instCache();

	// Initialization
	for(LBlockSet::Iterator lblock(*lbset); lblock; lblock++) {
		
		// Add node
		lblock->set(ID_Node, new CCGNode(lblock));
		BasicBlock *bb = lblock->bb();
		if(!bb)
			continue;
		
		// Link BB variable
		ilp::Var *bbvar = bb->use<ilp::Var *>(ipet::IPET::ID_Var);
		lblock->add(ID_BBVar, bbvar);
		
		// Create x_hit variable
		String namex;
		if(_explicit) {
			StringBuffer buf;
			buf << "xhit" << lblock->address() << "(" << bb->number() << ")";
			namex = buf.toString();
		}
		ilp::Var *vhit = system->newVar(namex);
		lblock->add(ID_HitVar, vhit);
		
		// Create x_miss variable
		String name1;
		if(_explicit) {
			StringBuffer buf1;
			buf1 << "xmiss" << lblock->address() << "(" << bb->number() << ")";
			 name1 = buf1.toString();
		}
		ilp::Var *miss = system->newVar(name1);
		lblock->add(ID_MissVar, miss);
	}

	// Run the DFA
	CCGDFA dfa(lbset, cfg, cache);
	dfa.DFA::resolve(cfg, &ID_In, &ID_Out);

	// Detecting the non conflict state of each lblock
	BasicBlock *BB;
	LBlock *line;
	int length = lbset->count();		
	for(Iterator<LBlock *> lbloc(lbset->visit()); lbloc; lbloc++)
		if(lbloc->id() != 0 && lbloc->id() != length - 1) {
			BB = lbloc->bb();
			DFABitSet *inid = BB->use<DFABitSet *>(ID_In);
			for(int i = 0; i < inid->size(); i++)
				if(inid->contains(i)) {
					line = lbset->lblock(i);
					if(cache->block(line->address()) == cache->block(lbloc->address())
					&& BB != line->bb())
						lbloc->set<bool>(ID_NonConflict, true);
				}
		}
	
	// Building the ccg edges using DFA
	//dfa.addCCGEDGES(cfg ,&ID_In, &ID_Out);
	length = lbset->count();
	Inst *inst;
	address_t adinst;			
    PseudoInst *pseudo;
    LBlock *aux;
	    
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++) {
		if(!bb->isEntry() && !bb->isExit()) {
			DFABitSet *info = bb->use<DFABitSet *>(ID_In);
			assert(info);
			bool test = false;
			bool visit;
			for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
				visit = false;
				pseudo = inst->toPseudo();			
				if(!pseudo){
					adinst = inst->address();				
					for (Iterator<LBlock *> lbloc(lbset->visit()); lbloc; lbloc++){
						address_t address = lbloc->address();
						// the first lblock in the BB it's a conflict
						//if ((adinst == address)&&(!lbloc->returnSTATENONCONF())&& (!test)){
						if(adinst == address && !test && bb == lbloc->bb()) {		
							for (int i = 0; i< length; i++) {
								if (info->contains(i)) {
									LBlock *lblock = lbset->lblock(i);
									// naming variables
									ilp::Var *arc;
									if(!_explicit)
										arc = system->newVar();
									else {
										StringBuffer buf1;
										buf1 << "eccg_" << lblock->address() << "_" << lbloc->address() << "(" << vars << ")";
										vars = vars + 1;
										String name1 = buf1.toString();
										arc = system->newVar(name1);
									}
									CCGNode *node = lblock->use<CCGNode *>(CCGBuilder::ID_Node);
									new CCGEdge (node, lbloc->use<CCGNode *>(CCGBuilder::ID_Node), arc);
								}
							}
							aux = lbloc;
							test = true;
							visit = true;
							break;
						}
				
						if(adinst == address && !visit && bb == lbloc->bb()) {
							// naming variables
							ilp::Var *arc;
							if(!_explicit)
								arc = system->newVar();
							else {
								StringBuffer buf3;
								buf3 << "eccg_" << aux->address() << "_" << lbloc->address() << "(" << vars << ")";
								vars = vars +1;
								String name3 = buf3.toString();
								arc = system->newVar(name3);
							}
							new CCGEdge(
								aux->use<CCGNode *>(CCGBuilder::ID_Node),
								lbloc->use<CCGNode *>(CCGBuilder::ID_Node),
								arc);
							aux = lbloc;
							break;
						}
					}
				}
				else if(pseudo->id() == bb->ID)
					break;		
			}
		}
	}
		
	// build edge to LBlock end
	BasicBlock *exit = cfg->exit();
	LBlock *end = lbset->lblock(length-1);
	DFABitSet *info = exit->use<DFABitSet *>(ID_In);
	for (int i = 0; i< length; i++){
		if (info->contains(i)){
			LBlock *ccgnode1 = lbset->lblock(i);
			//naming variables
			ilp::Var *arc;
			if(!_explicit)
				arc = system->newVar();
			else {
				StringBuffer buf4;
				buf4 << "eccg_" << ccgnode1->address() << "_END";
				String name4 = buf4.toString();
				arc = system->newVar(name4);
			}
			new CCGEdge(
				ccgnode1->use<CCGNode *>(CCGBuilder::ID_Node),
				end->use<CCGNode *>(CCGBuilder::ID_Node),
				arc);
		}
	}
	
	// Build edge from 'S' till 'end'
	LBlock *s = lbset->lblock(0);
	ilp::Var *arc;
	if(!_explicit)
		arc = system->newVar();
	else {
		StringBuffer buf4;
		buf4 << "eccg_S" << vars << "_END"; 
		vars = vars + 1;
		String name4 = buf4.toString();
		arc = system->newVar(name4);
	}
	new CCGEdge(
		s->use<CCGNode *>(CCGBuilder::ID_Node),
		end->use<CCGNode *>(CCGBuilder::ID_Node),
		arc);
}


/**
 */
void CCGBuilder::processCFG(FrameWork *fw, CFG *cfg) {
	assert(fw);
	assert(cfg);
	
	// Check the cache
	const Cache *cache = fw->platform()->cache().instCache();
	if(!cache)
		return;
	
	// Get the l-block sets
	LBlockSet **lbsets = cfg->get<LBlockSet **>(LBlockSet::ID_LBlockSet, 0);
	if(!lbsets) {
		LBlockBuilder builder;
		builder.processCFG(fw, cfg);
		lbsets = cfg->use<LBlockSet **>(LBlockSet::ID_LBlockSet);
	}
		
	// Process the l-block sets
	for(int i = 0; i < cache->lineCount(); i++)
		processLBlockSet(fw, cfg, lbsets[i]);
}


/**
 * Initialize the processor.
 * @param props	Configuration properties.
 */
void CCGBuilder::initialize(const PropList& props) {
	_explicit = props.get<bool>(IPET::ID_Explicit, false);
}


/**
 * Create a new CCGBuilder.
 * @param props	Configuration properties.
 */
CCGBuilder::CCGBuilder(const PropList& props):
CFGProcessor("CCGBuilder", Version(1, 0, 0), props), _explicit(false), vars(0) {
	initialize(props);
}


/**
 */
void CCGBuilder::configure(const PropList& props) {
	CFGProcessor::configure(props);
	initialize(props);
}

} //otawa
