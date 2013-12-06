/*
 *	$Id$
 *	BPredProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <otawa/bpred/BPredProcessor.h>
#include "BHG.h"
#include "BBHG.h"
#include "BBHGDrawer.h"
#include "BHGDrawer.h"
#include "BCG.h"
#include "BCGDrawer.h"


#include <otawa/ipet/BasicConstraintsBuilder.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/ilp/System.h>


namespace otawa {
namespace bpred {

using namespace otawa::ilp;
using namespace elm;
using namespace elm::genstruct;

/////// CREATION DE CONTRAINTES ET VARIABLES
#define NEW_BASIC_CONSTRAINT(cons_name) Constraint *cons_name = system->newConstraint(Constraint::EQ); \
										assert(cons_name);

#define NEW_SPECIAL_CONSTRAINT(cons_name,op,val) 	Constraint *cons_name = system->newConstraint(Constraint::op, val); \
													ASSERT(cons_name);

#define NEW_VAR_FROM_BUFF(var_name,buff_expr)	{ \
													StringBuffer sb##var_name; \
													sb##var_name << buff_expr; \
													String s##var_name = sb##var_name.toString(); \
													if(ht_vars.exists(s##var_name)) { \
														var_name = ht_vars.get(s##var_name); \
														ASSERT(var_name); \
													} \
													else { \
														if(this->explicit_mode) var_name = system->newVar(s##var_name); \
														else var_name = system->newVar(String("")); \
														ASSERT(var_name); \
														ht_vars.put(s##var_name,var_name); \
													} \
												}
//////////////////////////////////////////////////


#define GLOBAL 1
#define H0_mitra 	0+GLOBAL
#define H1_mitra 	0+GLOBAL
#define H2_mitra 	0+GLOBAL
#define H41_mitra 	0+GLOBAL
#define H42_mitra 	0+GLOBAL

#define H5_mitra 	0+GLOBAL
#define C11_1b		0+GLOBAL
#define C12_1b		0+GLOBAL
#define C21_1b		0+GLOBAL
#define C22_1b		0+GLOBAL
#define C3_1b		0+GLOBAL
#define P1_1b		0+GLOBAL 
#define P21_24_1b	0+GLOBAL 
#define P3_1b		1+GLOBAL 


/**
 * Generate the constraints system according to the "global 1bit" method, without history-based constraints.
 * 
 * @param fw		Current Workspace.
 * @param cfg		CFG.
 * @param bhg		"Branch History Graph".
 * @param bbhg		"BasicBlock History Graph".
 * @param bcgs		Vector containg the BCGs (Branch Conflict Graphs) generated from the BHG.
 * @param ht_vars	Hash Table used to ensure unicity of the variables.
 */
