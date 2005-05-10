/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ets_FlowFactReader.cpp -- FlowFactReader class implementation.
 */

#include <otawa/ets/ETS.h>
#include <otawa/ast.h>
#include <otawa/ets/FlowFactReader.h>
#include <elm/debug.h>

#define FFR_TRACE TRACE
//#define FFR_TRACE
#define FFR_OUT(txt) txt
//#define FFR_OUT(txt)

using namespace elm::io;

namespace otawa { namespace ets {
	
/**
 * @class FlowFactReader
 * This class is used for finding in a file the number of iteration of each loop
 */
	

/**
 * @fn FlowFactReader::FlowFactReader(HashTable<String, int> *hash_table);
 * 
 */	
	FlowFactReader::FlowFactReader(genstruct::HashTable<String, int> *hash_table){
		loop_table = hash_table;
	}
				

/**
 * See @ref ASTProcessor::processAST().
 */		
	void FlowFactReader::processAST(AST *ast){
		if (loop_table->isEmpty())
			FFR_OUT(cout << "|| il n'y a pas de boucle.\n");
		else{
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
					val = loop_table->get(ast->toWhile()->condition()->toBlock()->first()->get<String>(File::ID_Label, "problem! "),-1);
					FFR_OUT(cout << "|| " << ast->toWhile()->condition()->toBlock()->first()->get<String>(File::ID_Label, "problem! ") << " a pour nb d'iter : " << val << '\n');
			 		ast->toWhile()->set<int>(ETS::ID_LOOP_COUNT,val);
			 		processAST(ast->toWhile()->condition());
			 		processAST(ast->toWhile()->body());
			 		break;
				case AST_DoWhile:
					val = loop_table->get(ast->toDoWhile()->condition()->toBlock()->first()->get<String>(File::ID_Label, "problem! "),-1);
					FFR_OUT(cout << "|| " << ast->toDoWhile()->condition()->toBlock()->first()->get<String>(File::ID_Label, "problem! ") << " a pour nb d'iter : " << val << '\n');
					ast->toDoWhile()->set<int>(ETS::ID_LOOP_COUNT,val);
					processAST(ast->toDoWhile()->condition());
					processAST(ast->toDoWhile()->body());
					break;
				case AST_For:
					val = loop_table->get(ast->toFor()->condition()->toBlock()->first()->get<String>(File::ID_Label, "problem! "),-1);
					FFR_OUT(cout << "|| " << ast->toFor()->condition()->toBlock()->first()->get<String>(File::ID_Label, "problem! ") << " a pour nb d'iter : " << val << '\n');
					ast->toFor()->set<int>(ETS::ID_LOOP_COUNT,val);
					processAST(ast->toFor()->initialization());
					processAST(ast->toFor()->condition());
					processAST(ast->toFor()->body());
					processAST(ast->toFor()->incrementation());
					break;
				default:
					;
			}
		}
	}

} } //otawa::ets	
