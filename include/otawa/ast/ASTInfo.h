/*
 *	ASTInfo class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
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
#ifndef OTAWA_AST_AST_INFO_H
#define OTAWA_AST_AST_INFO_H

#include <elm/genstruct/HashTable.h>
#include <elm/genstruct/Vector.h>
#include <otawa/ast/FunAST.h>

// Extern
int heptane_parse(void);

namespace otawa { namespace ast {

// ASTInfo class
class ASTInfo: public PropList {
public:
	~ASTInfo(void);
	static ASTInfo *getInfo(WorkSpace *ws);
	FunAST *getFunction(Inst *inst);

	class Iterator: public elm::genstruct::Vector<FunAST *>::Iterator {
	public:
		inline Iterator(ASTInfo *info):  elm::genstruct::Vector<FunAST *>::Iterator(info->funs) { }
	};
	
	void add(FunAST *fun);
	inline Option<FunAST *> get(const string& name) { return _map.get(name); }

private:	
	friend class CallAST;
	friend class FunAST;
	friend class GenericProperty<ASTInfo *>;
	friend int ::heptane_parse(void);
	elm::genstruct::Vector<FunAST *> funs;
	elm::genstruct::HashTable<String, FunAST *> _map;
	ASTInfo(WorkSpace *ws);
};
	
} } // otawa::ast

#endif // OTAWA_AST_AST_INFO_H