void BPredProcessor::CS__Global1b(WorkSpace *fw, CFG *cfg, BHG *bhg,BBHG *bbhg, elm::genstruct::Vector<BCG*> &bcgs,elm::genstruct::HashTable<String ,ilp::Var*>& ht_vars ) {
	// Recuperation de l'ensemble des contraintes
	System *system = ipet::SYSTEM(fw);
	ASSERT(system);

	// creation des classes d'appartenance des branchements A PARTIR DU BHG (indirectement depuis les BCG)
	HashTable<BasicBlock*,elm::genstruct::Vector<BCGNode*> > BB_classes;
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		elm::genstruct::Vector<BCGNode*> list_nodes;
		
		for(unsigned int i = 0 ; i< bcgs.length();++i) {
			BCG* g=bcgs[i];
			for(BCG::Iterator n(g);n;n++)
				if(n->getCorrespondingBBNumber() == bb->number()) { list_nodes.add(n);break; }
		}
		
		if(list_nodes.length()>0) BB_classes.add(bb,list_nodes);
	}

	HashTable<BasicBlock*, elm::genstruct::Vector<BBHGNode*> > BB_classes_BBHG;
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		elm::genstruct::Vector<BBHGNode*> v;
		for(BBHG::Iterator node(bbhg);node;node++) {
			if(node->getCorrespondingBB()->number() == bb->number()) {
				v.add(node);
			}
		}
		BB_classes_BBHG.add(bb,v);
	}
	
	
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		Var *Xb=ipet::VAR(bb);
		ASSERT(Xb);

		if(BB_classes.exists(bb)) {
			elm::genstruct::Vector<BCGNode*> v = BB_classes.get(bb);
			
			
			for(int i = 0 ;  i < v.length() ; i++) {
				Var *XbApi;
				NEW_VAR_FROM_BUFF(XbApi, Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()));
				
			////////
			// C11:
			////////
#if C11_1b>0
				{
					HashTable<Var*, Var*> added_vars;
					NEW_SPECIAL_CONSTRAINT(C11,EQ,0);
					C11->addLeft(1,XbApi);
					
					for(BCG::OutIterator s(v[i]);s;s++) {
						Var *XbApiSs;
						NEW_VAR_FROM_BUFF(XbApiSs,XbApi->name() << "S" << s->target()->getCorrespondingBBNumber());
						if(!added_vars.exists(XbApiSs)) {
							C11->addRight(1,XbApiSs);
							added_vars.add(XbApiSs,XbApiSs);
						}
					}
					if(v[i]->isExit()) {
						Var *XbApiEnd;
						NEW_VAR_FROM_BUFF(XbApiEnd,XbApi->name() << "end");
						C11->addRight(1,XbApiEnd);					
					}
				}
#endif
			
			////////
			// C12:
			////////
#if C12_1b>0
				{
					HashTable<Var*, Var*> added_vars;
					NEW_SPECIAL_CONSTRAINT(C12,EQ,0);
					C12->addLeft(1,XbApi);
					
					for(BCG::InIterator p(v[i]);p;p++) {
						Var *XpApiSb;
						Var *Xp = ipet::VAR(getBB(p->source()->getCorrespondingBBNumber(),cfg));
						NEW_VAR_FROM_BUFF(XpApiSb,Xp->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "S" << bb->number());
						if(!added_vars.exists(XpApiSb)) {
							C12->addRight(1,XpApiSb);
							added_vars.add(XpApiSb,XpApiSb);
						}
					}
					if(v[i]->isEntry()) {
						Var *XbApiStart;
						NEW_VAR_FROM_BUFF(XbApiStart,XbApi->name() << "start");
						C12->addRight(1,XbApiStart);					
					}
				}
#endif
			
			////////
			// C21:
			////////
#if C21_1b>0
				{
					BCG* bcg;
					for(unsigned int ig=0;ig<bcgs.length();ig++) {
						if(bcgs[ig]->getHistory() == v[i]->getHistory()) {
							bcg=bcgs[ig];
							for(BCG::Iterator n(bcg);n;n++) {
								bool withT=false, withNT=false;
								if(v[i]->isSuccessor(n,withT,withNT)) {
									NEW_SPECIAL_CONSTRAINT(C21,EQ,0);
									Var *XbApiSs;
									NEW_VAR_FROM_BUFF(XbApiSs,XbApi->name() << "S" << n->getCorrespondingBBNumber());
									C21->addLeft(1,XbApiSs);
		
									if(withT) {
										Var *XbApiSsD1;
										NEW_VAR_FROM_BUFF(XbApiSsD1,XbApi->name() << "D1S" << n->getCorrespondingBBNumber());
										C21->addRight(1,XbApiSsD1);
									}
									if(withNT) {
										Var *XbApiSsD0;
										NEW_VAR_FROM_BUFF(XbApiSsD0, XbApi->name() << "D0S" << n->getCorrespondingBBNumber());								
										C21->addRight(1,XbApiSsD0);
									}							
								}
							}
							if(v[i]->isExit()) {
								NEW_SPECIAL_CONSTRAINT(C21,EQ,0);
								Var *XbApiEnd;
								NEW_VAR_FROM_BUFF(XbApiEnd,XbApi->name() << "end");
								C21->addLeft(1,XbApiEnd);
								if(v[i]->exitsWithT()) {
									Var *XbApiD1End;
									NEW_VAR_FROM_BUFF(XbApiD1End,XbApi->name() << "D1end");
									C21->addRight(1,XbApiD1End);							
								}
								if(v[i]->exitsWithNT()) {
									Var *XbApiD0End;
									NEW_VAR_FROM_BUFF(XbApiD0End,XbApi->name() << "D0end");
									C21->addRight(1,XbApiD0End);														
								}
							}
						}
					}
				}
#endif

			}
		}
	}
				
	////////
	// C22:
	////////				
#if C22_1b>0
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		Var *Xb=ipet::VAR(bb);
		ASSERT(Xb);
		if(BB_classes.exists(bb)) {
			elm::genstruct::Vector<BCGNode*> v = BB_classes.get(bb);
			
			for(BasicBlock::OutIterator edge(bb);edge;edge++) {
				int d=(edge->kind()==Edge::TAKEN)?1:0;
				Var *Eb_sd = ipet::VAR(edge);
				for(int i=0;i<v.length();i++) {
					NEW_SPECIAL_CONSTRAINT(C22,EQ,0);
					Var *Eb_sdApi;
					NEW_VAR_FROM_BUFF(Eb_sdApi, Eb_sd->name() << "A" << BitSet_to_String(v[i]->getHistory()));
					C22->addRight(1,Eb_sdApi);
					
					for(BCG::OutIterator s(v[i]);s;s++) {
						if(s->isTaken() == (d==1)) {
							Var *XbApiDdSs;
							NEW_VAR_FROM_BUFF(XbApiDdSs,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "D" << d << "S" << s->target()->getCorrespondingBBNumber());
							C22->addLeft(1,XbApiDdSs);
						}
					}
					if(v[i]->isExit() && v[i]->exitsWithT()==(d==1) && d==1) {
						Var *XbApiDdend;
						NEW_VAR_FROM_BUFF(XbApiDdend,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "D" << d << "end");
						C22->addLeft(1,XbApiDdend);						
					}
					if(v[i]->isExit() && v[i]->exitsWithNT()==(d==0) && d==0) {
						Var *XbApiDdend;
						NEW_VAR_FROM_BUFF(XbApiDdend,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "D" << d << "end");
						C22->addLeft(1,XbApiDdend);						
					}
					
				}
			}
		}
	}
