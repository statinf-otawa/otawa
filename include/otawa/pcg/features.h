/*
 *	PCG module features
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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
#ifndef OTAWA_PCG_FEATURES_H_
#define OTAWA_PCG_FEATURES_H_

#include <otawa/proc/Feature.h>
#include "PCG.h"

namespace otawa {

class PCGBlock;

extern p::feature PCG_FEATURE;
extern p::id<PCG *> PROGRAM_CALL_GRAPH;

extern p::feature RECURSIVITY_ANALYSIS;
extern p::id<PCGBlock *> RECURSE_HEAD;
extern p::id<bool> RECURSE_ENTRY;
extern p::id<bool> RECURSE_BACK;

} // otawa

#endif /* OTAWA_PCG_FEATURES_H_ */
