/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog_cache_CCGConstraintsBuilder.h -- CCGConstraintsBuilder class implementation.
 */
#include <elm/io.h>
#include <otawa/cache/ccg/CCGConstraintBuilder.h>
#include <otawa/cfg.h>
#include <otawa/instruction.h>
#include <otawa/cache/ccg/CCGNode.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cache/ccg/CCGDFA.h>
#include <otawa/ilp.h>
#include <otawa/ipet/IPET.h>
#include <elm/Collection.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/util/Dominance.h>
#include <otawa/util/DFABitSet.h>
#include <otawa/util/ContextTree.h>
#include <otawa/cfg.h>
#include <otawa/hardware/CacheConfiguration.h>

using namespace otawa::ilp;
using namespace otawa;
using namespace elm::genstruct;
using namespace otawa::ipet;

namespace otawa {

/**
 */
void CCGConstraintBuilder::processLBlockSet(CFG *cfg, LBlockSet *lbset) {
	assert(cfg);
	
	// Initialization
	System *system = cfg->get<System *>(IPET::ID_System, 0);
	assert (system);
	const Cache *cach = fw->platform()->cache().instCache();
	int dec = cach->blockBits();
	
	// Building all the contraintes of each lblock
	for (Iterator<LBlock *> lbloc(lbset->visit()); lbloc; lbloc++) {
		
		// building constraint 18
		if (lbloc->id() == 0){
			Constraint *cons18 = system->newConstraint(Constraint::EQ,1);
			for(Iterator<CCGEdge *> outedg(lbloc->ccgNode()->outEdges()); outedg; outedg++)
				cons18->add(1, outedg->varEDGE());
		}
		
		// Non-entry, non-exit node
		if(lbloc->id() != 0 && lbloc->id() != lbset->count() - 1) {
			int identif = lbloc->id();				
			address_t address = lbloc->address();
			
			// Building contraints 	(13)	
			Constraint *cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, lbloc->bbVar());
			cons->addRight(1, lbloc->hitVar());
			cons->addRight(1, lbloc->missVar());
		
			//contraints of output (17)	
			Constraint *cons17 = system->newConstraint(Constraint::EQ);
			cons17->addLeft(1, lbloc->bbVar());
			bool used = false;
			bool findend = false;
			bool findlooplb = false;
			ilp::Var *xhit;
			ilp::Var *pii;
			Constraint *cons2;
			for(Iterator<CCGEdge *> outedge(lbloc->ccgNode()->outEdges()); outedge ; outedge++) {
				cons17->addRight(1, outedge->varEDGE());
				CCGNode *target = outedge->target();
				if (target->lblock()->id() == lbset->count() - 1)
					findend = true;
				if (target->lblock()->id() == lbloc->id()) {
					findlooplb = true;
					pii = outedge->varEDGE();
				}
				used = true;
				cons2 = system->newConstraint(Constraint::LE);
				cons2->addLeft(1,outedge->varEDGE());
				cons2->addRight(1, lbloc->bbVar());
			}			
			if(!used)
				delete cons17;
				
			// contraint of input(17)	
			cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, lbloc->bbVar());
			used = false;
			bool finds = false;
			ilp::Var * psi;
			for(Iterator<CCGEdge *> inedge(lbloc->ccgNode()->inEdges()); inedge; inedge++){
				cons->addRight(1, inedge->varEDGE());
				CCGNode *source = inedge->source();
				if (source->lblock()->id() == 0){
					 finds = true;
					 psi = inedge->varEDGE();
				}
				used = true;
				
				// building contraint (30)
				if (lbloc->id() != inedge->source()->lblock()->id()) {
					cons2 = system->newConstraint(Constraint::LE);
					cons2->addLeft(1,inedge->varEDGE());
					cons2->addRight(1, lbloc->bbVar());
				}
		}
		if(!used)
				delete cons;
				
		// building contraints (19) & (20)
		 if (findlooplb) {
		  	if (finds && findend) {
		 		cons = system->newConstraint(Constraint::LE);
		 		cons2 = system->newConstraint(Constraint::LE);
				cons->addLeft(1, lbloc->hitVar());
				
				unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
				for(Iterator<CCGEdge *> inedge(lbloc->ccgNode()->inEdges()); inedge; inedge++){
					unsigned long taginedge = ((unsigned long)inedge->source()->lblock()->address()) >> dec;
					if(inedge->source()->lblock()->id() != 0
					&& inedge->source()->lblock()->id() != lbset->count() - 1){
						if (taglbloc == taginedge) {
								cons->addRight(1, inedge->varEDGE());
								cons2->addLeft(1, inedge->varEDGE());
						}
					}
		 		}
				cons->addRight(1, psi);
				cons2->addRight(1, lbloc->hitVar());
		 	}
		 	
		 	else {
				// contraint 20
		 		cons2 = system->newConstraint(Constraint::EQ);
		 		cons2->addLeft(1, lbloc->hitVar());
		 		unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
		 		for(Iterator<CCGEdge *> inedge(lbloc->ccgNode()->inEdges()); inedge; inedge++){
		 			unsigned long taginedge = ((unsigned long)inedge->source()->lblock()->address()) >> dec;
					if(inedge->source()->lblock()->id() != 0
					&& inedge->source()->lblock()->id() != lbset->count() - 1){
						if (taglbloc == taginedge)
							cons2->addRight(1, inedge->varEDGE());
					}
		 		}
		 	} 
		 }
		
		// building the (16)
		if(lbloc->getNonConflictState() && !findlooplb){
			cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, lbloc->hitVar());
			unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
			for(Iterator<CCGEdge *> inedge(lbloc->ccgNode()->inEdges()); inedge; inedge++){
				unsigned long taginedge = ((unsigned long)inedge->source()->lblock()->address()) >> 3;
				if(inedge->source()->lblock()->id() != 0
				&& inedge->source()->lblock()->id() != lbset->count() - 1) {
					if(taglbloc == taginedge)
						cons->addRight(1,  inedge->varEDGE());
				}					
			}
		}
				