#endif
	
	///////
	// C3:
	///////
#if C3_1b>0
	for(int ig=0;ig<bcgs.length();ig++) {
		NEW_SPECIAL_CONSTRAINT(C3_start,LE,1);
		NEW_SPECIAL_CONSTRAINT(C3_end,LE,1);
		
		for(BCG::Iterator n(bcgs[ig]);n;n++) {
			Var *Xb=ipet::VAR(getBB(n->getCorrespondingBBNumber(),cfg));
			ASSERT(Xb);
			if(n->isEntry()) {
				Var *XbApiStart;
				NEW_VAR_FROM_BUFF(XbApiStart,Xb->name() << "A" << BitSet_to_String(n->getHistory()) << "start");
				C3_start->addLeft(1,XbApiStart);
			}
			if(n->isExit()) {
				Var *XbApiEnd;
				NEW_VAR_FROM_BUFF(XbApiEnd,Xb->name() << "A" << BitSet_to_String(n->getHistory()) << "end");				
				C3_end->addLeft(1,XbApiEnd);
			}
		}
	}
#endif

	
	///////
	// P1:
	///////
#if P1_1b>0
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		if(!BB_classes.exists(bb)) {
			Var *Xb=ipet::VAR(bb);
			ASSERT(Xb);
			elm::genstruct::Vector<BBHGNode*> v = BB_classes_BBHG.get(bb);
			for(int i=0;i<v.length();i++) {
				for(BasicBlock::OutIterator edge(bb);edge;edge++) {
					Var* Mb_dApi;
					NEW_SPECIAL_CONSTRAINT(P1,EQ,0);
					NEW_VAR_FROM_BUFF(Mb_dApi, "m" << bb->number() << "_" << edge->target()->number() << "A" << BitSet_to_String(v[i]->getHistory()) );
					P1->addRight(1,Mb_dApi);
				}
				if(bb == cfg->exit()) {
					Var* Mb_endApi;
					NEW_SPECIAL_CONSTRAINT(P1,EQ,0);
					NEW_VAR_FROM_BUFF(Mb_endApi, "m" << bb->number() << "_endA" << BitSet_to_String(v[i]->getHistory()) );
					P1->addRight(1,Mb_endApi);
				}
			}
		}		
	}
#endif
	
	
	///////////
	// P21_24:
	///////////
