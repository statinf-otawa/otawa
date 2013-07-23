/*
 *	ets::FlowFactLoader class implementation
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
#include <otawa/ets/FlowFactLoader.h>
#include <otawa/util/FlowFactLoader.h>

//#define FFL_OUT(txt) txt	//with debuging
#define FFL_OUT(txt)		//without debuging

namespace otawa { namespace ets {
	
/**
 * @class FlowFactLoader
 * This processor allows using extern flow facts in ETS.
 *
 * @p Provided Features
 * @li @ref FLOWFACT_FEATURE
 *
 * @p Required Features
 * @li @ref otawa::FLOW_FACTS_FEATURE
 */


p::declare FlowFactLoader::reg = p::init("otawa::ets::FlowFactLoader", Version(1, 2, 0))
	.base(ASTProcessor::reg)
	.maker<FlowFactLoader>()
	.provide(FLOWFACT_FEATURE)
	.require(otawa::FLOW_FACTS_FEATURE);


/**
 */
FlowFactLoader::FlowFactLoader(p::declare& r): ASTProcessor(r) {
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
			break;
	}
}

} } //otawa::ets	
