#ifndef OTAWA_TEST_PCG_PCGBUILDER_H
#define OTAWA_TEST_PCG_PCGBUILDER_H

#include <otawa/proc/Processor.h>
#include <otawa/cfg/CFGInfo.h>
#include <otawa/cfg.h>
#include <elm/genstruct/HashTable.h>

#include "PCG.h"

namespace otawa{

class PCGBuilder: public Processor
{
	elm::genstruct::HashTable <void *, PCGBlock *> mapCFG;
	elm::genstruct::HashTable <void *, PCGBlock *> mapBB;

public:	
	void processCFGBlocks(BasicBlock *bb,CFG* cfg,PCG* pcg,PCGBlock *src);
	PCGBuilder(const PropList& props = PropList::EMPTY);
	virtual void processCFG(CFG* cfg, PCG *pcg,CFG*src);
	virtual void configure(const PropList& props);
	virtual void processFrameWork(FrameWork *fw){};
	PCG* buildPCG(CFG*cfg);
	void addPCGBlock(BasicBlock*bb,CFG*cfg,PCG*pcg,CFG*src);
};
}
#endif //OTAWA_TEST_PCG_PCGBUILDER_H
