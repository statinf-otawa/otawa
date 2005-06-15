/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/proc_ASTProcessor.cpp -- ASTProcessor class implementation.
 */

#include <otawa/proc/ASTProcessor.h>
#include <otawa/ast/ASTInfo.h>

namespace otawa {

/**
 * @class ASTProcessor
 * This is a specialization of the processor class dedicated to AST
 * processing. The @ref Processor::processFrameWork() method just take each
 * AST of the framework and apply the processor on.
 */

/**
 * @fn void ASTProcessor::processAST(FrameWork *fw, AST *ast);
 * Process the given AST.
 * @param fw	Container framework.
 * @param AST	AST to process.
 */


/**
 */
ASTProcessor::~ASTProcessor(void) {
}


/**
 */
void ASTProcessor::processFun(FrameWork *fw, FunAST *fa) {
	processAST(fw, fa->ast());
}




} // otawa
