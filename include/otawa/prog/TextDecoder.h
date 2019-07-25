/*
 *	TextDecoder class interface
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
#ifndef OTAWA_PROG_TEXT_DECODER_H
#define OTAWA_PROG_TEXT_DECODER_H

#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// TextDecoder class
class TextDecoder: public Processor {
public:
	static TextDecoder _;
	TextDecoder(void);
	static Identifier<bool> FOLLOW_PATHS;
	virtual void configure(const PropList& props);

protected:
	virtual void processWorkSpace(WorkSpace *fw);
	bool follow_paths;
private:
	const PropList *conf_props;
};

// Features
extern Feature<TextDecoder> DECODED_TEXT;

} // otawa

#endif	// OTAWA_PROG_TEXT_DECODER_H
