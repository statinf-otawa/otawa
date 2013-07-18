/*
 *	FunProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005, IRIT UPS.
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
#include <otawa/ast.h>
#include <otawa/ast/FunProcessor.h>
#include <otawa/ast/features.h>


namespace otawa { namespace ast {

/**
 * @class FunProcessor
 * This is a specialization of the processor class dedicated to function
 * processing. The @ref Processor::processFrameWork() method just take each
 * function of the framework and apply the processor on.
 *
 * @ingroup ast
 */


p::declare FunProcessor::reg = p::init("otawa::ast::Processor", Version(1, 0, 0))
	.require(FEATURE);


/**
 */
FunProcessor::FunProcessor(p::declare& r): Processor(r) {
}

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
void FunProcessor::processWorkSpace(WorkSpace *ws) {

	// Get AST information
	ASTInfo *info = ASTInfo::getInfo(ws);
	
	// For each AST
	for(ASTInfo::Iterator ast(info); ast; ast++)
		processFun(ws, ast);
}

} }	// otawa::ast
