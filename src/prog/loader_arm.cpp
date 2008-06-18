/**
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

#include <otawa/loader/arm.h>

namespace otawa { namespace arm {


/**
 * This property is put the ARM loader on instruction representing the ARM
 * "MLA" instruction.
 * 
 * @par Hooks
 * @li @ref otawa::Inst
 */
Identifier<bool> IS_MLA("otawa::arm::IS_MLA", false);


/**
 * This property is put the ARM loader on instruction representing the ARM
 * "LDMxx" and "STMxx" instruction.
 * 
 * @par Hooks
 * @li @ref otawa::Inst
 */

Identifier<int> NUM_REGS_LOAD_STORE("otawa::arm::NUM_REGS_LOAD_STORE", 1);


/**
 * This identifier is set to true on load/store instructions accessing the
 * memory through the SP or the FP registers.
 * 
 * @par Hooks
 * @li @ref otawa::Inst
 */
Identifier <bool> IS_SP_RELATIVE("otawa::arm::IS_SP_RELATIVE", false);

/**
 * This identifier is set to true on load/store instructions accessing the
 * memory through the PC register.
 * 
 * @par Hooks
 * @li @ref otawa::Inst
 */
Identifier <bool> IS_PC_RELATIVE("otawa::arm::IS_PC_RELATIVE", false);

} } // otawa::arm

