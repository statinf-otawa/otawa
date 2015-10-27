/*
 *	$Id$
 *	Features for the prog module.
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
#ifndef OTAWA_PROG_FEATURES_H_
#define OTAWA_PROG_FEATURES_H_

#include <otawa/proc/Feature.h>

namespace otawa {

// delayed_t type
typedef enum delayed_t {
	DELAYED_None = 0,
	DELAYED_Always = 1,
	DELAYED_Taken = 2
} delayed_t;

// DelayedInfo class
class DelayedInfo {
public:
	virtual ~DelayedInfo(void);
	virtual delayed_t type(Inst *inst) = 0;
	virtual int count(Inst *inst);
};
extern Identifier<DelayedInfo *> DELAYED_INFO;


// Process information
extern Identifier<Address> ARGV_ADDRESS;
extern Identifier<Address> ENVP_ADDRESS;
extern Identifier<Address> AUXV_ADDRESS;
extern Identifier<Address> SP_ADDRESS;
extern Identifier<delayed_t> DELAYED;

// Features
extern p::feature CONTROL_DECODING_FEATURE;
extern p::feature DELAYED_FEATURE;
extern p::feature DELAYED2_FEATURE;
extern p::feature FLOAT_MEMORY_ACCESS_FEATURE;
extern p::feature MEMORY_ACCESS_FEATURE;
extern p::feature MEMORY_ACCESSES;
extern p::feature REGISTER_USAGE_FEATURE;
extern p::feature SEMANTICS_INFO_EXTENDED;
extern p::feature SEMANTICS_INFO_FLOAT;
extern p::feature SEMANTICS_INFO;
extern p::feature SOURCE_LINE_FEATURE;
extern p::feature VLIW_SUPPORTED;

} // otawa

#endif /* OTAWA_PROG_FEATURES_H_ */
