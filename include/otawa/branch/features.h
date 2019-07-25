/*
 *	branch module features
 *	Copyright (c) 2011, IRIT UPS.
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
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_BRANCH_FEATURES_H_
#define OTAWA_BRANCH_FEATURES_H_

#include <elm/io.h>
#include <otawa/proc/AbstractFeature.h>
#include "../prop.h"

namespace otawa {

using namespace elm;
class Block;
namespace ilp { class Var; }

namespace branch {

// numbered conditions feature
extern Identifier<int> COND_NUMBER;
extern Identifier<int *> COND_MAX;
extern p::feature NUMBERED_CONDITIONS_FEATURE;

// category feature
typedef enum category_t {
	UNDEF = 0,
	ALWAYS_D = 1,
	ALWAYS_H = 2,
	FIRST_UNKNOWN = 3,
	NOT_CLASSIFIED = 4,
	STATIC_TAKEN = 5,
	STATIC_NOT_TAKEN = 6,
	MAX = 7
} category_t;
io::Output& operator<<(io::Output& out, category_t cat);

// category feature
extern Identifier<category_t> CATEGORY;
extern Identifier<Block *> HEADER;
extern p::feature CATEGORY_FEATURE;

// branch constraints feature
extern p::feature CONSTRAINTS_FEATURE;
extern Identifier<ilp::Var*> MISSPRED_VAR;

// support feature
extern p::feature SUPPORT_FEATURE;

// etime feature
extern p::feature EVENT_FEATURE;

} }		// otawa::branch

#endif /* OTAWA_BRANCH_FEATURES_H_ */
