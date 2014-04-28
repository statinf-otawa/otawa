/*
 *	EdgeTimeBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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
#ifndef OTAWA_ETIME_EDGETIMEBUILDER_H_
#define OTAWA_ETIME_EDGETIMEBUILDER_H_

#include <otawa/parexegraph/GraphBBTime.h>
#include "features.h"

namespace otawa { namespace etime {

class EdgeTimeBuilder: public GraphBBTime<ParExeGraph> {
public:
	static p::declare reg;
	EdgeTimeBuilder(p::declare& r = reg);
protected:
	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
	virtual ParExeGraph *make(ParExeSequence *seq);
	virtual void clean(ParExeGraph *graph);
private:
	void processEdge(WorkSpace *ws, CFG *cfg, Edge *edge);
	void apply(Event *event, ParExeInst *inst);
	void rollback(Event *event, ParExeInst *inst);
};

} }	// otawa::etime

#endif /* OTAWA_ETIME_EDGETIMEBUILDER_H_ */
