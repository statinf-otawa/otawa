/*
 *	oslice identifiers
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "otawa/oslice/oslice.h"

// common functions are defined here
namespace otawa { namespace oslice {
Identifier<interested_instructions_t*> INTERESTED_INSTRUCTIONS("otawa::oslice::INTERESTED_INSTRUCTIONS", 0);
Identifier<String> SLICED_CFG_OUTPUT_PATH("otawa::oslice::SLICED_CFG_OUTPUT_PATH", "");
Identifier<String> SLICING_CFG_OUTPUT_PATH("otawa::oslice::SLICING_CFG_OUTPUT_PATH", "");
Identifier<InstSet*> SET_OF_REMAINED_INSTRUCTIONS("otawa::oslice::SET_OF_REMAINED_INSTRUCTIONS", 0);
Identifier<int> SLICE_DEBUG_LEVEL("otawa::oslice::DEBUG_LEVEL", 0);
Identifier<bool> CFG_OUTPUT("otawa::oslice::CFG_OUTPUT", false);
Identifier<bool> ENABLE_LIGHT_SLICING("otawa::oslice::ENABLE_LIGHT_SLICING", false);
Identifier<otawa::oslice::BBSet*> SetOfCallers("otawa::oslice::SET_OF_CALLERS", 0);

}}
