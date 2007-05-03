/*
 *	$$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/CacheFirstMissComputation.h -- CacheFirstMissComputation class interface.
 */
 
#ifndef OTAWA_ETS_CACHE_FIRST_MISS_COMPUTATION_H
#define OTAWA_ETS_CACHE_FIRST_MISS_COMPUTATION_H

#include <otawa/proc/ASTProcessor.h>

namespace otawa { namespace ets {

class CacheFirstMissComputation: public ASTProcessor {
	public:
	// ASTProcessor overload
	void processAST(WorkSpace *fw, AST *ast);
	
	protected:
	int computation(WorkSpace *fw, AST *ast);
};

} } // otawa::ets

#endif // OTAWA_ETS_CACHE_FIRST_MISS_COMPUTATION_H
