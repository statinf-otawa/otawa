/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test/ets/test_ets.cpp -- test for ETS feature.
 */

#include <elm/debug.h>
#include <otawa/ets.h>
#include <otawa/ast.h>

//#define TEST_OUT(txt) txt
#define TEST_OUT(txt)

using namespace otawa;
using namespace elm::io;
using namespace otawa::ets;

void outputAST(AST *ast, int ind);
void outputSeq(AST *ast, int ind);

int main(int argc, char **argv) {
	Manager manager;
	PropList props;
	FrameWork *fw;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Heptane_PowerPC);
	try { 
		fw=manager.load(argv[1], props);
		
		// Functions
		ASTInfo *info = fw->getASTInfo();
		
		// Find main AST
		TEST_OUT(cout << ">>Looking for the main AST\n");
		Option< FunAST *> result = info->map().get("main");
		if(!result) {
			TRACE;
			throw IOException(String("Cannot find main !"));
		}
		AST *ast = (*result)->ast();
		ast->set<int>(ETS::ID_WCET,-1);
		
		// Compute each AB times
		TEST_OUT(cout << ">>Timing the AB\n");
		TrivialAstBlockTime tabt(5);
		tabt.processAST(fw, ast);
		TEST_OUT(cout << ">>OK for Timing the AB\n");
		
		// assignment for each loop
		TEST_OUT(cout << ">>Setting assignment for loop\n");
		ets::FlowFactLoader ffl(fw);
		ffl.processAST(fw, ast);
		TEST_OUT(cout << ">>OK for setting assignment for loop\n");
		
		// compute wcet
		TEST_OUT(cout << ">>Computing the AST\n");
		WCETComputation wcomp;
		wcomp.processAST(fw, ast);
		TEST_OUT(cout << ">>OK for Computing the AST\n");
		
		//Display the AST and each WCET
		for(Iterator<FunAST *> fun(info->functions()); fun; fun++){
			cout << "-> " << fun->name()<<'\n';
			Option< FunAST *> fun_res = info->map().get(fun->name());
			if (fun_res){
				outputAST((*fun_res)->ast(),0);
			}
			else {
				TRACE;
				throw IOException(String("\n Cannot find "+fun->name()+" !"));
			}
			cout << '\n';
		}
		cout << ">>WCET = " << ast->use<int>(ETS::ID_WCET) << '\n';
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
	}
	catch(IOException e){
		cout << "ERROR : " << e.message() << '\n';
	}
	return 0;
}

/**
 * Output the given AST to the given output stream.
 * @param out	Output channel.
 * @param ast	AST to output.
 * @param ind	Current indentation.
 */
void outputAST(AST *ast, int ind) {
	
	// Display indentation
	for(int i = 0; i < ind; i++)
		cout << "  ";
	ind++;
	
	// Display the AST
	switch(ast->kind()) {
	case AST_Undef:
		cout << "UNDEFINED\n";
		break;
	case AST_Nop:
		cout << "NOP\n";
		break;
	case AST_Block:
		cout << "BLOCK : " << ast->toBlock()->use<int>(ETS::ID_WCET)<< '\n';
		break;
	case AST_Call:
		cout << "CALL : " << ast->toCall()->use<int>(ETS::ID_WCET);
		if(ast->toCall()->function()->name())
			cout << " (" << ast->toCall()->function()->name() << ')';
		cout << '\n';
		break;
	case AST_Seq:
		cout << "SEQUENCE : " << ast->toSeq()->use<int>(ETS::ID_WCET)<<'\n';
		outputSeq(ast, ind + 1);
		break;
	case AST_If:
		cout << "IF : "<<ast->toIf()->use<int>(ETS::ID_WCET)<<'\n';
		outputAST(ast->toIf()->condition(), ind);
		outputAST(ast->toIf()->thenPart(), ind);
		outputAST(ast->toIf()->elsePart(), ind);
		break;
	case AST_While:
		cout << "WHILE "<<"("<< ast->toWhile()->use<int>(ETS::ID_LOOP_COUNT)<<" iterations) : "<< ast->toWhile()->use<int>(ETS::ID_WCET)<<'\n';
		outputAST(ast->toWhile()->condition(), ind);
		outputAST(ast->toWhile()->body(), ind);
		break;
	case AST_DoWhile:
		cout << "DOWHILE "<<"("<< ast->toDoWhile()->use<int>(ETS::ID_LOOP_COUNT)<<" iterations) : "<< ast->toDoWhile()->use<int>(ETS::ID_WCET)<<'\n';
		outputAST(ast->toDoWhile()->body(), ind);
		outputAST(ast->toDoWhile()->condition(), ind);
		break;
	case AST_For:
		cout << "FOR "<<"("<< ast->toFor()->use<int>(ETS::ID_LOOP_COUNT)<<" iterations) : "<< ast->toFor()->use<int>(ETS::ID_WCET)<<'\n';
		outputAST(ast->toFor()->initialization(), ind);
		outputAST(ast->toFor()->condition(), ind);
		outputAST(ast->toFor()->incrementation(), ind);
		outputAST(ast->toFor()->body(), ind);
		break;
	default:
		assert(0);
	}
}

/**
 * Output the given sequence AST.
 * @param out	Output channel.
 * @param ast	AST to output.
 * @param ind	Current indentation.
 */
void outputSeq(AST *ast, int ind) {
	
	// Sequence AST ?
	if(ast->kind() != AST_Seq)
		outputAST(ast, ind);
	
	// Process children else
	else {
		outputSeq(ast->toSeq()->child1(), ind);
		outputSeq(ast->toSeq()->child2(), ind);
	}
}


	
