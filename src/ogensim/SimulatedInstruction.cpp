/*
 *	$Id$
 *	SimulatedInstruction class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
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


#include <otawa/gensim/GenericState.h>

namespace otawa { namespace gensim {

SimulatedInstruction::SimulatedInstruction(
	otawa::Inst* inst,
	int index,
	elm::genstruct::SLList<SimulatedInstruction *> * _active_instructions,
	GenericState *state)
:
	active_instructions(_active_instructions),
	instruction(inst), instruction_state(READY),
	_index(index), 
	Mem_Read_First(state->lowerRead()),
	Mem_Read_Last(state->upperRead()),
	Mem_Write_First(state->lowerWrite()),
	Mem_Write_Last(state->upperWrite())
{
	active_instructions->addLast(this);
	_type = inst->kind();
	_is_control = inst->isControl();
}

} } // otawa::gensim
