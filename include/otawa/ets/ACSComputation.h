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
#include <otawa/hardware/Cache.h>

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
	switch (fw->caches().count()){
		case 1 : 
			//L1
			cache_size = fw->caches().get(0)->lineCount();
			cache_line_length = 0;
			break;
		default :
			;//L2, L3 ...
	}
}

inline ACSComputation::~ACSComputation(void) {
	
}

} } // otawa::ets

#endif // OTAWA_ETS_ACS_COMPUTATION_H


