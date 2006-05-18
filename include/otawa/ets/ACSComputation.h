/*
 *	$$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/ACSComputation.h -- ACSComputation class interface.
 */
 
#ifndef OTAWA_ETS_ACS_COMPUTATION_H
#define OTAWA_ETS_ACS_COMPUTATION_H

#include <elm/genstruct/HashTable.h>
#include <otawa/proc/ASTProcessor.h>
#include <otawa/ets/AbstractCacheState.h>
#include <otawa/hard/CacheConfiguration.h>

namespace otawa { namespace ets {

// ACSComputation class	
class ACSComputation: public ASTProcessor {
	public :
		int cache_line_length;
		int cache_size;
		
		inline ACSComputation(FrameWork *fw);
		inline ~ACSComputation(void);
		
		// ASTProcessor overload
		void processAST(FrameWork *fw, AST *ast);
		
	protected :	
		AbstractCacheState *applyProcess(FrameWork *fw, AST *ast, AbstractCacheState *acs);
		void initialization(FrameWork *fw, AST *ast, AbstractCacheState *acs);
};

// Inlines
inline ACSComputation::ACSComputation(FrameWork *fw) {
	if(fw->cache().hasInstCache() && !fw->cache().isUnified()) {
		cache_size = fw->cache().instCache()->lineCount();
			cache_line_length = 0;
	}
}

inline ACSComputation::~ACSComputation(void) {
	
}

} } // otawa::ets

#endif // OTAWA_ETS_ACS_COMPUTATION_H


