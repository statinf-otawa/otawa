/*
 *	$$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/CacheHitComputation.h -- CacheHitComputation class interface.
 */
 
#ifndef OTAWA_ETS_CACHE_HIT_COMPUTATION_H
#define OTAWA_ETS_CACHE_HIT_COMPUTATION_H

#include <otawa/proc/ASTProcessor.h>

namespace otawa { namespace ets {

class CacheHitComputation: public ASTProcessor {
	public:
	// ASTProcessor overload
	void processAST(FrameWork *fw, AST *ast);
	
	protected:
	int computation(FrameWork *fw, AST *ast);
};

} } // otawa::ets

#endif // OTAWA_ETS_CACHE_HIT_COMPUTATION_H



