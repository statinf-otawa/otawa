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
#include "BHGDrawer.h"
#include "BBHGDrawer.h"
#include "BCGDrawer.h"
#include "BCG.h"
#include "BHG.h"
#include "BBHG.h"

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
		ASSERT(cons_name);

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



#define GLOB 1
#define B_1   0+GLOB
#define B_21  0+GLOB
#define B_22  0+GLOB
#define B_23  0+GLOB
#define B_24  0+GLOB
#define B_25  0+GLOB
#define H_11  0+GLOB
#define H_12  0+GLOB
#define H_41  0+GLOB
#define H_42  0+GLOB
#define H_2_3 0+GLOB
#define G_1   0+GLOB

#define MITRA 0



/**
 * Generate the constraints system according to the "global 2bits" method, without history-based constraints.
 * 
 * @param fw		Current Workspace.
 * @param cfg		CFG.
 * @param bhg		"Branch History Graph".
 * @param graphs	Vector containg the BCGs (Branch Conflict Graphs) generated from the BHG.
 * @param ht_vars	Hash Table used to ensure unicity of the variables.
 */
void BPredProcessor::CS__Global2b(WorkSpace *fw, CFG *cfg, BHG* bhg, elm::genstruct::Vector<BCG*> &graphs ,	HashTable<String ,Var*>& ht_vars) {
	// Recuperation de l'ensemble des contraintes
	System *system = ipet::SYSTEM(fw);
	ASSERT(system);

	HashTable<BasicBlock*,elm::genstruct::Vector<BCGNode*> > classes_of_BB;

	// creation des classes d'appartenance des branchements A PARTIR DU BHG (indirectement depuis les BCG)
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		elm::genstruct::Vector<BCGNode*> list_nodes;
		
		for(unsigned int i = 0 ; i< graphs.length();++i) {
			BCG* g=graphs[i];
			for(BCG::Iterator n(g);n;n++)
				if(n->getCorrespondingBBNumber() == bb->number()) { list_nodes.add(n);break; }
		}
		
		if(list_nodes.length()>0) classes_of_BB.add(bb,list_nodes);
	}


	
#if B_1 > 0
	///////////////////////////////////////////////////
	//B1: Boucle sur toutes les classes de branchement
	/////////////////////////////////////////////////// 
	for(unsigned int i=0 ; i<graphs.length();++i) {
		
		// on recupere le BCG correspondant a la classe en cours
		BCG* bcg=graphs[i];
		NEW_SPECIAL_CONSTRAINT(Cs,LE,1);
		NEW_SPECIAL_CONSTRAINT(Ce,LE,1);

		// Boucle sur les BB de la classe addr
		for(BCG::Iterator n(bcg);n;n++) {
			// Recuperation de la variable associee au BB
			BasicBlock* bb = getBB(n->getCorrespondingBBNumber(),cfg);
			ASSERT(bb);
			Var *Xi = ipet::VAR(bb);
			ASSERT(Xi);

			if(n->isEntry()) {
				Var *C00s, *C01s, *C10s, *C11s;
				NEW_VAR_FROM_BUFF(C00s,Xi->name() << "A" << BitSet_to_String(bcg->getHistory()) << "C00start");
				NEW_VAR_FROM_BUFF(C01s,Xi->name() << "A" << BitSet_to_String(bcg->getHistory()) << "C01start");
				NEW_VAR_FROM_BUFF(C10s,Xi->name() << "A" << BitSet_to_String(bcg->getHistory()) << "C10start");
				NEW_VAR_FROM_BUFF(C11s,Xi->name() << "A" << BitSet_to_String(bcg->getHistory()) << "C11start");
				Cs->addLeft(1,C00s);
				Cs->addLeft(1,C01s);
				Cs->addLeft(1,C10s);
				Cs->addLeft(1,C11s);
			}
			
			if(n->isExit()) {
				Var *C00e, *C01e, *C10e, *C11e;
				NEW_VAR_FROM_BUFF(C00e,Xi->name() << "A" << BitSet_to_String(bcg->getHistory()) << "C00end");
				NEW_VAR_FROM_BUFF(C01e,Xi->name() << "A" << BitSet_to_String(bcg->getHistory()) << "C01end");
				NEW_VAR_FROM_BUFF(C10e,Xi->name() << "A" << BitSet_to_String(bcg->getHistory()) << "C10end");
				NEW_VAR_FROM_BUFF(C11e,Xi->name() << "A" << BitSet_to_String(bcg->getHistory()) << "C11end");
				Ce->addLeft(1,C00e);
				Ce->addLeft(1,C01e);
				Ce->addLeft(1,C10e);
				Ce->addLeft(1,C11e);
			}
		}
	} /// B1
#endif	


	///////////////////////////////////////////////
	// 2: Boucle sur tous les noeuds du CFG
	///////////////////////////////////////////////
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		// Recuperation de la variable associee au BB
		Var *Xi = ipet::VAR(bb);
		ASSERT(Xi);

#if G_1 > 0
		{
			if(classes_of_BB.exists(bb)) {
				NEW_SPECIAL_CONSTRAINT(M_T,EQ,0);
				NEW_SPECIAL_CONSTRAINT(M_NT,EQ,0);
				for(BasicBlock::OutIterator edge(bb); edge ; edge++ ) {
					Var *m0, *m1;
					if(edge->kind() == Edge::TAKEN) { // WARNING ces accolades sont IMPERATIVES car NEW_VAR_FROM_BUFF definit un bloc
						NEW_VAR_FROM_BUFF(m1,	"m" << bb->number() << "_" << edge->target()->number() );
						system->addObjectFunction(5.0, m1);
						M_T->addLeft(1,m1);
						elm::genstruct::Vector<BCGNode*> v = classes_of_BB.get(bb);
						for(unsigned int i = 0 ; i < v.length();++i) {
							Var *m;
							NEW_VAR_FROM_BUFF(m,"m" << bb->number() << "_" << edge->target()->number() << "A" << BitSet_to_String(v[i]->getHistory()))
							M_T->addRight(1,m);
						}
					}
					else if(edge->kind() == Edge::NOT_TAKEN) {// WARNING ces accolades sont IMPERATIVES car NEW_VAR_FROM_BUFF definit un bloc
						NEW_VAR_FROM_BUFF(m0,	"m" << bb->number() << "_" << edge->target()->number() );
						system->addObjectFunction(5.0, m0);
						M_NT->addLeft(1,m0);
						elm::genstruct::Vector<BCGNode*> v = classes_of_BB.get(bb);
						for(unsigned int i = 0 ; i < v.length();++i) {
							Var *m;
							NEW_VAR_FROM_BUFF(m,"m" << bb->number() << "_" << edge->target()->number() << "A" << BitSet_to_String(v[i]->getHistory()))
							M_NT->addRight(1,m);
						}
					}
				}
			}
		}		
#endif
		
