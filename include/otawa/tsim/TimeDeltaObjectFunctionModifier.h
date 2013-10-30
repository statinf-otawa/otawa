/*
 *	TimeDeltaObjectFunctionModifier class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-07, IRIT UPS.
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
#ifndef OTAWA_TSIM_TIME_DELTA_OBJECT_FUNCTION_MODIFIER_H
#define OTAWA_TSIM_TIME_DELTA_OBJECT_FUNCTION_MODIFIER_H

#include <elm/assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace tsim {


// TimeDeltaObjectFunctionModifier class
class TimeDeltaObjectFunctionModifier: public BBProcessor {
public:
	static p::declare reg;
	TimeDeltaObjectFunctionModifier(p::declare& r = reg);

	// BBProcessor overload
	virtual void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb);
};

// Features
extern Feature<TimeDeltaObjectFunctionModifier> EDGE_TIME_FEATURE;

} } // otawa::tsim

#endif // OTAWA_TSIM_TIME_DELTA_OBJECT_FUNCTION_MODIFIER_H
