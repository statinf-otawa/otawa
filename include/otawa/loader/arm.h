/*
 * $Id$
 * Copyright (c) 2007, IRIT - UPS <casse@irit.fr>
 *
 * Star12X class declarations
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
#ifndef OTAWA_LOADER_ARM_H
#define OTAWA_LOADER_ARM_H

#include <otawa/properties.h>
#include <otawa/prog/Inst.h>

namespace otawa { namespace arm {

typedef enum multiple_t {
	no_multiple = 0,
	only_one = 1,
	several = 2
} multiple_t;

extern Identifier<bool> IS_MLA;
extern Identifier<int> NUM_REGS_LOAD_STORE;
extern Identifier <bool> IS_SP_RELATIVE;
extern Identifier <bool> IS_PC_RELATIVE;

// ARM information
class Info {
public:
	static Identifier <Info *> ID;
	virtual ~Info(void);
	virtual void *decode(Inst *inst) = 0;
	virtual void free(void *decoded) = 0;
	virtual t::uint16 multiMask(Inst *inst) = 0;
};

} } // otawa::arm

#endif	// OTAWA_LOADER_ARM_H
