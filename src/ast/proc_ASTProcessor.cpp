/*
 *	ASTProcessor class implementation
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
#include <otawa/ast/ASTProcessor.h>
#include <otawa/ast/ASTInfo.h>

namespace otawa { namespace ast {

/**
 * @class ASTProcessor
 * This is a specialization of the processor class dedicated to AST
 * processing. The @ref Processor::processFrameWork() method just take each
 * AST of the framework and apply the processor on.
 *
 * @ingroup ast
 */


/**
 */
ASTProcessor::ASTProcessor(p::declare& r): FunProcessor(r) {
}


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




} }	// otawa::ast
