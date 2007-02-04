/*
 *	$$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ets_CacheMissComputation.cpp -- CacheMissComputation class implementation.
 */

#include <otawa/ets/ETS.h>
#include <otawa/ast.h>
#include <otawa/ets/CacheMissComputation.h>
#include <elm/debug.h>

//#define CHC_OUT(txt) txt	//with debuging
#define CHC_OUT(txt)		//without debuging

namespace otawa { namespace ets {

/**
 * @class CacheMissComputation
 * This processor is used for computing the Miss accesses after the cache simulating.
 */
 
/**
 * @fn void CacheMissComputation::processAST(FrameWork *fw, AST *ast);
 * Get the number of Miss of ast with the recursive function: CacheMissComputation::computation(FrameWork *fw, AST *ast).
 * @param fw	Container framework.
 * @param ast	AST to process.
 */	
void CacheMissComputation::processAST(FrameWork *fw, AST *ast) {
	int tmp = computation(fw, ast);
}

/**
 * @fn int CacheMissComputation::computation(FrameWork *fw, AST *ast);
 * Compute the number of Miss for each AST node by using annotations coming from other modules. 
 * Furthermore put annotations (ETS::MISSES) of each AST node.
 * @param fw	Container framework.
 * @param ast	AST to process.
 * @return Miss of the current AST.
 */
int CacheMissComputation::computation(FrameWork *fw, AST *ast){
	int misses;
	switch (ast->kind()){
		case AST_Call:{	
			ASTInfo *ast_info = fw->getASTInfo();
			Option< FunAST *> fun_res = ast_info->map().get(ast->toCall()->function()->name());
			if (fun_res){
				AST *fun_ast = (*fun_res)->ast();
				misses = computation(fw, fun_ast);
				MISSES(ast->toCall()) = misses;
				CHC_OUT(cout << "|| " << ast->toCall()->function()->name() << " a pour nb de miss : " << ast->toCall()->use<int>(ETS::ID_MISSES)<< '\n');	
				return MISSES(ast->toCall());
			}
			break;
		}
		case AST_Block: 
			CHC_OUT(cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de miss : " << ast->toBlock()->use<int>(ETS::ID_MISSES)<< '\n');
			return MISSES(ast->toBlock()) + CONFLICTS(ast->toBlock());
			break;
		case AST_Seq: {	
			misses = 	computation(fw, ast->toSeq()->child1()) 
						+ computation(fw, ast->toSeq()->child2());
			MISSES(ast->toSeq()) = misses;
			CHC_OUT(cout << "|| " << ast->toSeq()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de miss : " << ast->toSeq()->use<int>(ETS::ID_MISSES)<< '\n');
			return MISSES(ast->toSeq());
			break;
		}
		case AST_If: {
		 	misses = computation(fw, ast->toIf()->elsePart());
			int misses1 = computation(fw, ast->toIf()->thenPart());
			if(misses < misses1)
				MISSES(ast->toIf()) = misses1 + computation(fw, ast->toIf()->condition());
			else
				MISSES(ast->toIf()) = misses + computation(fw, ast->toIf()->condition());
			CHC_OUT(cout << "|| " << ast->toIf()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de miss : " << ast->toIf()->use<int>(ETS::ID_MISSES)<< '\n');
			return MISSES(ast->toIf());
			break;
		}
		case AST_While:	{
			int N = LOOP_COUNT(ast->toWhile());
			
			misses = 	computation(fw, ast->toWhile()->condition())
						+ N
						*( 	computation(fw, ast->toWhile()->condition())
							+ computation(fw, ast->toWhile()->body()));
							
			int misses_coming_from_first_misses =
				FIRST_MISSES(ast->toWhile()->condition())
				+ FIRST_MISSES(ast->toWhile()->body());	
			
			MISSES(ast->toWhile()) = misses + misses_coming_from_first_misses;
			CHC_OUT(cout << "|| " << ast->toWhile()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de miss : " << ast->toWhile()->use<int>(ETS::ID_MISSES)<< '\n');
			return MISSES(ast->toWhile());
			break;
		}
		case AST_For:	{	
			int N = LOOP_COUNT(ast->toFor());
			
			misses = 	computation(fw, ast->toFor()->condition())
						+ computation(fw, ast->toFor()->initialization())
						+ N
						*( 	computation(fw, ast->toFor()->condition())
							+ computation(fw, ast->toFor()->body())
							+ computation(fw, ast->toFor()->incrementation()));
			
			int misses_coming_from_first_misses =
				FIRST_MISSES(ast->toFor()->condition())
				+ FIRST_MISSES(ast->toFor()->body()) 
				+ FIRST_MISSES(ast->toFor()->incrementation());
					
			MISSES(ast->toFor()) = misses + misses_coming_from_first_misses;
			CHC_OUT(cout << "|| " << ast->toFor()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de miss : " << ast->toFor()->use<int>(ETS::ID_MISSES)<< '\n');
			return MISSES(ast->toFor());
			break;
		}
		case AST_DoWhile:	{
			int N = LOOP_COUNT(ast->toDoWhile());
			
			misses = 	computation(fw, ast->toDoWhile()->body())
						+ N
						*( 	computation(fw, ast->toDoWhile()->condition())
							+ computation(fw, ast->toDoWhile()->body()));
							
			int misses_coming_from_first_misses =
				FIRST_MISSES(ast->toDoWhile()->body())
				+ FIRST_MISSES(ast->toDoWhile()->condition());
			
			MISSES(ast->toDoWhile()) = misses + misses_coming_from_first_misses;
			CHC_OUT(cout << "|| " << ast->toDoWhile()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de miss : " << ast->toDoWhile()->use<int>(ETS::ID_MISSES)<< '\n');
			return MISSES(ast->toDoWhile());
			break;
		}
		default :
			return 0;
	}
}

} }// otawa::ets