		//builduig the constraint (32)
		ContextTree *cont = new ContextTree(cfg);
		addConstraintHeader(cfg, lbset, cont, lbloc);
		
		}
	}
}

/**
 */
void CCGConstraintBuilder::processCFG(FrameWork *fw, CFG *cfg ) {
	assert(fw);
	assert(cfg);
	LBlockSet **lbsets = cfg->use<LBlockSet **>(LBlockSet::ID_LBlockSet);
	const Cache *cache = fw->platform()->cache().instCache();
	if(!cache)
		return;
	for(int i = 0; i < cache->lineCount(); i++)
		processLBlockSet(cfg, lbsets[i]);
}

/**
 */
void CCGConstraintBuilder::addConstraintHeader(CFG *cfg, LBlockSet *graph,
ContextTree *cont, LBlock *boc) {
	int size = graph->count();
	bool dominate = false;
	for(Iterator<ContextTree *> son(cont->children()); son; son++)
		addConstraintHeader(cfg, graph, son, boc);
	BasicBlock *b = boc->bb();
	if(cont->kind() != ContextTree::ROOT){
		for(Iterator<BasicBlock *> bs(cont->bbs()); bs; bs++) {
			if (b == bs){
				BasicBlock *header = cont->bb();
				bool used = false;
				System *system1 = cfg->get<System *>(IPET::ID_System, 0);
				Constraint *cons32 = system1->newConstraint(Constraint::LE);
				for(Iterator<CCGEdge *> inedge(boc->ccgNode()->inEdges()); inedge ; inedge++) {
					CCGNode *source = inedge->source();
					if(source->lblock()->id() != 0
					&& source->lblock()->id() !=  size-1) {
						BasicBlock *bblock = source->lblock()->bb();
						dominate = Dominance::dominates(header, bblock);
					}
					if(boc != source->lblock()
					&& (!dominate || source->lblock()->id() == 0)) {
						cons32->addLeft(1, inedge->varEDGE());
						dominate = false;
						used = true;
					}
					
				}
				if(used) {
					for(Iterator<Edge *> inedg(header->inEdges()); inedg ; inedg++) {
						BasicBlock *preheader = inedg->source();
						if(!Dominance::dominates(header, inedg->source()))
						cons32->addRight(1, preheader->use<ilp::Var *>(IPET::ID_Var));
					}
				}
				else
					delete cons32;
				break;
			}
		}
	}	
}

}//otawa	



















