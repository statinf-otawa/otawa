/*
 *	$$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ets_CacheHitComputation.cpp -- CacheHitComputation class implementation.
 */

#include <otawa/ets/ETS.h>
#include <otawa/ast.h>
#include <otawa/ets/CacheHitComputation.h>
#include <elm/debug.h>

//#define CHC_OUT(txt) txt	//with debuging
#define CHC_OUT(txt)		//without debuging

namespace otawa { namespace ets {

/**
 * @class CacheHitComputation
 * This processor is used for computing the Hit accesses after the cache simulating.
 */
 
/**
 * @fn void CacheHitComputation::processAST(FrameWork *fw, AST *ast);
 * Get the number of Hit of ast with the recursive function: CacheHitComputation::computation(FrameWork *fw, AST *ast).
 * @param fw	Container framework.
 * @param ast	AST to process.
 */	
void CacheHitComputation::processAST(FrameWork *fw, AST *ast) {
	int tmp = computation(fw, ast);
}

/**
 * @fn int CacheHitComputation::computation(FrameWork *fw, AST *ast);
 * Compute the number of Hit for each AST node by using annotations coming from other modules. 
 * Furthermore put annotations (ETS::HITS) of each AST node.
 * @param fw	Container framework.
 * @param ast	AST to process.
 * @return Hits of the current AST.
 */
int CacheHitComputation::computation(FrameWork *fw, AST *ast){
	int hits;
	switch (ast->kind()){
		case AST_Block: 
			CHC_OUT(cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toBlock()->use<int>(ETS::ID_HITS)<< '\n');
			return ast->toBlock()->use<int>(ETS::ID_HITS);
			break;
		case AST_Seq: {	
			hits = 	computation(fw, ast->toSeq()->child1()) 
						+ computation(fw, ast->toSeq()->child2());
			ast->toSeq()->set<int>(ETS::ID_HITS, hits);
			CHC_OUT(cout << "|| " << ast->toSeq()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toSeq()->use<int>(ETS::ID_HITS)<< '\n');
			return ast->toSeq()->use<int>(ETS::ID_HITS);
			break;
		}
		case AST_If: {
		 	hits = computation(fw, ast->toIf()->elsePart());
			int hits1 = computation(fw, ast->toIf()->thenPart());
			if(hits < hits1)
				ast->toIf()->set<int>(ETS::ID_HITS, hits1 + computation(fw, ast->toIf()->condition()));
			else
				ast->toIf()->set<int>(ETS::ID_HITS, hits + computation(fw, ast->toIf()->condition()));
			CHC_OUT(cout << "|| " << ast->toIf()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toIf()->use<int>(ETS::ID_HITS)<< '\n');
			return ast->toIf()->use<int>(ETS::ID_HITS);
			break;
		}
		case AST_While:	{
			int N = ast->toWhile()->use<int>(ETS::ID_LOOP_COUNT);
			
			hits = 	computation(fw, ast->toWhile()->condition())
						+ N
						*( 	computation(fw, ast->toWhile()->condition())
							+ computation(fw, ast->toWhile()->body()));
			
			int hits_coming_from_first_misses = ast->toWhile()->condition()->use<int>(ETS::ID_FIRST_MISSES) 
												+ ast->toWhile()->body()->use<int>(ETS::ID_FIRST_MISSES) ;
			
			if(N == 0)
				hits_coming_from_first_misses = 0;	
			ast->toWhile()->set<int>(ETS::ID_HITS, hits + (hits_coming_from_first_misses * (N-1)));
			
			CHC_OUT(cout << "|| " << ast->toWhile()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toWhile()->use<int>(ETS::ID_HITS)<< '\n');
			return ast->toWhile()->use<int>(ETS::ID_HITS);
			break;
		}
		case AST_For:	{	
			int N = ast->toFor()->use<int>(ETS::ID_LOOP_COUNT);
			
			hits = 	computation(fw, ast->toFor()->condition())
						+ computation(fw, ast->toFor()->initialization())
						+ N
						*( 	computation(fw, ast->toFor()->condition())
							+ computation(fw, ast->toFor()->body())
							+ computation(fw, ast->toFor()->incrementation()));
			
			int hits_coming_from_first_misses = ast->toFor()->condition()->use<int>(ETS::ID_FIRST_MISSES) 
												+ ast->toFor()->body()->use<int>(ETS::ID_FIRST_MISSES) 
												+ ast->toFor()->incrementation()->use<int>(ETS::ID_FIRST_MISSES);
					
			if(N == 0)
				hits_coming_from_first_misses = 0;	
			ast->toFor()->set<int>(ETS::ID_HITS, hits + (hits_coming_from_first_misses * (N-1)));
			
			CHC_OUT(cout << "|| " << ast->toFor()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toFor()->use<int>(ETS::ID_HITS)<< '\n');
			return ast->toFor()->use<int>(ETS::ID_HITS);
			break;
		}
		case AST_DoWhile:	{
			int N = ast->toDoWhile()->use<int>(ETS::ID_LOOP_COUNT);
			
			hits = 	computation(fw, ast->toDoWhile()->body())
						+ N
						*( 	computation(fw, ast->toDoWhile()->condition())
							+ computation(fw, ast->toDoWhile()->body()));
			
			int hits_coming_from_first_misses = ast->toDoWhile()->condition()->use<int>(ETS::ID_FIRST_MISSES) 
												+ ast->toDoWhile()->body()->use<int>(ETS::ID_FIRST_MISSES) ;
			if(N == 0)
				hits_coming_from_first_misses = 0;
			ast->toDoWhile()->set<int>(ETS::ID_HITS, hits + (hits_coming_from_first_misses * (N-1)));
			
			CHC_OUT(cout << "|| " << ast->toDoWhile()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toDoWhile()->use<int>(ETS::ID_HITS)<< '\n');
			return ast->toDoWhile()->use<int>(ETS::ID_HITS);
			break;
		}
		case AST_Call:{	
			ASTInfo *ast_info = fw->getASTInfo();
			Option< FunAST *> fun_res = ast_info->map().get(ast->toCall()->function()->name());
			if (fun_res){
				AST *fun_ast = (*fun_res)->ast();
				hits = computation(fw, fun_ast);
				ast->toCall()->set<int>(ETS::ID_HITS, hits);
				CHC_OUT(cout << "|| " << ast->toCall()->function()->name() << " a pour nb de hit : " << ast->toCall()->use<int>(ETS::ID_HITS)<< '\n');	
				return ast->toCall()->use<int>(ETS::ID_HITS);
			}
			break;
		}
		default :
			return 0;
	}
}

} }// otawa::ets
