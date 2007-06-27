/*
 *	$Id$
 *	exegraph module implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#include <otawa/exegraph/Microprocessor.h>
#include <otawa/hard/Register.h>
#include <otawa/hard/Processor.h>

using namespace elm::genstruct;

namespace otawa { 

static const unsigned long MUL_MASK = Inst::IS_MUL | Inst::IS_INT;
static const unsigned long DIV_MASK = Inst::IS_DIV | Inst::IS_INT;
	
instruction_category_t instCategory(Inst *inst) {
	Inst::kind_t kind = inst->kind();
	if(kind & Inst::IS_FLOAT)
		return FALU;
	else if(kind & Inst::IS_MEM)
		return MEMORY;
	else if(kind & Inst::IS_CONTROL)
		return CONTROL;
	else if((kind & MUL_MASK) == MUL_MASK)
		return MUL;
	else if((kind & DIV_MASK) == DIV_MASK)
		return MUL;
	else
		return IALU;
}





} // otawa
