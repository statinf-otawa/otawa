/*
 *	$Id$
 *	BBStatCollector class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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
#ifndef OTAWA_STATS_BBSTATCOLLECTOR_H_
#define OTAWA_STATS_BBSTATCOLLECTOR_H_

#include <otawa/cfg.h>
#include <otawa/proc/ContextualProcessor.h>
#include "StatCollector.h"

namespace otawa {

class BBStatCollector: public StatCollector {
public:
	BBStatCollector(WorkSpace *ws);
	virtual void collect(Collector& collector);

protected:

	inline WorkSpace *ws(void) const { return _ws; }
	inline CFG *cfg(void) const { return _cfg; }
	virtual int total(void);
	virtual void collect(Collector& collector, BasicBlock *bb, const ContextualPath& path);
	virtual int getStat(BasicBlock *bb);
	virtual int total(BasicBlock *bb);

private:
	virtual void processCFG(Collector& collector, CFG *cfg);
	void process(Collector& collector);
	WorkSpace *_ws;
	CFG *_cfg;
	int _total;
};

}	// otawa

#endif /* OTAWA_STATS_BBSTATCOLLECTOR_H_ */
