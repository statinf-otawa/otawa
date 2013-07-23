/*
 *	CacheHitComputation class implementation
 *-
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
#include <otawa/ets/CacheMissComputation.h>
#include <elm/debug.h>

//#define CHC_OUT(txt) txt	//with debuging
#define CHC_OUT(txt)		//without debuging

namespace otawa { namespace ets {

/**
 * @class CacheMissComputation
 * This processor is used for computing the Miss accesses after the cache simulating.
 *
 * @p Provided Features
 * @li @ref CACHE_MISS_FEATURE
 *
 * @p Required Features
 * @li @ref ACS_FEATURE
 */

p::declare CacheMissComputation::reg = p::init("otawa::ets::CacheMissComputation", Version(1, 2, 0))
	.base(ASTProcessor::reg)
	.maker<CacheMissComputation>()
	.require(ACS_FEATURE)
	.provide(CACHE_MISS_FEATURE);


/**
 */
CacheMissComputation::CacheMissComputation(p::declare& r): ASTProcessor(r) {
}
 
/**
 * @fn void CacheMissComputation::processAST(FrameWork *ws, AST *ast);
 * Get the number of Miss of ast with the recursive function: CacheMissComputation::computation(FrameWork *ws, AST *ast).
 * @param ws	Container framework.
 * @param ast	AST to process.
 */	
void CacheMissComputation::processAST(WorkSpace *ws, AST *ast) {
	/*int tmp =*/ computation(ws, ast);
}

/**
 * @fn int CacheMissComputation::computation(FrameWork *ws, AST *ast);
 * Compute the number of Miss for each AST node by using annotations coming from other modules. 
 * Furthermore put annotations (ETS::MISSES) of each AST node.
 * @param ws	Container framework.
 * @param ast	AST to process.
 * @return Miss of the current AST.
 */
int CacheMissComputation::computation(WorkSpace *ws, AST *ast){
	int misses;
	switch (ast->kind()){
		case AST_Call:{	
			ASTInfo *ast_info = ws->getASTInfo();
			Option< FunAST *> fun_res = ast_info->get(ast->toCall()->function()->name());
			if (fun_res){
				AST *fun_ast = (*fun_res)->ast();
				misses = computation(ws, fun_ast);
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
			misses = 	computation(ws, ast->toSeq()->child1())
						+ computation(ws, ast->toSeq()->child2());
			MISSES(ast->toSeq()) = misses;
			CHC_OUT(cout << "|| " << ast->toSeq()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de miss : " << ast->toSeq()->use<int>(ETS::ID_MISSES)<< '\n');
			return MISSES(ast->toSeq());
			break;
		}
		case AST_If: {
		 	misses = computation(ws, ast->toIf()->elsePart());
			int misses1 = computation(ws, ast->toIf()->thenPart());
			if(misses < misses1)
				MISSES(ast->toIf()) = misses1 + computation(ws, ast->toIf()->condition());
			else
				MISSES(ast->toIf()) = misses + computation(ws, ast->toIf()->condition());
			CHC_OUT(cout << "|| " << ast->toIf()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de miss : " << ast->toIf()->use<int>(ETS::ID_MISSES)<< '\n');
			return MISSES(ast->toIf());
			break;
		}
		case AST_While:	{
			int N = LOOP_COUNT(ast->toWhile());
			
			misses = 	computation(ws, ast->toWhile()->condition())
						+ N
						*( 	computation(ws, ast->toWhile()->condition())
							+ computation(ws, ast->toWhile()->body()));
							
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
			
			misses = 	computation(ws, ast->toFor()->condition())
						+ computation(ws, ast->toFor()->initialization())
						+ N
						*( 	computation(ws, ast->toFor()->condition())
							+ computation(ws, ast->toFor()->body())
							+ computation(ws, ast->toFor()->incrementation()));
			
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
			
			misses = 	computation(ws, ast->toDoWhile()->body())
						+ N
						*( 	computation(ws, ast->toDoWhile()->condition())
							+ computation(ws, ast->toDoWhile()->body()));
							
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
	return 0;
}

} }// otawa::ets
