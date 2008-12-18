/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/proc_FunProcessor.cpp -- FunProcessor class implementation.
 */

#include <otawa/ast.h>
#include <otawa/proc/FunProcessor.h>


namespace otawa {

/**
 * @class FunProcessor
 * This is a specialization of the processor class dedicated to function
 * processing. The @ref Processor::processFrameWork() method just take each
 * function of the framework and apply the processor on.
 * @ingroup proc
 */


/**
 */
FunProcessor::~FunProcessor(void) {
}


/**
 * @fn void FunProcessor::processFun(FunAst *fa);
 * Process the given function.
 * @param FunAST	FunAST to process.
 */


/**
 * Override @ref Processor::processFrameWork().
 */
void FunProcessor::processWorkSpace(WorkSpace *fw) {

	// Get AST information
	ASTInfo *info = fw->getASTInfo();
	
	// For each AST
	for(Iterator<FunAST *> ast(info->functions()); ast; ast++)
		processFun(fw, ast);	
}

} // otawa
