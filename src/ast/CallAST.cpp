/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/CallAST.h -- interface for CallAST class.
 */

#include <otawa/ast/CallAST.h>
#include <otawa/ast/ASTInfo.h>

namespace otawa {

/**
 * @class CallAST
 * This class is a specialized block AST ended by a function call.
 */


/**
 * Build a new AST call calling the given function.
 * @param block	First instruction of the block.
 * @param size	Size of the block.
 * @param _fun	Called function.
 */
CallAST::CallAST(Inst *block, size_t size, FunAST *_fun)
: BlockAST(block, size), fun(_fun) {
}


/**
 * Build a call AST with only the first instruction of the called function.
 * @param fw		Container framework.
 * @param block		First instruction of the caller block.
 * @param size		Size of the caller block.
 * @param callee	Callee instruction.
 */
CallAST::CallAST(FrameWork *fw, Inst *block, size_t size, Inst *callee)
: BlockAST(block, size) {
	
	// Find the ASTInfo
	ASTInfo *info = fw->get<ASTInfo *>(ASTInfo::ID, 0);
	if(!info)
		info = new ASTInfo(fw);
	
	// Find the function
	fun = info->getFunction(callee);
}


/**
 * @fn inline FunAST *CallAST::function(void) const;
 * Get the AST function called by this AST.
 * @return Called AST function.
 */

} // otawa
