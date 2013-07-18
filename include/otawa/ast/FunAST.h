/*
 *	FunAST class interface
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
#ifndef OTAWA_AST_FUN_AST_H
#define OTAWA_AST_FUN_AST_H

#include <otawa/ast/AST.h>
#include <otawa/manager.h>

namespace otawa { namespace ast {

class ASTInfo;

// FunAST class
class FunAST: public Lock {
	friend class ASTInfo;
	ASTInfo *info;
	Inst *ent;
	String _name;
	AST *_ast;
	~FunAST(void);
public:
	FunAST(WorkSpace *ws, Inst *entry, String name = "");
	FunAST(ASTInfo *info, Inst *entry, String name = "");
	inline Inst *entry(void) const { return ent; };
	inline String& name(void) { return _name; };
	inline AST *ast(void) const { return _ast; };
	void setAst(AST *ast);
	inline void setName(const String& name) { _name = name; };
};

} }	 // otawa::ast

#endif	// OTAWA_AST_FUN_AST_H
