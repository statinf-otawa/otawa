/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/prog/PCGBuilder.cpp -- implementation of PCGBuilder class.
 */

#include <elm/assert.h>
#include <elm/PreIterator.h>
#include <otawa/manager.h>
#include <otawa/cfg/Edge.h>
#include <otawa/pcg/PCGBuilder.h>
#include <otawa/cfg/CFGCollector.h>

using namespace elm;
using namespace otawa;

//#define DO_TRACE
#if defined(NDEBUG) && !defined(DO_TRACE)
#	define TRACE(c)
#else
#	define TRACE(c) cout << c
#endif

namespace otawa {

/*
 * Get the main CFG, build the corresponding PCG and print it to sceen.
 * @class PCGBlock
 */
PCG* PCGBuilder::buildPCG(CFG *cfg)
{
	ASSERT(cfg);
	PCG *pcg=new PCG(cfg);
	processCFG(cfg,pcg,NULL, 0);

	//cout<<"nb outs de main: "<<(((pcg->pcgbs()).get(0))->getSons()).length()<<"\n";
	//cout<<"nb pcgb: "<<pcg->pcgbs().length()<<"\n";
	/*TRACE("\nPCG\n");

	for(PCG::PCGIterator pcgb(pcg);pcgb;pcgb++)
	{
		TRACE("\n"<<pcgb->getName()<<" "<<pcgb->getAddress()<<"\n");
		for(PCGBlock::PCGBlockOutIterator _pcgb(pcgb);_pcgb;_pcgb++)
			TRACE("\t"<<_pcgb->getName()<<" "<<_pcgb->getAddress()<<"\n");
	}
	TRACE("\n");
	TRACE("liste des fonctions appelantes de chaque fonction\n");
	for(PCG::PCGIterator pcgb(pcg);pcgb;pcgb++)
	{
		TRACE("\n"<<pcgb->getName()<<" "<<pcgb->getAddress()<<"\n");
		for(PCGBlock::PCGBlockInIterator _pcgb(pcgb);_pcgb;_pcgb++)
			TRACE("\t"<<_pcgb->getName()<<" "<<_pcgb->getAddress()<<"\n");
	}*/
	return pcg;
}

/*
 * Gets the CFG of the called Function, the PCG to complete and the CFG of the calling function
 * It builds a PCGBlock for the current function and links it to the PCGBlock of the calling function,
 * then it scans the Called Function CFG. If it finds a new function in this CFG, it will create a corresponding
 * PCGBlock in the  PCG and build a link between this block and the block of the current function and then
 * we have the recursive call of this method for every function found in this scan in order to cover every function call.
 * @class PCGBlock
 */
void PCGBuilder::processCFG(CFG* cfg,PCG* pcg,CFG * src, stack_t *up)
{	//il  faut parcourir le cfg de base ainsi que le cfg des fonctions appelees. On conserve la trace du cfg appelant
	//pour la construction du PCG

	// Look for recursivity
	for(stack_t *cur = up; cur; cur = cur->up)
		if(cur->cfg == cfg)
			return;

	// Prepare stack
	stack_t ablock = { up, cfg };

	// Look for call BB
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		if(bb->isEntry())
			addPCGBlock(bb,cfg,pcg,src);
		if(bb->isCall())
		{
			for(otawa::BasicBlock::OutIterator edge(bb);edge;edge++)
			{
				if(edge->kind()==Edge::CALL)
					processCFG(edge->calledCFG(), pcg, cfg, &ablock);
			}
		}
	}

}
/*
 * adds the PCGBlock to the PCG
 */
