/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	FixedTextDecoder class interface
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

protected:
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void setup(WorkSpace *fw);

private:
	int size;
};

} // otawa

#endif	// OTAWA_PROG_FIXED_TEXT_DECODER_H
