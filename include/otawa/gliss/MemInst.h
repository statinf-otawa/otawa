/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/gliss/MemInst.h -- gliss::MeInst class interface.
 */
#ifndef OTAWA_GLISS_MEM_INST_H
#define OTAWA_GLISS_MEM_INST_H

#include <otawa/gliss/Inst.h>

namespace otawa { namespace gliss {
	
// MemInst class
class MemInst: public Inst {
	//virtual void scanCustom(instruction_t *inst);
public:
	inline MemInst(CodeSegment& segment, address_t address); 

	// Inst overload	
	/*virtual bool isMem(void);
	virtual bool isLoad(void);
	virtual bool isStore(void);*/
};

// Inlines
inline MemInst::MemInst(CodeSegment& segment, address_t address)
: Inst(segment, address) {
};

} } // otawa::gliss

#endif // OTAWA_GLISS_MEM_INST_H
