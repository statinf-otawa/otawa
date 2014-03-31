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
#include <otawa/proc/SilentFeature.h>

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
extern Feature<NoProcessor> DELAYED2_FEATURE;
extern Identifier<DelayedInfo *> DELAYED_INFO;


// Process information
extern Identifier<Address> ARGV_ADDRESS;
extern Identifier<Address> ENVP_ADDRESS;
extern Identifier<Address> AUXV_ADDRESS;
extern Identifier<Address> SP_ADDRESS;
extern Identifier<delayed_t> DELAYED;

// Features
extern Feature<NoProcessor> MEMORY_ACCESS_FEATURE;
extern Feature<NoProcessor> FLOAT_MEMORY_ACCESS_FEATURE;
extern Feature<NoProcessor> STACK_USAGE_FEATURE;
extern Feature<NoProcessor> REGISTER_USAGE_FEATURE;
extern Feature<NoProcessor> CONTROL_DECODING_FEATURE;
extern Feature<NoProcessor> SOURCE_LINE_FEATURE;
extern SilentFeature MEMORY_ACCESSES;
extern SilentFeature SEMANTICS_INFO;
extern Feature<NoProcessor> DELAYED_FEATURE;
extern SilentFeature SEMANTICS_INFO_EXTENDED;
extern SilentFeature SEMANTICS_INFO_FLOAT;

} // otawa

#endif /* OTAWA_PROG_FEATURES_H_ */
