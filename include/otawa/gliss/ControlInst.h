/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	Inst.h -- GLISS ControlInst class interface.
 */
#ifndef OTAWA_GLISS_CONTROLINST_H
#define OTAWA_GLISS_CONTROLINST_H

#include <otawa/gliss/Inst.h>

namespace otawa { namespace gliss {
	
// ControlInst class
class ControlInst: public Inst {
	Inst *_target;
	virtual void scanCustom(instruction_t *inst);
public:
	inline ControlInst(CodeSegment& segment, address_t address)
		: Inst(segment, address), _target(0) { }; 

	// Inst overload	
	virtual bool isControl(void);
	virtual bool isBranch(void);
	virtual bool isCall(void);
	virtual bool isReturn(void);
	virtual void dump(io::Output& out);

	// ControlInst overload
	virtual bool isConditional(void);
	virtual otawa::Inst *target(void);
};

} } // otawa::gliss

#endif // OTAWA_GLISS_CONTROLINST_H
