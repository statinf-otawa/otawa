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

#define WC_TRACE TRACE
//#define WC_TRACE
#define WC_OUT(txt) txt
//#define WC_OUT(txt)


namespace otawa { namespace ets {

/**
 * @class WCETComputation
 * This class is used for computing the WCET from the root AST
 */

/**
 * See @ref ASTProcessor::processAST().
 */	
void WCETComputation::processAST(AST *ast) {
	int tmp=computation(ast);
}

/**
 */
int WCETComputation::computation(AST *ast) {
		int ELSE, THEN, wcet, N;
		switch(ast->kind()) {
			case AST_Block:
				WC_OUT(cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toBlock()->use<int>(ETS::ID_WCET)<< '\n');
				return ast->toBlock()->use<int>(ETS::ID_WCET);
				break;
			case AST_Seq:
				wcet=computation(ast->toSeq()->child1())
						+ computation(ast->toSeq()->child2());
				ast->toSeq()->set<int>(ETS::ID_WCET,wcet);
				WC_OUT(cout << "|| " << ast->toSeq()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toSeq()->use<int>(ETS::ID_WCET)<< '\n');
				return wcet;
				break;
			case AST_If:
				THEN=computation(ast->toIf()->condition())
					+ computation(ast->toIf()->thenPart());
				ELSE=computation(ast->toIf()->condition())
					+ computation(ast->toIf()->elsePart());
				if (THEN>ELSE) {
					ast->toIf()->set<int>(ETS::ID_WCET,THEN);
					WC_OUT(cout << "|| " << ast->toIf()->thenPart()->toBlock()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toIf()->use<int>(ETS::ID_WCET)<< '\n');
					return THEN;
				}
				else {
					ast->toIf()->set<int>(ETS::ID_WCET,ELSE);
					WC_OUT(cout << "|| " << ast->toIf()->elsePart()->toBlock()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toIf()->use<int>(ETS::ID_WCET)<< '\n');
					return ELSE;
				}
				break;
			case AST_While:
			 	N=ast->toWhile()->use<int>(ETS::ID_LOOP_COUNT);
			 	if (N == -1){
						WC_TRACE;
						throw io::IOException(String("Il manque le nb d'itérations du noeud : "+ ast->toWhile()->condition()->toBlock()->first()->get<String>(File::ID_Label, "problem! ")));
				}
				wcet=N*(computation(ast->toWhile()->condition())
							+ computation(ast->toWhile()->body()))
						+ computation(ast->toWhile()->condition());
				ast->toWhile()->set<int>(ETS::ID_WCET,wcet);
				WC_OUT(cout << "|| " << ast->toWhile()->condition()->toBlock()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toWhile()->use<int>(ETS::ID_WCET)<< '\n');		
				return wcet;
				break;
			case AST_DoWhile:
				N=ast->toDoWhile()->use<int>(ETS::ID_LOOP_COUNT);
				if (N == -1){
						WC_TRACE;
						throw io::IOException(String("Il manque le nb d'itération du noeud : "+ ast->toDoWhile()->condition()->toBlock()->first()->get<String>(File::ID_Label, "problem! ")));
				}
				wcet=N*(computation(ast->toDoWhile()->body())
							+ computation(ast->toDoWhile()->condition()));
				ast->toDoWhile()->set<int>(ETS::ID_WCET,wcet);	
				WC_OUT(cout << "|| " << ast->toDoWhile()->condition()->toBlock()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toDoWhile()->use<int>(ETS::ID_WCET)<< '\n');		
				return wcet;
				break;
			case AST_For:
				N=ast->toFor()->use<int>(ETS::ID_LOOP_COUNT);
				if (N == -1){
						WC_TRACE;
						throw io::IOException(String("Il manque le nb d'itération du noeud : "+ ast->toFor()->condition()->toBlock()->first()->get<String>(File::ID_Label, "problem! ")));
				}
				wcet=computation(ast->toFor()->initialization())
						+ N*(computation(ast->toFor()->condition())
							+ computation(ast->toFor()->incrementation())
							+ computation(ast->toFor()->body()))
						+ computation(ast->toFor()->condition());
				ast->toFor()->set<int>(ETS::ID_WCET,wcet);
				WC_OUT(cout << "|| " << ast->toFor()->condition()->toBlock()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toFor()->use<int>(ETS::ID_WCET)<< '\n');	
				return wcet;
				break;
			case AST_Call:
				ast->toCall()->set<int>(ETS::ID_WCET,10); 
				return 10;
				break;
			default:
				return 0;
		}
}

} }// otawa::ets
