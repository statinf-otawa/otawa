/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test/ets/test_ets.cpp -- test for ETS feature.
 */

#include <elm/debug.h>
#include <otawa/ets.h>
#include <otawa/ast.h>
#include <otawa/hard/Cache.h>
#include <otawa/ast/ASTLoader.h>
#include <otawa/proc/ProcessorException.h>

//#define TEST_TRACE TRACE	//with 
//#define TEST_OUT(txt) txt	//	degugging.
#define TEST_OUT(txt)		//without 
#define TEST_TRACE			//	debuging.

#define WITHOUT_CACHE(txt) txt 	//with cache managing. 
//#define WITHOUT_CACHE(txt)		//without cache managing.

using namespace otawa;
using namespace elm::io;
using namespace otawa::ets;
using namespace otawa::hard;

void outputAST(AST *ast, int ind);
void outputSeq(AST *ast, int ind);

int main(int argc, char **argv) {
	Cache::info_t info;
 	info.block_bits = 3;  // 2^3 octets par bloc
 	info.line_bits = 2;   // 2^3 lignes
 	info.set_bits = 0;    // 2^0 �l�ment par ensemble 
 	info.replace = Cache::NONE;
 	info.write = Cache::WRITE_THROUGH;
 	info.access_time = 0;
 	info.miss_penalty = 10;
 	info.allocate = false;
 	Cache *level1 = new Cache(info);
 	CacheConfiguration *caches = new CacheConfiguration(level1, 0);
 
 	Manager manager;
	PropList props;
	FrameWork *fw;
	CACHE_CONFIG(props) = caches;
//	LOADER(props) = &Loader::LOADER_Gliss_PowerPC;
	
	try { 
		fw = manager.load(argv[1], props);
		
		// Functions
		ASTLoader loader;
		loader.processFrameWork(fw);
		ASTInfo *info = fw->getASTInfo();
		
		// Find main AST
		TEST_OUT(cout << ">>Looking for the main AST\n");
		Option< FunAST *> result = info->map().get("main");
		if(!result) {
			TRACE;
			throw IOException("Cannot find main !");
		}
		AST *ast = (*result)->ast();
		ast->set<int>(ETS::ID_WCET,-1);
		
		// Compute each AB times
		TEST_OUT(cout << ">>Timing the AB\n");
		TrivialAstBlockTime tabt(5);
		tabt.processAST(fw, ast);
		TEST_OUT(cout << ">>OK for Timing the AB\n");
		
		// assignment for each loop
		TEST_OUT(cout << ">>Setting flow fact\n");
		ets::FlowFactLoader ffl(fw);
		ffl.processAST(fw, ast);
		TEST_OUT(cout << ">>OK for setting flow fact\n");
		
		// compute wcet
		TEST_OUT(cout << ">>Computing the WCET\n");
		WCETComputation wcomp;
		wcomp.processAST(fw, ast);
		TEST_OUT(cout << ">>OK for Computing the WCET\n");
		
		// simulate cache
		WITHOUT_CACHE(TEST_OUT(cout << ">>Simulating the cache\n"));
		WITHOUT_CACHE(ACSComputation ac(fw));
		WITHOUT_CACHE(ac.processAST(fw, ast));
		WITHOUT_CACHE(TEST_OUT(cout << ">>OK for Simulating the cache\n"));
		
		// compute first miss
		WITHOUT_CACHE(TEST_OUT(cout << ">>Computing FIRST MISS\n"));
		WITHOUT_CACHE(CacheFirstMissComputation cfmc);
		WITHOUT_CACHE(cfmc.processAST(fw, ast));
		WITHOUT_CACHE(TEST_OUT(cout << ">>OK for Computing FIRST MISS\n"));
		
		// compute hit
		WITHOUT_CACHE(TEST_OUT(cout << ">>Computing HIT\n"));
		WITHOUT_CACHE(CacheHitComputation chc);
		WITHOUT_CACHE(chc.processAST(fw, ast));
		WITHOUT_CACHE(TEST_OUT(cout << ">>OK for Computing HIT\n"));
		
		// compute miss
		WITHOUT_CACHE(TEST_OUT(cout << ">>Computing MISS\n"));
		WITHOUT_CACHE(CacheMissComputation cmc);
		WITHOUT_CACHE(cmc.processAST(fw, ast));
		WITHOUT_CACHE(TEST_OUT(cout << ">>OK for Computing MISS\n"));
		
		//Display the AST (WCET, HIT, MISS)
		for(Iterator<FunAST *> fun(info->functions()); fun; fun++){
			cout << "-> " << fun->name()<<'\n';
			Option< FunAST *> fun_res = info->map().get(fun->name());
			if (fun_res){
				outputAST((*fun_res)->ast(),0);
			}
			else {
				TRACE;
				throw IOException("\n Cannot find %s !", &fun->name());
			}
			cout << '\n';
		}
		cout << ">>WCET = " << ast->get<int>(ETS::ID_WCET, -6) << '\n';
		WITHOUT_CACHE(cout << ">>HITS = " << ast->get<int>(ETS::ID_HITS, -6) << '\n');
		WITHOUT_CACHE(cout << ">>MISSES = " << ast->get<int>(ETS::ID_MISSES, -6) << '\n');
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
	}
	catch(IOException e){
		cout << "ERROR : " << e.message() << '\n';
	}
	catch(ProcessorException e){
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
		cout 	<< "BLOCK : ("
				<< LABEL(ast->toBlock()->first())
				<<") [" 
				<< ast->toBlock()->get<int>(ETS::ID_WCET, 0)
	WITHOUT_CACHE(	<< ", "
				<< ast->toBlock()->get<int>(ETS::ID_HITS, 0)
				<< ", "
				<< ast->toBlock()->get<int>(ETS::ID_MISSES, 0)
				<< ", "
				<< ast->toBlock()->get<int>(ETS::ID_FIRST_MISSES, 0))
				<<"]\n";
		break;
	case AST_Call:
		cout 	<< "CALL : [" 
				<< ast->toCall()->use<int>(ETS::ID_WCET)
	WITHOUT_CACHE(	<< ", "
				<< ast->toCall()->use<int>(ETS::ID_HITS)
				<< ", "
				<< ast->toCall()->use<int>(ETS::ID_MISSES)
				<< ", "
				<< ast->toCall()->use<int>(ETS::ID_FIRST_MISSES));
		if(ast->toCall()->function()->name())
			cout << "] (" << ast->toCall()->function()->name() << ')';
		cout << '\n';
		break;
	case AST_Seq:
		cout 	<< "SEQUENCE : [" 
				<< ast->toSeq()->use<int>(ETS::ID_WCET)
	WITHOUT_CACHE(	<< ", "
				<< ast->toSeq()->use<int>(ETS::ID_HITS)
				<< ", "
				<< ast->toSeq()->use<int>(ETS::ID_MISSES)
				<< ", "
				<< ast->toSeq()->use<int>(ETS::ID_FIRST_MISSES))
				<<"]\n";
		outputSeq(ast, ind + 1);
		break;
	case AST_If:
		cout 	<< "IF : ["
				<<ast->toIf()->use<int>(ETS::ID_WCET)
	WITHOUT_CACHE(	<< ", "
				<< ast->toIf()->use<int>(ETS::ID_HITS)
				<< ", "
				<< ast->toIf()->use<int>(ETS::ID_MISSES)
				<< ", "
				<< ast->toIf()->use<int>(ETS::ID_FIRST_MISSES))
				<<"]\n";
		outputAST(ast->toIf()->condition(), ind);
		outputAST(ast->toIf()->thenPart(), ind);
		outputAST(ast->toIf()->elsePart(), ind);
		break;
	case AST_While:
		cout 	<< "WHILE "
				<<"("
				<< ast->toWhile()->use<int>(ETS::ID_LOOP_COUNT)
				<<" iterations) : ["
				<< ast->toWhile()->use<int>(ETS::ID_WCET)
	WITHOUT_CACHE(	<< ", "
				<< ast->toWhile()->use<int>(ETS::ID_HITS)
				<< ", "
				<< ast->toWhile()->use<int>(ETS::ID_MISSES)
				<< ", "
				<< ast->toWhile()->use<int>(ETS::ID_FIRST_MISSES))
				<<"]\n";
		outputAST(ast->toWhile()->condition(), ind);
		outputAST(ast->toWhile()->body(), ind);
		break;
	case AST_DoWhile:
		cout 	<< "DOWHILE "
				<<"("
				<< ast->toDoWhile()->use<int>(ETS::ID_LOOP_COUNT)
				<<" iterations) : ["
				<< ast->toDoWhile()->use<int>(ETS::ID_WCET)
	WITHOUT_CACHE(	<< ", "
				<< ast->toDoWhile()->use<int>(ETS::ID_HITS)
				<< ", "
				<< ast->toDoWhile()->use<int>(ETS::ID_MISSES)
				<< ", "
				<< ast->toDoWhile()->use<int>(ETS::ID_FIRST_MISSES))
				<<"]\n";
		outputAST(ast->toDoWhile()->body(), ind);
		outputAST(ast->toDoWhile()->condition(), ind);
		break;
	case AST_For:
		cout 	<< "FOR "
				<<"("
				<< ast->toFor()->use<int>(ETS::ID_LOOP_COUNT)
				<<" iterations) : ["
				<< ast->toFor()->use<int>(ETS::ID_WCET)
	WITHOUT_CACHE(	<< ", "
				<< ast->toFor()->use<int>(ETS::ID_HITS)
				<< ", "
				<< ast->toFor()->use<int>(ETS::ID_MISSES)
				<< ", "
				<< ast->toFor()->use<int>(ETS::ID_FIRST_MISSES))
				<<"]\n";
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


	
