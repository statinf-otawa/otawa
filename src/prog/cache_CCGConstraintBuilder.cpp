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
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/cache/ccg/CCGBuilder.h>

using namespace otawa::ilp;
using namespace otawa;
using namespace elm::genstruct;
using namespace otawa::ipet;

namespace otawa {

/**
 */
void CCGConstraintBuilder::processLBlockSet(FrameWork *fw, LBlockSet *lbset) {
	
	// Initialization
	CFG *entry_cfg = ENTRY_CFG(fw);
	System *system = entry_cfg->get<System *>(SYSTEM, 0);
	assert (system);
	const hard::Cache *cach = fw->platform()->cache().instCache();
	int dec = cach->blockBits();
	
	// Building all the contraintes of each lblock
	for (Iterator<LBlock *> lbloc(lbset->visit()); lbloc; lbloc++) {
		
		
		/* P(x,y) == eccg_x_y */
		
		/* 
		 * (entry node) Rule 18:
		 * sum P(entry,*) = 1
		 */
		if (lbloc->id() == 0){
			Constraint *cons18 = system->newConstraint(Constraint::EQ,1);
			for(GenGraph<CCGNode,CCGEdge>::Successor outedg(lbloc->use<CCGNode *>(CCGBuilder::ID_Node)); outedg; outedg++)
				cons18->add(1, outedg.edge()->varEDGE());
		}
		
		// Non-entry, non-exit node
		if(lbloc->id() != 0 && lbloc->id() != lbset->count() - 1) {
			int identif = lbloc->id();				
			address_t address = lbloc->address();
			
			/*
			 * Rule 13:
                         * xi = xhit_xxxxx_i + xmiss_xxxx_i 
			 */
			Constraint *cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_BBVar));
			cons->addRight(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_HitVar));
			cons->addRight(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_MissVar));
		
			//contraints of input/output (17)	
			/*
			 * Rule 17:
			 * xi = sum p(*, xij) = sum p(xij, *)
                         */
			Constraint *cons17 = system->newConstraint(Constraint::EQ);
			cons17->addLeft(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_BBVar));
			bool used = false;
			bool findend = false;
			bool findlooplb = false;
			ilp::Var *xhit;
			ilp::Var *pii;
			Constraint *cons2;
			
			for(GenGraph<CCGNode,CCGEdge>::Successor outedg(lbloc->use<CCGNode *>(CCGBuilder::ID_Node)); outedg; outedg++) {
				cons17->addRight(1, outedg.edge()->varEDGE());
				CCGNode *target = outedg;
				if (target->lblock()->id() == lbset->count() - 1)
					findend = true;
				if (target->lblock()->id() == lbloc->id()) {
					findlooplb = true;
					pii = outedg.edge()->varEDGE();
				}
				used = true;
				// p(ij, uv)  <= xi
				/*cons2 = system->newConstraint(Constraint::LE);
				cons2->addLeft(1,outedg.edge()->varEDGE());
				cons2->addRight(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_BBVar));*/
			}
			
			if(!used)
				delete cons17;
				
			// contraint of input(17)	
			cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_BBVar));
			used = false;
			bool finds = false;
			ilp::Var * psi;
			for(GenGraph<CCGNode,CCGEdge>::Predecessor inedge(lbloc->use<CCGNode *>(CCGBuilder::ID_Node)); inedge; inedge++) {

				cons->addRight(1, inedge.edge()->varEDGE());
				CCGNode *source = inedge;
				if (source->lblock()->id() == 0){
					 finds = true;
					 psi = inedge.edge()->varEDGE();
				}
				used = true;
				
				// building contraint (30)
				// p(uv, ij) <= xi
				/*if (lbloc->id() != inedge->lblock()->id()) {
					cons2 = system->newConstraint(Constraint::LE);
					cons2->addLeft(1,inedge.edge()->varEDGE());
					cons2->addRight(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_BBVar));
				}*/
                        }
                if(!used)
                        delete cons;
				
		// building contraints (19) & (20)
		// cache_block(uv) = cach_block(ij)
		// (19) p(ij, ij) + p(uv, ij) <= xihit <= p(ij, ij) + p(uv, ij) + p(entry, ij) if p(entry, ij) and p(ij, exit)
		// (20) p(ij, ij) + p(uv, ij) = xihit else
		if (findlooplb) {
		  	if (finds && findend) {
		  	        // constraint 19
		 		cons = system->newConstraint(Constraint::LE);
		 		cons2 = system->newConstraint(Constraint::LE);
				cons->addLeft(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_HitVar));
				
				unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
				for(GenGraph<CCGNode,CCGEdge>::Predecessor inedge(lbloc->use<CCGNode *>(CCGBuilder::ID_Node)); inedge; inedge++) {
					unsigned long taginedge = ((unsigned long)inedge->lblock()->address()) >> dec;
					if(inedge->lblock()->id() != 0
					&& inedge->lblock()->id() != lbset->count() - 1){
						if (taglbloc == taginedge) {
								cons->addRight(1, inedge.edge()->varEDGE());
								cons2->addLeft(1, inedge.edge()->varEDGE());
						}
					}
		 		}
				cons->addRight(1, psi);
				cons2->addRight(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_HitVar));
		 	}
		 	
		 	else {
				// contraint 20
		 		cons2 = system->newConstraint(Constraint::EQ);
		 		cons2->addLeft(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_HitVar));
		 		unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
		 		for(GenGraph<CCGNode,CCGEdge>::Predecessor inedge(lbloc->use<CCGNode *>(CCGBuilder::ID_Node)); inedge; inedge++) {
		 			unsigned long taginedge = ((unsigned long)inedge->lblock()->address()) >> dec;
					if(inedge->lblock()->id() != 0
					&& inedge->lblock()->id() != lbset->count() - 1){
						if (taglbloc == taginedge)
							cons2->addRight(1, inedge.edge()->varEDGE());
					}
		 		}
		 	} 
		 }
		
		// building the (16)
