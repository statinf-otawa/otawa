/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	src/prog/SeqAST.cpp -- implementation for SeqAST class.
 */

#include <otawa/ast/SeqAST.h>

namespace otawa {

/**
 * @class SeqAST
 * This AST represents sequences. It accept only two children so that sequences must 
 * be currified.
 */

	
/**
 * Build a new sequence with the given children.
 * @param child1	First child.
 * @param child2	Second child.
 */
SeqAST::SeqAST(AST *child1, AST *child2): c1(child1), c2(child2) {
}


/**
 * Delete its children.
 */
SeqAST::~SeqAST(void) {
	if(c1 != &NOP)
		c1->release();
	if(c2 != &NOP)
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

} // otawa
