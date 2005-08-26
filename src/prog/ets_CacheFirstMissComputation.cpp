/*
 *	$$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ets_CacheFirstMissComputation.cpp -- CacheFirstMissComputation class implementation.
 */

#include <otawa/ets/ETS.h>
#include <otawa/ast.h>
#include <otawa/ets/CacheFirstMissComputation.h>
#include <elm/debug.h>

//#define CFMC_OUT(txt) txt	//with debuging
#define CFMC_OUT(txt)		//without debuging

namespace otawa { namespace ets {

/**
 * @class CacheFirstMissComputation
 * This processor is used for computing the First Misses accesses after the cache simulating.
 */
 
/**
 * @fn void CacheFirstMissComputation::processAST(FrameWork *fw, AST *ast);
 * Get the number of First Miss of ast with the recursive function: CacheFirstMissComputation::computation(FrameWork *fw, AST *ast).
 * @param fw	Container framework.
 * @param ast	AST to process.
 */	
void CacheFirstMissComputation::processAST(FrameWork *fw, AST *ast) {
	int tmp = computation(fw, ast);
}

/**
 * @fn int CacheFirstMissComputation::computation(FrameWork *fw, AST *ast);
 * Compute the number of First Miss for each AST node by using annotations coming from other modules. 
 * Furthermore put annotations (ETS::FIRST_MISSES) of each AST node.
 * @param fw	Container framework.
 * @param ast	AST to process.
 * @return First Miss of the current AST.
 */
int CacheFirstMissComputation::computation(FrameWork *fw, AST *ast){
	int first_misses;
	switch (ast->kind()){
		case AST_Call:{	
			ASTInfo *ast_info = fw->getASTInfo();
			Option< FunAST *> fun_res = ast_info->map().get(ast->toCall()->function()->name());
			if (fun_res){
				AST *fun_ast = (*fun_res)->ast();
				first_misses = computation(fw, fun_ast);
				ast->toCall()->set<int>(ETS::ID_FIRST_MISSES, first_misses);
				CFMC_OUT(cout << "|| " << ast->toCall()->function()->name() << " a pour nb de first miss : " << ast->toCall()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');	
				return ast->toCall()->use<int>(ETS::ID_FIRST_MISSES);
			}
			break;
		}
		case AST_Block: 
			CFMC_OUT(cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toBlock()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');
			return ast->toBlock()->use<int>(ETS::ID_FIRST_MISSES);
			break;
		case AST_Seq: {	
			first_misses = 	computation(fw, ast->toSeq()->child1()) 
						+ computation(fw, ast->toSeq()->child2());
			ast->toSeq()->set<int>(ETS::ID_FIRST_MISSES, first_misses);
			CFMC_OUT(cout << "|| " << ast->toSeq()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toSeq()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');
			return ast->toSeq()->use<int>(ETS::ID_FIRST_MISSES);
			break;
		}
		case AST_If: {
		 	first_misses = computation(fw, ast->toIf()->elsePart());
			int first_misses1 = computation(fw, ast->toIf()->thenPart());
			if(first_misses < first_misses1)
				ast->toIf()->set<int>(ETS::ID_FIRST_MISSES, first_misses1 + computation(fw, ast->toIf()->condition()));
			else
				ast->toIf()->set<int>(ETS::ID_FIRST_MISSES, first_misses + computation(fw, ast->toIf()->condition()));
			CFMC_OUT(cout << "|| " << ast->toIf()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toIf()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');
			return ast->toIf()->use<int>(ETS::ID_FIRST_MISSES);
			break;
		}
		case AST_While:	{
			int N = ast->toWhile()->use<int>(ETS::ID_LOOP_COUNT);
			
			first_misses = 	computation(fw, ast->toWhile()->condition())
						+ N
						*( 	computation(fw, ast->toWhile()->condition())
							+ computation(fw, ast->toWhile()->body()));
			
			ast->toWhile()->set<int>(ETS::ID_FIRST_MISSES, first_misses);
			CFMC_OUT(cout << "|| " << ast->toWhile()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toWhile()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');
			return ast->toWhile()->use<int>(ETS::ID_FIRST_MISSES);
			break;
		}
		case AST_For:	{	
			int N = ast->toFor()->use<int>(ETS::ID_LOOP_COUNT);
			
			first_misses = 	computation(fw, ast->toFor()->condition())
						+ computation(fw, ast->toFor()->initialization())
						+ N
						*( 	computation(fw, ast->toFor()->condition())
							+ computation(fw, ast->toFor()->body())
							+ computation(fw, ast->toFor()->incrementation()));
			
			ast->toFor()->set<int>(ETS::ID_FIRST_MISSES, first_misses);
			CFMC_OUT(cout << "|| " << ast->toFor()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toFor()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');
			return ast->toFor()->use<int>(ETS::ID_FIRST_MISSES);
			break;
		}
		case AST_DoWhile:	{
			int N = ast->toDoWhile()->use<int>(ETS::ID_LOOP_COUNT);
			
			first_misses = 	computation(fw, ast->toDoWhile()->body())
						+ N
						*( 	computation(fw, ast->toDoWhile()->condition())
							+ computation(fw, ast->toDoWhile()->body()));
			
			ast->toDoWhile()->set<int>(ETS::ID_FIRST_MISSES, first_misses);
			CFMC_OUT(cout << "|| " << ast->toDoWhile()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toDoWhile()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');
			return ast->toDoWhile()->use<int>(ETS::ID_FIRST_MISSES);
			break;
		}
		default :
			return 0;
	}
}

} }// otawa::ets
