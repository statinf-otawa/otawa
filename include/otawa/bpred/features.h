/*
 *	features for bpred module
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_BPRED_FEATURES_H_
#define OTAWA_BPRED_FEATURES_H_

#include <otawa/proc/Feature.h>

namespace otawa { namespace bpred {

typedef enum {
	NO_CONFLICT_2BITS_COUNTER	= 0,
	BI_MODAL					= 1,
	GLOBAL_2B					= 2,
	GLOBAL_1B					= 3
} method_t;

// feature
extern p::feature BRANCH_PREDICTION_FEATURE;

// Configuration Properties
extern Identifier<method_t>		METHOD;
extern Identifier<int> 			BHT_SIZE;
extern Identifier<int> 			HISTORY_SIZE;
extern Identifier<bool> 		DUMP_BCG;
extern Identifier<bool>			DUMP_BHG;
extern Identifier<bool> 		DUMP_BBHG;
extern Identifier<bool> 		WITH_MITRA;
extern Identifier<bool> 		WITH_STATS;
extern Identifier<const char*> 	INIT_HISTORY_BINARYVALUE;
extern Identifier<bool> 		EXPLICIT_MODE;

} }	// otawa::bpred

#endif /* OTAWA_BPRED_FEATURES_H_ */
