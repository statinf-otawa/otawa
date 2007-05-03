/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/WCETComputation.h -- WCETComputation class interface.
 */
 
#ifndef OTAWA_ETS_WCET_COMPUTATION_H
#define OTAWA_ETS_WCET_COMPUTATION_H

#include <otawa/proc/ASTProcessor.h>

namespace otawa { namespace ets {
	
	
// WCETComputation class
class WCETComputation: public ASTProcessor {
	public:
	// ASTProcessor overload
	void processAST(WorkSpace *fw, AST *ast);
	
	protected:
	int computation(WorkSpace *fw, AST *ast);
};

} } // otawa::ets

#endif	// OTAWA_ETS_WCET_COMPUTATION_H
