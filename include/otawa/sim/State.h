/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/sim/State.h -- State class interface.
 */
#ifndef OTAWA_SIM_STATE_H
#define OTAWA_SIM_STATE_H

#include <otawa/base.h>

namespace otawa {

// External classes
class Inst;
	
namespace sim {

// mode_t enumeration
typedef enum mode_t {
	NORMAL = 0,
	EXCEPTION,
	ERROR,
	SLEEPING,
	HALTED
} mode_t;


// State class
class State {
public:
	virtual ~State(void);
	virtual State *clone(void) = 0;
	virtual mode_t step(void) = 0;
	virtual mode_t run(void) = 0;
	virtual mode_t runUntil(address_t addr) = 0;
	virtual mode_t runUntil(Inst *inst) = 0;
	virtual mode_t runUntilBranch(void) = 0;
	virtual mode_t flush(void) = 0;
	virtual int cycle(void) = 0;
	virtual void reset(void) = 0;
	virtual address_t getPC(void) = 0;
	virtual Inst *pcInst(void) = 0;
	virtual void setPC(address_t pc) = 0;
	//virtual void setPC(Inst *inst) = 0;
};

} } // otawa::sim
	
#endif /* OTAWA_SIM_STATE_H */