void PCGBuilder::addPCGBlock(BasicBlock *bb,CFG* cfg,PCG* pcg,CFG *src)
{
	PCGBlock* pcg_bb;
	if(mapBB.get(bb,0)==0)
	{
		pcg_bb=new PCGBlock(bb,cfg);
		mapCFG.put(cfg,pcg_bb);
		mapBB.put(bb,pcg_bb);
		pcg->addPCGBlock(pcg_bb);
	}
	else
		pcg_bb=mapBB.get(bb,0);
	//avant d'établir de nouveaux liens entre les PCGBlocks il faut voir si ces leins existent déjà
	if((src==NULL)&&(cfg->label()!="main"))
	{
		if(!pcg_bb->getFathers().contains(mapCFG.get(cfg,0)))
			pcg_bb->addInLink(mapCFG.get(cfg,0));
		if(!mapCFG.get(cfg,0)->getSons().contains(pcg_bb))
			mapCFG.get(cfg,0)->addOutLink(pcg_bb);
	}
	if(src!=NULL)
	{
		if(!mapCFG.get(src,0)->getSons().contains(pcg_bb))
			mapCFG.get(src,0)->addOutLink(pcg_bb);
		if(!pcg_bb->getFathers().contains(mapCFG.get(src,0)))
			pcg_bb->addInLink(mapCFG.get(src,0));
	}
}
/*
 * PCGBuilder constructer
 *
 * @par Required Features
 * @li @ref COLLECTED_CFG_FEATURE
 *
 * @para Provided Features
 * @li @ref PCG_FEATURE
 */
PCGBuilder::PCGBuilder(void):Processor("PCGBuilder", Version(1, 0, 0)){
	require(COLLECTED_CFG_FEATURE);
	provide(PCG_FEATURE);
}


/**
 */
void PCGBuilder::processWorkSpace(WorkSpace *fw) {
	CFG *cfg = ENTRY_CFG(fw);
	ASSERT(cfg);
	PCG::ID(fw) = buildPCG(cfg);
}


/**
 * This feature ensure that a PCG is provided.
 *
 * @par Properties
 * @li @ref PCG::ID (FrameWork)
 */
Feature<PCGBuilder> PCG_FEATURE("otawa::PCG_FEATURE");


/**
 * This properties is used to hook the PCG of the current task to the FrameWork.
 *
 * @par Hooks
 * @li @ref FrameWork
 */
Identifier<PCG *> PCG::ID("otawa::PCG::ID", 0);


} // otawa