#if B_21>0
		if(classes_of_BB.exists(bb)) {
			elm::genstruct::Vector<BCGNode*> v = classes_of_BB.get(bb);
			if(v.length()>0) {
				//////////////////////////////////////////
				// 2.1: Pour chacun des successeurs de br
				//////////////////////////////////////////
				// recherche des membres de gauche à ajouter
				for( int cpt=0 ; cpt<2; cpt++ ) {
					Edge::kind_t t = (cpt == 1) ? (Edge::TAKEN) : (Edge::NOT_TAKEN);
					NEW_SPECIAL_CONSTRAINT(B21,LE,0); 
		
		
			
					// on recherche l'edge ayant le meme 'd' que celui du bcg pour en extraire le nom de variable associe
					Var *eb = NULL;
					for(BasicBlock::OutIterator edge(bb); edge ; edge++) {
						if(edge->kind() == t ) {
							eb = ipet::VAR(edge);
							ASSERT(edge);
							break;
						}
					}
		
					// ajout du membre de droite
					B21->addRight(1,eb);
		
					for(unsigned int i=0;i<v.length();++i) {
						BCGNode* br=v[i];
						for(BCG::OutIterator s(br);s;s++) {
							if( s->isTaken() == (t == Edge::TAKEN) ) {
								Var *C00,*C01,*C10,*C11;
								NEW_VAR_FROM_BUFF(C00,Xi->name() << "A" << BitSet_to_String(s->target()->getHistory()) << "C00D" << cpt << "S" << s->target()->getCorrespondingBBNumber());
								NEW_VAR_FROM_BUFF(C01,Xi->name() << "A" << BitSet_to_String(s->target()->getHistory()) << "C01D" << cpt << "S" << s->target()->getCorrespondingBBNumber());
								NEW_VAR_FROM_BUFF(C10,Xi->name() << "A" << BitSet_to_String(s->target()->getHistory()) << "C10D" << cpt << "S" << s->target()->getCorrespondingBBNumber());
								NEW_VAR_FROM_BUFF(C11,Xi->name() << "A" << BitSet_to_String(s->target()->getHistory()) << "C11D" << cpt << "S" << s->target()->getCorrespondingBBNumber());
								B21->addLeft(1,C00);
								B21->addLeft(1,C01);
								B21->addLeft(1,C10);
								B21->addLeft(1,C11);
							}
						}
						
						if(br->isExit() && ( (br->exitsWithT() == (t == Edge::TAKEN) ) || (br->exitsWithNT() == (t == Edge::NOT_TAKEN) ) )){
							Var *C00,*C01,*C10,*C11;
							NEW_VAR_FROM_BUFF(C00,Xi->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "C00D" << cpt << "end");
							NEW_VAR_FROM_BUFF(C01,Xi->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "C01D" << cpt << "end");
							NEW_VAR_FROM_BUFF(C10,Xi->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "C10D" << cpt << "end");
							NEW_VAR_FROM_BUFF(C11,Xi->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "C11D" << cpt << "end");
							B21->addLeft(1,C00);
							B21->addLeft(1,C01);
							B21->addLeft(1,C10);
							B21->addLeft(1,C11);
							
						}
					}
				}
			} 
		} /// fin B2.1
