/*
 *	DynamicBranching Features
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
#ifndef OTAWA_DYNAMIC_BRANCHING_FEATURES_H
#define OTAWA_DYNAMIC_BRANCHING_FEATURES_H

#include <otawa/otawa.h>

namespace otawa { namespace dynbranch {

extern p::feature FEATURE;
extern Identifier<bool> TIME ;

// if any target is found in the dynamic branching analysis
// if the values of DYNBRANCH_TARGET_COUNT and DYNBRANCH_TARGET_COUNT_PREV differ, it means that there is a new target found, NEW_BRANCH_TARGET_FOUND will be set to true
extern Identifier<bool> NEW_BRANCH_TARGET_FOUND;
    
// the current branching target counts for a given instruction
extern Identifier<int> DYNBRANCH_TARGET_COUNT;

// the previous branching target counts for a given instruction
extern Identifier<int> DYNBRANCH_TARGET_COUNT_PREV;

} } // otawa::dynbranch

#endif	// OTAWA_DYNAMIC_BRANCHING_ANALYSIS_H

