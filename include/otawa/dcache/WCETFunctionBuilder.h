/*
 *	dcache::WCETFunctionBuilder class interface
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
#ifndef OTAWA_DCACHE_WCETFUNCTIONBUILDER_H_
#define OTAWA_DCACHE_WCETFUNCTIONBUILDER_H_

#include <otawa/proc/BBProcessor.h>
#include "features.h"

namespace otawa {

namespace ilp { class System; }

namespace dcache {

class WCETFunctionBuilder: public BBProcessor {
public:
	static p::declare reg;
	WCETFunctionBuilder(p::declare& r = reg);

protected:
	virtual void setup(WorkSpace *ws);
	virtual void processBB(WorkSpace *ws, CFG *cfg, otawa::Block *bb);

private:
	ilp::System *sys;
	const hard::Memory *mem;
	ot::time worst_read, worst_write;
};

} }		// otawa::dcache

#endif /* OTAWA_DCACHE_WCETFUNCTIONBUILDER_H_ */
