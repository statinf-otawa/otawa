/*
 *	$Id$
 *	This is the Abstract Cache State builder.
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
#ifndef CACHE_ACSMAYBUILDER_H_
#define CACHE_ACSMAYBUILDER_H_

#include <otawa/prop/Identifier.h>
#include <otawa/proc/Processor.h>
#include <otawa/hard/Cache.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cache/cat2/MAYProblem.h>
#include <elm/genstruct/Vector.h>

namespace otawa {
	
using namespace elm;

	
extern Identifier<genstruct::Vector<MAYProblem::Domain*>* > CACHE_ACS_MAY;
extern Identifier<genstruct::Vector<MAYProblem::Domain*>*> CACHE_ACS_MAY_ENTRY;

class ACSMayBuilder : public otawa::Processor {

	void processLBlockSet(otawa::WorkSpace*, LBlockSet *, const hard::Cache *);	
	
	bool unrolling;
	genstruct::Vector<MAYProblem::Domain *> *may_entry;
	
	public:
	ACSMayBuilder(void);
	virtual void processWorkSpace(otawa::WorkSpace*);
	virtual void configure(const PropList &props);
	
	
	
};

extern Feature<ACSMayBuilder> ICACHE_ACS_MAY_FEATURE;

}

#endif /*CACHE_ACSMAYBUILDER_H_*/
