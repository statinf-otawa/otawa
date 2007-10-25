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
 * @param fw	Container framework.
 * @param ast	AST to process.
 */		
void FlowFactLoader::processAST(WorkSpace *fw, AST *ast){
	int val;
	switch(ast->kind()) {
		case AST_Seq:
			processAST(fw, ast->toSeq()->child1());
			processAST(fw, ast->toSeq()->child2());
			break;
		case AST_If:
			processAST(fw, ast->toIf()->condition());
			processAST(fw, ast->toIf()->thenPart());
			processAST(fw, ast->toIf()->elsePart());
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
				processAST(fw, ast->toWhile()->condition());
				processAST(fw, ast->toWhile()->body());
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
				processAST(fw, ast->toDoWhile()->condition());
				processAST(fw, ast->toDoWhile()->body());
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
				processAST(fw, ast->toFor()->initialization());
				processAST(fw, ast->toFor()->condition());
				processAST(fw, ast->toFor()->body());
				processAST(fw, ast->toFor()->incrementation());
			}
			break;
		case AST_Call:{
			ASTInfo *ast_info = fw->getASTInfo();
			Option< FunAST *> fun_res = ast_info->map().get(ast->toCall()->function()->name());
			if(fun_res) {
				AST *fun_ast = (*fun_res)->ast();
				processAST(fw, fun_ast);
			}
			break;
		}
		default:
			;
	}
}

} } //otawa::ets	
