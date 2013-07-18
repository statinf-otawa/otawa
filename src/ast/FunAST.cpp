/*
 *	FunAST class implementation
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
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/ast/FunAST.h>
#include <otawa/ast/ASTInfo.h>
#include <otawa/ast/features.h>

namespace otawa { namespace ast {


/**
 * @class FunAST
 * This class represents functions in the AST representation. It provides the root of the AST.
 *
 * @ingroup ast
 */

/**
 * A property with this identifier is put on each instruction, start of an AST function.
 * Its value is of type "AST *".
 *
 * @p Hook
 * @li @ref Inst
 *
 * @p Feature
 * @li @ref otawa::ref::FEATURE
 *
 * @ingroup ast
 */
Identifier<FunAST *> FUN("otawa::ast::FUN", 0);


/**
 * Build a new function AST.
 * @param ws		workspace containing the function.
 * @param entry	First instruction of the function.
 * @param name	Function name.
 */
FunAST::FunAST(WorkSpace *ws, Inst *entry, String name)
: ent(entry), _name(name), _ast(&AST::UNDEF) {

	// Mark the entry
	FUN(ent) = this;

	// Obtain an AST information
	info = INFO(ws);
	if(!info)
		info = new ASTInfo(ws);

	// If no name, look for a name
	String label = LABEL(ent);
	if(label)
		_name = label;

	// Record the function
	info->add(this);
}


/**
 * Build a new function AST.
 * @param info		AST information.
 * @param entry	First instruction of the function.
 * @param name	Function name.
 */
FunAST::FunAST(ASTInfo *info, Inst *entry, String name)
: ent(entry), _name(name), _ast(&AST::UNDEF) {

	// Mark the entry
	FUN(ent) = this;

	// If no name, look for a name
	String label = LABEL(ent);
	if(label)
		_name = label;

	// Record the function
	info->add(this);
}


/**
 * Remove any link with the instruction representation.
 */
FunAST::~FunAST(void) {
	//ent->removeProp(ID);
	_ast->release();
}


/**
 * @fn Inst *FunAST::entry(void) const
 * Get the instruction entry of this function.
 * @return	Entry instruction.
 */


/**
 * @fn const String& FunAST::name(void) const;
 * Get the name of the function.
 * @return Function name.
 */


/**
 * @fn AST *FunAST::ast(void) const;
 * Get the AST representing the body of this function.
 * @return Body AST.
 */


/**
 * Modify the AST representing the body of this function.
 * @param ast New body AST.
 */
void FunAST::setAst(AST *ast) {
	_ast = ast;
}


} }	// otawa::ast
