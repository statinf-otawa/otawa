/*
 *	ipet::BasicObjectFunctionBuilder interface
 *	Copyright (c) 2005-12, IRIT UPS <casse@irit.fr>
 *
 *	This file is part of OTAWA
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_IPET_BASICOBJECTFUNCTIONBUILDER_H
#define OTAWA_IPET_BASICOBJECTFUNCTIONBUILDER_H

#include <elm/assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace ipet {


// BasicObjectFunctionBuilder class
class BasicObjectFunctionBuilder: public BBProcessor {
public:
	static p::declare reg;
	BasicObjectFunctionBuilder(p::declare& r = reg);

	// BBProcessor overload
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *bb);
};

} } // otawa::ipet

#endif // OTAWA_IPET_BASICOBJECTFUNCTIONBUILDER_H
