/*
 *	ConsBuilder processor interface
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

namespace otawa {

namespace ilp { class Var; }

namespace branch {

// numbered conditions feature
extern Identifier<int> COND_NUMBER;
extern Identifier<int*> COND_MAX;
extern SilentFeature NUMBERED_CONDITIONS_FEATURE;


// category feature
typedef enum category_t {
	ALWAYS_D,
	ALWAYS_H,
	FIRST_UNKNOWN,
	NOT_CLASSIFIED
} category_t;

extern Identifier<category_t> CATEGORY;
extern Identifier<BasicBlock*> HEADER;
extern SilentFeature CATEGORY_FEATURE;


// branch constraints feature
extern SilentFeature CONSTRAINTS_FEATURE;
extern Identifier<ilp::Var*> MISSPRED_VAR;

// support feature
extern SilentFeature SUPPORT_FEATURE;

} }		// otawa::branch

#endif /* OTAWA_BRANCH_FEATURES_H_ */
