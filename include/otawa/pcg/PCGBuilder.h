#ifndef OTAWA_PCG_PCGBUILDER_H
#define OTAWA_PCG_PCGBUILDER_H

#include <elm/data/HashMap.h>
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
	virtual void destroy(WorkSpace *ws);

private:
	elm::HashMap<CFG *, PCGBlock *> map;
	PCG *_pcg;
};

// Features
extern p::feature PCG_FEATURE;

} // otawa

#endif //OTAWA_PCG_PCGBUILDER_H
