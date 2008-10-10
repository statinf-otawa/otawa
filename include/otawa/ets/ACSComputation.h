/*
 *	$$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/ACSComputation.h -- ACSComputation class interface.
 */
 
#ifndef OTAWA_ETS_ACS_COMPUTATION_H
#define OTAWA_ETS_ACS_COMPUTATION_H

#include <elm/genstruct/HashTable.h>
#include <otawa/ast/ASTProcessor.h>
#include <otawa/ets/AbstractCacheState.h>
#include <otawa/hard/CacheConfiguration.h>

namespace otawa { namespace ets {

// ACSComputation class	
class ACSComputation: public ASTProcessor {
	public :
		int cache_line_length;
		int cache_size;
		
		inline ACSComputation(WorkSpace *fw);
		inline ~ACSComputation(void);
		
		// ASTProcessor overload
		void processAST(WorkSpace *fw, AST *ast);
		
	protected :	
		AbstractCacheState *applyProcess(WorkSpace *fw, AST *ast, AbstractCacheState *acs);
		void initialization(WorkSpace *fw, AST *ast, AbstractCacheState *acs);
};

// Inlines
inline ACSComputation::ACSComputation(WorkSpace *fw) {
	if(fw->cache().hasInstCache() && !fw->cache().isUnified()) {
		cache_size = fw->cache().instCache()->rowCount();
			cache_line_length = 0;
	}
}

inline ACSComputation::~ACSComputation(void) {
	
}

} } // otawa::ets

#endif // OTAWA_ETS_ACS_COMPUTATION_H


