/*
 *	features for the ilp module
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_ILP_FEATURES_H_
#define OTAWA_ILP_FEATURES_H_

#include <elm/sys/Path.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace ilp {

typedef enum format_t {
	DEFAULT = 0,
	LP_SOLVE = 1,
	CPLEX = 2,
	MOSEK = 3
} format_t;

// Output feature
extern p::feature OUTPUT_FEATURE;
extern Identifier<format_t> OUTPUT_FORMAT;
extern Identifier<sys::Path> OUTPUT_PATH;
extern Identifier<elm::io::OutStream *> OUTPUT;

}

}	// otawa::ilp

namespace elm { template <> struct type_info<otawa::ilp::format_t>: enum_info<otawa::ilp::format_t> { }; } // elm

#endif /* FEATURES_H_ */
