/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	src/prog/WhileAST.h -- implementation for WhileAST class.
 */

#include <otawa/ast/WhileAST.h>

namespace otawa {

/**
 * @class WhileAST
 * Representation of an iteration with test at start of the loop.
 */


/**
 * Build a new "while" AST.
 * @param condition	Iteration condition.
 * @param body		Iteration body.
 */
WhileAST::WhileAST(AutoPtr<AST> condition, AutoPtr<AST> body)
: cnd(condition), bod(body) {
	assert(condition && body);
}


/**
 * @fn AST *WhileAST::condition(void) const;
 * Get the condition of the iteration.
 * @return Iteration condition.
 */


/**
 * @fn AST *WhileAST::body(void) const;
 * Get the body of the iteration.
 * @return Iteration body.
 */


// AST overload
Inst *WhileAST::first(void) {
	return cnd->first();
}

}	// otawa