#if P21_24_1b>0
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		if(BB_classes.exists(bb)) {
			elm::genstruct::Vector<BCGNode*> v = BB_classes.get(bb);
			Var *Xb=ipet::VAR(bb);
			ASSERT(Xb);
			for(int i = 0 ; i < v.length() ; i++ ) {
				NEW_SPECIAL_CONSTRAINT(P21,LE,0);
				NEW_SPECIAL_CONSTRAINT(P22,LE,0);
				NEW_SPECIAL_CONSTRAINT(P23,LE,0);
				NEW_SPECIAL_CONSTRAINT(P24,LE,0);
				Var *Mb_sApiD0, *Mb_sApiD1;
				for(BasicBlock::OutIterator edge(bb);edge;edge++) {

					if(edge->kind()==Edge::TAKEN) {
						NEW_VAR_FROM_BUFF(Mb_sApiD1, "m" << bb->number() << "_" << edge->target()->number() << "A" << BitSet_to_String(v[i]->getHistory()));						
						P23->addLeft(1,Mb_sApiD1);
						P24->addLeft(1,Mb_sApiD1);
					}
					else {
						NEW_VAR_FROM_BUFF(Mb_sApiD0, "m" << bb->number() << "_" << edge->target()->number() << "A" << BitSet_to_String(v[i]->getHistory()));						
						P21->addLeft(1,Mb_sApiD0);
						P22->addLeft(1,Mb_sApiD0);
					}
				}
				// exit
				if(v[i]->isExit() && v[i]->exitsWithT()) {
					Var *XbApiD1End;
					NEW_VAR_FROM_BUFF(XbApiD1End,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "D1end");
					P23->addRight(1,XbApiD1End);
				}
				if(v[i]->isExit() && v[i]->exitsWithNT()) {
					Var *XbApiD0End;
					NEW_VAR_FROM_BUFF(XbApiD0End,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "D0end");
					P21->addRight(1,XbApiD0End);							
				}

				//entry
				if(v[i]->isEntry() && this->mitraInit->contains(0)) {
					Var *XbApiD1Start;
					NEW_VAR_FROM_BUFF(XbApiD1Start,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "start");
					P22->addRight(1,XbApiD1Start);
				}
				if(v[i]->isEntry() && !this->mitraInit->contains(0)) {
					Var *XbApiStart;
					NEW_VAR_FROM_BUFF(XbApiStart,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "start");
					P24->addRight(1,XbApiStart);
				}

				for(BCG::OutIterator s(v[i]);s;s++) {
					if(s->isTaken()) {
						Var *XbApiD1Ss;
						NEW_VAR_FROM_BUFF(XbApiD1Ss,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "D1S" << s->target()->getCorrespondingBBNumber());								
						P23->addRight(1,XbApiD1Ss);
					}
					else {
						Var *XbApiD0Ss;
						NEW_VAR_FROM_BUFF(XbApiD0Ss,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "D0S" << s->target()->getCorrespondingBBNumber());															
						P21->addRight(1,XbApiD0Ss);
					}
				}

				for(BCG::InIterator p(v[i]);p;p++) {
					Var *Xp=ipet::VAR(getBB(p->source()->getCorrespondingBBNumber(),cfg));
					if(p->isTaken()) {
						Var *XpApiD1Sb;
						NEW_VAR_FROM_BUFF(XpApiD1Sb,Xp->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "D1S" << bb->number());
						P22->addRight(1,XpApiD1Sb);
					}
					else {
						Var *XpApiD0Sb;
						NEW_VAR_FROM_BUFF(XpApiD0Sb,Xp->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "D0S" << bb->number());
						P24->addRight(1,XpApiD0Sb);
					}
				}
				
			}
		}
	}
#endif
	
	
	///////
	// P3:
	///////
#if P3_1b>0
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		int cpt_e =0;
		for(BasicBlock::OutIterator edge(bb);edge;edge++) {
			if(edge->kind()==Edge::TAKEN || edge->kind() == Edge::NOT_TAKEN) cpt_e++;
		}
		if(cpt_e==2) { // branchement 
			for(BasicBlock::OutIterator edge(bb);edge;edge++) {
				NEW_SPECIAL_CONSTRAINT(P3,EQ,0);
				if(edge->kind()==Edge::TAKEN) {
					Var *Mb_s;
					NEW_VAR_FROM_BUFF(Mb_s,"m" << bb->number() << "_" << edge->target()->number() );
					P3->addLeft(1,Mb_s);
					if(BB_classes.exists(bb)) {
						elm::genstruct::Vector<BCGNode*> v = BB_classes.get(bb);
						for(int i = 0 ; i<v.length();++i) {
							Var *Mb_sApi;
							NEW_VAR_FROM_BUFF(Mb_sApi,Mb_s->name() << "A" << BitSet_to_String(v[i]->getHistory()));
							P3->addRight(1,Mb_sApi);
						}
					}
					
					system->addObjectFunction(5.0, Mb_s);
				}
				else {
					Var *Mb_s;
					NEW_VAR_FROM_BUFF(Mb_s,"m" << bb->number() << "_" << edge->target()->number() );
					P3->addLeft(1,Mb_s);
					if(BB_classes.exists(bb)) {
						elm::genstruct::Vector<BCGNode*> v = BB_classes.get(bb);
						for(int i = 0 ; i<v.length();++i) {
							Var *Mb_sApi;
							NEW_VAR_FROM_BUFF(Mb_sApi,Mb_s->name() << "A" << BitSet_to_String(v[i]->getHistory()));
							P3->addRight(1,Mb_sApi);
						}
					}
					system->addObjectFunction(5.0, Mb_s);
				}
			}			
		}

	}	
#endif

	/////////////
	// H5_mitra:
	/////////////
#if H5_mitra>0
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		if(BB_classes_BBHG.exists(bb)) {
			Var *Xb=ipet::VAR(bb);
			ASSERT(Xb);
			
		
			elm::genstruct::Vector<BBHGNode*> v=BB_classes_BBHG.get(bb);
	
			for(unsigned int i=0;i<v.length();++i) {
				for(BasicBlock::OutIterator edge(v[i]->getCorrespondingBB());edge;edge++) {
					NEW_SPECIAL_CONSTRAINT(H5,LE,0);
					Var *XbApi, *MbApi;
					NEW_VAR_FROM_BUFF(XbApi,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()));
					NEW_VAR_FROM_BUFF(MbApi,"m" << edge->source()->number() << "_" << edge->target()->number() << "A" << BitSet_to_String(v[i]->getHistory()));
					H5->addRight(1,XbApi);
					H5->addLeft(1,MbApi);
				}
			}

		}
	}
