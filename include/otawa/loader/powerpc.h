/*
 * $Id$
 * Copyright (c) 2010, IRIT - UPS <casse@irit.fr>
 *
 * OTAWA PowerPC loader interface
 * This file is part of OTAWA
 *
 * OTAWA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * OTAWA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_LOADER_POWERPC_H_
#define OTAWA_LOADER_POWERPC_H_

#include <otawa/prog/Inst.h>
#include <otawa/proc/DynFeature.h>
#include <otawa/prop/DynIdentifier.h>

namespace otawa { namespace ppc {

// static prediction type
typedef enum {
	PRED_NONE = -1,
	PRED_NOT_TAKEN = 0,
	PRED_TAKEN = 1
} prediction_t;

#define USE_GLISS2_DECODER namespace otawa { namespace ppc { static DynIdentifier<void *> GLISS2_DECODER("otawa::ppc::GLISS2_DECODER"); } }
#define USE_STATIC_PREDICTION namespace otawa { namespace ppc { static DynIdentifier<prediction_t> STATIC_PREDICTION("otawa::ppc::STATIC_PREDICTION"); } }
#define USE_INFO_FEATURE namespace otawa { namespace ppc { static DynFeature INFO_FEATURE("otawa::ppc::INFO_FEATURE"); } }

} }	// ppc

#endif /* POWERPC_H_ */
