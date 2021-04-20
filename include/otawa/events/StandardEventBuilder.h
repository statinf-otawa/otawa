/*
 *	StandardEventBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2021, IRIT UPS.
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
#ifndef OTAWA_EVENTS_STANDARD_EVENT_BUILDER
#define OTAWA_EVENTS_STANDARD_EVENT_BUILDER

#include <otawa/proc/BBProcessor.h>
#include "features.h"

namespace otawa {

namespace hard { class Machine; }
	
class StandardEventBuilder: public BBProcessor {
public:
	static p::declare reg;
	StandardEventBuilder(p::declare& r = reg);
protected:
	void setup(WorkSpace *ws) override;
	void processBB(WorkSpace *ws, CFG *g, Block *b) override;
	void destroy(WorkSpace *ws) override;
	void dumpBB(Block *b, io::Output& out) override;
private:
	const hard::Machine *mach;
	bool has_icache, has_dcache;
};

}	// 

#endif // OTAWA_EVENTS_STANDARD_EVENT_BUILDER
