/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/FunCursor.h -- interface for FunCursor class.
 */
#ifndef OTAWA_OSHELL_FUNCURSOR_H
#define OTAWA_OSHELL_FUNCURSOR_H

#include <otawa/ast/FunAST.h>
#include "oshell.h"

namespace otawa {

// FunCursor class
class FunCursor: public Cursor {
	FunAST *fun;
	void outputAST(Output& out, AST *ast, int ind);
	void outputSeq(Output& out, AST *ast, int ind);
public:
	FunCursor(Cursor *back, FunAST *function);
	virtual void path(Output& out);
	virtual void info(Output& out);
	virtual void list(Output& out);
	virtual void display(Output& out);
};
	
}	// otawa

#endif // OTAWA_OSHELL_FUNCURSOR_H
