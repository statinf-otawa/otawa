/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	src/prog/DoWhileAST.cpp -- implementation for DoWhileAST class.
 */

#include <otawa/ast/DoWhileAST.h>

namespace otawa {

/**
 * @class DoWhileAST
 * Representation of C do{ ... } while(...); control structure.
  */


/**
 * Build a new DO-WHILE AST.
 * @param body		Loop body.
 * @param condition	Loop condition.
 */
DoWhileAST::DoWhileAST(AST *body, AST *condition)
: bod(body), cnd(condition) {
	assert(body && condition);
}


/**
 */
DoWhileAST::~DoWhileAST(void) {
	bod->release();
	cnd->release();
}


/**
 * @fn AST *DoWhileAST::condition(void) const;
 * Get the condition of the loop.
 * @return Loop condition.
 */


/**
 * @fn AST *DoWhileAST::body(void) const;
 * Get the body of the loop.
 * @return loop body.
 */


/**
 */
Inst *DoWhileAST::first(void) {
	Inst *result = bod->first();
	if(!result)
		result = cnd->first();
	return result;
}


/**
 */
int DoWhileAST::countInstructions(void) const {
	return bod->countInstructions() + cnd->countInstructions();
}

} // otawa
