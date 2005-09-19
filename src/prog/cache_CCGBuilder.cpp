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
	
	
Identifier CCGBuilder::ID_In("ipet.ccg.dfain");

Identifier CCGBuilder::ID_Out("ipet.ccg.dfaout");

void CCGBuilder::processCFG(FrameWork *fw, CFG *cfg ) {
	assert(cfg);
	System *system = cfg->get<System *>(IPET::ID_System, 0);
	assert (system);
	LBlockSet *idccg = cfg->use<LBlockSet *>(LBlockSet::ID_LBlockSet);
	
	// cache configuration
	const Cache *cach = fw->platform()->cache().instCache();
	
	// decallage of x bits where each block containts 2^x ocets
	int dec = cach->blockBits();	
	
	string ccg = "ccg";
	CCGDFA dfa(idccg,cfg,cach);
	// Node 's' of CCG
	new LBlock(idccg, 0, 0, 0, 0, 0, ccg );
	
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++) {
		if ((!bb->isEntry())&&(!bb->isExit())){
			ilp::Var *bbv = bb->use<ilp::Var *>(IPET::ID_Var);
			Inst *inst;
			bool find = false;
			PseudoInst *pseudo;
			for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
				pseudo = inst->toPseudo();
				address_t address = inst->address();
				if (!pseudo){
					/*
					//decallage de "dec" bites et masquage de 3 bites
					unsigned long tag = (((unsigned long)inst->address()) >> dec)&0X7;
					// on cherche les lblocks de la ligne j dans une cache de 8 lignes
					if ((tag % 8) != idccg->cacheline())find = false;
					*/
					if (((int)cach->line(address))!= idccg->cacheline())find = false;
					//if ((!find)&&((tag % 8) == idccg->cacheline())){
					if ((!find)&&(((int)cach->line(address))==idccg->cacheline())){
					
						//naming variables
						StringBuffer buf;
						// buf.print("xhit%lx(%lx)", address,*bb);
						buf << "xhit" << address << "(" << *bb << ")";
						String namex = buf.toString();
						ilp::Var *vhit = system->newVar(namex);
						StringBuffer buf1;
						//buf1.print("xmiss%lx(%lx)", address,*bb);
						buf1 << "xmiss" << address << "(" << *bb << ")";
						String name1 = buf1.toString();
						ilp::Var *miss = system->newVar(name1);
						new LBlock(idccg , address ,bb, vhit, miss, bbv, ccg);
						find = true;
					}
				}
			
		else if(pseudo->id() == bb->ID)
			break;
			}
		}
				
	}//endBB
	
	// Node 'END' of the CCG	
	new LBlock(idccg, 0, 0, 0, 0, 0, ccg);
	// display the lblocks which have found
	int length = idccg->returnCOUNTER();	
	//cout <<length-2 << " "<< "lblocks has found \n";
	for (Iterator<LBlock *> lbloc(idccg->visitLBLOCK()); lbloc; lbloc++){			
		int identif = lbloc->identificateurLBLOCK();				
		address_t address = lbloc->addressLBLOCK();
	
		/*if (identif == 0) cout << "S" <<" "<< identif << " " <<address <<'\n';
		 else if (identif == (length - 1)) cout << "END" <<" " << identif << " "<<address <<'\n';
			else cout << "Lblock " << identif << " " << address <<'\n';		*/
	}
	
	//cout<<" starting DFA \n";
	// DFA prossecing
	dfa.DFA::resolve(cfg,&ID_In,&ID_Out);
	//cout << " DFA has constructed \n";

	// Detecting the non conflict state of each lblock
	BasicBlock *BB;
	LBlock *line;
	for (Iterator<LBlock *> lbloc(idccg->visitLBLOCK()); lbloc; lbloc++){
		if((lbloc->identificateurLBLOCK()!=0) &&(lbloc->identificateurLBLOCK()!= (length - 1))){
			BB = lbloc->blockbasicLBLOCK();
			DFABitSet *inid = BB->use<DFABitSet *>(ID_In);
			for (int i=0; i<inid->size();i++){
				if (inid->contains(i)){
					line = idccg->returnLBLOCK(i);
				
					unsigned long tagline = ((unsigned long)line->addressLBLOCK()) >> dec;
					unsigned long taglbloc = ((unsigned long)lbloc->addressLBLOCK()) >> dec;
					if ((tagline == taglbloc)&&(BB != line->blockbasicLBLOCK())){
					//if ((cach->tag(line->addressLBLOCK())) == (cach->tag(lbloc->addressLBLOCK()))
						//&&(BB != line->blockbasicLBLOCK())){
							lbloc->changeSTATENONCONF(true);
					}
			}
		}
		}
	}
	
	
	// Building the ccg edges using DFA
	dfa.addCCGEDGES(cfg ,&ID_In,&ID_Out);
	//cout << "all CCG EDGES has construted" <<"\n";	 
 }
} //otawa




