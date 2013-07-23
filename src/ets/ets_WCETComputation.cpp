/*
 *	WCETComputation class implementation
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
#include <otawa/ets/WCETComputation.h>
#include <elm/debug.h>
#include <otawa/proc/ProcessorException.h>

//#define WC_TRACE TRACE	//with 
//#define WC_OUT(txt) txt	//	degugging.
#define WC_TRACE		//without 
#define WC_OUT(txt)	//	debugging.


namespace otawa { namespace ets {

/**
 * @class WCETComputation
 * This processor is used for computing the WCET with the Extended Timing Schema.
 *
 * @p Provided Features
 * @li @ref WCET_FEATURE
 *
 * @p Required Features
 * @li @ref FLOWFACT_FEATURE
 * @li @ref BLOCK_TIME_FEATURE
 */

p::declare WCETComputation::reg = p::init("otawa::ets::WCETComputation", Version(1, 2, 0))
	.base(ASTProcessor::reg)
	.maker<WCETComputation>()
	.require(FLOWFACT_FEATURE)
	.require(BLOCK_TIME_FEATURE)
	.provide(WCET_FEATURE);


/**
 */
WCETComputation::WCETComputation(p::declare& r): ASTProcessor(r) {
}

/**
 * Get the WCET of ast with the recursive function: WCETComputation::computation(FrameWork *ws, AST *ast).
 * @param ws	Container framework.
 * @param ast	AST to process.
 */	
void WCETComputation::processAST(WorkSpace *ws, AST *ast) {
	ASSERT(ast);
}


/**
 * @fn int WCETComputation::computation(FrameWork *ws, AST *ast);
 * Compute the WCET for each AST node by using annotations coming from other modules. 
 * Furthermore put annotations (WCET) of each AST node.
 * @param ws	Container workspace.
 * @param ast	AST to process.
 * @return	WCET of the current AST.
 * @exception	io::IOException if one number of iteration of loop or one WCET of function cannot be found.
 */
int WCETComputation::computation(WorkSpace *ws, AST *ast) {
		ASSERT(ast);
		int ELSE, THEN, wcet, N;
		switch(ast->kind()) {
			case AST_Call:{
				ASTInfo *ast_info = ws->getASTInfo();
				Option< FunAST *> fun_res = ast_info->get(ast->toCall()->function()->name());
				if (fun_res){
					AST *fun_ast = (*fun_res)->ast();
					wcet=computation(ws, fun_ast);
					WCET(ast->toCall()) = wcet;
					WC_OUT(cout << "|| " << ast->toCall()->function()->name() << " a pour wcet : " << ast->toCall()->use<int>(ETS::ID_WCET)<< '\n');	
					return wcet;
				}
				else{
					WC_TRACE;
					throw ProcessorException(*this, _
						<< "no wcet for the function : "
						<< ast->toCall()->function()->name());
				}
			}
			case AST_Block:
				WC_OUT(cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour wcet : " << ast->toBlock()->use<int>(ETS::ID_WCET)<< '\n');
				return WCET(ast->toBlock());
				break;
			case AST_Seq:
				wcet=computation(ws, ast->toSeq()->child1())
						+ computation(ws, ast->toSeq()->child2());
				WCET(ast->toSeq()) = wcet;
				WC_OUT(cout << "|| " << ast->toSeq()->first()->get<String>(File::ID_Label,"unknown ") << " a pour wcet : " << ast->toSeq()->use<int>(ETS::ID_WCET)<< '\n');
				return wcet;
				break;
			case AST_If:
				THEN=computation(ws, ast->toIf()->condition())
					+ computation(ws, ast->toIf()->thenPart());
				ELSE=computation(ws, ast->toIf()->condition())
					+ computation(ws, ast->toIf()->elsePart());
				if (THEN>ELSE) 
					WCET(ast->toIf()) = THEN;
				else 
					WCET(ast->toIf()) = ELSE;
				WC_OUT(cout << "|| " << ast->toIf()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour wcet : " << ast->toIf()->use<int>(ETS::ID_WCET)<< '\n');
				return WCET(ast->toIf());
				break;
			case AST_While:
			 	N=LOOP_COUNT(ast->toWhile());
			 	if (N == -1){
					WC_TRACE;
					throw ProcessorException(*this, _ << "no loop bound at : "
						<< LABEL(ast->toWhile()->condition()->first())
						<< " ("
						<< ast->toWhile()->condition()->first()->address()
						<< ")");
				}
				wcet=N*(computation(ws, ast->toWhile()->condition())
							+ computation(ws, ast->toWhile()->body()))
						+ computation(ws, ast->toWhile()->condition());
				WCET(ast->toWhile()) = wcet;
				WC_OUT(cout << "|| " << ast->toWhile()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour wcet : " << ast->toWhile()->use<int>(ETS::ID_WCET)<< '\n');		
				return wcet;
				break;
			case AST_DoWhile:
				N=LOOP_COUNT(ast->toDoWhile());
				if (N == -1){
						WC_TRACE;
						throw io::IOException(_ << "no iteration count for loop"
							<< LABEL(ast->toDoWhile()->condition()->first()) /* "unknown "*/);
				}
				wcet=N*(computation(ws, ast->toDoWhile()->body())
							+ computation(ws, ast->toDoWhile()->condition()));
				WCET(ast->toDoWhile()) = wcet;	
				WC_OUT(cout << "|| " << ast->toDoWhile()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour wcet : " << ast->toDoWhile()->use<int>(ETS::ID_WCET)<< '\n');		
				return wcet;
				break;
			case AST_For:
				N=LOOP_COUNT(ast->toFor());
				if (N == -1){
					WC_TRACE;
					throw io::IOException(_ << "no iteration count for loop "
						<< LABEL(ast->toFor()->condition()->first()) /* "unknown " */);
				}
				wcet=computation(ws, ast->toFor()->initialization())
						+ N*(computation(ws, ast->toFor()->condition())
							+ computation(ws, ast->toFor()->incrementation())
							+ computation(ws, ast->toFor()->body()))
						+ computation(ws, ast->toFor()->condition());
				WCET(ast->toFor()) = wcet;
				WC_OUT(cout << "|| " << ast->toFor()->condition()->first()->get<String>(File::ID_Label,"unknown ") << " a pour wcet : " << ast->toFor()->use<int>(ETS::ID_WCET)<< '\n');	
				return wcet;
				break;
			default:
				WC_OUT(cout << "DEFAULT !!!\n");
				return 0;
		}
}

} }// otawa::ets
