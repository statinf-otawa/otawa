/*
 *	WhileAST class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <otawa/ast/WhileAST.h>

namespace otawa { namespace ast {

/**
 * @class WhileAST
 * Representation of an iteration with test at start of the loop.
 *
 * @ingroup ast
 */


/**
 * Build a new "while" AST.
 * @param condition	Iteration condition.
 * @param body		Iteration body.
 */
WhileAST::WhileAST(AST *condition, AST *body)
: cnd(condition), bod(body) {
	ASSERT(condition && body);
}


/**
 */
WhileAST::~WhileAST(void) {
	cnd->release();
	bod->release();
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


/**
 */
Inst *WhileAST::first(void) {
	return cnd->first();
}


/**
 */
int WhileAST::countInstructions(void) const {
	return cnd->countInstructions() + bod->countInstructions();
}

} }	// otawa
