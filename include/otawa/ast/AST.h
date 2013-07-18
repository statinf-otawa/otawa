/*
 *	AST class interface
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
#ifndef OTAWA_AST_AST_H
#define OTAWA_AST_AST_H

#include <elm/util/AutoPtr.h>
#include <otawa/instruction.h>
#include <otawa/program.h>

namespace otawa { namespace ast {

// Defined AST
class BlockAST;
class CallAST;
class SeqAST;
class IfAST;
class WhileAST;
class DoWhileAST;
class ForAST;
	
// AST Kind
typedef enum ast_kind_t {
	AST_Undef,
	AST_Nop,
	AST_Block,
	AST_Call,
	AST_Seq,
	AST_If,
	AST_While,
	AST_DoWhile,
	AST_For
} ast_kind_t;

// AST Class
class AST: public elm::Lock, public PropList {
	friend class FunAST;
protected:
	virtual ~AST(void) { };
public:
	static AST& NOP;
	static AST& UNDEF;
	virtual ast_kind_t kind(void) const = 0;
	virtual Inst *first(void) = 0;
	virtual void release(void);
	
	virtual bool isNOP(void) { return false; };
	virtual bool isUndef(void) { return false; };
	virtual BlockAST *toBlock(void) { return 0; };
	virtual CallAST *toCall(void) { return 0; };
	virtual SeqAST *toSeq(void) { return 0; };
	virtual IfAST *toIf(void) { return 0; };
	virtual WhileAST *toWhile(void) { return 0; };
	virtual DoWhileAST *toDoWhile(void) { return 0; };
	virtual ForAST *toFor(void) { return 0; };
	virtual int countInstructions(void) const = 0;
};

} }	// otawa::ast

#endif	// OTAWA_AST_AST_H
