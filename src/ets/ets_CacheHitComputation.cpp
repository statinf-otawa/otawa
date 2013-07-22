/*
 *	CacheHitComputation class implementation
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
#include <otawa/ets/features.h>
#include <otawa/ast.h>
#include <otawa/ets/CacheHitComputation.h>
#include <elm/debug.h>

//#define CHC_OUT(txt) txt	//with debuging
#define CHC_OUT(txt)		//without debuging

namespace otawa { namespace ets {

/**
 * @class CacheHitComputation
 * This processor is used for computing the Hit accesses after the cache simulating.
 *
 * @p Provided Features
 * @li @ref CACHE_HIT_FEATURE
 *
 * @p Required Features
 * @li @ref ACS_FEATURE
 */

p::declare CacheHitComputation::reg = p::init("otawa::ets::CacheHitComputation", Version(1, 2, 0))
	.base(ASTProcessor::reg)
	.maker<CacheHitComputation>()
	.require(ACS_FEATURE)
	.provide(CACHE_HIT_FEATURE);


/**
 */
CacheHitComputation::CacheHitComputation(p::declare& r): ASTProcessor(r) {
}


/**
 * @fn void CacheHitComputation::processAST(FrameWork *ws, AST *ast);
 * Get the number of Hit of ast with the recursive function: CacheHitComputation::computation(FrameWork *ws, AST *ast).
 * @param ws	Container framework.
 * @param ast	AST to process.
 */	
void CacheHitComputation::processAST(WorkSpace *ws, AST *ast) {
	/*int tmp =*/ computation(ws, ast);
}

/**
 * @fn int CacheHitComputation::computation(FrameWork *ws, AST *ast);
 * Compute the number of Hit for each AST node by using annotations coming from other modules. 
 * Furthermore put annotations (ETS::HITS) of each AST node.
 * @param ws	Container framework.
 * @param ast	AST to process.
 * @return Hits of the current AST.
 */
int CacheHitComputation::computation(WorkSpace *ws, AST *ast){
	int hits;
	switch (ast->kind()){
		case AST_Block: 
			CHC_OUT(cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toBlock()->use<int>(ETS::ID_HITS)<< '\n');
			return HITS(ast->toBlock());
			break;
		case AST_Seq: {	
			hits = 	computation(ws, ast->toSeq()->child1())
						+ computation(ws, ast->toSeq()->child2());
			HITS(ast->toSeq()) = hits;
			CHC_OUT(cout << "|| " << ast->toSeq()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toSeq()->use<int>(ETS::ID_HITS)<< '\n');
			return HITS(ast->toSeq());
			break;
		}
		case AST_If: {
		 	hits = computation(ws, ast->toIf()->elsePart());
			int hits1 = computation(ws, ast->toIf()->thenPart());
			if(hits < hits1)
				HITS(ast->toIf()) = hits1 + computation(ws, ast->toIf()->condition());
			else
				HITS(ast->toIf()) = hits + computation(ws, ast->toIf()->condition());
			CHC_OUT(cout << "|| " << ast->toIf()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toIf()->use<int>(ETS::ID_HITS)<< '\n');
			return HITS(ast->toIf());
			break;
		}
		case AST_While:	{
			int N = LOOP_COUNT(ast->toWhile());
			
			hits = 	computation(ws, ast->toWhile()->condition())
						+ N
						*( 	computation(ws, ast->toWhile()->condition())
							+ computation(ws, ast->toWhile()->body()));
			
			int hits_coming_from_first_misses =
				FIRST_MISSES(ast->toWhile()->condition()) 
				+ FIRST_MISSES(ast->toWhile()->body());
			
			if(N == 0)
				hits_coming_from_first_misses = 0;	
			HITS(ast->toWhile()) = hits + (hits_coming_from_first_misses * (N-1));
			
			CHC_OUT(cout << "|| " << ast->toWhile()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toWhile()->use<int>(ETS::ID_HITS)<< '\n');
			return HITS(ast->toWhile());
			break;
		}
		case AST_For:	{	
			int N = LOOP_COUNT(ast->toFor());
			
			hits = 	computation(ws, ast->toFor()->condition())
						+ computation(ws, ast->toFor()->initialization())
						+ N
						*( 	computation(ws, ast->toFor()->condition())
							+ computation(ws, ast->toFor()->body())
							+ computation(ws, ast->toFor()->incrementation()));
			
			int hits_coming_from_first_misses =
				FIRST_MISSES(ast->toFor()->condition()) 
				+ FIRST_MISSES(ast->toFor()->body()) 
				+ FIRST_MISSES(ast->toFor()->incrementation());
					
			if(N == 0)
				hits_coming_from_first_misses = 0;	
			HITS(ast->toFor()) = hits + (hits_coming_from_first_misses * (N-1));
			
			CHC_OUT(cout << "|| " << ast->toFor()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toFor()->use<int>(ETS::ID_HITS)<< '\n');
			return HITS(ast->toFor());
			break;
		}
		case AST_DoWhile:	{
			int N = LOOP_COUNT(ast->toDoWhile());
			
			hits = 	computation(ws, ast->toDoWhile()->body())
						+ N
						*( 	computation(ws, ast->toDoWhile()->condition())
							+ computation(ws, ast->toDoWhile()->body()));
			
			int hits_coming_from_first_misses =
				FIRST_MISSES(ast->toDoWhile()->condition())
				+ FIRST_MISSES(ast->toDoWhile()->body()) ;
			if(N == 0)
				hits_coming_from_first_misses = 0;
			HITS(ast->toDoWhile()) = hits + (hits_coming_from_first_misses * (N-1));
			
			CHC_OUT(cout << "|| " << ast->toDoWhile()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toDoWhile()->use<int>(ETS::ID_HITS)<< '\n');
			return HITS(ast->toDoWhile());
			break;
		}
		case AST_Call:{	
			ASTInfo *ast_info = ws->getASTInfo();
			Option< FunAST *> fun_res = ast_info->get(ast->toCall()->function()->name());
			if (fun_res){
				AST *fun_ast = (*fun_res)->ast();
				hits = computation(ws, fun_ast);
				HITS(ast->toCall()) = hits;
				CHC_OUT(cout << "|| " << ast->toCall()->function()->name() << " a pour nb de hit : " << ast->toCall()->use<int>(ETS::ID_HITS)<< '\n');	
				return HITS(ast->toCall());
			}
			break;
		}
		default :
			return 0;
	}
	return 0;
}

} }// otawa::ets
