/*
 *	CacheFirstMissComputation class implementation
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
#include <otawa/ets/CacheFirstMissComputation.h>
#include <elm/debug.h>

//#define CFMC_OUT(txt) txt	//with debuging
#define CFMC_OUT(txt)		//without debuging

namespace otawa { namespace ets {

/**
 * @class CacheFirstMissComputation
 * This processor is used for computing the First Misses accesses after the cache simulating.
 *
 * @p Provided Features
 * @li @ref CACHE_FIRST_MISS_FEATURE
 *
 * @p Required Features
 * @li @ref ACS_FEATURE
 */

p::declare CacheFirstMissComputation::reg = p::init("otawa::ets::CacheFirstMissComputation", Version(1, 2, 0))
	.base(ASTProcessor::reg)
	.maker<CacheFirstMissComputation>()
	.require(ACS_FEATURE)
	.provide(CACHE_FIRST_MISS_FEATURE);


/**
 */
CacheFirstMissComputation::CacheFirstMissComputation(p::declare& r): ASTProcessor(r) {
}


/**
 * @fn void CacheFirstMissComputation::processAST(FrameWork *ws, AST *ast);
 * Get the number of First Miss of ast with the recursive function: CacheFirstMissComputation::computation(FrameWork *ws, AST *ast).
 * @param ws	Container framework.
 * @param ast	AST to process.
 */	
void CacheFirstMissComputation::processAST(WorkSpace *ws, AST *ast) {
	/*int tmp =*/ computation(ws, ast);
}

/**
 * @fn int CacheFirstMissComputation::computation(FrameWork *ws, AST *ast);
 * Compute the number of First Miss for each AST node by using annotations coming from other modules. 
 * Furthermore put annotations (ETS::FIRST_MISSES) of each AST node.
 * @param ws	Container framework.
 * @param ast	AST to process.
 * @return First Miss of the current AST.
 */
int CacheFirstMissComputation::computation(WorkSpace *ws, AST *ast){
	int first_misses;
	switch (ast->kind()){
		case AST_Call:{	
			ASTInfo *ast_info = ws->getASTInfo();
			Option< FunAST *> fun_res = ast_info->get(ast->toCall()->function()->name());
			if (fun_res){
				AST *fun_ast = (*fun_res)->ast();
				first_misses = computation(ws, fun_ast);
				FIRST_MISSES(ast->toCall()) = first_misses;
				CFMC_OUT(cout << "|| " << ast->toCall()->function()->name() << " a pour nb de first miss : " << ast->toCall()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');	
				return FIRST_MISSES(ast->toCall());
			}
			break;
		}
		case AST_Block: 
			CFMC_OUT(cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toBlock()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');
			return FIRST_MISSES(ast->toBlock());
			break;
		case AST_Seq: {	
			first_misses = 	computation(ws, ast->toSeq()->child1())
						+ computation(ws, ast->toSeq()->child2());
			FIRST_MISSES(ast->toSeq()) = first_misses;
			CFMC_OUT(cout << "|| " << ast->toSeq()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toSeq()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');
			return FIRST_MISSES(ast->toSeq());
			break;
		}
		case AST_If: {
		 	first_misses = computation(ws, ast->toIf()->elsePart());
			int first_misses1 = computation(ws, ast->toIf()->thenPart());
			if(first_misses < first_misses1)
				FIRST_MISSES(ast->toIf()) = first_misses1
					+ computation(ws, ast->toIf()->condition());
			else
				FIRST_MISSES(ast->toIf()) = first_misses
					+ computation(ws, ast->toIf()->condition());
			CFMC_OUT(cout << "|| " << ast->toIf()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toIf()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');
			return FIRST_MISSES(ast->toIf());
			break;
		}
		case AST_While:	{
			int N = LOOP_COUNT(ast->toWhile());
			
			first_misses = 	computation(ws, ast->toWhile()->condition())
						+ N
						*( 	computation(ws, ast->toWhile()->condition())
							+ computation(ws, ast->toWhile()->body()));
			
			FIRST_MISSES(ast->toWhile()) = first_misses;
			CFMC_OUT(cout << "|| " << ast->toWhile()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toWhile()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');
			return FIRST_MISSES(ast->toWhile());
			break;
		}
		case AST_For:	{	
			int N = LOOP_COUNT(ast->toFor());
			
			first_misses = 	computation(ws, ast->toFor()->condition())
						+ computation(ws, ast->toFor()->initialization())
						+ N
						*( 	computation(ws, ast->toFor()->condition())
							+ computation(ws, ast->toFor()->body())
							+ computation(ws, ast->toFor()->incrementation()));
			
			FIRST_MISSES(ast->toFor()) = first_misses;
			CFMC_OUT(cout << "|| " << ast->toFor()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toFor()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');
			return FIRST_MISSES(ast->toFor());
			break;
		}
		case AST_DoWhile:	{
			int N = LOOP_COUNT(ast->toDoWhile());
			
			first_misses = 	computation(ws, ast->toDoWhile()->body())
						+ N
						*( 	computation(ws, ast->toDoWhile()->condition())
							+ computation(ws, ast->toDoWhile()->body()));
			
			FIRST_MISSES(ast->toDoWhile()) = first_misses;
			CFMC_OUT(cout << "|| " << ast->toDoWhile()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toDoWhile()->use<int>(ETS::ID_FIRST_MISSES)<< '\n');
			return FIRST_MISSES(ast->toDoWhile());
			break;
		}
		default :
			return 0;
	}
	return 0;
}

} }// otawa::ets
