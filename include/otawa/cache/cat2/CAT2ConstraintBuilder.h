/*
 *	CAT2ConstraintBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
#ifndef OTAWA_CACHE_CAT2CONSTRAINTBUILDER_H_
#define OTAWA_CACHE_CAT2CONSTRAINTBUILDER_H_

#include <otawa/proc/Processor.h>
#include <otawa/prop/Identifier.h>

namespace otawa {

class CAT2OnlyConstraintBuilder: public otawa::Processor {
public:
	static p::declare reg;
	CAT2OnlyConstraintBuilder(p::declare& r = reg);
	virtual void configure(const PropList& props);
	virtual void collectStats(WorkSpace *ws);
protected:
	virtual void processWorkSpace(otawa::WorkSpace*);
	bool _explicit;
};


class CAT2ConstraintBuilder: public CAT2OnlyConstraintBuilder {
public:
	static p::declare reg;
	CAT2ConstraintBuilder(p::declare& r = reg);
protected:
	virtual void processWorkSpace(otawa::WorkSpace*);
};

}	// otawa

#endif /* OTAWA_CACHE_CAT2CONSTRAINTBUILDER_H_*/
