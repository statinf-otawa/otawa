/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cache/ccg/CCGConstraintsBuilder.h -- CCGConstraintsBuilder class implementation.
 */
#include<stdio.h>
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


void CCGConstraintBuilder::processCFG(FrameWork *fw, CFG *cfg ) {
	assert(cfg);
	System *system = cfg->get<System *>(IPET::ID_System, 0);
	assert (system);
	LBlockSet *ccg = cfg->use<LBlockSet *>(LBlockSet::ID_LBlockSet);
	
	// cache configuration
	const Cache *cach = fw->platform()->cache().instCache();
	// decallage of x bits where each block containts 2^x ocets
	int dec = cach->blockBits();
	
	// Building all the contraintes of each lblock
	for (Iterator<LBlock *> lbloc(ccg->visitLBLOCK()); lbloc; lbloc++){
		// building constraint 18
		if (lbloc->identificateurLBLOCK() == 0){
			Constraint *cons18 = system->newConstraint(Constraint::EQ,1);
			for(Iterator<CCGEdge *> outedg(lbloc->ccgnode()->outEdges()); outedg; outedg++){
			cons18->add(1, outedg->varEDGE());
			}
		}
		 
		if((lbloc->identificateurLBLOCK()!=0) &&(lbloc->identificateurLBLOCK()!= (ccg->returnCOUNTER() - 1))){
			// Building contraints 	(13)	
			Constraint *cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, lbloc->varBB());
			cons->addRight(1, lbloc->varHIT());
			cons->addRight(1, lbloc->varMISS());
			int identif = lbloc->identificateurLBLOCK();				
			address_t address = lbloc->addressLBLOCK();
			//cout << "Lblock  " << identif << " " ;
		
			//contraints of output (17)	
			Constraint *cons17 = system->newConstraint(Constraint::EQ);
			cons17->addLeft(1, lbloc->varBB());
			bool used = false;
			bool findend = false;
			bool findlooplb = false;
			ilp::Var *xhit;
			ilp::Var *pii;
			Constraint *cons2;
			for(Iterator<CCGEdge *> outedge(lbloc->ccgnode()->outEdges()); outedge ; outedge++){
				cons17->addRight(1, outedge->varEDGE());
				CCGNode *target = outedge->target();
				if (target->lblock()->identificateurLBLOCK()== (ccg->returnCOUNTER()-1)) findend = true;
				if (target->lblock()->identificateurLBLOCK() == lbloc->identificateurLBLOCK()){
					findlooplb = true;
					pii = outedge->varEDGE();
				}
				used = true;
			 	//building contraint (30)
				cons2 = system->newConstraint(Constraint::LE);
				cons2->addLeft(1,outedge->varEDGE());
				cons2->addRight(1, lbloc->varBB());
				//cout <<"OUT"<<outedge->target()->lblock()->identificateurLBLOCK();
			}
			
			if(!used)
				delete cons17;
				
		
			// contraint of input(17)	
			cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, lbloc->varBB());
			used = false;
			bool finds = false;
			ilp::Var * psi;
			for(Iterator<CCGEdge *> inedge(lbloc->ccgnode()->inEdges()); inedge; inedge++){
				cons->addRight(1, inedge->varEDGE());
				CCGNode *source = inedge->source();
				if (source->lblock()->identificateurLBLOCK()== 0){
					 finds = true;
					 psi = inedge->varEDGE();
				}
				used = true;
				// building contraint (30)
				if (lbloc->identificateurLBLOCK()!= inedge->source()->lblock()->identificateurLBLOCK()){
					cons2 = system->newConstraint(Constraint::LE);
					cons2->addLeft(1,inedge->varEDGE());
					cons2->addRight(1, lbloc->varBB());
				}
			//cout <<"IN"<< inedge->source()->lblock()->identificateurLBLOCK();
		}
		//cout <<"\n";
		if(!used)
				delete cons;
		// building contraints (19) & (20)
		 if (findlooplb){
		  	if ((finds) && (findend)){
		 		cons = system->newConstraint(Constraint::LE);
		 		cons2 = system->newConstraint(Constraint::LE);
				cons->addLeft(1, lbloc->varHIT());
				
				unsigned long taglbloc = ((unsigned long)lbloc->addressLBLOCK()) >> dec;
				for(Iterator<CCGEdge *> inedge(lbloc->ccgnode()->inEdges()); inedge; inedge++){
					unsigned long taginedge = ((unsigned long)inedge->source()->lblock()->addressLBLOCK()) >> dec;
					if ((inedge->source()->lblock()->identificateurLBLOCK() != 0)
					&&((inedge->source()->lblock()->identificateurLBLOCK()) != (ccg->returnCOUNTER()-1))){
						if (taglbloc == taginedge){
						//if (cach->tag(lbloc->addressLBLOCK())== cach->tag(inedge->source()->lblock()->addressLBLOCK())){
						 //|| (inedge->source()->blockbasicLBLOCK()->address() == lbloc->blockbasicLBLOCK()->address())){
								cons->addRight(1, inedge->varEDGE());
								cons2->addLeft(1, inedge->varEDGE());
						}
					}
		 		}
				
				cons->addRight(1, psi);
				
				
				cons2->addRight(1, lbloc->varHIT());
				//cout <<"Loop has detected with edges from S till End \n";
		 	}
		 	else {
		  // contraint 20
		 		cons2 = system->newConstraint(Constraint::EQ);
		 		cons2->addLeft(1,lbloc->varHIT());
		 		//************* >> dec 
		 		unsigned long taglbloc = ((unsigned long)lbloc->addressLBLOCK()) >> dec;
		 		for(Iterator<CCGEdge *> inedge(lbloc->ccgnode()->inEdges()); inedge; inedge++){
		 			unsigned long taginedge = ((unsigned long)inedge->source()->lblock()->addressLBLOCK()) >> dec;
					if ((inedge->source()->lblock()->identificateurLBLOCK() != 0)
					&&((inedge->source()->lblock()->identificateurLBLOCK()) != (ccg->returnCOUNTER()-1))){
						if (taglbloc == taginedge)
						//if (cach->tag(lbloc->addressLBLOCK())== cach->tag(inedge->source()->lblock()->addressLBLOCK()))
							cons2->addRight(1,inedge->varEDGE());
					}
		 		}
				//cout <<"Loop has detected without edges from S till end \n";
		 	} 
		 }
		 /*
		// building contraint (15)		
		if (length ==3){
				if ((lbloc->identificateurLBLOCK() )== 1){
				cons = system->newConstraint(Constraint::LE, 1);
				cons->add(1, lbloc->varMISS());
			}// end if	
		}// end if
		*/
		
		// building the (16)
		if ((lbloc->returnSTATENONCONF())&&(!findlooplb)){
			cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, lbloc->varHIT());
			//*******************>> dec
			unsigned long taglbloc = ((unsigned long)lbloc->addressLBLOCK()) >> dec;
			for(Iterator<CCGEdge *> inedge(lbloc->ccgnode()->inEdges()); inedge; inedge++){
				unsigned long taginedge = ((unsigned long)inedge->source()->lblock()->addressLBLOCK()) >> 3;
				if ((inedge->source()->lblock()->identificateurLBLOCK() != 0)
					&&((inedge->source()->lblock()->identificateurLBLOCK()) != (ccg->returnCOUNTER()-1))){
						if (taglbloc == taginedge){
						//if (cach->tag(lbloc->addressLBLOCK())== cach->tag(inedge->source()->lblock()->addressLBLOCK())){	
							cons->addRight(1,  inedge->varEDGE());
							//cout << "non conflict detected";
						}					
				}					
			}
		}//end if
				
		//builduig the constraint (32)
		ContextTree *cont = new ContextTree(cfg);
		addConstraintHEADER (cfg,ccg,cont,lbloc);
		
		}// end if (without S end end)	
		
	}
}

	
	

