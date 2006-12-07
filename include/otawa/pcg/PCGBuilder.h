#ifndef OTAWA_TEST_PCG_PCGBUILDER_H
#define OTAWA_TEST_PCG_PCGBUILDER_H

#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/CFGInfo.h>
#include <otawa/cfg.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/proc/Feature.h>

#include "PCG.h"

namespace otawa{

class PCGBuilder: public Processor
{
	elm::genstruct::HashTable <void *, PCGBlock *> mapCFG;
	elm::genstruct::HashTable <void *, PCGBlock *> mapBB;

	void processCFGBlocks(BasicBlock *bb,CFG* cfg,PCG* pcg,PCGBlock *src);
	PCG* buildPCG(CFG*cfg);
	void addPCGBlock(BasicBlock*bb,CFG*cfg,PCG*pcg,CFG*src);

protected:
	virtual void processFrameWork(FrameWork *fw);
	virtual void processCFG(CFG* cfg, PCG *pcg, CFG*src);

public:	
	PCGBuilder(void);
};

// Features
extern Feature<PCGBuilder> PCG_FEATURE;

} // otawa

#endif //OTAWA_TEST_PCG_PCGBUILDER_H