//		if(lbloc->getNonConflictState() && !findlooplb){
                // xihit = sum p(uv, ij) / cache_block(uv) = cache_block(ij)
		else if(lbloc->get<bool>(CCGBuilder::ID_NonConflict, false) /*&& !findlooplb*/){
			cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, lbloc->use<ilp::Var *>(CCGBuilder::ID_HitVar));
			unsigned long taglbloc = ((unsigned long)lbloc->address()) >> dec;
			for(GenGraph<CCGNode,CCGEdge>::Predecessor inedge(lbloc->use<CCGNode *>(CCGBuilder::ID_Node));
			inedge; inedge++) {
				unsigned long taginedge = ((unsigned long)inedge->lblock()->address()) >> 3;
				if(inedge->lblock()->id() != 0
				&& inedge->lblock()->id() != lbset->count() - 1) {
					if(taglbloc == taginedge)
						cons->addRight(1,  inedge.edge()->varEDGE());
				}					
			}
		}
				
		//builduig the constraint (32)
                ContextTree *cont = new ContextTree(entry_cfg);
                addConstraintHeader(entry_cfg, lbset, cont, lbloc);
        }
  }

}

/**
 */
void CCGConstraintBuilder::processFrameWork(FrameWork *fw) {
	assert(fw);
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = fw->platform()->cache().instCache();
	if(!cache)
		return;
	for(int i = 0; i < cache->lineCount(); i++)
		processLBlockSet(fw, lbsets[i]);
}

/**
 */
void CCGConstraintBuilder::addConstraintHeader(CFG *cfg, LBlockSet *graph, ContextTree *cont, LBlock *boc) {
	int size = graph->count();
	bool dominate = false;
	for(Iterator<ContextTree *> son(cont->children()); son; son++)
		addConstraintHeader(cfg, graph, son, boc);
	BasicBlock *b = boc->bb();
	if(cont->kind() == ContextTree::LOOP){
		for(Iterator<BasicBlock *> bs(cont->bbs()); bs; bs++) {
			if (b == bs){
			        // p(uv, ij) / header not dom bb(uv) <= sum xi / (xi, header) in E and header not dom xi
				BasicBlock *header = cont->bb();
				bool used = false;
				System *system1 = cfg->get<System *>(SYSTEM, 0);
				Constraint *cons32 = system1->newConstraint(Constraint::LE);
				for(GenGraph<CCGNode,CCGEdge>::Predecessor inedge(boc->use<CCGNode *>(CCGBuilder::ID_Node)); inedge; inedge++) {
					CCGNode *source = inedge;
					if(source->lblock()->id() != 0
					&& source->lblock()->id() !=  size-1) {
						BasicBlock *bblock = source->lblock()->bb();
						dominate = Dominance::dominates(header, bblock);
					}
					if(boc != source->lblock()
					&& (!dominate || source->lblock()->id() == 0)) {
						cons32->addLeft(1, inedge.edge()->varEDGE());
						dominate = false;
						used = true;
					}
					
				}
				if(used) {
				        bool set = false;
					for(Iterator<Edge *> inedg(header->inEdges()); inedg ; inedg++) {
						BasicBlock *preheader = inedg->source();
						if(!Dominance::dominates(header, inedg->source()))
						cons32->addRight(1, preheader->use<ilp::Var *>(VAR));
						set = true;
					}
					assert(set);
				}
				else
					delete cons32;
				break;
			}
		}
	}	
}

}//otawa	



















