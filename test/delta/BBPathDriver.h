#ifndef SIM_BBPATHDRIVER_H
#define SIM_BBPATHDRIVER_H

#include <otawa/cfg.h>
#include <otawa/ipet/BBPath.h>
#include <otawa/sim/State.h>

namespace otawa { namespace sim {

class InstIterator;

class BBPathDriver: public Driver {
	Iterator<BasicBlock*> bb_iter;
	Iterator<Inst*> *inst_iter;
	bool ended;
public:
	BBPathDriver(ipet::BBPath& bbpath);
	virtual ~BBPathDriver();
	virtual Inst* firstInstruction(State &state);
	virtual Inst* nextInstruction(State &state, Inst *inst);
	virtual void terminateInstruction(State &state, Inst *inst);
};

} }

#endif /*SIM_BBPATHDRIVER_H*/
