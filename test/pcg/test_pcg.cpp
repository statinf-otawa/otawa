/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test/ipet/test_ipet.cpp -- test for IPET feature.
 */

#include <stdlib.h>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/TrivialBBTime.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/ipet/BasicConstraintsBuilder.h>
#include <otawa/ipet/WCETComputation.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/ipet/BasicObjectFunctionBuilder.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/TrivialDataCacheManager.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/ilp.h>

#include <otawa/pcg.h>

using namespace elm;
using namespace otawa;
using namespace otawa::hard;

int main(int argc, char **argv) 
{
	// Construction de la hiérarchie de cache
 	Cache::info_t info;
 	info.block_bits = 3;  // 2^3 octets par bloc
 	info.row_bits = 3;   // 2^3 lignes
 	info.way_bits = 0;    // 2^0 élément par ensemble (cache direct)
 	info.replace = Cache::NONE;
 	info.write = Cache::WRITE_THROUGH;
 	info.access_time = 0;
 	info.miss_penalty = 10;
 	info.allocate = false;
 	Cache *level1 = new Cache(info);
 	CacheConfiguration *caches = new CacheConfiguration(level1);

	Manager manager;
	PropList props;
	CACHE_CONFIG(props) = caches;
	NO_SYSTEM(props) = true;
	
	try {
		
		// Load program
		if(argc < 2) {
			cerr << "ERROR: no argument.\n"
				 << "Syntax is : test_ipet <executable>\n";
			exit(2);
		}
		WorkSpace *fw = manager.load(argv[1], props);
		
		// Find main CFG
		//cout << "Looking for the main CFG\n";
		CFG *cfg = fw->getCFGInfo()->findCFG("main");
		if(cfg == 0) {
			cerr << "ERROR: cannot find main !\n";
			return 1;
		}
		//else
			//cout << "main   0x" << cfg->address() << '\n';
		
		// Removing __eabi call if available
		for(CFG::BBIterator bb(cfg); bb; bb++)
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					break;
				}
		
		// Now, use the CFG to build the PCG
		PCGBuilder PCGBD;
		PCGBD.process(fw, props);
		
		// Display the PCH
		PCG *pcg = PCG::ID(fw);
		assert(pcg);
		cout << "\nPCG\n";
		for(PCG::PCGIterator pcgb(pcg);pcgb;pcgb++) {
		cout << "\n" << pcgb->getName() << " " << pcgb->getAddress() << "\n";
			for(PCGBlock::PCGBlockOutIterator _pcgb(pcgb);_pcgb;_pcgb++)
				cout << "\t" << _pcgb->getName() << " "
					 << _pcgb->getAddress() << "\n";
		}
		cout << "\n";
		cout << "liste des fonctions appelantes de chaque fonction\n";
		for(PCG::PCGIterator pcgb(pcg); pcgb; pcgb++) {
			cout << "\n" << pcgb->getName() << " "
				 << pcgb->getAddress() << "\n";
			for(PCGBlock::PCGBlockInIterator _pcgb(pcgb); _pcgb; _pcgb++)
				cout << "\t" << _pcgb->getName() << " "
					 << _pcgb->getAddress() << "\n";
		}
		
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << '\n';
		return 1;
	}
	return 0;
}

/* version incomplete car il faut cionsiderer les boucles et les eviter
inline void PCG::PCG_Construction2(BasicBlock *bb,CFG* cfg,PCGBlock* callingBlock)
{
	static elm::genstruct::HashTable <void *, PCGBlock *> map;
	PCGBlock *pcg_bb=NULL;

	if(bb->isExit())
		return;
	
	if(bb->isEntry()||bb->isCall())
	{
		if(bb->isEntry())
		{
			pcg_bb=new PCGBlock(bb);
			map.put(cfg,pcg_bb);
			_pcgbs.add(pcg_bb);
		}
		else
		{
			if(map.get(cfg,NULL)->getBB()==bb)
			{
				callingBlock->addOutLink(map.get(cfg,0));
				map.get(cfg,NULL)->addInLink(callingBlock);
			}
			else
			{	
				pcg_bb=new PCGBlock(bb);	
				_pcgbs.add(pcg_bb);
				map.put(cfg,pcg_bb);
				if(callingBlock!=NULL)
				{
					callingBlock->addOutLink(pcg_bb);
					pcg_bb->addInLink(callingBlock);
				}
			}
		}
				
		for(BasicBlock::OutIterator edge(bb); edge; edge++)
		{
			if(edge->kind()==Edge::CALL)
				PCG_Construction2(edge->target(),edge->calledCFG(),pcg_bb);
		}	
	}
	else //pour visiter tous les bb du cfg en laissant une trace du block entree du cfg du main ou d'une fct qlq
	{
		for(BasicBlock::OutIterator edge(bb); edge; edge++)
			PCG_Construction2(edge->target(),cfg,callingBlock);
	}
}*/
