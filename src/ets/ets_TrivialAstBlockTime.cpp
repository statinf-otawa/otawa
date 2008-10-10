/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *  src/ets_TrivialAstBlockTime.cpp -- TrivialAstBlockTime class implementation.
 */

#include <otawa/ets/TrivialAstBlockTime.h>
#include <otawa/ets/ETS.h>
#include <otawa/ast.h>

//#define TABT_OUT(txt) txt	//with debuging
#define TABT_OUT(txt)		//without debuging

namespace otawa { namespace ets {

/**
 * @class TrivialAstBlockTime
 * This processor is used for computing execution of ast blocks in a trivial
 * way, that is, the multiplication of ast block instruction count by the
 * pipeline depth.
 */

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
 * Edit the WCET of the ast blocks to ETS::ID_WCET.
 * @param fw	Container framework.
 * @param ast	AST to process.
 */
void TrivialAstBlockTime::processAST(WorkSpace *fw, AST *ast) {
	switch(ast->kind()) {
			case AST_Call:{
				ASTInfo *ast_info = fw->getASTInfo();
				Option< FunAST *> fun_res = ast_info->get(ast->toCall()->function()->name());
				if(fun_res) {
					AST *fun_ast = (*fun_res)->ast();
					processAST(fw, fun_ast);
				}
			}
			case AST_Block:
				WCET(ast->toBlock()) = ast->toBlock()->countInstructions() * dep;
				TABT_OUT(cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour wcet : " << ast->toBlock()->use<int>(ETS::ID_WCET)<< '\n');
				break;
			case AST_Seq:
				processAST(fw, ast->toSeq()->child1());
				processAST(fw, ast->toSeq()->child2());
				break;
			case AST_If:
				processAST(fw, ast->toIf()->condition());
				processAST(fw, ast->toIf()->thenPart());
				processAST(fw, ast->toIf()->elsePart());
				break;
			case AST_While:
			 	processAST(fw, ast->toWhile()->condition());
				processAST(fw, ast->toWhile()->body());
				break;
			case AST_DoWhile:
				processAST(fw, ast->toDoWhile()->body());
				processAST(fw, ast->toDoWhile()->condition());
				break;
			case AST_For:
				processAST(fw, ast->toFor()->initialization());
				processAST(fw, ast->toFor()->condition());
				processAST(fw, ast->toFor()->incrementation());
				processAST(fw, ast->toFor()->body());
				break;
			default:
				;
	}
}

} } // otawa::ets
