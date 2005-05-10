/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *  src/ets_TrivialAstBlockTime.cpp -- TrivialAstBlockTime class implementation.
 */

#include <otawa/ets/TrivialAstBlockTime.h>
#include <otawa/ets/ETS.h>
#include <otawa/ast.h>
#include <elm/debug.h>

#define TABT_TRACE TRACE
//#define TABT_TRACE
#define TABT_OUT(txt) txt
//#define TABT_OUT(txt)


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
 * See @ref ASTProcessor::processAST().
 */
void TrivialAstBlockTime::processAST(AST *ast) {
	switch(ast->kind()) {
			case AST_Block:
				ast->toBlock()->set<int>(ETS::ID_WCET,ast->toBlock()->countInstructions());
				TABT_OUT(cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toBlock()->use<int>(ETS::ID_WCET)<< '\n');
				break;
			case AST_Seq:
				processAST(ast->toSeq()->child1());
				processAST(ast->toSeq()->child2());
				break;
			case AST_If:
				processAST(ast->toIf()->condition());
				processAST(ast->toIf()->thenPart());
				processAST(ast->toIf()->elsePart());
				break;
			case AST_While:
			 	processAST(ast->toWhile()->condition());
				processAST(ast->toWhile()->body());
				break;
			case AST_DoWhile:
				processAST(ast->toDoWhile()->body());
				processAST(ast->toDoWhile()->condition());
				break;
			case AST_For:
				processAST(ast->toFor()->initialization());
				processAST(ast->toFor()->condition());
				processAST(ast->toFor()->incrementation());
				processAST(ast->toFor()->body());
				break;
			default:
				;
	}
}


} } // otawa::ets
