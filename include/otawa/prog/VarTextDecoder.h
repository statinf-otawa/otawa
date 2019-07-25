/*
 *	VarTextDecoder class interface
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_PROG_VAR_TEXT_DECODER_H
#define OTAWA_PROG_VAR_TEXT_DECODER_H

#include <otawa/proc/Processor.h>

namespace otawa {

// External classes
class Inst;

// VarTextDecoder class
class VarTextDecoder: public Processor {
public:
	static VarTextDecoder _;
	VarTextDecoder(void);

protected:
	virtual void processWorkSpace(WorkSpace *fw);

private:
	void processEntry(WorkSpace *ws, address_t address);
	Inst *getInst(WorkSpace *ws, address_t address, Inst *source = 0);
	string getBytes(Address addr, int size);
};

} // otawa

#endif	// OTAWA_PROG_VAR_TEXT_DECODER_H
