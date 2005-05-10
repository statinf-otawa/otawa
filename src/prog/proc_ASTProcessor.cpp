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
 * @fn void ASTProcessor::processAST(AST *ast);
 * Process the given AST.
 * @param AST	AST to process.
 */


/**
 */
ASTProcessor::~ASTProcessor(void) {
}


/**
 */
void ASTProcessor::processFun(FunAST *fa) {
	processAST(fa->ast());
}




} // otawa