#endif
}

/**
 * Generate the constraints system according to the "global 1bit" method, only history-based constraints.
 * 
 * @param fw		Current Workspace.
 * @param cfg		CFG.
 * @param bhg		"Branch History Graph".
 * @param bbhg		"BasicBlock History Graph".
 * @param bcgs		Vector containg the BCGs (Branch Conflict Graphs) generated from the BHG.
 * @param ht_vars	Hash Table used to ensure unicity of the variables.
 */
void BPredProcessor::CS__Global1b_mitra(WorkSpace *fw, CFG *cfg, BBHG* bbhg,HashTable<String ,Var*>& ht_vars) {
	// Recuperation de l'ensemble des contraintes
	System *system = ipet::SYSTEM(fw);
	ASSERT(system);

	
	HashTable<BasicBlock*, elm::genstruct::Vector<BBHGNode*> > BB_classes;
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		elm::genstruct::Vector<BBHGNode*> v;
		for(BBHG::Iterator node(bbhg);node;node++) {
			if(node->getCorrespondingBB()->number() == bb->number()) {
				v.add(node);
			}
		}
		BB_classes.add(bb,v);
	}
	
	
	/////////////
	// H1_mitra:
	/////////////
#if H1_mitra>0
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		NEW_SPECIAL_CONSTRAINT(H1,EQ,0);
		Var *Xb=ipet::VAR(bb);
		ASSERT(Xb);
		
		H1->addLeft(1,Xb);
		
		if(BB_classes.exists(bb)) {
			elm::genstruct::Vector<BBHGNode*> v=BB_classes.get(bb);
	
			for(unsigned int i=0;i<v.length();++i) {
				Var *XbApi;
				NEW_VAR_FROM_BUFF(XbApi,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()));
				H1->addRight(1,XbApi);
			}

		}
	}
#endif



	
	
	/////////////
	// H41_mitra:
	/////////////
#if H41_mitra>0
	for(CFG::BBIterator bb(cfg);bb;bb++) {

		if(BB_classes.exists(bb)) {
			Var *Xb=ipet::VAR(bb);
			ASSERT(Xb);

			elm::genstruct::Vector<BBHGNode*> v=BB_classes.get(bb);
	
			for(unsigned int i=0;i<v.length();++i) {
				NEW_SPECIAL_CONSTRAINT(H41,EQ,0);
//				HashTable<Var* , Var*> hist_done;
				Var *XbApi;
				NEW_VAR_FROM_BUFF(XbApi,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()));
				
				H41->addLeft(1,XbApi);
				if(v[i]->isEntry()) {
					Var *XpApi;
					NEW_VAR_FROM_BUFF(XpApi,"estart" << "_" << bb->number() << "A" << BitSet_to_String(v[i]->getHistory()));
					H41->addRight(1,XpApi);
				}
				for(BBHG::InIterator p(v[i]);p;p++) {

					Var *Xp=ipet::VAR(p->source()->getCorrespondingBB());
					ASSERT(Xp);
					
					Var *XpApi;
					NEW_VAR_FROM_BUFF(XpApi,"e" << p->source()->getCorrespondingBB()->number() << "_" << bb->number() << "A" << BitSet_to_String(p->source()->getHistory()));
//					if(!hist_done.exists(XpApi)) {
						H41->addRight(1,XpApi);
//						hist_done.add(XpApi,XpApi);
//					}
				}
			}
		}
	}
#endif

	
	/////////////
	// H42_mitra:
	/////////////
#if H42_mitra>0
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		if(BB_classes.exists(bb)) {
			Var *Xb=ipet::VAR(bb);
			ASSERT(Xb);

			elm::genstruct::Vector<BBHGNode*> v=BB_classes.get(bb);
	
			for(unsigned int i=0;i<v.length();++i) {
				NEW_SPECIAL_CONSTRAINT(H42,EQ,0);
				Var *XbApi;
				NEW_VAR_FROM_BUFF(XbApi,Xb->name() << "A" << BitSet_to_String(v[i]->getHistory()));
				H42->addLeft(1,XbApi);
				if(v[i]->isExit()) {
					Var *XpApi;
					NEW_VAR_FROM_BUFF(XpApi,"e" << v[i]->getCorrespondingBB()->number() << "_end" << "A" << BitSet_to_String(v[i]->getHistory()));
					H42->addRight(1,XpApi);
				}
				for(BBHG::OutIterator s(v[i]);s;s++) {
					Var *Xs=ipet::VAR(s->target()->getCorrespondingBB());
					ASSERT(Xs);
					
					Var *XsApi;
					NEW_VAR_FROM_BUFF(XsApi,"e" << bb->number() << "_" << s->target()->getCorrespondingBB()->number() << "A" << BitSet_to_String(v[i]->getHistory()));
					H42->addRight(1,XsApi);
					
				}
			}
		}
	}