/*void PCGBuilder::processCFG(CFG* cfg) old
{	//cette version n'est pas bonne car le parcous du cfg n'est pas bien implementé on saute des bb utils
	ASSERT(cfg);
	int count=0;
	int countbbisEntryCFG=0;
	PCG *pcg=new PCG(cfg);
	//affichage des appels issu de chaque bloc call
	//cout<<"\n\nles calls deduits du parcours du cfg sans la construction du PCG\n";
	//afficherCFG(cfg);
	for(Iterator<BasicBlock*> bb(cfg->bbs()); bb; bb++)
	{
		//processCFGBlocks1(bb,cfg,pcg,NULL);
		if(bb->isEntry())
			processCFGBlocks(bb,cfg,pcg,NULL);
		if(bb->isCall())
		{
			for(otawa::BasicBlock::OutIterator edge(bb);edge;edge++)
			{
				if(edge->kind()==Edge::CALL)
				//on va tenir compte de ce NULL dans processCFGBlocks
				//car edge->target() est traité précedemment s'il est appelé par main ou
				//par une autre fonction donc son père est connu sinon alors son père est main
				//car il doit etre appelé de quelque part
					processCFGBlocks(edge->target(),edge->calledCFG(),pcg,NULL);
			}
		}
	}

	cout<<"\nPCG\n";
	for(PCG::PCGIterator pcgb(pcg);pcgb;pcgb++)
	{
		cout<<"\n"<<pcgb->getName()<<" "<<pcgb->getAddress()<<"\n";

		for(PCGBlock::PCGBlockOutIterator _pcgb(pcgb);_pcgb;_pcgb++)
			cout<<"\t"<<_pcgb->getName()<<" "<<_pcgb->getAddress()<<"\n";

	}
	cout<<"\n";
}
void PCGBuilder::afficherCFG(CFG*cfg)
{
	for(Iterator<BasicBlock*> bb(cfg->bbs()); bb; bb++)
	{	if(bb->isEntry()) cout<<cfg->label()<<"\n";
		if(bb->isCall())
		{
			for(otawa::BasicBlock::OutIterator edge(bb);edge;edge++)
			{
				if(edge->kind()==Edge::CALL)
				{
					//cout<<"\t"<<edge->calledCFG()->label();
						cout<<"\t";
					afficherCFG(edge->calledCFG());
				}
			}
		}
	}
}
void PCGBuilder::processCFGBlocks(BasicBlock *bb, CFG* cfg, PCG* pcg,PCGBlock *src)with the old processCFG
{	//cette version n'est pas bonne car on ne tient pas compte de tous les calls
	PCGBlock *pcg_bb;
	if(!mapCFG.get(cfg,0))
	{
		pcg_bb=new PCGBlock(bb,cfg);
		pcg->addPCGBlock(pcg_bb);
		mapCFG.put(cfg,pcg_bb);
		mapBB.put(bb,pcg_bb);

	}
	else
		pcg_bb=mapBB.get(bb,NULL);

	if(src==NULL)
	{
		if((cfg->label()!="main") && (pcg_bb->getFathers().length()==0))
		{//si c'est le pcg_bb du main donc il n'a pas de père
		//si ce n'est pas le pcg du main et il n'aps de pères c.a.d il n'a pas été traité par la récursivité
		//donc son père est main, ceci est du au fait qu'il est appelé par un
		//bb (call) qui n'est pas un fils direct de main, et le fils de main accédant à ce bb n'est pas un call
			pcg_bb->addInLink(mapCFG.get(pcg->getCFG(),0));
			mapCFG.get(pcg->getCFG(),0)->addOutLink(pcg_bb);
		}
	}
	else
	{
		src->addOutLink(pcg_bb);
		pcg_bb->addInLink(src);
	}

	for(otawa::BasicBlock::OutIterator edge(bb);edge;edge++)
	{	cout<<"\t";
		if(edge->kind()==Edge::CALL)
			processCFGBlocks(edge->target(),edge->calledCFG(),pcg,pcg_bb);
	}
}*/


/*void PCGBuilder::processCFGBlocks2(BasicBlock *bb, CFG* cfg, PCG* pcg,PCGBlock *src)
{
	PCGBlock *pcg_bb;
	if(!bb->isCall() && !bb->isEntry())
	{
		for(otawa::BasicBlock::OutIterator edge(bb);edge;edge++)
		{
			if(edge->kind()==Edge::cout<<"nb pcgb: "<<pcg->pcgbs().length()<<"\n";	CALL)
				processCFGBlocks(edge->target(),edge->calledCFG(),pcg,NULL);
		}
	}
else{
	if(!mapCFG.get(cfg,0))
	{
		pcg_bb=new PCGBlock(bb,cfg);
		pcg->addPCGBlock(pcg_bb);
		mapCFG.put(cfg,pcg_bb);
		mapBB.put(bb,pcg_bb);
	}
	else
		pcg_bb=mapBB.get(bb,NULL);

	if(src==NULL)
	{
		if(pcg_bb->getName()!="main" && (pcg_bb->getFathers().length()==0))
		{//si c'est le pcg_bb du main donc il n'a pas de père
		//si ce n'est pas le pcg du main et il n'aps de pères c.a.d il n'a pas été traité par la récursivité
		//donc son père est main, ceci est du au fait qu'il est appelé par un
		//bb (call) qui n'est pas un fils direct de main, et le fils de main accédant à ce bb n'est pas un call
			pcg_bb->addInLink(mapCFG.get(pcg->getCFG(),0));
			mapCFG.get(pcg->getCFG(),0)->addOutLink(pcg_bb);
		}
	}
	else
	{
		src->addOutLink(pcg_bb);
		pcg_bb->addInLink(src);
	}

	for(otawa::BasicBlock::OutIterator edge(bb);edge;edge++)
	{
		if(edge->kind()==Edge::CALL)
			processCFGBlocks(edge->target(),edge->calledCFG(),pcg,pcg_bb);
	}
}//end else

}*/
