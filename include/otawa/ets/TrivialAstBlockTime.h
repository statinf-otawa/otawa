/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/TrivialAstBlockTime.h -- TrivialAstBlockTime class interface.
 */
 
#ifndef OTAWA_ETS_TRIVIALASTBLOCKTIME_H
#define OTAWA_ETS_TRIVIALASTBLOCKTIME_H

#include <assert.h>
#include <otawa/proc/ASTProcessor.h>

namespace otawa { namespace ets {

// TrivialAstBlockTime class	
class TrivialAstBlockTime: public ASTProcessor {
public:	
	int dep;
	ASTInfo *ast_info;
	
	inline TrivialAstBlockTime(int depth, ASTInfo *info);
	inline int depth(void) const;

	// ASTProcessor overload
	void processAST(AST *ba);
};


// Inlines
inline TrivialAstBlockTime::TrivialAstBlockTime(int depth, ASTInfo *info) {
	ast_info=info;
	assert(depth > 0);
}

inline int TrivialAstBlockTime::depth(void) const {
	return dep;
}

} } // otawa::ets

#endif // OTAWA_ETS_TRIVIALASTBLOCKTIME_H