#endif
	
	/////////////
	// H0_mitra:
	/////////////
#if H0_mitra>0
	{
		NEW_SPECIAL_CONSTRAINT(H0start,EQ,1);
		NEW_SPECIAL_CONSTRAINT(H0end,EQ,1);
		Var *estart, *eend;
		NEW_VAR_FROM_BUFF(estart,"estart_" << cfg->entry()->number());
		H0start->addLeft(1,estart);
		NEW_VAR_FROM_BUFF(eend,"e" <<  cfg->exit()->number() << "_end");
		H0end->addLeft(1,eend);
	}
#endif	
	
	/////////////
	// H2_mitra:
	/////////////
#if H2_mitra>0
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		for(BasicBlock::OutIterator edge(bb);edge;edge++) {
			NEW_SPECIAL_CONSTRAINT(H2,EQ,0);
			Var *eb_s=ipet::VAR(edge);
			ASSERT(eb_s);
			H2->addLeft(1,eb_s);
			
			
			if(BB_classes.exists(bb)) {
				elm::genstruct::Vector<BBHGNode*> v=BB_classes.get(bb);			
				HashTable<Var* ,Var*> hist_done;					
				for(unsigned int i = 0 ; i<v.length();++i) {
					for(BBHG::OutIterator s(v[i]);s;s++) {
						if(s->target()->getCorrespondingBB()->number() == edge->target()->number()) {
							Var *eb_sApi;
							NEW_VAR_FROM_BUFF(eb_sApi,"e" << bb->number() << "_" << s->target()->getCorrespondingBB()->number() << "A" << BitSet_to_String(v[i]->getHistory()));
							if(!hist_done.exists(eb_sApi)) {
								H2->addRight(1,eb_sApi);
								hist_done.add(eb_sApi,eb_sApi);
							}
						}
					}
				}
				
			}
		}
	}
#endif
	
}

/**
 * Checks if a BasicBlock is a branch.
 * 
 * @param bb	BasicBlock to check.
 * 
 * @return Boolean value set to true if the given BasicBlock is a branch.
 */
bool BPredProcessor::isBranch(BasicBlock* bb) {
	int nb_edges=0;
	for(BasicBlock::OutIterator edge(bb);edge;edge++)
		if(edge->kind()==Edge::TAKEN || edge->kind()==Edge::NOT_TAKEN)
			nb_edges++;
	
	return nb_edges==2;
}

/**
 * Tests if a similar BBHG Node is contained in a given vector of BHG Nodes and returns the BBHG Node found.
 * Two BBHG Nodes are similar if they have same hisotry and comes from the same BasicBlock.
 * 
 * @param v			Vector containing the BHG Nodes.
 * @param n			BBHG Node to compare with those contained in the vector.
 * @param contained If any similar BBHG Node is found, then it will contains the found BBHG Node, else it will contains a NULL pointer.
 * 
 * @return returns a boolean value set to True if a similar BBHG Node has been found.
 */
bool BPredProcessor::contains(const elm::genstruct::Vector< BBHGNode* >& v, BBHGNode& n, BBHGNode * &contained) {
	for(unsigned int i=0;i<v.length();++i) {
		if(n.equals(*(v[i])) ) {
			contained = v[i];
			return true;
		}
	}
	contained = NULL;
	return false;
}

/**
 * Creates a BBHG from a given CFG.
 * 
 * @param cfg	CFG to generate the BBHG from.
 * @param bbhg	Reference to the BBHG to fill.
 */
