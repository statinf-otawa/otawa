/*
 *	$Id: ISPConstraintBuilder.h
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef ISP_CONSTRAINTBUILDER_H_
#define ISP_CONSTRAINTBUILDER_H_

#include <otawa/proc/Processor.h>
#include <otawa/prop/Identifier.h>
namespace otawa {
	
extern Identifier<ilp::Var *> ISP_HIT_VAR;
extern Identifier<ilp::Var *> ISP_MISS_VAR;
extern Identifier<int> RAM_PENALTY;

class ISPConstraintBuilder : public otawa::Processor {
	bool _explicit;
	int penalty;
	public:
	ISPConstraintBuilder(void);
	virtual void processWorkSpace(otawa::WorkSpace*);
	virtual void configure(const PropList& props);
	virtual void setup(otawa::WorkSpace*);
};

extern Feature<ISPConstraintBuilder> ISP_SUPPORT_FEATURE;

}

#endif