#endif



		if(classes_of_BB.exists(bb)) {
			elm::genstruct::Vector<BCGNode*> v = classes_of_BB.get(bb);
			if(v.length()>0) {
				for(unsigned int i=0;i<v.length();++i) {
					BCGNode *br = v[i];
	#if B_22>0
					/////////////////////////////////////////////////
					// 2.2: Tous les {Succ. ; end} / {Pred. ; start}
					/////////////////////////////////////////////////
					{
						// il faut s'assurer qu'on n'ajoute qu'une seule fois chaque variable 
						// en creant un tableau d'indicateurs et en l'initialisant a 0
						int var_added[cfg->countBB()];
						for(int i=0;i<cfg->countBB();++i) var_added[i]=0;
			
			
						
						NEW_SPECIAL_CONSTRAINT(B22_pred,EQ,0);
						Var *Xb_Api;
						NEW_VAR_FROM_BUFF(Xb_Api,Xi->name() << "A" << BitSet_to_String(br->getHistory()) );
						Var* v_pred;
						B22_pred->addLeft(1,Xb_Api);
						
						// Recherche de tous les predecesseurs de br
						for(BCG::InIterator p(br) ; p ; p++ ) {
							if(var_added[p->source()->getCorrespondingBBNumber()]==0) {
								Var *C00,*C01,*C10,*C11;
								BasicBlock* bb_pred=getBB(p->source()->getCorrespondingBBNumber(), cfg);
								Var *Xj = ipet::VAR( bb_pred);
								ASSERT(Xj);
								NEW_VAR_FROM_BUFF(C00,Xj->name() << "A" << BitSet_to_String(br->getHistory()) << "C00S" << br->getCorrespondingBBNumber());
								NEW_VAR_FROM_BUFF(C01,Xj->name() << "A" << BitSet_to_String(br->getHistory()) << "C01S" << br->getCorrespondingBBNumber());
								NEW_VAR_FROM_BUFF(C10,Xj->name() << "A" << BitSet_to_String(br->getHistory()) << "C10S" << br->getCorrespondingBBNumber());
								NEW_VAR_FROM_BUFF(C11,Xj->name() << "A" << BitSet_to_String(br->getHistory()) << "C11S" << br->getCorrespondingBBNumber());
								B22_pred->addRight(1,C00);
								B22_pred->addRight(1,C01);
								B22_pred->addRight(1,C10);
								B22_pred->addRight(1,C11);
								
								var_added[p->source()->getCorrespondingBBNumber()] ++;
							}
						}
						// s'il s'agit d'une entrée on ajoute les variables correspondantes
						if(br->isEntry()) {
							Var *C00,*C01,*C10,*C11;
							NEW_VAR_FROM_BUFF(C00,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00start");
							NEW_VAR_FROM_BUFF(C01,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01start");
							NEW_VAR_FROM_BUFF(C10,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10start");
							NEW_VAR_FROM_BUFF(C11,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11start");
							B22_pred->addRight(1,C00);
							B22_pred->addRight(1,C01);
							B22_pred->addRight(1,C10);
							B22_pred->addRight(1,C11);					
						}
			
						
						// on réinitialise le tableau d'incdicateurs pour les successeurs
						for(int i=0;i<cfg->countBB();++i) var_added[i]=0;
						
						NEW_SPECIAL_CONSTRAINT(B22_succ,EQ,0);
						Var *v_succ;
						B22_succ->addLeft(1,Xb_Api);
						// Recherche de tous les successeurs de br
						for(BCG::OutIterator s(br) ; s ; s++ ) {
							if(var_added[s->target()->getCorrespondingBBNumber()]==0) {
								Var *C00,*C01,*C10,*C11;
								NEW_VAR_FROM_BUFF(C00,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00S" << s->target()->getCorrespondingBBNumber());
								NEW_VAR_FROM_BUFF(C01,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01S" << s->target()->getCorrespondingBBNumber());
								NEW_VAR_FROM_BUFF(C10,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10S" << s->target()->getCorrespondingBBNumber());
								NEW_VAR_FROM_BUFF(C11,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11S" << s->target()->getCorrespondingBBNumber());
								B22_succ->addRight(1,C00);
								B22_succ->addRight(1,C01);
								B22_succ->addRight(1,C10);
								B22_succ->addRight(1,C11);
								
								var_added[s->target()->getCorrespondingBBNumber()] ++;
							}
						}
						// s'il s'agit d'une sortie on ajoute les variables correspondantes
						if(br->isExit()) {
							Var *C00,*C01,*C10,*C11;
							NEW_VAR_FROM_BUFF(C00,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00end");
							NEW_VAR_FROM_BUFF(C01,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01end");
							NEW_VAR_FROM_BUFF(C10,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10end");
							NEW_VAR_FROM_BUFF(C11,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11end");
							B22_succ->addRight(1,C00);
							B22_succ->addRight(1,C01);
							B22_succ->addRight(1,C10);
							B22_succ->addRight(1,C11);					
						}
						
					} //// Fin B2.2
	#endif
	#if B_23>0
					//////////////////////////////////////////////////////
					// 2.3: Ajout des successeurs et des aretes y menant
					//////////////////////////////////////////////////////
					{
						// on doit s'assurer de l'unicité
						int var_added[cfg->countBB()];
						for(int i=0;i<cfg->countBB();++i) var_added[i]=0;
		
						for(BCG::OutIterator s(br) ; s ; s++ ){
							if(var_added[s->target()->getCorrespondingBBNumber()]==0) {
									
								NEW_SPECIAL_CONSTRAINT(B23_00,EQ,0);
								NEW_SPECIAL_CONSTRAINT(B23_01,EQ,0);
								NEW_SPECIAL_CONSTRAINT(B23_10,EQ,0);
								NEW_SPECIAL_CONSTRAINT(B23_11,EQ,0);
								Var *v00, *v01, *v10, *v11;
			
								NEW_VAR_FROM_BUFF(v00,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00S" << s->target()->getCorrespondingBBNumber());
								NEW_VAR_FROM_BUFF(v01,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01S" << s->target()->getCorrespondingBBNumber());
								NEW_VAR_FROM_BUFF(v10,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10S" << s->target()->getCorrespondingBBNumber());
								NEW_VAR_FROM_BUFF(v11,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11S" << s->target()->getCorrespondingBBNumber());
								B23_00->addLeft(1,v00);
								B23_01->addLeft(1,v01);
								B23_10->addLeft(1,v10);
								B23_11->addLeft(1,v11);
								
								bool withT=false, withNT=false;
								br->isSuccessor(s->target(),withT,withNT);
								if(withT) {
									Var *T00,*T01,*T10,*T11;
									NEW_VAR_FROM_BUFF(T00,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00D1S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(T01,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01D1S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(T10,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10D1S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(T11,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11D1S" << s->target()->getCorrespondingBBNumber());
									B23_00->addRight(1,T00);
									B23_01->addRight(1,T01);
									B23_10->addRight(1,T10);
									B23_11->addRight(1,T11);
								}
								if(withNT) {
									Var *NT00,*NT01,*NT10,*NT11;
									NEW_VAR_FROM_BUFF(NT00,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00D0S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(NT01,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01D0S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(NT10,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10D0S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(NT11,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11D0S" << s->target()->getCorrespondingBBNumber());
									B23_00->addRight(1,NT00);
									B23_01->addRight(1,NT01);
									B23_10->addRight(1,NT10);
									B23_11->addRight(1,NT11);
								}
								
								var_added[s->target()->getCorrespondingBBNumber()] ++;
		
							}
						}
		
						if(br->isExit()) {
		
							NEW_SPECIAL_CONSTRAINT(B23_exit00,EQ,0);
							NEW_SPECIAL_CONSTRAINT(B23_exit01,EQ,0);
							NEW_SPECIAL_CONSTRAINT(B23_exit10,EQ,0);
							NEW_SPECIAL_CONSTRAINT(B23_exit11,EQ,0);
							
							
							Var *v00end, *v01end, *v10end, *v11end;
							NEW_VAR_FROM_BUFF(v00end,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00end");
							NEW_VAR_FROM_BUFF(v01end,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01end");
							NEW_VAR_FROM_BUFF(v10end,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10end");
							NEW_VAR_FROM_BUFF(v11end,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11end");
							B23_exit00->addLeft(1,v00end);
							B23_exit01->addLeft(1,v01end);
							B23_exit10->addLeft(1,v10end);
							B23_exit11->addLeft(1,v11end);
		
							if(br->exitsWithT()) {
								Var *eT00,*eT01,*eT10,*eT11;
								NEW_VAR_FROM_BUFF(eT00,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00D1end");
								NEW_VAR_FROM_BUFF(eT01,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01D1end");
								NEW_VAR_FROM_BUFF(eT10,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10D1end");
								NEW_VAR_FROM_BUFF(eT11,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11D1end");
								B23_exit00->addRight(1,eT00);
								B23_exit01->addRight(1,eT01);
								B23_exit10->addRight(1,eT10);
								B23_exit11->addRight(1,eT11);							
							}
							if(br->exitsWithNT()) {
								Var *eNT00,*eNT01,*eNT10,*eNT11;
								NEW_VAR_FROM_BUFF(eNT00,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00D0end");
								NEW_VAR_FROM_BUFF(eNT01,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01D0end");
								NEW_VAR_FROM_BUFF(eNT10,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10D0end");
								NEW_VAR_FROM_BUFF(eNT11,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11D0end");
								B23_exit00->addRight(1,eNT00);
								B23_exit01->addRight(1,eNT01);
								B23_exit10->addRight(1,eNT10);
								B23_exit11->addRight(1,eNT11);							
							}
						}
		
					} /// Fin B2.3
	#endif
				
	#if B_24>0	
					///////////////////////////////
					// 2.4: ...
					///////////////////////////////
					{
						NEW_SPECIAL_CONSTRAINT(C00_1,EQ,0);
						NEW_SPECIAL_CONSTRAINT(C00_2,EQ,0);
						NEW_SPECIAL_CONSTRAINT(C01_1,EQ,0);
						NEW_SPECIAL_CONSTRAINT(C01_2,EQ,0);
						NEW_SPECIAL_CONSTRAINT(C10_1,EQ,0);
						NEW_SPECIAL_CONSTRAINT(C10_2,EQ,0);
						NEW_SPECIAL_CONSTRAINT(C11_1,EQ,0);
						NEW_SPECIAL_CONSTRAINT(C11_2,EQ,0);
		
						Var *v00, *v01, *v10, *v11;
						NEW_VAR_FROM_BUFF(v00,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00");
						NEW_VAR_FROM_BUFF(v01,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01");
						NEW_VAR_FROM_BUFF(v10,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10");
						NEW_VAR_FROM_BUFF(v11,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11");
						C00_1->addLeft(1,v00);
						C00_2->addLeft(1,v00);
						C01_1->addLeft(1,v01);
						C01_2->addLeft(1,v01);
						C10_1->addLeft(1,v10);
						C10_2->addLeft(1,v10);
						C11_1->addLeft(1,v11);
						C11_2->addLeft(1,v11);
		
						// on doit s'assurer de l'unicité
						int var_added[cfg->countBB()];
						for(int i=0;i<cfg->countBB();++i) var_added[i]=0;
		
						for(BCG::InIterator p(br);p;p++) {
							if(var_added[p->source()->getCorrespondingBBNumber()]==0) {
								bool withT=false, withNT=false;
								
								BasicBlock* bb_pred=getBB(p->source()->getCorrespondingBBNumber(), cfg);
								Var *Xj = ipet::VAR( bb_pred);
								ASSERT(Xj);
								
								p->source()->isSuccessor(br,withT,withNT);
								if(withT) {
									Var *x00d1, *x01d1, *x10d1, *x11d1;
									NEW_VAR_FROM_BUFF(x00d1,Xj->name() << "A" << BitSet_to_String(br->getHistory()) << "C00D1S" << br->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(x01d1,Xj->name() << "A" << BitSet_to_String(br->getHistory()) << "C01D1S" << br->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(x10d1,Xj->name() << "A" << BitSet_to_String(br->getHistory()) << "C10D1S" << br->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(x11d1,Xj->name() << "A" << BitSet_to_String(br->getHistory()) << "C11D1S" << br->getCorrespondingBBNumber());
									C01_1->addRight(1,x00d1);
									C10_1->addRight(1,x01d1);
									C11_1->addRight(1,x10d1);
									C11_1->addRight(1,x11d1);							
								}
								if(withNT) {
									Var *x00d0, *x01d0, *x10d0, *x11d0;
									NEW_VAR_FROM_BUFF(x00d0,Xj->name() << "A" << BitSet_to_String(br->getHistory()) << "C00D0S" << br->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(x01d0,Xj->name() << "A" << BitSet_to_String(br->getHistory()) << "C01D0S" << br->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(x10d0,Xj->name() << "A" << BitSet_to_String(br->getHistory()) << "C10D0S" << br->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(x11d0,Xj->name() << "A" << BitSet_to_String(br->getHistory()) << "C11D0S" << br->getCorrespondingBBNumber());
									C00_1->addRight(1,x00d0);
									C00_1->addRight(1,x01d0);
									C01_1->addRight(1,x10d0);
									C10_1->addRight(1,x11d0);
								}
								
								var_added[p->source()->getCorrespondingBBNumber()] ++;
							}
						}
						if(br->isEntry()) {
							Var *x00start,*x01start,*x10start,*x11start;
							NEW_VAR_FROM_BUFF(x00start,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00start");
							NEW_VAR_FROM_BUFF(x01start,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01start");
							NEW_VAR_FROM_BUFF(x10start,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10start");
							NEW_VAR_FROM_BUFF(x11start,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11start");
							C00_1->addRight(1,x00start);
							C01_1->addRight(1,x01start);
							C10_1->addRight(1,x10start);
							C11_1->addRight(1,x11start);
						}
						
						// toujours pour l'unicité des contraintes
						for(int i=0;i<cfg->countBB();++i) var_added[i]=0;
						for(BCG::OutIterator s(br);s;s++) {
							if(var_added[s->target()->getCorrespondingBBNumber()]==0) {
								bool withT=false, withNT=false;
								br->isSuccessor(s->target(),withT,withNT);
								if(withT) {
									Var *x00_d1, *x01_d1, *x10_d1, *x11_d1;
									NEW_VAR_FROM_BUFF(x00_d1,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00D1S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(x01_d1,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01D1S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(x10_d1,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10D1S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(x11_d1,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11D1S" << s->target()->getCorrespondingBBNumber());
									C00_2->addRight(1,x00_d1);
									C01_2->addRight(1,x01_d1);
									C10_2->addRight(1,x10_d1);
									C11_2->addRight(1,x11_d1);
								}
								if(withNT) {
									Var *x00_d0, *x01_d0, *x10_d0, *x11_d0;
									NEW_VAR_FROM_BUFF(x00_d0,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00D0S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(x01_d0,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01D0S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(x10_d0,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10D0S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(x11_d0,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11D0S" << s->target()->getCorrespondingBBNumber());
									C00_2->addRight(1,x00_d0);
									C01_2->addRight(1,x01_d0);
									C10_2->addRight(1,x10_d0);
									C11_2->addRight(1,x11_d0);							
								}
		
								var_added[s->target()->getCorrespondingBBNumber()] ++;
							}
						}
						if(br->isExit()) {
							if(br->exitsWithT()) {
								Var *e00_d1,*e01_d1,*e10_d1,*e11_d1;
								NEW_VAR_FROM_BUFF(e00_d1,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00D1end");
								NEW_VAR_FROM_BUFF(e01_d1,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01D1end");
								NEW_VAR_FROM_BUFF(e10_d1,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10D1end");
								NEW_VAR_FROM_BUFF(e11_d1,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11D1end");
								C00_2->addRight(1,e00_d1);
								C01_2->addRight(1,e01_d1);
								C10_2->addRight(1,e10_d1);
								C11_2->addRight(1,e11_d1);							
							}
							if(br->exitsWithNT()) {
								Var *e00_d0,*e01_d0,*e10_d0,*e11_d0;
								NEW_VAR_FROM_BUFF(e00_d0,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00D0end");
								NEW_VAR_FROM_BUFF(e01_d0,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01D0end");
								NEW_VAR_FROM_BUFF(e10_d0,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10D0end");
								NEW_VAR_FROM_BUFF(e11_d0,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11D0end");
								C00_2->addRight(1,e00_d0);
								C01_2->addRight(1,e01_d0);
								C10_2->addRight(1,e10_d0);
								C11_2->addRight(1,e11_d0);													
							}
						}
						
					} /// Fin B2.4
	#endif			
	
	#if B_25 > 0
					////////////////////////
					// 2.5: wrong branches
					////////////////////////
					{
						NEW_SPECIAL_CONSTRAINT(M_T,EQ,0);
						NEW_SPECIAL_CONSTRAINT(M_NT,EQ,0);
						Var *m0, *m1;
						for(BasicBlock::OutIterator edge(bb); edge ; edge++ ) {
							if(edge->kind() == Edge::TAKEN) { // WARNING ces accolades sont IMPERATIVES car NEW_VAR_FROM_BUFF definit un bloc
								NEW_VAR_FROM_BUFF(m1,	"m" << bb->number() << "_" << edge->target()->number() << "A" << BitSet_to_String(br->getHistory()) )
							}
							else if(edge->kind() == Edge::NOT_TAKEN) {// WARNING ces accolades sont IMPERATIVES car NEW_VAR_FROM_BUFF definit un bloc
								NEW_VAR_FROM_BUFF(m0,	"m" << bb->number() << "_" << edge->target()->number() << "A" << BitSet_to_String(br->getHistory()))
							}
						}
						M_T->addLeft(1,m1);
						M_NT->addLeft(1,m0);
		
						// unicité
						int var_added[cfg->countBB()];
						for(int i=0;i<cfg->countBB();++i) var_added[i]=0;
		
						for(BCG::OutIterator s(br);s;s++) {
			
							if(var_added[s->target()->getCorrespondingBBNumber()]==0) {
								bool withT, withNT;
								br->isSuccessor(s->target(),withT,withNT);
								if(withT) {
									Var *v00, *v01;
									NEW_VAR_FROM_BUFF(v00, Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00D1S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(v01, Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01D1S" << s->target()->getCorrespondingBBNumber());
									M_T->addRight(1,v00);
									M_T->addRight(1,v01);
								}
								if(withNT) {
									Var *v10, *v11;
									NEW_VAR_FROM_BUFF(v10, Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10D0S" << s->target()->getCorrespondingBBNumber());
									NEW_VAR_FROM_BUFF(v11, Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11D0S" << s->target()->getCorrespondingBBNumber());
									M_NT->addRight(1,v10);
									M_NT->addRight(1,v11);
									
								}
								var_added[s->target()->getCorrespondingBBNumber()] ++;
							}
						}
						if(br->isExit()) {
							if(br->exitsWithT()) {
								Var *e00_d1,*e01_d1;
								NEW_VAR_FROM_BUFF(e00_d1,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C00D1end");
								NEW_VAR_FROM_BUFF(e01_d1,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C01D1end");
								M_T->addRight(1,e00_d1);
								M_T->addRight(1,e01_d1);
							}
							if(br->exitsWithNT()) {
								Var *e10_d0,*e11_d0;
								NEW_VAR_FROM_BUFF(e10_d0,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C10D0end");
								NEW_VAR_FROM_BUFF(e11_d0,Xi->name() << "A" << BitSet_to_String(br->getHistory()) << "C11D0end");
								M_NT->addRight(1,e10_d0);
								M_NT->addRight(1,e11_d0);													
							}
						}
		
					} /// Fin B2.5
#endif
				
				}
			}
		}
	}
	

}

/**
 * Generate the constraints system according to the "global 2bits" method, only Claire's history-based constraints.
 * 
 * @param fw		Current Workspace.
 * @param cfg		CFG.
 * @param bhg		"Branch History Graph".
 * @param bbhg		"BasicBlock History Graph".
 * @param bcgs		Vector containg the BCGs (Branch Conflict Graphs) generated from the BHG.
 * @param ht_vars	Hash Table used to ensure unicity of the variables.
 */
void BPredProcessor::CS__Global2b_not_mitra(WorkSpace *fw, CFG *cfg, BHG* bhg, elm::genstruct::Vector<BCG*> &graphs ,	HashTable<String ,Var*>& ht_vars) {
	// Recuperation de l'ensemble des contraintes
	System *system = ipet::SYSTEM(fw);
	ASSERT(system);

	HashTable<BasicBlock*,elm::genstruct::Vector<BCGNode*> > classes_of_BB;

	// creation des classes d'appartenance des branchements A PARTIR DU BHG (indirectement depuis les BCG)
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		elm::genstruct::Vector<BCGNode*> list_nodes;
		
		for(unsigned int i = 0 ; i< graphs.length();++i) {
			BCG* g=graphs[i];
			for(BCG::Iterator n(g);n;n++)
				if(n->getCorrespondingBBNumber() == bb->number()) { list_nodes.add(n);break; }
		}
		
		if(list_nodes.length()>0) classes_of_BB.add(bb,list_nodes);
	}

	//////////
	// H11:
	//////////
#if H_11 > 0
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		if(classes_of_BB.exists(bb)) {
			Var *Xi = ipet::VAR(bb);
			ASSERT(Xi);
			
	
			NEW_SPECIAL_CONSTRAINT(H11,EQ,0);
	
			H11->addLeft(1,Xi);
			elm::genstruct::Vector<BCGNode*> v=classes_of_BB.get(bb);
			for(unsigned int i = 0 ; i<v.length();++i) {
				Var *XbApi;
				NEW_VAR_FROM_BUFF(XbApi, Xi->name() << "A" << BitSet_to_String(v[i]->getHistory()));
				H11->addRight(1,XbApi);
			}
		}
	}
#endif


	//////////
	// H12:
	//////////
#if H_12 > 0
	HashTable<BasicBlock*, elm::genstruct::Vector<BHGNode*> > BB_classes;
	
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		elm::genstruct::Vector<BHGNode*> v;
		for(BHG::Iterator node(bhg);node;node++) {
			if(node->getCorrespondingBB()->number() == bb->number()) {
				v.add(node);
			}
		}
		if(v.length()>0)BB_classes.add(bb,v);
	}

	for(CFG::BBIterator bb(cfg);bb;bb++) {
		if(BB_classes.exists(bb)) {
			Var *Xi = ipet::VAR(bb);
			ASSERT(Xi);
			
	
			for(BasicBlock::OutIterator edge(bb);edge;edge++) {
				NEW_SPECIAL_CONSTRAINT(H12,EQ,0);
				Var *Eb_s=ipet::VAR(edge);
				H12->addLeft(1,Eb_s);
				elm::genstruct::Vector<BHGNode*> v=BB_classes.get(bb);
				for(unsigned int i = 0 ; i<v.length();++i) {
					Var *XbApi;
					int d=(edge->kind() == Edge::TAKEN)?1:0; 
					if(v[i]->isExit() && (v[i]->exitsWithT()==(d==1))) {
						NEW_VAR_FROM_BUFF(XbApi, Xi->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "end");
						H12->addRight(1,XbApi);						
					}
					else {
						NEW_VAR_FROM_BUFF(XbApi, Xi->name() << "A" << BitSet_to_String(v[i]->getHistory()) << "D" << d);
						H12->addRight(1,XbApi);
					}
				}
			}
		}
	}
#endif


	
	
	
	//////////////////////////////////
	/// H2-4: parcours des noeuds du BHG
	//////////////////////////////////
#if H_2_3 >0
	NEW_SPECIAL_CONSTRAINT(H2,LE,1);
	NEW_SPECIAL_CONSTRAINT(H3,LE,1);
#endif
	
	for(BHG::Iterator node(bhg) ; node ; node++ ) {
		Var *Xi = ipet::VAR( node->getCorrespondingBB());
		ASSERT(Xi);

		/////////
		// H4.1:
		/////////
#if H_41 > 0
//		{
//			NEW_SPECIAL_CONSTRAINT(H41,LE,0);
//			Var *XbApi;
//			NEW_VAR_FROM_BUFF(XbApi,Xi->name() << "A" << BitSet_to_String(node->getHistory()));
//			H41->addLeft(1,XbApi);
//			
//			for(BHG::Predecessor p(node);p;p++) {
//				Var *Xj = ipet::VAR( p->getCorrespondingBB());
//				ASSERT(Xj);
//				
//					Var* XsbApi;
//					NEW_VAR_FROM_BUFF(XsbApi,Xj->name() << "A" << BitSet_to_String(p->getHistory()));
//					H41->addRight(1,XsbApi);
//				
//			}
//			if(node->isEntry()) {
//				Var *XbApistart;
//				NEW_VAR_FROM_BUFF(XbApistart,Xi->name() << "A" << BitSet_to_String(node->getHistory()) << "start");
//				H41->addRight(1,XbApistart);
//			}
//		}
		{
			NEW_SPECIAL_CONSTRAINT(H41,EQ,0);
			Var *XbApi;
			NEW_VAR_FROM_BUFF(XbApi,Xi->name() << "A" << BitSet_to_String(node->getHistory()));
			H41->addLeft(1,XbApi);
			
			for(BHG::InIterator p(node);p;p++) {
				Var *Xj = ipet::VAR( p->source()->getCorrespondingBB());
				ASSERT(Xj);
					int d = (p->isTaken())?1:0;
					Var* XsbApi;
					NEW_VAR_FROM_BUFF(XsbApi,Xj->name() << "A" << BitSet_to_String(p->source()->getHistory()) << "D" << d);
					H41->addRight(1,XsbApi);
				
			}
			if(node->isEntry()) {
				Var *XbApistart;
				NEW_VAR_FROM_BUFF(XbApistart,Xi->name() << "A" << BitSet_to_String(node->getHistory()) << "start");
				H41->addRight(1,XbApistart);
			}
		}
#endif

		/////////
		// H4.2:
		/////////
#if H_42 > 0
		{
			NEW_SPECIAL_CONSTRAINT(H42,EQ,0);
			Var *XbApi;
			NEW_VAR_FROM_BUFF(XbApi,Xi->name() << "A" << BitSet_to_String(node->getHistory()));
			H42->addLeft(1,XbApi);
			
			for(BHG::OutIterator s(node);s;s++) {
				Var *Xj = ipet::VAR( s->target()->getCorrespondingBB());
				ASSERT(Xj);
				int d = (s->isTaken())?1:0;
				
				Var* XsbApi;
				NEW_VAR_FROM_BUFF(XsbApi,Xi->name() << "A" << BitSet_to_String(node->getHistory()) << "D" << d);
				H42->addRight(1,XsbApi);
				
			}
			if(node->isExit()) {
				Var *XbApiend;
				NEW_VAR_FROM_BUFF(XbApiend,Xi->name() << "A" << BitSet_to_String(node->getHistory()) << "end");
				H42->addRight(1,XbApiend);
			}
		}
#endif

		/////////
		// H2+3:
		/////////
#if H_2_3 > 0
		{
			elm::genstruct::Vector<BCGNode*> v=classes_of_BB.get(node->getCorrespondingBB());
			if(node->isEntry()) {
				Var* XbApistart;
				NEW_VAR_FROM_BUFF(XbApistart,Xi->name() << "A" << BitSet_to_String(node->getHistory()) << "start");
				H2->addLeft(1,XbApistart);
			}
			if(node->isExit()) {
				Var* XbApiend;
				NEW_VAR_FROM_BUFF(XbApiend,Xi->name() << "A" << BitSet_to_String(node->getHistory()) << "end");
				H3->addLeft(1,XbApiend);
			}
		}
#endif
	}
} 

/**
 * Recursive function that returns the first branch from a given BasicBlock in a CFG.
 * 
 * @param bb	BasicBlock to start the search from.
 * @param cfg	CFG containing the BasicBlock.
 * 
 * @return	The first BasicBlock which is a branch.
 */
BasicBlock* BPredProcessor::getFirstBranch(BasicBlock* bb,CFG* cfg) {
	unsigned int nb_OE = 0;
	if(bb==cfg->exit()) return NULL;
	// Parcours des OutEdges
	for(BasicBlock::OutIterator edge(bb); edge ; edge++ ) {
		// on incremente que s'il s'agit d'un edge TAKEN ou NOT_TAKEN
		if(edge->kind() == Edge::TAKEN) nb_OE++;
		else if(edge->kind() == Edge::NOT_TAKEN) nb_OE++;
	}

	// Si un branchement a ete trouve ...
	if(nb_OE == 2 ) {
		return bb;
	}
	else{
		for(BasicBlock::OutIterator edge(bb); edge ; edge++ ) {
			// on incremente que s'il s'agit d'un edge TAKEN ou NOT_TAKEN
			return getFirstBranch(edge->target(),cfg);
		}	
	}
}

/**
 * Creates BHG Nodes from the branches that are successors of a given BasickBlock of the CFG and an history.
 * 
 * @param bb		BasicBlock to start from.
 * @param history	BitSet containing the history value.
 * @param suivants	vector which will contain the BHG Nodes created.
 * @param cfg		CFG of the BasicBlock bb.
 * @param entryBr	First branch of the CFG.
 */
void BPredProcessor::getBranches(BasicBlock* bb,dfa::BitSet history,elm::genstruct::Vector<BHGNode* >& suivants,CFG* cfg,BasicBlock* entryBr) {
	for(BasicBlock::OutIterator edge(bb); edge ; edge++ ) {
		if(edge->kind() == Edge::TAKEN) {
			dfa::BitSet h=lshift_BitSet(history,1,true);
			BasicBlock *c_bb = getFirstBranch(edge->target(),cfg);
			BHGNode* n;
			if(c_bb==NULL) 
				n=NULL;
			else
				n=new BHGNode(c_bb,h,c_bb==entryBr); 

			suivants.add(n);
		}
		else {
			dfa::BitSet h=lshift_BitSet(history,1);
			BasicBlock *c_bb = getFirstBranch(edge->target(),cfg);
			BHGNode* n;
			if(c_bb==NULL) 
				n=NULL;
			else
				n=new BHGNode(c_bb,h,c_bb==entryBr); 

			suivants.add(n);
		}
	}
}


/**
 * Tests if a similar BHG Node is contained in a given vector of BHG Nodes and returns the BHG Node found.
 * Two BHG Nodes are similar if they have same hisotry and comes from the same BasicBlock.
 * 
 * @param v			Vector containing the BHG Nodes.
 * @param n			BHG Node to compare with those contained in the vector.
 * @param contained If any similar BHG Node is found, then it will contains the found BHG Node, else it will contains a NULL pointer.
 * 
 * @return returns a boolean value set to True if a similar BHG Node has been found.
 */
bool BPredProcessor::contains(const elm::genstruct::Vector< BHGNode* >& v, BHGNode& n, BHGNode * &contained) {
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
 * Creates a BHG from a given CFG.
 * 
 * @param cfg	CFG to generate the BHG from.
 * @param bhg	Reference to the BHG to fill.
 */
void BPredProcessor::generateBHG(CFG* cfg,BHG& bhg) {
	elm::genstruct::Vector< BHGNode* > final_nodes; 	// => S dans l'algo
	elm::genstruct::Vector< BHGNode* > todo_nodes; 	// => F dans l'algo
	elm::genstruct::Vector< BHGEdge* > final_edges;	// => E dans l'algo

	// creation du BitSet unité
	dfa::BitSet unite(this->BHG_history_size);
	unite.add(0);
	///////////////////////////
	
	BasicBlock* entryBr=getFirstBranch(cfg->entry(), cfg);
	int position=0;
	dfa::BitSet h(this->BHG_history_size);
	while(true) {
		BHGNode *n= new BHGNode(entryBr,h,true);

		BHGNode *tmp;
		if(!contains(final_nodes,*n,tmp)) {
			final_nodes.add(n);
			todo_nodes.add(n);
			bhg.add(n);
	
			while(!todo_nodes.isEmpty()) {
				BHGNode *courant=todo_nodes.pop();
				elm::genstruct::Vector<BHGNode *> suivants;
	
				getBranches(courant->getCorrespondingBB(),courant->getHistory(),suivants,cfg,entryBr);

				for(unsigned int i=0;i<suivants.length();++i) {
					BHGNode *s = suivants[i];
					if(s!=NULL) {
						BHGNode *cs;
						bool todo_n=false, final_n=false;
						if(!(final_n=contains(final_nodes,*s,cs))) {
							final_nodes.add(s);
							bhg.add(s);
							
							todo_nodes.add(s);
							BHGEdge *e = new BHGEdge(courant,s,s->getHistory().contains(0));
							final_edges.add(e);
	
						}
						else {
							BHGEdge *e = new BHGEdge(courant,cs,cs->getHistory().contains(0));
							final_edges.add(e);
							delete s; // cree par suivant
						}

					}
				}
			}
		}
		else delete n;

		// SORTIE
		if(h.isFull()) break;
		/////////
		
		
		historyPlusOne(h);
		
	}


	// Rajout des sorties
	for(BHG::Iterator bb(&bhg); bb; bb++){
		int n=0;
		for(BHG::OutIterator succ(bb); succ; succ++){
			if(succ->isTaken()) 
				n |= 1; // 01 => sortie =/= T
			else 
				n |= 2; // 10 => sortie =/= NT
		}

		if(n!=3) // 11
			bb->setExit((n & 1) == 0, (n & 2) == 0);
	}

}


/**
 * Increments a bitset by 1 as if it was a binary number.
 * 
 * @param h	BitSet to increments.
 */
void BPredProcessor::historyPlusOne(dfa::BitSet& h) {
	int i=0;
	while(i<h.size() && h.contains(i)) {
		h.remove(i);
		i++;
	}
	if(i<h.size()) h.add(i);
	
}

/**
 * Checks if a BHG Node is an entry of the BHG.
 * It is an entry if a way exists between this BHG Node and an entry BHG Node of the BHG graph, composed of BHG Nodes with different history values.
 * 
 * @param bhg	BHG containing the BHGNode.
 * @param src	The BHG Node to check.
 * 
 * @return A boolean value if the BHG Node is an entry.
 */
bool BPredProcessor::isClassEntry(BHG* bhg, BHGNode* src) {
	HashTable<BHGNode*,BHGNode*> visited_nodes;
	for(BHG::Iterator node(bhg);node;node++) {
		if(node->isEntry() && src->getHistory() != node->getHistory() ) {
			for(BHG::OutIterator s_node(node);s_node;s_node++) {
				visited_nodes.clear();
				if(isLinked(s_node, src, src->getHistory(), visited_nodes)) {
					return true;
				}
			}
		}
	}
	return false;
	
}

/**
 * Checks if a BHG Node is an exit of the BHG, and gives weather it exits with  a TAKEN branch or with a NOT_TAKEN branch or with both.
 * It is an exit if a way exists between this BHG Node and an exit BHG Node of the BHG graph, composed of BHG Nodes with different history values.
 * 
 * @param bhg			BHG containing the BHGNode.
 * @param src			The BHG Node to check.
 * @param src_withT		Boolean reference set to true if the exit is reached by the TAKEN branch.
 * @param src_withNT	Boolean reference set to true if the exit is reached by the NOT_TAKEN branch.
 * 
 * @return A boolean value if the BHG Node is an exit.
 */
bool BPredProcessor::isClassExit(BHG* bhg, BHGNode* src, bool& src_withT,bool& src_withNT) {
	HashTable<BHGNode*,BHGNode*> visited_nodes;
	src_withNT = src_withT = false;
	bool res = false;
	for(BHG::Iterator node(bhg);node;node++) {
		if(node->isExit() && src->getHistory() != node->getHistory() ) {
			for(BHG::OutIterator s_src(src);s_src;s_src++) {
				visited_nodes.clear();
				if(isLinked(s_src, node, src->getHistory(), visited_nodes)) {
					if(s_src->isTaken()) src_withT = true;
					else src_withNT = true;
					res = true;
				}
			}
		}
		if(src_withT && src_withNT) break;
	}
	return res;
}

/**
 * Recursive function that checks if a BHGNode can be reach from a given BHG Edge.
 * True is returned if there is a way from the edge compouned of BHGNode with different history value from the given one.
 * 
 * @param dir			edge to start from.
 * @param dest			Destination BHG Node.
 * @param h				History that must not be found before finfding the destination Node.
 * @param visited_nodes	An empty HashTable of BHG Nodes filled as a BHG Node is visited.
 * 
 * @return A boolean value set to true if a way is found to the destination Node.
 */
bool BPredProcessor::isLinked(BHGEdge* dir, BHGNode* dest, dfa::BitSet& h, HashTable<BHGNode* , BHGNode*>& visited_nodes) {
	BHGNode* src = dir->target();
	if(visited_nodes.exists(src)) {
		return false;
	}
	else {
		visited_nodes.add(src,src);
		if(src->equals(*dest) ) {
			return true;
		}
		else {
			if(h == src->getHistory()) 
				return false;
			else {
				bool res = false;
				for (BHG::OutIterator s(src);s;s++) {
					BHGEdge* e = s;
					res = res || isLinked(e,dest, h,visited_nodes);
				}
				return res;
			}
		}
	}
}

/**
 * Creates the BCGs from a given BHG and stores them in a vector.
 * 
 * @param bcgs	Refernce to the vector to store the generated BCGs in.
 * @param bhg	BHG to extract the BCGs from.
 */
void BPredProcessor::generateBCGs(elm::genstruct::Vector<BCG*>& bcgs, BHG& bhg) {

	elm::genstruct::Vector<BHGNode*> v_class_nodes;
	dfa::BitSet h(this->BHG_history_size);
	PropList props;

	unsigned int cpt_nodes;
	h.empty();
	while(true) {
		cpt_nodes = 0;
		
		// recherche de tous les noeuds d'une classe
		 v_class_nodes.clear();
		for(BHG::Iterator node(&bhg); node ; node++) {
			if(node->getHistory() == h)
				v_class_nodes.add(node);
				
		}
		
		BCG* bcg=new BCG(h);
		HashTable<int,BCGNode*> created_nodes;
		for(unsigned int i = 0 ; i < v_class_nodes.length() ; ++i) {
			BHGNode* src = v_class_nodes[i];
			for(unsigned int j = 0 ; j < v_class_nodes.length() ; ++j) {
				BHGNode* dest = v_class_nodes[j];

				BCGNode* src_bcg;
				BCGNode* dest_bcg;
				for(BHG::OutIterator s(src);s;s++) {
					BHGEdge* dir = s;
					
					HashTable<BHGNode*,BHGNode*> visited_nodes;
					if(isLinked(dir,dest,h,visited_nodes)) {

						bool src_withT, src_withNT, dest_withT, dest_withNT;

						
						if(created_nodes.exists(src->getCorrespondingBB()->number()))
							src_bcg = created_nodes.get(src->getCorrespondingBB()->number());
						else {
							visited_nodes.clear();
							bool src_exit  = isClassExit(&bhg,src,src_withT,src_withNT) || src->isExit();
							src_withT = src_withT || src->exitsWithT();
							src_withNT = src_withNT || src->exitsWithNT();
							
							bool src_entry = src->isEntry() || isClassEntry(&bhg,src);
							
							src_bcg = new BCGNode(src->getCorrespondingBB()->number(),src_entry, src_exit, &h, src_withT, src_withNT);
							bcg->add(src_bcg);
							cpt_nodes++;
							created_nodes.add (src->getCorrespondingBB()->number(),src_bcg);
						}

						if(created_nodes.exists(dest->getCorrespondingBB()->number()))
							dest_bcg = created_nodes.get(dest->getCorrespondingBB()->number());
						else  {
							visited_nodes.clear();
							bool dest_exit = isClassExit(&bhg,dest,dest_withT,dest_withNT) || dest->isExit() ;
							dest_withT = dest_withT || dest->exitsWithT();
							dest_withNT = dest_withNT || dest->exitsWithNT();

							bool dest_entry = dest->isEntry() || isClassEntry(&bhg,dest);
							
							dest_bcg = new BCGNode(dest->getCorrespondingBB()->number(),dest_entry, dest_exit, &h, dest_withT, dest_withNT); 
							bcg->add(dest_bcg);
							cpt_nodes++;
							created_nodes.add (dest->getCorrespondingBB()->number(),dest_bcg);
						}
						
						
						BCGEdge* edge_bcg = new BCGEdge(src_bcg,dest_bcg,dir->isTaken());
					}
				}
			}
		}
	
		if(cpt_nodes>0) {
			bcgs.add(bcg);
			if(this->dumpBCG) {
				StringBuffer buf;
				buf << "bcg_H" << BitSet_to_String(h) << ".ps";
				String filename = buf.toString();
				otawa::display::OUTPUT_PATH(props) = filename.toCString();
				BCGDrawer drawer(bcgs.top(), props);
				drawer.display();
			}
		}
		
		
		// terminaison
		if(h.isFull()) break;
		historyPlusOne(h);
	}

}

/**
 * Creates all the required graphs and structures to create the constraints system based on the "global 2bits" model.
 * 
 * @param ws	Current workspace.
 * @param cfg	CFG to extracts the graphs and structures from.
 */
void BPredProcessor::processCFG__Global2B(WorkSpace *ws,CFG* cfg) {
	BHG bhg(this->BHG_history_size);
	generateBHG(cfg,bhg);
	
	elm::genstruct::Vector<BCG*> bcgs;
	generateBCGs(bcgs,bhg);	
	
#if CLAIRE_INFOS >0
	////// Max pred
	int max_in=0;
	for(BHG::NodeIterator n(&bhg);n ;n++) {
		int nb_pred=0;
		for(BHG::Predecessor p(n);p;p++)
			nb_pred++;
		if(max_in<nb_pred) max_in = nb_pred;
	}
	cerr << "A_2b =" << max_in << "\n";

	///// Sum PI pour chaque brcht
	int sum=0;
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		for(BHG::NodeIterator n(&bhg);n ;n++)
			if(n->getCorrespondingBB()->number()==bb->number()) sum++;
	}
	cerr << "B_2b = " << sum << "\n";

	///// Sum successeurs
	sum=0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			for(BCG::Successor s(n);s;s++) sum++;
		}
	}
	cerr << "C_2b = " << sum << "\n";

	///// Sum Predecesseurs
	sum=0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			for(BCG::Predecessor s(n);s;s++) sum++;
		}
	}
	cerr << "D_2b = " << sum << "\n";
	
	///// Sum PI possibles pour les sorties
	sum=0;
	HashTable<String ,void*> ht_pi;
	for(BHG::NodeIterator n(&bhg);n;n++) {
		if(n->isExit() && !ht_pi.exists(BitSet_to_String(n->getHistory()))) {
			ht_pi.add(BitSet_to_String(n->getHistory()),NULL);
			sum++;
		}
	}
	cerr << "E_2b = " << sum << "\n";
	
	///// Sum de tous les Branchements en sortie dans les BCG (avec pi differents)
	sum=0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			if(n->isExit()) sum++;
		}
	}
	cerr << "F_2b = " << sum << "\n";
	
	///// Sum de tous les Branchements en entree dans les BCG (avec pi differents)
	sum=0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			if(n->isEntry()) sum++;
		}
	}
	cerr << "G_2b = " << sum << "\n";

	///// Max des branchements pour une classe
	int max = 0;
	for(int i=0;i<bcgs.length();i++) {
		sum=0;
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			sum++;
		}
		if(sum>max) max=sum;
	}
	cerr << "H_2b = " << max << "\n";
	
	///// Max de preds pour un branchement dans un BCG
	max = 0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			sum=0;
			for(BCG::Predecessor p(n);p;p++) sum++;
			if(sum>max) max=sum;
		}
	}
	cerr <<  "I_2b = " << max << "\n";
	
	///// Max de succs pour un branchement dans un BCG
	max = 0;
	for(int i=0;i<bcgs.length();i++) {
		for(BCG::NodeIterator n(bcgs[i]);n;n++) {
			sum=0;
			for(BCG::Successor p(n);p;p++) sum++;
			if(sum>max) max=sum;
		}
	}
	cerr <<  "J_2b = " << max << "\n";
	
	///// Max successeurs pour 1 BCG
	int max_t=0,max_nt=0;
	int sum_t,sum_nt=0;
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		if(isBranch(bb)) {
			sum_t=0;
			sum_nt=0;
			for(int i=0;i<bcgs.length();i++) {
				for(BCG::NodeIterator n(bcgs[i]);n;n++) {
					if(n->getCorrespondingBBNumber()==bb->number()) {
						for(BCG::Successor s(n);s;s++) 
							if(s.edge()->isTaken()) sum_t++;
							else sum_nt++;
						if(n->isExit()) {
							if(n->exitsWithT()) sum_t++;
							if(n->exitsWithNT()) sum_nt++;
						}
					}
				}
			}
			if(max_t<sum_t)max_t=sum_t;
			if(max_t<sum_nt)max_t=sum_nt;
		}
	}
	if(max_nt>max_t) max=max_nt; else max=max_t;
	cerr << "K_2b = " << max << "\n";
	
	///// Max PI possibles pour les sorties
	HashTable<BHGNode* ,void*> ht_pi2;
	max=0;
	for(CFG::BBIterator bb(cfg);bb;bb++) {
		sum=0;
		for(BHG::NodeIterator n(&bhg);n;n++) {
			if(n->isExit() && n->getCorrespondingBB()->number() == bb->number() && !ht_pi2.exists(n)) {
				ht_pi2.add(n,NULL);
				sum++;
			}
		}
		if(sum>max)max=sum;
	}
	cerr << "L_2b = " << max << "\n";

	/////
#endif
	
	PropList props;
	if(this->dumpBHG) {
		StringBuffer buf;
		buf << "bhg.ps";
		String filename = buf.toString();
		otawa::display::OUTPUT_PATH(props) = filename.toCString();
		BHGDrawer drawer(&bhg, props);
		drawer.display();
	}


	
	if(this->withStats) {
		// compte les BCGs
		this->stat_nbbr.clear();
		this->stat_hist.clear();
		for(unsigned int i = 0 ; i < bcgs.length() ; i++) {
			int nb_nodes=0;
			BCG* bcg=bcgs[i];
			for(BCG::Iterator node(bcg);node;node++)
				nb_nodes++;
			
			this->stat_nbbr.add(nb_nodes);
			dfa::BitSet *bs= new dfa::BitSet(bcg->getHistory());
			this->stat_hist.add(bs);
		}
	}
	HashTable<String ,Var*> ht_vars;
	if(this->withMitra) {
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
		
		CS__Global1b_mitra(ws,cfg,&bbhg,ht_vars);
	}
	else {
		CS__Global2b_not_mitra(ws,cfg,&bhg,bcgs,ht_vars);
	}
	
	CS__Global2b(ws,cfg,&bhg,bcgs,	ht_vars);
}


} // ::ipet
} // ::otawa


