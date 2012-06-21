/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	VarTextDecoder class interface
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
