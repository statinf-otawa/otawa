/*
 *	$Id$
 *	FixedTextDecoder class interface
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
#ifndef OTAWA_PROG_FIXED_TEXT_DECODER_H
#define OTAWA_PROG_FIXED_TEXT_DECODER_H

#include <otawa/proc/Processor.h>

namespace otawa {

// FixedTextDecoder class
class FixedTextDecoder: public Processor {
public:
	static FixedTextDecoder _;
	FixedTextDecoder(void);
	static p::declare reg;

protected:
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void setup(WorkSpace *fw);

private:
	t::uint32 size;
};

} // otawa

#endif	// OTAWA_PROG_FIXED_TEXT_DECODER_H
