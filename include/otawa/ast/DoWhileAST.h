/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/DoWhileAST.h -- interface for DoWhileAST class.
 */
#ifndef OTAWA_AST_DO_WHILE_AST_H
#define OTAWA_AST_DO_WHILE_AST_H

#include <otawa/ast/AST.h>

namespace otawa {

// DoWhileAST class
class DoWhileAST: public AST {
	AST *bod, *cnd;
protected:
	~DoWhileAST(void);
public:
	DoWhileAST(AST *body, AST *condition);

};
	
} // otawa

#endif // OTAWA_AST_DO_WHILE_AST_H
