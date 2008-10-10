/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/proc/FunProcessor.h -- FunProcessor class interface.
 */
#ifndef OTAWA_PROC_FUNPROCESSOR_H
#define OTAWA_PROC_FUNPROCESSOR_H

#include <otawa/proc/Processor.h>
#include <otawa/ast/FunAST.h>

namespace otawa {
	
// Processor class
class FunProcessor: public Processor {
public:
	~FunProcessor(void);
	virtual void processFun(WorkSpace *fw, FunAST *fa) = 0;

	// Processor overload
	virtual void processWorkSpace(WorkSpace *fw);
};

} // otawa

#endif // OTAWA_PROC_FUNPROCESSOR_H
