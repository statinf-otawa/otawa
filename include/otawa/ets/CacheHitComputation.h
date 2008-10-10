/*
 *	$$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/CacheHitComputation.h -- CacheHitComputation class interface.
 */
 
#ifndef OTAWA_ETS_CACHE_HIT_COMPUTATION_H
#define OTAWA_ETS_CACHE_HIT_COMPUTATION_H

#include <otawa/ast/ASTProcessor.h>

namespace otawa { namespace ets {

class CacheHitComputation: public ASTProcessor {
	public:
	// ASTProcessor overload
	void processAST(WorkSpace *fw, AST *ast);
	
	protected:
	int computation(WorkSpace *fw, AST *ast);
};

} } // otawa::ets

#endif // OTAWA_ETS_CACHE_HIT_COMPUTATION_H



