/*
 *	FlowAwareRanking class interface
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_AI_FLOWAWARERANKING_H_
#define OTAWA_AI_FLOWAWARERANKING_H_

#include <otawa/cfg/CFG.h>
#include <otawa/proc/Processor.h>
#include "features.h"

namespace otawa { namespace ai {

class FlowAwareRanking: public Processor, public CFGRanking {
public:
	static p::declare reg;
	FlowAwareRanking(p::declare& r = reg);

	int rankOf(Block *v) override;

protected:
	void *interfaceFor(const AbstractFeature& f) override;
	void processWorkSpace(WorkSpace *ws) override;
	void destroy(WorkSpace *ws) override;
};

} }		// otawa::ai

#endif /* OTAWA_AI_FLOWAWARERANKING_H_ */
