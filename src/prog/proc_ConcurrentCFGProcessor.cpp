/*
 *	ConcurrentCFGProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include <elm/sys/Thread.h>
#include <otawa/proc/ConcurrentCFGProcessor.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {

/**
 * @class ConcurrentCFGProcessor
 * Implements a concurrent version of @ref CFGProcessor.
 * The CFG are processed in each thread and the thread calls
 * the processCFG() method.
 *
 * OTAWA is only responsible for maintaining the property list
 * state consistent: a thread processing a CFG is only allowed
 * to modify properties of its own CFG, blocks and edges.
 * Other shared state data item must be handled by its own
 * way by the user class.
 *
 * Notice that for readability purpose, concurrency is disabled
 * as soon as logging for CFG level is used.
 *
 * @ingroup proc
 */

#ifdef OTAWA_CONC
class CFGRunnable: public sys::Runnable {
public:
	CFGRunnable(ConcurrentCFGProcessor& p): proc(p) { }
	virtual void run(void) { proc.nextCFG(); }
private:
	ConcurrentCFGProcessor& proc;
};
#endif

/**
 */
ConcurrentCFGProcessor::ConcurrentCFGProcessor(p::declare& r): CFGProcessor(r), mutex(0) {
}

/**
 */
void ConcurrentCFGProcessor::processWorkSpace(WorkSpace *ws) {
#	ifndef OTAWA_CONC
		CFGProcessor::processWorkSpace(ws);
#	else
		if(logFor(LOG_CFG))
			CFGProcessor::processWorkSpace(ws);
		else {
			mutex = sys::Mutex::make();
			const CFGCollection *coll = INVOLVED_CFGS(ws);
			it = CFGCollection::Iterator(coll);
			CFGRunnable run(*this);
			WorkSpace::runAll(run);
			delete mutex;
		}
#	endif
}

/**
 * Process the next available CFG.
 */
void ConcurrentCFGProcessor::nextCFG(void) {
	WorkSpace *ws = workspace();
	while(true) {

		// get the next CFG
		mutex->lock();
		CFG *cfg = 0;
		if(it()) {
			cfg = *it;
			it++;
		}
		mutex->unlock();

		// process it
		if(!cfg)
			return;
		else {
			//cerr << string(_ << "DEBUG: in " << (void *)sys::Thread::current() << " processing " << cfg << io::endl);
			processCFG(ws, cfg);
		}
	}
}

}	// otawa
