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
#include <otawa/proc/Feature.h>

namespace otawa { namespace ppc {

// Info class
class Info {
public:
	typedef enum {
		NO_PRED = 0,
		TAKEN = 1,
		NOT_TAKEN = 2
	} prediction_t;

	virtual prediction_t prediction(Inst *inst) = 0;
};

extern Identifier<Info *> INFO;
extern Feature<NoProcessor> INFO_FEATURE;

} }	// ppc

#endif /* POWERPC_H_ */