void BPredProcessor::generateBBHG(CFG* cfg,BBHG& bbhg) {
	elm::genstruct::Vector< BBHGNode* > final_nodes; 	// => S dans l'algo
	elm::genstruct::Vector< BBHGNode* > todo_nodes; 	// => F dans l'algo
	elm::genstruct::Vector< BBHGEdge* > final_edges;	// => E dans l'algo

	// creation du BitSet unité
	dfa::BitSet unite(this->BHG_history_size);
	unite.add(0);
	///////////////////////////
	
	BasicBlock* entry=cfg->entry();
	int position=0;
	dfa::BitSet h(*this->mitraInit);
//	while(true) {
		BBHGNode *n= new BBHGNode(entry,h,isBranch(entry),true);

		BBHGNode *tmp;
		if(!contains(final_nodes,*n,tmp)) {
			final_nodes.add(n);
			todo_nodes.add(n);
			bbhg.add(n);
	
			while(!todo_nodes.isEmpty()) {
				BBHGNode *courant=todo_nodes.pop();
				elm::genstruct::Vector<BBHGNode *> suivants;
	
////////////////////
				bool isBr=isBranch(courant->getCorrespondingBB());
				for(BasicBlock::OutIterator edge(courant->getCorrespondingBB());edge;edge++) {
					BBHGNode *n;
					if(isBr) {
						if(edge->kind() == Edge::TAKEN) {
							n=new BBHGNode(edge->target(),lshift_BitSet(courant->getHistory(),1,true),true,edge->target()==cfg->entry());
						}
						else {
							n=new BBHGNode(edge->target(),lshift_BitSet(courant->getHistory(),1,false),true,edge->target()==cfg->entry());
						}
					}
					else {
						n=new BBHGNode(edge->target(),courant->getHistory(),true,edge->target()==cfg->entry());
					}
					suivants.add(n);
				}
////////////////////

				for(unsigned int i=0;i<suivants.length();++i) {
					BBHGNode *s = suivants[i];
					if(s!=NULL) {
						BBHGNode *cs;
						bool todo_n=false, final_n=false;
						if(!(final_n=contains(final_nodes,*s,cs))) {
							final_nodes.add(s);
							bbhg.add(s);
							
							todo_nodes.add(s);
							BBHGEdge *e = new BBHGEdge(courant,s,s->getHistory().contains(0),isBr);
							final_edges.add(e);
	
						}
						else {
							BBHGEdge *e = new BBHGEdge(courant,cs,cs->getHistory().contains(0),isBr);
							final_edges.add(e);
							delete s; // cree par suivant
						}

					}
				}
			}
		}
		else delete n;

		// SORTIE
//		if(h.isFull()) break;
		/////////
		
		
//		historyPlusOne(h);
		
//	}


	// Rajout des sorties
	for(BBHG::Iterator bb(&bbhg); bb; bb++){
			bb->setExit(bb->getCorrespondingBB()==cfg->exit(),false,false);
	}

}


/**
 * Creates all the required graphs and structures to create the constraints system based on the "global 1bit" model.
 * 
 * @param ws	Current workspace.
 * @param cfg	CFG to extracts the graphs and structures from.
 */
