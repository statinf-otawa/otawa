/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/proc_ASTProcessor.cpp -- ASTProcessor class implementation.
 */

#include <otawa/ast/ASTProcessor.h>
#include <otawa/ast/ASTInfo.h>

namespace otawa {

/**
 * @class ASTProcessor
 * This is a specialization of the processor class dedicated to AST
 * processing. The @ref Processor::processFrameWork() method just take each
 * AST of the framework and apply the processor on.
 */

/**
 * @fn void ASTProcessor::processAST(FrameWork *ws, AST *ast);
 * Process the given AST.
 * @param ws	Container framework.
 * @param AST	AST to process.
 */


/**
 */
ASTProcessor::~ASTProcessor(void) {
}


/**
 */
void ASTProcessor::processFun(WorkSpace *ws, FunAST *fa) {
	processAST(ws, fa->ast());
}




} // otawa
