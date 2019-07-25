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
#ifndef OTAWA_CACHE_ACSBUILDER_H_
#define OTAWA_CACHE_ACSBUILDER_H_

#include <elm/data/Vector.h>
#include <otawa/hard/Cache.h>
#include <otawa/proc/Processor.h>
#include <otawa/prop/Identifier.h>
#include <otawa/cache/LBlockSet.h>
#include "features.h"

namespace otawa {
	
using namespace elm;


class ACSBuilder : public otawa::Processor {
public:
	static p::declare reg;
	ACSBuilder(p::declare& r = reg);
	virtual void processWorkSpace(otawa::WorkSpace*);
	virtual void configure(const PropList &props);

private:
	void processLBlockSet(otawa::WorkSpace*, LBlockSet *, const hard::Cache *);	
	fmlevel_t level;
	bool unrolling;
	Vector<MUSTProblem::Domain *> *must_entry;
	Vector<PERSProblem::Domain *> *pers_entry;
	
};

}	// elm

#endif /* OTAWA_CACHE_ACSBUILDER_H_*/
