/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cache/CCG/CCGBuilder.h -- CCGConstraintsBuilder class implementation.
 */
#include<stdio.h>
#include <elm/io.h>
#include <otawa/cache/ccg/CCGBuilder.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg.h>
#include <otawa/dfa/XIterativeDFA.h>
#include <otawa/dfa/XCFGVisitor.h>
#include <otawa/instruction.h>
#include <otawa/cache/LBlock.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/util/BitSet.h>
#include <otawa/util/GenGraph.h>
#include <elm/Collection.h>
#include <otawa/hard/Cache.h>
#include <otawa/cfg.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
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



static Identifier ID_CCGGraph("ipet.ccg.graph");

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
void CCGBuilder::processLBlockSet(FrameWork *fw, LBlockSet *lbset) {

	assert(fw);
	assert(lbset);
	GenGraph<CCGNode,CCGEdge> *ccg = new GenGraph<CCGNode,CCGEdge>();

	// Get some information
	System *system = getSystem(fw, ENTRY_CFG(fw));
	const hard::Cache *cache = fw->platform()->cache().instCache();

	// Initialization
	for(LBlockSet::Iterator lblock(*lbset); lblock; lblock++) {
		
		// Add node
		CCGNode *node = new CCGNode(lblock);
		ccg->add(node);
		lblock->set(ID_Node,node);
		BasicBlock *bb = lblock->bb();
		if(!bb)
			continue;
		
		// Link BB variable
		ilp::Var *bbvar = bb->use<ilp::Var *>(VAR);
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
	
	
	CCGProblem prob(lbset, lbset->count(), cache, fw);
	CFGCollection *coll = INVOLVED_CFGS(fw);
	dfa::XCFGVisitor<CCGProblem> visitor(*coll, prob);
	dfa::XIterativeDFA<dfa::XCFGVisitor<CCGProblem> > engine(visitor);
	engine.process();
	
	// Add the annotations from the DFA result 
	
	/*
	for (LBlockSet::Iterator lb(*lbset); lb; lb++) {
		if (lb->bb() != NULL)
			cout << "Le lblock " << lb->id() << " appartient au BB: " << lb->bb()->number() << "\n";
		   else cout << "Le lblock " << lb->id() << " est un lblock de debut ou de fin\n";
	}
	*/
	
	for (CFGCollection::Iterator cfg(coll); cfg; cfg++) {
		for (CFG::BBIterator block(*cfg); block; block++) {
			dfa::XCFGVisitor<CCGProblem>::key_t pair(*cfg, *block);
			BitSet *bitset = engine.in(pair);
			block->addDeletable<BitSet *>(ID_In, new BitSet(*bitset));
		}
	}
	

		// Detecting the non conflict state of each lblock
	BasicBlock *BB;
	LBlock *line;
	int length = lbset->count();		
	for(Iterator<LBlock *> lbloc(lbset->visit()); lbloc; lbloc++)
		if(lbloc->id() != 0 && lbloc->id() != length - 1) {
			BB = lbloc->bb();
			BitSet *inid = BB->use<BitSet *>(ID_In);
			for(BitSet::Iterator bit(*inid); bit; bit++)
				line = lbset->lblock(*bit);
				if(cache->block(line->address()) == cache->block(lbloc->address())
					&& BB != line->bb())
					lbloc->set<bool>(ID_NonConflict, true);
				
		}
	
		// Building the ccg edges using DFA
		//dfa.addCCGEDGES(cfg ,&ID_In, &ID_Out);
		length = lbset->count();
		Inst *inst;
		address_t adinst;			
		PseudoInst *pseudo;
		LBlock *aux;

		for (CFGCollection::Iterator cfg(coll); cfg; cfg++) {
		  for (CFG::BBIterator bb(*cfg); bb; bb++) {
		  	if ((cfg != ENTRY_CFG(fw)) || (!bb->isEntry() && !bb->isExit())) {
				BitSet *info = bb->use<BitSet *>(ID_In);
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
					else if(pseudo->id() == &bb->ID)
						break;		
				}
			}
		  }
		}
		// build edge to LBlock end
		BasicBlock *exit = ENTRY_CFG(fw)->exit();
		LBlock *end = lbset->lblock(length-1);
		BitSet *info = exit->use<BitSet *>(ID_In);
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
			
	
			
	// Cleanup the DFA annotations
	for (CFGCollection::Iterator cfg(coll); cfg; cfg++)
		for (CFG::BBIterator block(cfg); block; block++)
			block->removeProp(&ID_In);
			
}
			
			
/**
 */
void CCGBuilder::processFrameWork(FrameWork *fw) {
	assert(fw);
	
	// Check the cache
	const hard::Cache *cache = fw->platform()->cache().instCache();
	if(!cache)
		return;
	
	// Get the l-block sets
	LBlockSet **lbsets = LBLOCKS(fw);
	if(!lbsets) {
		LBlockBuilder builder;
		builder.process(fw);
		lbsets = LBLOCKS(fw);
	}
		
	// Process the l-block sets
	for(int i = 0; i < cache->lineCount(); i++)
		processLBlockSet(fw, lbsets[i]);
}


/**
 * Initialize the processor.
 * @param props	Configuration properties.
 */
void CCGBuilder::initialize(const PropList& props) {
	_explicit = props.get<bool>(EXPLICIT, false);
}


/**
 * Create a new CCGBuilder.
 * @param props	Configuration properties.
 */
CCGBuilder::CCGBuilder(const PropList& props):
Processor("CCGBuilder", Version(1, 0, 0), props), _explicit(false), vars(0) {
	initialize(props);
}


/**
 */
void CCGBuilder::configure(const PropList& props) {
	Processor::configure(props);
	initialize(props);
}

} //otawa
