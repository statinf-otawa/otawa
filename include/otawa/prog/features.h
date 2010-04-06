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

#include <otawa/proc/SilentFeature.h>

namespace otawa {

// Process information
extern Identifier<Address> ARGV_ADDRESS;
extern Identifier<Address> ENVP_ADDRESS;
extern Identifier<Address> AUXV_ADDRESS;
extern Identifier<Address> SP_ADDRESS;

// Features
extern Feature<NoProcessor> MEMORY_ACCESS_FEATURE;
extern Feature<NoProcessor> FLOAT_MEMORY_ACCESS_FEATURE;
extern Feature<NoProcessor> STACK_USAGE_FEATURE;
extern Feature<NoProcessor> REGISTER_USAGE_FEATURE;
extern Feature<NoProcessor> CONTROL_DECODING_FEATURE;
extern Feature<NoProcessor> SOURCE_LINE_FEATURE;
extern SilentFeature MEMORY_ACCESSES;
extern SilentFeature SEMANTICS_INFO;

} // otawa

#endif /* OTAWA_PROG_FEATURES_H_ */
