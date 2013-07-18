/*
 *	SeqAST class implementation
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
#include <otawa/ast/SeqAST.h>

namespace otawa { namespace ast {

/**
 * @class SeqAST
 * This AST represents sequences. It accept only two children so that sequences must 
 * be currified.
 *
 * @ingroup ast
 */

	
/**
 * Build a new sequence with the given children.
 * @param child1	First child.
 * @param child2	Second child.
 */
SeqAST::SeqAST(AST *child1, AST *child2)
: c1(child1), c2(child2) {
}


/**
 */
SeqAST::~SeqAST(void) {
	c1->release();
	c2->release();
}


/**
 * @fn AST *SeqAST::child1(void) const;
 * Get the first child AST of the sequence.
 * @return First child in sequence.
 */


/**
 * @fn AST *SeqAST::child2(void) const;
 * Get the second child AST of the sequence.
 * @return Second child in sequence.
 */


/**
 */
Inst *SeqAST::first(void) {
	Inst *result = c1->first();
	if(!result)
		result = c2->first();
	return result;
}


/**
 */
int SeqAST::countInstructions(void) const {
	return c1->countInstructions() + c2->countInstructions();
}

} } // otawa::ast