void CCGConstraintBuilder::addConstraintHEADER (CFG *cfg,LBlockSet *graph, ContextTree *cont, LBlock *boc){
	int size = graph->returnCOUNTER();
	bool dominate = false;
	for(Iterator<ContextTree *> son(cont->children()); son; son++){
			addConstraintHEADER (cfg, graph, son, boc);
	}//end iterator ContextTree
	BasicBlock *b = boc->blockbasicLBLOCK ();
	if(cont->kind() != ContextTree::ROOT){
		for(Iterator<BasicBlock *> bs(cont->bbs()); bs; bs++){
			if (b == bs){
				BasicBlock *header = cont->bb();
				bool used = false;
				System *system1 = cfg->get<System *>(IPET::ID_System, 0);
				Constraint *cons32 = system1->newConstraint(Constraint::LE);
				for(Iterator<CCGEdge *> inedge(boc->ccgnode()->inEdges()); inedge ; inedge++){
					CCGNode *source = inedge->source();
					if ((source->lblock()->identificateurLBLOCK() != 0)&&(source->lblock()->identificateurLBLOCK()!= size-1)){
						BasicBlock *bblock = source->lblock()->blockbasicLBLOCK();
						dominate = Dominance::dominates(header, bblock);
					}
					if ((boc != source->lblock())&&((!dominate)||(source->lblock()->identificateurLBLOCK() == 0))){
								cons32->addLeft(1, inedge->varEDGE());
								dominate = false;
								used = true;
					}
					
				}
			if (used){
				for(Iterator<Edge *> inedg(header->inEdges()); inedg ; inedg++){
					BasicBlock *preheader = inedg->source();
					if (!Dominance::dominates(header, inedg->source()))
					cons32->addRight(1,preheader->use<ilp::Var *>(IPET::ID_Var));
				}
			}
			else
				delete cons32;
			break;
			}
			}
		}
		
	};
}//otawa	



















