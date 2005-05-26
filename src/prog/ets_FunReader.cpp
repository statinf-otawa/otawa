/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/FunReader.cpp -- FunReader class implementation.
 */

#include <otawa/ets.h>
#include <otawa/ast.h>
#include <otawa/ets/FunReader.h>
#include <elm/genstruct/HashTable.h>

//#define FR_OUT(txt) txt
#define FR_OUT(txt)


namespace otawa { namespace ets{

/**
 * @class FunReader
 * This processor is used for editing the WCET to each function.
 */


/**
 * @fn FunReader::FunReader(genstruct::HashTable<String, int> *hash_table, ASTInfo *info);
 * Build the processor.
 * @param hash_table Hash table containing the WCET of function.
 * @param info All information about the current AST .
 */
	

/**
 * Use the fun_table for editing the WCET of each function to ETS::ID_WCET.
 * <br>If the fun_table contains the key put its value,
 * <br>else put -1.
 * @param ast	AST to process.
 */
void FunReader::processAST(AST *ast) {
	switch(ast->kind()) {
		case AST_Call:{
			String fun_name =ast->toCall()->function()->name();
			int val = fun_table->get(fun_name,-1);
			FR_OUT(cout << "|| " << fun_name << " a pour wcet : " << val << '\n');
		 	ast->toCall()->set<int>(ETS::ID_WCET,val);
		 	Option< FunAST *> fun_res = ast_info->map().get(ast->toCall()->function()->name());
			if (fun_res){
				AST *fun_ast = (*fun_res)->ast();
				processAST(fun_ast);
			}
			else
				ast->toCall()->set<int>(ETS::ID_WCET,-1);
			break;
		}
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
		 	processAST(ast->toWhile()->condition());
			processAST(ast->toWhile()->body());
			break;
		case AST_DoWhile:
			processAST(ast->toDoWhile()->body());				
			processAST(ast->toDoWhile()->condition());
			break;
		case AST_For:
			processAST(ast->toFor()->initialization());
			processAST(ast->toFor()->condition());
			processAST(ast->toFor()->incrementation());
			processAST(ast->toFor()->body());
			break;
		default:
			;
	}
}

} } // otawa::ets
