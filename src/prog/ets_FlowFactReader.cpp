/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ets_FlowFactReader.cpp -- FlowFactReader class implementation.
 */

#include <otawa/ets/ETS.h>
#include <otawa/ast.h>
#include <otawa/ets/FlowFactReader.h>

//#define FFR_OUT(txt) txt
#define FFR_OUT(txt)


namespace otawa { namespace ets {
	
/**
 * @class FlowFactReader
 * This processor is used for editing the number of iteration to each loop. 
 */
	

/**
 * @fn FlowFactReader::FlowFactReader(genstruct::HashTable<String, int> *hash_table, ASTInfo *info);
 * Build the processor.
 * @param hash_table Hash table containing the number of iteration of loop (DoWhileAST, WhileAST and ForAST).
 * @param info All information about the current AST .
 */	
	
				
/**
 * Use the loop_table for editing the number of iteration of each loop to ETS::ID_LOOP_COUNT.
 * <br>If the loop_table contains the key put its value,
 * <br>else put -1.
 * @param ast	AST to process.
 */		
	void FlowFactReader::processAST(AST *ast){
		int val;
		switch(ast->kind()) {
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
				val = loop_table->get(ast->toWhile()->condition()->first()->get<String>(File::ID_Label, "problem! "),-1);
				FFR_OUT(cout << "|| " << ast->toWhile()->condition()->first()->get<String>(File::ID_Label, "problem! ")<< " a pour nb d'iter : "<< val << '\n');
			 	ast->toWhile()->set<int>(ETS::ID_LOOP_COUNT,val);
			 	processAST(ast->toWhile()->condition());
			 	processAST(ast->toWhile()->body());
			 	break;
			case AST_DoWhile:
				val = loop_table->get(ast->toDoWhile()->condition()->first()->get<String>(File::ID_Label, "problem! "),-1);
				FFR_OUT(cout << "|| "<< ast->toDoWhile()->condition()->first()->get<String>(File::ID_Label, "problem! ") << " a pour nb d'iter : "<< val << '\n');
				ast->toDoWhile()->set<int>(ETS::ID_LOOP_COUNT,val);
				processAST(ast->toDoWhile()->condition());
				processAST(ast->toDoWhile()->body());
				break;
			case AST_For:
				val = loop_table->get(ast->toFor()->condition()->first()->get<String>(File::ID_Label, "problem! "),-1);
				FFR_OUT(cout << "|| " << ast->toFor()->condition()->first()->get<String>(File::ID_Label, "problem! ") << " a pour nb d'iter : "<< val << '\n');
				ast->toFor()->set<int>(ETS::ID_LOOP_COUNT,val);
				processAST(ast->toFor()->initialization());
				processAST(ast->toFor()->condition());
				processAST(ast->toFor()->body());
				processAST(ast->toFor()->incrementation());
				break;
			case AST_Call:{
				Option< FunAST *> fun_res = ast_info->map().get(ast->toCall()->function()->name());
				if(fun_res) {
					AST *fun_ast = (*fun_res)->ast();
					processAST(fun_ast);
				}
				break;
			}
			default:
				;
		}
	}

} } //otawa::ets	
