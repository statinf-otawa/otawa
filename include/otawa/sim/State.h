/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/sim/State.h -- State class interface.
 */
#ifndef OTAWA_SIM_STATE_H
#define OTAWA_SIM_STATE_H

#include <otawa/base.h>
#include <otawa/sim/Driver.h>

namespace otawa {

// External classes
class Inst;
	
namespace sim {

// State class
class State {
protected:
	virtual ~State(void);
public:
	virtual State *clone(void) = 0;
	virtual void run(Driver& driver) = 0;
	virtual void stop(void) = 0;
	virtual void flush(void) = 0;
	virtual int cycle(void) = 0;
	virtual void reset(void) = 0;
};

} } // otawa::sim
	
#endif /* OTAWA_SIM_STATE_H */
