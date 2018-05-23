/*
 *	dcache::ACSBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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
#ifndef OTAWA_DCACHE_CATBUILDER_H_
#define OTAWA_DCACHE_CATBUILDER_H_

#include <otawa/dcache/BlockBuilder.h>
#include <otawa/dcache/ACSBuilder.h>
#include <otawa/dcache/MUSTPERS.h>
#include <otawa/dcache/ACSMayBuilder.h>
#include "features.h"

namespace otawa { namespace dcache {

// CATBuilder class
class CATBuilder: public Processor {
public:
	static p::declare reg;
	CATBuilder(p::declare& r = reg);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void configure(const PropList &props);
	virtual void cleanup(WorkSpace *ws);

private:
	void processLBlockSet(WorkSpace *ws, const BlockCollection& coll, const hard::Cache *cache);
	void processLBlock(otawa::Block *bb, BlockAccess& b, MUSTPERS::Domain& dom, MAYProblem::Domain& domMay);

	data_fmlevel_t firstmiss_level;
	cache::category_t wt_def_cat;

	MUSTPERS *prob;
	MAYProblem *probMay;
	bool has_pers;
};

} }	// otawa::dcache

#endif /* OTAWA_DCACHE_CATBUILDER_H_ */
