#ifndef OTAWA_PCG_PCGBUILDER_H
#define OTAWA_PCG_PCGBUILDER_H

#include <elm/genstruct/HashTable.h>
#include <otawa/cfg.h>
#include <otawa/proc/CFGProcessor.h>
#include "features.h"

namespace otawa{

class PCGBuilder: public Processor {
public:
	static p::declare reg;
	PCGBuilder(p::declare& r = reg);

protected:
	virtual void processWorkSpace(WorkSpace *fw);

private:
	elm::genstruct::HashTable <CFG *, PCGBlock *> map;
};

// Features
extern p::feature PCG_FEATURE;

} // otawa

#endif //OTAWA_PCG_PCGBUILDER_H
