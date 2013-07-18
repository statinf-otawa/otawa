/*
 *	CallAST class interface
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
#ifndef OTAWA_AST_CALL_AST_H
#define OTAWA_AST_CALL_AST_H

#include <otawa/ast/BlockAST.h>
#include <otawa/ast/FunAST.h>

namespace otawa { namespace ast {

// CallAST Class
class CallAST: public BlockAST {
	FunAST *fun;
public:
	CallAST(Inst *block, ot::size size, FunAST *fun);
	CallAST(WorkSpace *ws, Inst *block, ot::size size, Inst *called);
	inline FunAST *function(void) const { return fun; };
	
	// AST overload
	virtual ast_kind_t kind(void) const { return AST_Call; };
	virtual CallAST *toCall(void) { return this; };
};
	
} }		// otawa::ast

#endif // OTAWA_AST_CALL_AST_H
