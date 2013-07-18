/*
 *	CallAST class implementation
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
#include <otawa/ast/CallAST.h>
#include <otawa/ast/ASTInfo.h>
#include <otawa/ast/features.h>

namespace otawa { namespace ast {

/**
 * @class CallAST
 * This class is a specialized block AST ended by a function call.
 *
 * @ingroup ast
 */


/**
 * Build a new AST call calling the given function.
 * @param block	First instruction of the block.
 * @param size	Size of the block.
 * @param _fun	Called function.
 */
CallAST::CallAST(Inst *block, ot::size size, FunAST *_fun)
: BlockAST(block, size), fun(_fun) {
}


/**
 * Build a call AST with only the first instruction of the called function.
 * @param ws		Container workspace.
 * @param block		First instruction of the caller block.
 * @param size		Size of the caller block.
 * @param callee	Callee instruction.
 */
CallAST::CallAST(WorkSpace *ws, Inst *block, ot::size size, Inst *callee)
: BlockAST(block, size) {

	// Find the ASTInfo
	ASTInfo *info = INFO(ws);
	if(!info)
		info = new ASTInfo(ws);

	// Find the function
	fun = info->getFunction(callee);
}


/**
 * @fn inline FunAST *CallAST::function(void) const;
 * Get the AST function called by this AST.
 * @return Called AST function.
 */

} } // otawa::ast
