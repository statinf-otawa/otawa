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

#include <otawa/cfg.h>
#include <otawa/bpred/BPredProcessor.h>
#include "BCGDrawer.h"
#include "BSets.h"
#include "BCG.h"
#include <otawa/ipet/BasicConstraintsBuilder.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/ILPSystemGetter.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/ilp/System.h>

using namespace otawa::ilp;
using namespace elm;
using namespace elm::genstruct;

namespace otawa {
namespace bpred {


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
// AVCTIVER(>0) / DESACTIVER(0) LES BLOCS DE CONTRAINTES
#define ALL_C 1
#define C_1 ALL_C+0
#define C_21 ALL_C+0
#define C_22 ALL_C+0
#define C_23 ALL_C+0
#define C_24 ALL_C+0
#define C_25 ALL_C+0

////////////////////////////////////////////

/**
 * Finds and returns the BCG Node associated to the given BasicBlock number.
 * 
 * @param id	Number of the BasicBlock to find.
 * @param bcg	BCG to search in.
 * 
 * @return 		The BCG Node found.
 */
BCGNode* getBCGNode(int id,BCG* bcg) {
	for(BCG::Iterator node(bcg);node;node++) {
		if(node->getCorrespondingBBNumber()== id) return node;
	}
	return NULL;
}

/**
 * Finds and returns the BasicBlock associated to the given BasicBlock number.
 * 
 * @param id	Number of the BasicBlock to find.
 * @param cfg	CFG to search in.
 * 
 * @return 		The BasicBlock found.
 */
BasicBlock* BPredProcessor::getBB(int id,CFG* cfg) {
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		// on incremente que s'il s'agit d'un edge TAKEN ou NOT_TAKEN
		if(bb->number() == id) return bb;
	}
	return NULL;
}


/**
 * Generate the constraints system according to the "bimodal" method.
 * 
 * @param fw		Current Workspace.
 * @param cfg		CFG.
 * @param bs		BSet structure.
 * @param graphs	Vector containg the BCGs (Branch Conflict Graphs) generated from the BSet structure and the CFG.
 */
void BPredProcessor::CS__BiModal(WorkSpace *fw, CFG *cfg, BSets& bs, elm::genstruct::Vector<BCG*> &graphs ) {
	// Recuperation de l'ensemble des contraintes
	System *system = ipet::SYSTEM(fw);
	ASSERT(system);
	HashTable<String ,Var*> ht_vars;

	elm::genstruct::Vector<int> l_addr;
	bs.get_all_addr(l_addr);

#if C_1 > 0
	///////////////////////////////////////////////////
	//1: Boucle sur toutes les classes de branchement
	/////////////////////////////////////////////////// 
	for(int i=0 ; i<l_addr.length();++i) {
		elm::genstruct::Vector<int> l_bb;
		bs.get_vector(l_addr[i],l_bb);
		
		// on recupere le BCG correspondant a la classe en cours
		BCG* bcg=NULL;
		for(int g=0;g<graphs.length();++g) 
			if(graphs[g]->getClass() == l_addr[i]){
				bcg = graphs[g];
				break;
			}
		
		NEW_SPECIAL_CONSTRAINT(Cs,LE,1);
		NEW_SPECIAL_CONSTRAINT(Ce,LE,1);

		// Boucle sur les BB de la classe addr
		for(int b=0; b<l_bb.length();++b) {
			// Recuperation de la variable associee au BB
			BasicBlock* bb = getBB(l_bb[b],cfg);
			ASSERT(bb);
			Var *Xi = ipet::VAR( bb);
			ASSERT(Xi);
			BCGNode *n = getBCGNode(l_bb[b],bcg);
			if(n->isEntry()) {
				Var *C00s, *C01s, *C10s, *C11s;
				NEW_VAR_FROM_BUFF(C00s,Xi->name() << "A" << l_addr[i] << "C00start");
				NEW_VAR_FROM_BUFF(C01s,Xi->name() << "A" << l_addr[i] << "C01start");
				NEW_VAR_FROM_BUFF(C10s,Xi->name() << "A" << l_addr[i] << "C10start");
				NEW_VAR_FROM_BUFF(C11s,Xi->name() << "A" << l_addr[i] << "C11start");
				Cs->addLeft(1,C00s);
				Cs->addLeft(1,C01s);
				Cs->addLeft(1,C10s);
				Cs->addLeft(1,C11s);
			}
			
			if(n->isExit()) {
				Var *C00e, *C01e, *C10e, *C11e;
				NEW_VAR_FROM_BUFF(C00e,Xi->name() << "A" << l_addr[i] << "C00end");
				NEW_VAR_FROM_BUFF(C01e,Xi->name() << "A" << l_addr[i] << "C01end");
				NEW_VAR_FROM_BUFF(C10e,Xi->name() << "A" << l_addr[i] << "C10end");
				NEW_VAR_FROM_BUFF(C11e,Xi->name() << "A" << l_addr[i] << "C11end");
				Ce->addLeft(1,C00e);
				Ce->addLeft(1,C01e);
				Ce->addLeft(1,C10e);
				Ce->addLeft(1,C11e);
			}
		}
	} /// 1
#endif	

	
	///////////////////////////////////////////////
	// 2: Boucle sur tous les noeuds de chaque BCG
	///////////////////////////////////////////////
	for(int g=0;g<graphs.length();++g) {
		BCG* bcg = graphs[g];
		for(BCG::Iterator br(bcg);br; br++) {
			
			// on recupere le BB correspondant depuis le cfg
			BasicBlock* bb=getBB(br->getCorrespondingBBNumber(), cfg);
			Var *Xi = ipet::VAR(bb);
			ASSERT(Xi);
			
#if C_21>0
			//////////////////////////////////////////
			// 2.1: Pour chacun des successeurs de br
			//////////////////////////////////////////
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

				// recherche des membres de gauche à ajouter
				for(BCG::OutIterator s(br);s;s++) {
					if( s->isTaken() == (t == Edge::TAKEN) ) {
						Var *C00,*C01,*C10,*C11;
						NEW_VAR_FROM_BUFF(C00,Xi->name() << "A" << bcg->getClass() << "C00D" << cpt << "S" << s->target()->getCorrespondingBBNumber());
						NEW_VAR_FROM_BUFF(C01,Xi->name() << "A" << bcg->getClass() << "C01D" << cpt << "S" << s->target()->getCorrespondingBBNumber());
						NEW_VAR_FROM_BUFF(C10,Xi->name() << "A" << bcg->getClass() << "C10D" << cpt << "S" << s->target()->getCorrespondingBBNumber());
						NEW_VAR_FROM_BUFF(C11,Xi->name() << "A" << bcg->getClass() << "C11D" << cpt << "S" << s->target()->getCorrespondingBBNumber());
						B21->addLeft(1,C00);
						B21->addLeft(1,C01);
						B21->addLeft(1,C10);
						B21->addLeft(1,C11);
					}
				}
				
				if(br->isExit() && ( (br->exitsWithT() == (t == Edge::TAKEN) ) || (br->exitsWithNT() == (t == Edge::NOT_TAKEN) ) )){
					Var *C00,*C01,*C10,*C11;
					NEW_VAR_FROM_BUFF(C00,Xi->name() << "A" << bcg->getClass() << "C00D" << cpt << "end");
					NEW_VAR_FROM_BUFF(C01,Xi->name() << "A" << bcg->getClass() << "C01D" << cpt << "end");
					NEW_VAR_FROM_BUFF(C10,Xi->name() << "A" << bcg->getClass() << "C10D" << cpt << "end");
					NEW_VAR_FROM_BUFF(C11,Xi->name() << "A" << bcg->getClass() << "C11D" << cpt << "end");
					B21->addLeft(1,C00);
					B21->addLeft(1,C01);
					B21->addLeft(1,C10);
					B21->addLeft(1,C11);
					
				}
			} /// fin 2.1
#endif
		
#if C_22>0
			/////////////////////////////////////////////////
			// 2.2: Tous les {Succ. ; end} / {Pred. ; start}
			/////////////////////////////////////////////////
			{
				// il faut s'assurer qu'on n'ajoute qu'une seule fois chaque variable 
				// en creant un tableau d'indicateurs et en l'initialisant a 0
				int var_added[cfg->countBB()];
				for(int i=0;i<cfg->countBB();++i) var_added[i]=0;
	
	
				
				NEW_SPECIAL_CONSTRAINT(B22_pred,EQ,0);
				Var* v_pred;
				B22_pred->addLeft(1,Xi);
				
				// Recherche de tous les predecesseurs de br
				for(BCG::InIterator p(br) ; p ; p++ ) {
					if(var_added[p->source()->getCorrespondingBBNumber()]==0) {
						Var *C00,*C01,*C10,*C11;
						BasicBlock* bb_pred=getBB(p->source()->getCorrespondingBBNumber(), cfg);
						Var *Xj = ipet::VAR( bb_pred);
						ASSERT(Xj);
						NEW_VAR_FROM_BUFF(C00,Xj->name() << "A" << bcg->getClass() << "C00S" << br->getCorrespondingBBNumber());
						NEW_VAR_FROM_BUFF(C01,Xj->name() << "A" << bcg->getClass() << "C01S" << br->getCorrespondingBBNumber());
						NEW_VAR_FROM_BUFF(C10,Xj->name() << "A" << bcg->getClass() << "C10S" << br->getCorrespondingBBNumber());
						NEW_VAR_FROM_BUFF(C11,Xj->name() << "A" << bcg->getClass() << "C11S" << br->getCorrespondingBBNumber());
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
					NEW_VAR_FROM_BUFF(C00,Xi->name() << "A" << bcg->getClass() << "C00start");
					NEW_VAR_FROM_BUFF(C01,Xi->name() << "A" << bcg->getClass() << "C01start");
					NEW_VAR_FROM_BUFF(C10,Xi->name() << "A" << bcg->getClass() << "C10start");
					NEW_VAR_FROM_BUFF(C11,Xi->name() << "A" << bcg->getClass() << "C11start");
					B22_pred->addRight(1,C00);
					B22_pred->addRight(1,C01);
					B22_pred->addRight(1,C10);
					B22_pred->addRight(1,C11);					
				}
	
				
				// on réinitialise le tableau d'incdicateurs pour les successeurs
				for(int i=0;i<cfg->countBB();++i) var_added[i]=0;
				
				NEW_SPECIAL_CONSTRAINT(B22_succ,EQ,0);
				Var *v_succ;
				B22_succ->addLeft(1,Xi);
				// Recherche de tous les successeurs de br
				for(BCG::OutIterator s(br) ; s ; s++ ) {
					if(var_added[s->target()->getCorrespondingBBNumber()]==0) {
						Var *C00,*C01,*C10,*C11;
						NEW_VAR_FROM_BUFF(C00,Xi->name() << "A" << bcg->getClass() << "C00S" << s->target()->getCorrespondingBBNumber());
						NEW_VAR_FROM_BUFF(C01,Xi->name() << "A" << bcg->getClass() << "C01S" << s->target()->getCorrespondingBBNumber());
						NEW_VAR_FROM_BUFF(C10,Xi->name() << "A" << bcg->getClass() << "C10S" << s->target()->getCorrespondingBBNumber());
						NEW_VAR_FROM_BUFF(C11,Xi->name() << "A" << bcg->getClass() << "C11S" << s->target()->getCorrespondingBBNumber());
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
					NEW_VAR_FROM_BUFF(C00,Xi->name() << "A" << bcg->getClass() << "C00end");
					NEW_VAR_FROM_BUFF(C01,Xi->name() << "A" << bcg->getClass() << "C01end");
					NEW_VAR_FROM_BUFF(C10,Xi->name() << "A" << bcg->getClass() << "C10end");
					NEW_VAR_FROM_BUFF(C11,Xi->name() << "A" << bcg->getClass() << "C11end");
					B22_succ->addRight(1,C00);
					B22_succ->addRight(1,C01);
					B22_succ->addRight(1,C10);
					B22_succ->addRight(1,C11);					
				}
				
			} //// Fin 2.2
#endif

#if C_23>0
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
	
						NEW_VAR_FROM_BUFF(v00,Xi->name() << "A" << bcg->getClass() << "C00S" << s->target()->getCorrespondingBBNumber());
						NEW_VAR_FROM_BUFF(v01,Xi->name() << "A" << bcg->getClass() << "C01S" << s->target()->getCorrespondingBBNumber());
						NEW_VAR_FROM_BUFF(v10,Xi->name() << "A" << bcg->getClass() << "C10S" << s->target()->getCorrespondingBBNumber());
						NEW_VAR_FROM_BUFF(v11,Xi->name() << "A" << bcg->getClass() << "C11S" << s->target()->getCorrespondingBBNumber());
						B23_00->addLeft(1,v00);
						B23_01->addLeft(1,v01);
						B23_10->addLeft(1,v10);
						B23_11->addLeft(1,v11);
						
						bool withT=false, withNT=false;
						br->isSuccessor(s->target(),withT,withNT);
						if(withT) {
							Var *T00,*T01,*T10,*T11;
							NEW_VAR_FROM_BUFF(T00,Xi->name() << "A" << bcg->getClass() << "C00D1S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(T01,Xi->name() << "A" << bcg->getClass() << "C01D1S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(T10,Xi->name() << "A" << bcg->getClass() << "C10D1S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(T11,Xi->name() << "A" << bcg->getClass() << "C11D1S" << s->target()->getCorrespondingBBNumber());
							B23_00->addRight(1,T00);
							B23_01->addRight(1,T01);
							B23_10->addRight(1,T10);
							B23_11->addRight(1,T11);
						}
						if(withNT) {
							Var *NT00,*NT01,*NT10,*NT11;
							NEW_VAR_FROM_BUFF(NT00,Xi->name() << "A" << bcg->getClass() << "C00D0S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(NT01,Xi->name() << "A" << bcg->getClass() << "C01D0S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(NT10,Xi->name() << "A" << bcg->getClass() << "C10D0S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(NT11,Xi->name() << "A" << bcg->getClass() << "C11D0S" << s->target()->getCorrespondingBBNumber());
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
					NEW_VAR_FROM_BUFF(v00end,Xi->name() << "A" << bcg->getClass() << "C00end");
					NEW_VAR_FROM_BUFF(v01end,Xi->name() << "A" << bcg->getClass() << "C01end");
					NEW_VAR_FROM_BUFF(v10end,Xi->name() << "A" << bcg->getClass() << "C10end");
					NEW_VAR_FROM_BUFF(v11end,Xi->name() << "A" << bcg->getClass() << "C11end");
					B23_exit00->addLeft(1,v00end);
					B23_exit01->addLeft(1,v01end);
					B23_exit10->addLeft(1,v10end);
					B23_exit11->addLeft(1,v11end);

					if(br->exitsWithT()) {
						Var *eT00,*eT01,*eT10,*eT11;
						NEW_VAR_FROM_BUFF(eT00,Xi->name() << "A" << bcg->getClass() << "C00D1end");
						NEW_VAR_FROM_BUFF(eT01,Xi->name() << "A" << bcg->getClass() << "C01D1end");
						NEW_VAR_FROM_BUFF(eT10,Xi->name() << "A" << bcg->getClass() << "C10D1end");
						NEW_VAR_FROM_BUFF(eT11,Xi->name() << "A" << bcg->getClass() << "C11D1end");
						B23_exit00->addRight(1,eT00);
						B23_exit01->addRight(1,eT01);
						B23_exit10->addRight(1,eT10);
						B23_exit11->addRight(1,eT11);							
					}
					if(br->exitsWithNT()) {
						Var *eNT00,*eNT01,*eNT10,*eNT11;
						NEW_VAR_FROM_BUFF(eNT00,Xi->name() << "A" << bcg->getClass() << "C00D0end");
						NEW_VAR_FROM_BUFF(eNT01,Xi->name() << "A" << bcg->getClass() << "C01D0end");
						NEW_VAR_FROM_BUFF(eNT10,Xi->name() << "A" << bcg->getClass() << "C10D0end");
						NEW_VAR_FROM_BUFF(eNT11,Xi->name() << "A" << bcg->getClass() << "C11D0end");
						B23_exit00->addRight(1,eNT00);
						B23_exit01->addRight(1,eNT01);
						B23_exit10->addRight(1,eNT10);
						B23_exit11->addRight(1,eNT11);							
					}
				}

			} /// Fin 2.3
#endif
			
#if C_24>0	
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
				NEW_VAR_FROM_BUFF(v00,Xi->name() << "A" << bcg->getClass() << "C00");
				NEW_VAR_FROM_BUFF(v01,Xi->name() << "A" << bcg->getClass() << "C01");
				NEW_VAR_FROM_BUFF(v10,Xi->name() << "A" << bcg->getClass() << "C10");
				NEW_VAR_FROM_BUFF(v11,Xi->name() << "A" << bcg->getClass() << "C11");
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
							NEW_VAR_FROM_BUFF(x00d1,Xj->name() << "A" << bcg->getClass() << "C00D1S" << br->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(x01d1,Xj->name() << "A" << bcg->getClass() << "C01D1S" << br->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(x10d1,Xj->name() << "A" << bcg->getClass() << "C10D1S" << br->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(x11d1,Xj->name() << "A" << bcg->getClass() << "C11D1S" << br->getCorrespondingBBNumber());
							C01_1->addRight(1,x00d1);
							C10_1->addRight(1,x01d1);
							C11_1->addRight(1,x10d1);
							C11_1->addRight(1,x11d1);							
						}
						if(withNT) {
							Var *x00d0, *x01d0, *x10d0, *x11d0;
							NEW_VAR_FROM_BUFF(x00d0,Xj->name() << "A" << bcg->getClass() << "C00D0S" << br->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(x01d0,Xj->name() << "A" << bcg->getClass() << "C01D0S" << br->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(x10d0,Xj->name() << "A" << bcg->getClass() << "C10D0S" << br->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(x11d0,Xj->name() << "A" << bcg->getClass() << "C11D0S" << br->getCorrespondingBBNumber());
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
					NEW_VAR_FROM_BUFF(x00start,Xi->name() << "A" << bcg->getClass() << "C00start");
					NEW_VAR_FROM_BUFF(x01start,Xi->name() << "A" << bcg->getClass() << "C01start");
					NEW_VAR_FROM_BUFF(x10start,Xi->name() << "A" << bcg->getClass() << "C10start");
					NEW_VAR_FROM_BUFF(x11start,Xi->name() << "A" << bcg->getClass() << "C11start");
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
							NEW_VAR_FROM_BUFF(x00_d1,Xi->name() << "A" << bcg->getClass() << "C00D1S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(x01_d1,Xi->name() << "A" << bcg->getClass() << "C01D1S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(x10_d1,Xi->name() << "A" << bcg->getClass() << "C10D1S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(x11_d1,Xi->name() << "A" << bcg->getClass() << "C11D1S" << s->target()->getCorrespondingBBNumber());
							C00_2->addRight(1,x00_d1);
							C01_2->addRight(1,x01_d1);
							C10_2->addRight(1,x10_d1);
							C11_2->addRight(1,x11_d1);
						}
						if(withNT) {
							Var *x00_d0, *x01_d0, *x10_d0, *x11_d0;
							NEW_VAR_FROM_BUFF(x00_d0,Xi->name() << "A" << bcg->getClass() << "C00D0S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(x01_d0,Xi->name() << "A" << bcg->getClass() << "C01D0S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(x10_d0,Xi->name() << "A" << bcg->getClass() << "C10D0S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(x11_d0,Xi->name() << "A" << bcg->getClass() << "C11D0S" << s->target()->getCorrespondingBBNumber());
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
						NEW_VAR_FROM_BUFF(e00_d1,Xi->name() << "A" << bcg->getClass() << "C00D1end");
						NEW_VAR_FROM_BUFF(e01_d1,Xi->name() << "A" << bcg->getClass() << "C01D1end");
						NEW_VAR_FROM_BUFF(e10_d1,Xi->name() << "A" << bcg->getClass() << "C10D1end");
						NEW_VAR_FROM_BUFF(e11_d1,Xi->name() << "A" << bcg->getClass() << "C11D1end");
						C00_2->addRight(1,e00_d1);
						C01_2->addRight(1,e01_d1);
						C10_2->addRight(1,e10_d1);
						C11_2->addRight(1,e11_d1);							
					}
					if(br->exitsWithNT()) {
						Var *e00_d0,*e01_d0,*e10_d0,*e11_d0;
						NEW_VAR_FROM_BUFF(e00_d0,Xi->name() << "A" << bcg->getClass() << "C00D0end");
						NEW_VAR_FROM_BUFF(e01_d0,Xi->name() << "A" << bcg->getClass() << "C01D0end");
						NEW_VAR_FROM_BUFF(e10_d0,Xi->name() << "A" << bcg->getClass() << "C10D0end");
						NEW_VAR_FROM_BUFF(e11_d0,Xi->name() << "A" << bcg->getClass() << "C11D0end");
						C00_2->addRight(1,e00_d0);
						C01_2->addRight(1,e01_d0);
						C10_2->addRight(1,e10_d0);
						C11_2->addRight(1,e11_d0);													
					}
				}
				
			} /// Fin 2.4
#endif			

#if C_25 > 0
			////////////////////////
			// 2.5: wrong branches
			////////////////////////
			{
				NEW_SPECIAL_CONSTRAINT(M_T,EQ,0);
				NEW_SPECIAL_CONSTRAINT(M_NT,EQ,0);
				Var *m0, *m1;
				for(BasicBlock::OutIterator edge(bb); edge ; edge++ ) {
					if(edge->kind() == Edge::TAKEN) { // WARNING ces accolades sont IMPERATIVES car NEW_VAR_FROM_BUFF definit un bloc
						NEW_VAR_FROM_BUFF(m1,	"m" << bb->number() << "_" << edge->target()->number())
					}
					else if(edge->kind() == Edge::NOT_TAKEN) {// WARNING ces accolades sont IMPERATIVES car NEW_VAR_FROM_BUFF definit un bloc
						NEW_VAR_FROM_BUFF(m0,	"m" << bb->number() << "_" << edge->target()->number())
					}
				}
				system->addObjectFunction(5.0, m0);
				system->addObjectFunction(5.0, m1);
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
							NEW_VAR_FROM_BUFF(v00, Xi->name() << "A" << bcg->getClass() << "C00D1S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(v01, Xi->name() << "A" << bcg->getClass() << "C01D1S" << s->target()->getCorrespondingBBNumber());
							M_T->addRight(1,v00);
							M_T->addRight(1,v01);
						}
						if(withNT) {
							Var *v10, *v11;
							NEW_VAR_FROM_BUFF(v10, Xi->name() << "A" << bcg->getClass() << "C10D0S" << s->target()->getCorrespondingBBNumber());
							NEW_VAR_FROM_BUFF(v11, Xi->name() << "A" << bcg->getClass() << "C11D0S" << s->target()->getCorrespondingBBNumber());
							M_NT->addRight(1,v10);
							M_NT->addRight(1,v11);
							
						}
						var_added[s->target()->getCorrespondingBBNumber()] ++;
					}
				}
				if(br->isExit()) {
					if(br->exitsWithT()) {
						Var *e00_d1,*e01_d1;
						NEW_VAR_FROM_BUFF(e00_d1,Xi->name() << "A" << bcg->getClass() << "C00D1end");
						NEW_VAR_FROM_BUFF(e01_d1,Xi->name() << "A" << bcg->getClass() << "C01D1end");
						M_T->addRight(1,e00_d1);
						M_T->addRight(1,e01_d1);
					}
					if(br->exitsWithNT()) {
						Var *e10_d0,*e11_d0;
						NEW_VAR_FROM_BUFF(e10_d0,Xi->name() << "A" << bcg->getClass() << "C10D0end");
						NEW_VAR_FROM_BUFF(e11_d0,Xi->name() << "A" << bcg->getClass() << "C11D0end");
						M_NT->addRight(1,e10_d0);
						M_NT->addRight(1,e11_d0);													
					}
				}

			} /// Fin 2.5
#endif
		} /// Fin parcours des BR de la classe
		
	} /// Fin parcours des classes
}
	


/**
 * Generates a BCG from the given CFG and address class.
 * 
 * @param cfg		CFG.
 * @param bs		Bset structure.
 * @param addr		Address class.
 * @param graphs	vector to stroe the new BCG in.
 */
void BPredProcessor::simplifyCFG(CFG* cfg, BSets& bs, int addr, elm::genstruct::Vector<BCG*> &graphs) {
	// create the bitSet
	elm::genstruct::Vector<int> *bit_sets = new elm::genstruct::Vector<int>[cfg->countBB()+1];
	elm::genstruct::Vector<int> in_outs;
	for(int i = 0 ; i < cfg->countBB()+1 ; ++i){
		for(int j = 0 ; j<cfg->countBB() ; ++j ) {
			bit_sets[i].add(0);
		}
		in_outs.add(0);
	}
	
	int visited[cfg->countBB()];
	for(int i=0;i<cfg->countBB() ; ++i)
		visited[i]=0;
	
	computePredecessors(cfg, cfg->entry(), bit_sets, in_outs, bs, addr, visited);

//	///////////////
//	cout << "@" << addr << "\n";
//	for(int i=0;i<cfg->countBB();++i) {
//		cout << "[" << i << ":" << bit_sets[cfg->countBB()][i] << "] ";
//	}
//	cout << "\n";
//	///////////////

	BCGNode* bb_created[cfg->countBB()];
	for(int i=0; i<cfg->countBB(); ++i ) bb_created[i]=NULL;
	BCG* g=new BCG(addr);
	BCGNode *n_org, *n_pred;
	BCGEdge *e;
	for(int i = 0 ; i<cfg->countBB(); ++i) {
		if(addr == bs.get_addr(i)) {
			// si le BCGNode correspondant n'existe pas, on le crée
			if(bb_created[i] == NULL) {
				if(getBit(in_outs[i],1)==1) {
					n_org= new BCGNode(i,getBit(in_outs[i],0)==1 , true,
									getBit((bit_sets[cfg->countBB()])[i],1)==1, // Taken ?
									getBit((bit_sets[cfg->countBB()])[i],0)==1); // Not Taken ?
				}
				else {
					n_org= new BCGNode(i,getBit(in_outs[i],0)==1 , false, false, false);					
				}
				g->add(n_org);
				bb_created[i] = n_org;
			}
			else
				n_org = bb_created[i];
				
			for(int j=0 ; j<cfg->countBB(); ++j) {
				if(addr == bs.get_addr(j) && getBit((bit_sets[i])[j],2) == 1) {
					// si le BCGNode correspondant n'existe pas, on le crée
					if(bb_created[j] == NULL) {
						if(getBit(in_outs[j],1)==1) {
							n_pred= new BCGNode(j,getBit(in_outs[j],0)==1 , true, 
											getBit((bit_sets[cfg->countBB()])[j],1)==1, // Taken ?
											getBit((bit_sets[cfg->countBB()])[j],0)==1); // Not Taken ?
						}
						else {
							n_pred= new BCGNode(j,getBit(in_outs[j],0)==1 , false, false, false);					
						}
						
						g->add(n_pred);
						bb_created[j] = n_pred;
					}
					else
						n_pred = bb_created[j];

					
					// TAKEN ?
					if(getBit((bit_sets[i])[j],0) == 1) {
						e=new BCGEdge(n_pred,n_org,false);
					}

					// NOT TAKEN ?
					if(getBit((bit_sets[i])[j],1) == 1) {
						e=new BCGEdge(n_pred,n_org,true);
					}
				}
			} // for(j...)		
		} // if(addr...)
	}
	
	graphs.add(g);

	delete[] bit_sets;
}

/**
 * For a given address class, determines which blocs are predecessors to the given one, and if, in this particular class, it is an entry and/or exit block.
 * 
 * @param cfg		The CFG to analyze.
 * @param bb		The BasicBloc.
 * @param bit_sets	Vector that indicates which bloc are successors.
 * @param in_outs	Vector that stores which blocs are exits/entries.
 * @param bs		BSet structure.
 * @param addr		Address class of the BasicBlock.
 * @param visited	Array that remembers which basic block has been visited.
 */
void BPredProcessor::computePredecessors(CFG* cfg, BasicBlock* bb, elm::genstruct::Vector<int> *bit_sets, elm::genstruct::Vector<int> &in_outs, BSets& bs, int addr,int visited[]) {
	visited[bb->number()]++;
	if(bb != cfg->exit()) {
		
		// in_out
		if(bb == cfg->entry()) {
			setBit( in_outs[bb->number()], 0 ); // set IN bit
		}
		/////////

		for(BasicBlock::OutIterator edge(bb); edge ; edge++) {
			BasicBlock *next_bb = edge->target();
			
			if(bs.get_addr(bb->number()) == addr ){
				// si ce bloc appartient bien a la classe addr alors on met a jour les bitsets suivants 
				// si cela ne produit aucun changement, alors on ne continue pas recursivement
				if( getBit((bit_sets[next_bb->number()])[bb->number()],2) == 1){
						if( 	(edge->kind() == Edge::TAKEN && getBit((bit_sets[next_bb->number()])[bb->number()],1) == 1)
							||	(edge->kind() == Edge::NOT_TAKEN && getBit((bit_sets[next_bb->number()])[bb->number()],0) == 1)){
							continue;
						}
				}
				setBit((bit_sets[next_bb->number()])[bb->number()],2);
				if(edge->kind() == Edge::TAKEN ) setBit((bit_sets[next_bb->number()])[bb->number()],1);
				if(edge->kind() == Edge::NOT_TAKEN ) setBit((bit_sets[next_bb->number()])[bb->number()],0);
				

			}
			else {
				// sinon on propage le bitset aux suivants
				bool changed = false;
				for(int i=0 ; i<bit_sets[bb->number()].length() ; ++i) {
					int k = (bit_sets[next_bb->number()])[i] ;
					(bit_sets[next_bb->number()])[i] |= (bit_sets[bb->number()])[i];
					changed = changed || (k != (bit_sets[next_bb->number()])[i]) ;
				}

				//in_out
				if(getBit(in_outs[bb->number()],0) == 1) {
					setBit(in_outs[next_bb->number()],0);
				}
				////////
				
				if(visited[next_bb->number()] > 0 && !changed ) continue;
				

			}	

			// puis on appel recursivement generateBitSet
			computePredecessors(cfg,next_bb,bit_sets, in_outs,bs,addr, visited);

			// in_out
			if(bs.get_addr(next_bb->number()) != addr) {
				if(getBit(in_outs[next_bb->number()],1) == 1) {
					setBit(in_outs[bb->number()],1);
				}
			}
			/////////
		}
	}
	// in_out
	else {
		setBit(in_outs[bb->number()],1);
		int exit_num = cfg->countBB();

		if(bs.get_addr(bb->number()) == addr ) {
			setBit((bit_sets[exit_num])[bb->number()],2);
			setBit((bit_sets[exit_num])[bb->number()],1);
			setBit((bit_sets[exit_num])[bb->number()],0);
		}
		else {
			for(int i=0 ; i<bit_sets[bb->number()].length() ; ++i) {
				int k = (bit_sets[exit_num])[i] ;
				(bit_sets[exit_num])[i] |= (bit_sets[bb->number()])[i];
			}
			
		}
	}
	/////////
}

/**
 * Fill-in the BSet structure by identifying each brbanch address class.
 * 
 * @param cfg	CFG to analyze.
 * @param bs	BSets structure to fill-in.
 */
void BPredProcessor::generateClasses(CFG *cfg, BSets& bs) {
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		unsigned int nb_OE = 0;

		// Parcours des OutEdges
		for(BasicBlock::OutIterator edge(bb); edge ; edge++ ) {
			// on incremente que s'il s'agit d'un edge TAKEN ou NOT_TAKEN
			if(edge->kind() == Edge::TAKEN) nb_OE++;
			else if(edge->kind() == Edge::NOT_TAKEN) nb_OE++;
		}

		// Si un branchement a ete trouve ...
		if(nb_OE == 2 ) {
			Inst* inst = NULL;
			for(BasicBlock::InstIter i(bb); i; i++) {
				inst=i;
			}
			bs.add((inst->address() & (this->BHT)), bb->number());
		}
	}
}

/**
 * Creates all the required graphs and structures to create the constraints system based on the "bimodal" model.
 * 
 * @param ws	Current workspace.
 * @param cfg	CFG to extracts the graphs and structures from.
 */
void BPredProcessor::processCFG__Bimodal(WorkSpace *fw, CFG *cfg) {
	BSets bs;
	generateClasses(cfg,bs);

 
	elm::genstruct::Vector<int> l_addr;
	elm::genstruct::Vector<BCG*> bcgs;
	bs.get_all_addr(l_addr);

	PropList props;
	
	for(int a=0; a<l_addr.length(); ++a) {

		simplifyCFG(cfg,bs,l_addr[a],bcgs);
		
		if(this->dumpBCG) {
			StringBuffer buf;
			buf << "bcg_@" << l_addr[a] << ".ps";
			String filename = buf.toString();
			otawa::display::OUTPUT_PATH(props) = filename.toCString();
			BCGDrawer drawer(bcgs[a], props);
			drawer.display();
		}

		if(this->withStats) {
			this->stat_addr.add(l_addr[a]);
			elm::genstruct::Vector<int> v;
			bs.get_vector(l_addr[a],v);
			this->stat_nbbr.add(v.length());
		}

	}
	
	CS__BiModal(fw,cfg,bs,bcgs);
	
}

} // ::ipet
} // ::otawa
