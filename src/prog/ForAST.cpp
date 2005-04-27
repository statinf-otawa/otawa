/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	src/prog/ForAST.cpp -- implementation for ForAST class.
 */

#include <otawa/ast/ForAST.h>

namespace otawa {


/**
 * @class ForAST
 * Representation of C language FOR loop.
 */


/**
 * Build a new FOR AST.
 */
ForAST::ForAST(AST *initialization, AST *condition,
AST *incrementation, AST *body)
: init(initialization), cnd(condition), inc(incrementation), bod(body) {
	assert(initialization && condition && incrementation && body);
}


/**
 */
ForAST::~ForAST(void) {
	init->release();
	cnd->release();
	inc->release();
	bod->release();
}


/**
 * @fn AST *ForAST::initialization(void) const;
 * Get the initialization AST.
 * @return Initialization of the loop.
 */


/**
 * @fn AST *ForAST::condition(void) const;
 * Get the condition AST.
 * @return Loop condition.
 */


/**
 * @fn AST *ForAST::incrementation(void) const;
 * Get the incrementation AST.
 * @return Loop incrementation.
 */


/**
 * @fn AST *ForAST::body(void) const;
 * Get the body AST.
  * @return Loop body.
  */


// AST Overload
Inst *ForAST::first(void) {
	Inst *result = init->first();
	if(!result)
		result = cnd->first();
	assert(result);
	return result;
}

} // otawa
