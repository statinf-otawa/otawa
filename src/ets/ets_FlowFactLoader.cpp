/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ets_FlowFactLoader.cpp -- FlowFactLoader class implementation.
 */

#include <otawa/ets/ETS.h>
#include <otawa/ast.h>
#include <otawa/ets/FlowFactLoader.h>
#include <otawa/util/FlowFactLoader.h>

//#define FFL_OUT(txt) txt	//with debuging
#define FFL_OUT(txt)		//without debuging

namespace otawa { namespace ets {
	
/**
 * @class FlowFactLoader
 * This processor allows using extern flow facts in ETS. 
 */


/**
 */
FlowFactLoader::FlowFactLoader(void) {
	require(otawa::FLOW_FACTS_FEATURE);
}


/**
 * Use fft_parser for editing flow facts:
 * 	<br>-to ETS::ID_LOOP_COUNT for loop.
 * <br>If the table contains the key put its value,
 * <br>else put -1.
 * @param ws	Container framework.
 * @param ast	AST to process.
 */		
void FlowFactLoader::processAST(WorkSpace *ws, AST *ast){
	//int val;
	switch(ast->kind()) {
		case AST_Seq:
			processAST(ws, ast->toSeq()->child1());
			processAST(ws, ast->toSeq()->child2());
			break;
		case AST_If:
			processAST(ws, ast->toIf()->condition());
			processAST(ws, ast->toIf()->thenPart());
			processAST(ws, ast->toIf()->elsePart());
			break;
		case AST_While: {
				Inst *inst = ast->toWhile()->condition()->first();
				int count = MAX_ITERATION(inst);
				if(count < 0)
					warn(_ << "loop at " << inst->address() << " has no bound");
				else {
					FFL_OUT(cout << "|| " << ast->toWhile()->condition()->first()->get<String>(File::ID_Label, "unknown ")<< " a pour nb d'iter : "<< count << '\n');
					LOOP_COUNT(ast->toWhile()) = count;
				}
				processAST(ws, ast->toWhile()->condition());
				processAST(ws, ast->toWhile()->body());
			}
		 	break;
		case AST_DoWhile: {
				Inst *inst = ast->toDoWhile()->condition()->first();
				int count = MAX_ITERATION(inst);
				if(count < 0)
					warn(_ << "loop at " << inst->address() << " has no bound");
				else {
					FFL_OUT(cout << "|| "<< ast->toDoWhile()->condition()->first()->get<String>(File::ID_Label, "unknown ") << " a pour nb d'iter : "<< count << '\n');
					LOOP_COUNT(ast->toDoWhile()) = count;
				}
				processAST(ws, ast->toDoWhile()->condition());
				processAST(ws, ast->toDoWhile()->body());
			}
			break;
		case AST_For: {
				Inst *inst = ast->toFor()->condition()->first();
				int count = MAX_ITERATION(inst);
				if(count < 0)
					warn(_ << "loop at " << inst->address() << " has no bound");
				else {
					FFL_OUT(cout << "|| " << ast->toFor()->condition()->first()->address()<<" ~ "<<ast->toFor()->condition()->first()->get<String>(File::ID_Label, "unknown ") << " a pour nb d'iter : "<< count << '\n');
					LOOP_COUNT(ast->toFor()) = count;
				}
				processAST(ws, ast->toFor()->initialization());
				processAST(ws, ast->toFor()->condition());
				processAST(ws, ast->toFor()->body());
				processAST(ws, ast->toFor()->incrementation());
			}
			break;
		case AST_Call:{
			ASTInfo *ast_info = ws->getASTInfo();
			Option< FunAST *> fun_res = ast_info->get(ast->toCall()->function()->name());
			if(fun_res) {
				AST *fun_ast = (*fun_res)->ast();
				processAST(ws, fun_ast);
			}
			break;
		}
		default:
			;
	}
}

} } //otawa::ets	
