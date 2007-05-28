/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ets_WCETComputation.cpp -- WCETComputation class implementation.
 */

#include <otawa/ets/ETS.h>
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
 */
 
/**
 * Get the WCET of ast with the recursive function: WCETComputation::computation(FrameWork *fw, AST *ast).
 * @param fw	Container framework.
 * @param ast	AST to process.
 */	
void WCETComputation::processAST(WorkSpace *fw, AST *ast) {
	assert(ast);
	int tmp=computation(fw, ast);
}


/**
 * @fn int WCETComputation::computation(FrameWork *fw, AST *ast);
 * Compute the WCET for each AST node by using annotations coming from other modules. 
 * Furthermore put annotations (WCET) of each AST node.
 * @param fw	Container workspace.
 * @param ast	AST to process.
 * @return	WCET of the current AST.
 * @exception	io::IOException if one number of iteration of loop or one WCET of function cannot be found.
 */
int WCETComputation::computation(WorkSpace *fw, AST *ast) {
		assert(ast);
		int ELSE, THEN, wcet, N;
		switch(ast->kind()) {
			case AST_Call:{
				ASTInfo *ast_info = fw->getASTInfo();
				Option< FunAST *> fun_res = ast_info->map().get(ast->toCall()->function()->name());
				if (fun_res){
					AST *fun_ast = (*fun_res)->ast();
					wcet=computation(fw, fun_ast);
					WCET(ast->toCall()) = wcet;
					WC_OUT(cout << "|| " << ast->toCall()->function()->name() << " a pour wcet : " << ast->toCall()->use<int>(ETS::ID_WCET)<< '\n');	
					return wcet;
				}
				else{
					WC_TRACE;
					throw ProcessorException(*this, "Il manque le wcet de la fonction : %s",
						&ast->toCall()->function()->name());
				}
			}
			case AST_Block:
				WC_OUT(cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour wcet : " << ast->toBlock()->use<int>(ETS::ID_WCET)<< '\n');
				return WCET(ast->toBlock());
				break;
			case AST_Seq:
				wcet=computation(fw, ast->toSeq()->child1())
						+ computation(fw, ast->toSeq()->child2());
				WCET(ast->toSeq()) = wcet;
				WC_OUT(cout << "|| " << ast->toSeq()->first()->get<String>(File::ID_Label,"unknown ") << " a pour wcet : " << ast->toSeq()->use<int>(ETS::ID_WCET)<< '\n');
				return wcet;
				break;
			case AST_If:
				THEN=computation(fw, ast->toIf()->condition())
					+ computation(fw, ast->toIf()->thenPart());
				ELSE=computation(fw, ast->toIf()->condition())
					+ computation(fw, ast->toIf()->elsePart());
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
					throw ProcessorException(*this, "Il manque le nb d'it�rations du noeud : %s (%p)",
						&LABEL(ast->toWhile()->condition()->first()) /*  "unknown ") */,
						(void *)(int)ast->toWhile()->condition()->first()->address());
				}
				wcet=N*(computation(fw, ast->toWhile()->condition())
							+ computation(fw, ast->toWhile()->body()))
						+ computation(fw, ast->toWhile()->condition());
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
				wcet=N*(computation(fw, ast->toDoWhile()->body())
							+ computation(fw, ast->toDoWhile()->condition()));
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
				wcet=computation(fw, ast->toFor()->initialization())
						+ N*(computation(fw, ast->toFor()->condition())
							+ computation(fw, ast->toFor()->incrementation())
							+ computation(fw, ast->toFor()->body()))
						+ computation(fw, ast->toFor()->condition());
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
