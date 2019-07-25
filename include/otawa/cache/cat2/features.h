/*
 *	cache::cat2 module features
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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
#ifndef OTAWA_CACHE_CAT2_FEATURES_H_
#define OTAWA_CACHE_CAT2_FEATURES_H_

#include <otawa/cache/cat2/MUSTProblem.h>
#include <otawa/cache/cat2/PERSProblem.h>
#include <otawa/cache/cat2/MAYProblem.h>
#include <otawa/cache/features.h>

namespace otawa {

// ACS feature
typedef enum fmlevel {
		FML_INNER = 0,
		FML_OUTER = 1,
		FML_MULTI = 2,
		FML_NONE
} fmlevel_t;
otawa::Output& operator<<(otawa::Output& out, const fmlevel_t &fml);

extern Identifier<fmlevel_t> FIRSTMISS_LEVEL;
extern Identifier<bool> PSEUDO_UNROLLING;
extern Identifier<Vector<MUSTProblem::Domain*>*> CACHE_ACS_MUST_ENTRY;
extern Identifier<Vector<PERSProblem::Domain*>*> CACHE_ACS_PERS_ENTRY;
extern p::feature ICACHE_ACS_FEATURE;
extern Identifier<Vector<MUSTProblem::Domain*>* > CACHE_ACS_MUST;
extern Identifier<Vector<PERSProblem::Domain*>* > CACHE_ACS_PERS;


// MAY ACS feature
extern Identifier<Vector<MAYProblem::Domain*>* > CACHE_ACS_MAY;
extern Identifier<Vector<MAYProblem::Domain*>*> CACHE_ACS_MAY_ENTRY;
extern p::feature ICACHE_ACS_MAY_FEATURE;


// category feature
extern p::feature ICACHE_CATEGORY2_FEATURE;

// constraint feature2
extern p::feature ICACHE_ONLY_CONSTRAINT2_FEATURE;
extern p::feature ICACHE_CONSTRAINT2_FEATURE;
extern Identifier<ilp::Var *> MISS_VAR;

}	// otawa

#endif /* OTAWA_CACHE_CAT2_FEATURES_H_ */