void BPredProcessor::processCFG__Global1B(WorkSpace *ws,CFG* cfg) {
	BBHG bbhg(this->BHG_history_size);
	generateBBHG(cfg,bbhg);
	
	PropList props;
	if(this->dumpBBHG) {
		StringBuffer buf;
		buf << "bbhg.ps";
		String filename = buf.toString();
		otawa::display::OUTPUT_PATH(props) = filename.toCString();
		BBHGDrawer drawer(&bbhg, props);
		drawer.display();
	}
	HashTable<String ,Var*> ht_vars;

	BHG bhg(this->BHG_history_size);

	generateBHG(cfg,bhg);
	if(this->dumpBHG) {
		StringBuffer buf;
		buf << "bhg.ps";
		String filename = buf.toString();
		otawa::display::OUTPUT_PATH(props) = filename.toCString();
		BHGDrawer drawer(&bhg, props);
		drawer.display();
	}
	
	elm::genstruct::Vector<BCG*> bcgs;
	generateBCGs(bcgs,bhg);

	if(this->withMitra)
		CS__Global1b_mitra(ws,cfg,&bbhg,ht_vars);
	else
		CS__Global2b_not_mitra(ws,cfg,&bhg,bcgs,ht_vars);
	
	
	


	CS__Global1b(ws,cfg,&bhg, &bbhg, bcgs,ht_vars);
	
#if CLAIRE_INFOS > 0
	int nbA,nbB,nbPI;
	int maxB,maxA;
	
	
	// Nombre d'aretes dans le CFG
	nbA=0;
	for(CFG::BBIterator bb(cfg);bb;bb++)
		for(BasicBlock::OutIterator e(bb);e;e++) nbA++;
	cerr << "A_1b = " << nbA << "\n";
	
	// Nombre d'aretes dans le BBHG
	nbA=0;
	for(BBHG::NodeIterator node(&bbhg);node;node++)
		for(BBHG::Successor s(node);s;s++) nbA++;
	cerr << "B_1b = " << nbA << "\n";
	
	// Nombre de blocs dans le BBHG
	nbB=0;
	for(BBHG::NodeIterator node(&bbhg);node;node++) nbB++;
	cerr << "C_1b = " << nbB << "\n";
	
	// Nombre max de PI possibles pour un bloc du CFG.
	maxB=0;
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		nbB=0;
		for(BBHG::NodeIterator node(&bbhg);node;node++)
			if(node->getCorrespondingBB()->number() == bb->number()) {
				nbB++;
			}
		if(maxB<nbB)maxB=nbB;
	}
	cerr << "D_1b = " << maxB << "\n";
	
	// Nombre de blocs du CFG qui ne sont pas des branchements
	// Nombre de blocs du BBHG qui ne sont pas des branchements
	nbB=0;
	nbPI=0;
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		if(!isBranch(bb)) {
			nbB++;
			for(BBHG::NodeIterator node(&bbhg);node;node++)
				if(node->getCorrespondingBB()->number() == bb->number()) nbPI++;
		}
	}
	cerr << "E_1b = " << nbB << "\n"
		 << "F_1b = " << nbPI << "\n";
	
	// Nombre max de predecesseurs pour un bloc du CFG
	maxB=0;
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		nbB=0;
		for(BasicBlock::InIterator e(bb);e;e++) {
			nbB++;
		}
		if(nbB>maxB) maxB=nbB;
	}
	cerr << "G_1b = " << maxB << "\n";
	
	// Nombre max de PI possibles par arete.
	maxA=0;
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		for(BasicBlock::OutIterator e(bb);e;e++) {
			BasicBlock* tgt = e->target();
			nbA=0;
			for(BBHG::NodeIterator n(&bbhg);n;n++) {
				if(n->getCorrespondingBB()->number()==bb->number()) {
					for(BBHG::Successor s(n);s;s++) {
						if(s->getCorrespondingBB()->number()==tgt->number()) {
							nbA++;
						}
					}
				}
			}
			if(nbA>maxA)maxA=nbA;
		}
	}
	cerr << "H_1b = " << maxA << "\n";
	
	// Somme de tous les successeurs des branchements dans les BCG
	nbB=0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			for(BCG::Successor s(n);s;s++) nbB++;
		}
	}
	cerr << "I_1b = " << nbB << "\n";
	
	// Nombre max de PI possibles pour un branchement du CFG
	maxB=0;
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		if(isBranch(bb)) {
			nbB=0;
			for(BBHG::NodeIterator node(&bbhg);node;node++)
				if(node->getCorrespondingBB()->number() == bb->number()) {
					nbB++;
				}
			if(maxB<nbB)maxB=nbB;
		}
	}
	cerr << "J_1b = " << maxB << "\n";

	// Somme de tous les predecesseurs des branchements dans les BCG
	nbB=0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			for(BCG::Predecessor s(n);s;s++) nbB++;
		}
	}
	cerr << "K_1b = " << nbB << "\n";

	// Max de preds pour un branchement dans un BCG
	maxB = 0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			nbB=0;
			for(BCG::Predecessor p(n);p;p++) nbB++;
			if(n->isEntry())nbB++;
			if(nbB>maxB) maxB=nbB;
		}
	}
	cerr <<  "L_1b = " << maxB << "\n";

	// Nombre de branchements dans le CFG
	nbB=0;
	for(CFG::BBIterator bb(cfg);bb;bb++)
		if(isBranch(bb)) nbB++;
	cerr << "M_1b = " << nbB << "\n";
	
	///// Sum de tous les Branchements en sortie dans les BCG (avec pi differents)
	nbB=0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			if(n->isExit()) nbB++;
		}
	}
	cerr << "N_2b = " << nbB << "\n";
	
	///// Sum de tous les Branchements en entree dans les BCG (avec pi differents)
	nbB=0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			if(n->isEntry()) nbB++;
		}
	}
	cerr << "O_2b = " << nbB << "\n";
	
	// P_1b
	nbPI=0;
	elm::genstruct::HashTable<String,void*> ht;
	for(BBHG::NodeIterator node(&bbhg);node;node++)
		if(!ht.exists(BitSet_to_String(node->getHistory()))) {
			nbPI++;
			ht.add(BitSet_to_String(node->getHistory()),NULL);
		}
	cerr << "P_1b = " << nbPI << "\n";
	
	// Q_1b
	maxB=0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			if(n->isExit()) {
				nbB=0;
				for(int j=0;i<bcgs.length();i++) {
					for(BCG::NodeIterator m(bcgs[i]);m;m++) {
						if(n->getCorrespondingBBNumber() == m->getCorrespondingBBNumber() && m->isExit()) {
							nbB++;
						}
					}
				}
				if(maxB<nbB)maxB=nbB;
			}
		}
	}
	cerr << "Q_1b = " << maxB << "\n";

	// R_1b
	maxB=0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			if(n->isEntry()) {
				nbB=0;
				for(int j=0;i<bcgs.length();i++) {
					for(BCG::NodeIterator m(bcgs[i]);m;m++) {
						if(n->getCorrespondingBBNumber() == m->getCorrespondingBBNumber() && m->isEntry()) {
							nbB++;
						}
					}
				}
				if(maxB<nbB)maxB=nbB;
			}
		}
	}
	cerr << "R_1b = " << maxB << "\n";

	// S_1b
	maxB=0;
	for(BBHG::NodeIterator n(&bbhg);n;n++) {
		nbB=0;
		for(BBHG::Predecessor p(n);p;p++) nbB++;
		if(maxB<nbB)maxB=nbB;
	}
	cerr << "S_1b = " << maxB << "\n";
#endif
}

} } // otawa::bpred

