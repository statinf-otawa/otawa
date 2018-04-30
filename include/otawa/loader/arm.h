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

#include <otawa/prog/Inst.h>
#include "../prop.h"

namespace otawa { namespace arm {

class IOManager {
public:
	virtual ~IOManager(void);
	virtual void write(Address a, t::uint32 size, t::uint8 *data) = 0;
	virtual void read(Address a, t::uint32 size, t::uint8 *data) = 0;
};

class Info {
public:
	static Identifier <Info *> ID;
	static const t::uint32
		IS_MLA 		= 0x80000000,
		IS_TWO_REG	= 0x40000000,
		IS_SEMIHOST	= 0x20000000,
		IS_SWP		= 0x10000000,
		IS_THUMB_BX	= 0x08000000;

	virtual ~Info(void);
	virtual void *decode(Inst *inst) = 0;
	virtual void free(void *decoded) = 0;
	virtual t::uint16 multiMask(Inst *inst) = 0;
	virtual void handleIO(Address addr, t::uint32 size, IOManager& man) = 0;
};


// deprecated definitions
typedef enum multiple_t {
	no_multiple = 0,
	only_one = 1,
	several = 2
} multiple_t;

extern Identifier<bool> IS_MLA;
extern Identifier<int> NUM_REGS_LOAD_STORE;
extern Identifier <bool> IS_SP_RELATIVE;
extern Identifier <bool> IS_PC_RELATIVE;

} } // otawa::arm

#endif	// OTAWA_LOADER_ARM_H
