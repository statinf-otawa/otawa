/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/cache_CATBuilder.cpp -- CATBuilder class implementation.
 */
#include <stdio.h>
#include <elm/io.h>
#include <otawa/cache/categorisation/CATBuilder.h>
#include <otawa/instruction.h>
#include <otawa/cache/LBlock.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cfg.h>
#include <otawa/hardware/CacheConfiguration.h>

using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;

namespace otawa {



void CATBuilder::processCFG(FrameWork *fw, CFG *cfg ) {
	
	assert(cfg);
	ilp::System *system = cfg->get<System *>(ipet::IPET::ID_System, 0);
	assert (system);
	LBlockSet *id = cfg->use<LBlockSet *>(LBlockSet::ID_LBlockSet);
	
		
	const Cache *cach = fw->platform()->cache().instCache();
	
	

	string type = "cat";
	// Node 's' of CCG
	new LBlock(id, 0, 0, 0, 0, 0, type);
		
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++) {
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
				// on cheches les lblocs de la ligne j dans une cache de 8 lignes
				if ((tag % 8) != id->cacheline())find = false;
				*/
				if (((int)cach->line(address))!= id->cacheline())find = false;
				//if ((!find)&&((tag % 8) == id->cacheline())){
				if ((!find)&&(((int)cach->line(address))==id->cacheline())){
					address_t address = inst->address();
					StringBuffer buf;
					//buf.print("xhit%lx(%lx)",address,*bb);
					buf << "xhit" << address << *bb; 
					String namex = buf.toString();
					ilp::Var *vhit = system->newVar(namex);
					StringBuffer buf1;
					//buf1.print("xmiss%lx(%lx)", address, *bb);
					buf1 << "xmiss" << address << *bb; 
					String name1 = buf1.toString();
					ilp::Var *miss = system->newVar(name1);
					new LBlock(id , address ,bb, vhit, miss, bbv, type);
					find = true;
				}
			}
		else if(pseudo->id() == bb->ID)
			break;
		}
	}
	//Node 'END' of the CCG
	new LBlock(id, 0, 0, 0, 0, 0, type);

	
	
	int length = id->returnCOUNTER();	
	cout << length - 2 << " "<< "lblocks has found \n";
	for (Iterator<LBlock *> lbloc(id->visitLBLOCK()); lbloc; lbloc++){			
		int identif = lbloc->identificateurLBLOCK();				
		address_t address = lbloc->addressLBLOCK();
	
		if (identif == 0) cout << "S" <<" "<< identif << " " <<address <<'\n';
		 else if (identif == (length - 1)) cout << "END" <<" " << identif << " "<<address <<'\n';
			else cout << "Lblock " << identif << " " << address <<'\n';		
	}
}
}



