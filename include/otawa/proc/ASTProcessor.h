/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/proc/ASTProcessor.h -- ASTProcessor class interface.
 */

#ifndef OTAWA_PROC_ASTPROCESSOR_H_
#define OTAWA_PROC_ASTPROCESSOR_H_

#include <otawa/proc/Processor.h>
#include <otawa/proc/FunProcessor.h>
#include <otawa/ast/FunAST.h>
#include <otawa/ast/AST.h>

namespace otawa {
	
// Processor class
class ASTProcessor: public FunProcessor {
public:
	~ASTProcessor(void);
	
	virtual void processAST(AST *ast)=0 ;
	
	// FUNProcessor overload
	virtual void processFun(FunAST *fa);
};

} // otawa

#endif //OTAWA_PROC_ASTPROCESSOR_H_
