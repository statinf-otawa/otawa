/*
 *	TrivialASTBlockTime class implementation
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
#include <otawa/ets/TrivialAstBlockTime.h>
#include <otawa/ets/features.h>
#include <otawa/ast.h>

//#define TABT_OUT(txt) txt	//with debuging
#define TABT_OUT(txt)		//without debuging

namespace otawa { namespace ets {

/**
 * @class TrivialAstBlockTime
 * This processor is used for computing execution of ast blocks in a trivial
 * way, that is, the multiplication of ast block instruction count by the
 * pipeline depth.
 *
 * @p Provided Features
 * @li @ref BLOCK_TIME_FEATURE
 *
 * @p Configuration
 * @li @ref DEPTH
 *
 * @ingroup ets
 */


p::declare TrivialAstBlockTime::reg = p::init("otawa::ets::TrivialAstBlockTime", Version(1, 2, 0))
	.base(ASTProcessor::reg)
	.maker<TrivialAstBlockTime>()
	.provide(BLOCK_TIME_FEATURE);


/**
 */
TrivialAstBlockTime::TrivialAstBlockTime(p::declare& r): ASTProcessor(r), dep(0) {
}


/**
 * @fn TrivialAstBlockTime::TrivialAstBlockTime(int depth);
 * Build the processor.
 * @param depth	Depth of the pipeline.
 */


/**
 * @fn int TrivialAstBlockTime::depth(void) const;
 * Get the depth of the pipeline.
 * @return	Pipeline depth.
 */


/**
 */
void TrivialAstBlockTime::configure(PropList& props) {
	ASTProcessor::configure(props);
	dep = DEPTH(props);
}


/**
 * Edit the WCET of the ast blocks to ETS::ID_WCET.
 * @param ws	Container framework.
 * @param ast	AST to process.
 */
void TrivialAstBlockTime::processAST(WorkSpace *ws, AST *ast) {
	switch(ast->kind()) {
			case AST_Call:{
				ASTInfo *ast_info = ws->getASTInfo();
				Option< FunAST *> fun_res = ast_info->get(ast->toCall()->function()->name());
				if(fun_res) {
					AST *fun_ast = (*fun_res)->ast();
					processAST(ws, fun_ast);
				}
			}
			case AST_Block:
				WCET(ast->toBlock()) = ast->toBlock()->countInstructions() * dep;
				TABT_OUT(cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour wcet : " << ast->toBlock()->use<int>(ETS::ID_WCET)<< '\n');
				break;
			case AST_Seq:
				processAST(ws, ast->toSeq()->child1());
				processAST(ws, ast->toSeq()->child2());
				break;
			case AST_If:
				processAST(ws, ast->toIf()->condition());
				processAST(ws, ast->toIf()->thenPart());
				processAST(ws, ast->toIf()->elsePart());
				break;
			case AST_While:
			 	processAST(ws, ast->toWhile()->condition());
				processAST(ws, ast->toWhile()->body());
				break;
			case AST_DoWhile:
				processAST(ws, ast->toDoWhile()->body());
				processAST(ws, ast->toDoWhile()->condition());
				break;
			case AST_For:
				processAST(ws, ast->toFor()->initialization());
				processAST(ws, ast->toFor()->condition());
				processAST(ws, ast->toFor()->incrementation());
				processAST(ws, ast->toFor()->body());
				break;
			default:
				;
	}
}


/**
 * Depth of the pipeline for the code processor @ref TrivialAstBlockTime.
 *
 * @ingroup ets
 */
Identifier<int> DEPTH("otawa::ets::DEPTH", 5);

} } // otawa::ets
