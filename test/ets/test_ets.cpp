/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test/ets/test_ets.cpp -- test for ETS feature.
 */

#include <elm/debug.h>
#include <otawa/ets.h>
#include <otawa/ast.h>
#include <elm/io/InStream.h>
#include <elm/genstruct/HashTable.h>

//#define PATH	"/home/casse/Benchs/standard/insertsort/insertsort"
//#define PATH	"/home/casse/Benchs/standard/bs/bs"
//#define PATH	"/home/casse/Benchs/tests/simple/simple"
//#define PATH	"/home/casse/Benchs/standard/fft1/fft1"
//#define PATH	"/home/casse/Benchs/standard/fibcall/fibcall"
//#define PATH	"/home/casse/Benchs/standard/lms/lms"
//#define PATH	"/home/casse/Benchs/standard/fft1k/fft1k"
#define PATH	"/home/casse/Benchs/standard/qurt/qurt"
//#define PATH	"/home/casse/Benchs/standard/minver/minver"
//#define PATH	"/home/casse/Benchs/standard/ludcmp/ludcmp"
//#define PATH	"/home/casse/Benchs/standard/crc/crc"
//#define PATH	"/home/casse/Benchs/standard/fir/fir"
//#define PATH	"/home/casse/Benchs/standard/jfdctint/jfdctint"
//#define PATH	"/home/casse/Benchs/standard/matmul/matmul"
//#define PATH	"/home/casse/Benchs/standard/select/select"

//#define TEST_OUT(txt) txt
#define TEST_OUT(txt)


using namespace otawa;
using namespace elm::io;
using namespace otawa::ets;


/*
 * You have to write in files to use this test:
 * 	-#FlowFactReader#: the number of iteration of each loop equated to its label (separated with 'space' or 'tabulate').
 * 	-#FunReader#: the WCET of remote functions (not in a source file) equated to its name (separated with 'space' or 'tabulate').
 */

void reader (String file_name,genstruct::HashTable<String, int> *hash_table);
int stringToInt(String str);
void outputAST(AST *ast, int ind);
void outputSeq(AST *ast, int ind);

int main(int argc, char **argv) {

	Manager manager;
	PropList props;
	FrameWork *fw;
	genstruct::HashTable<String, int> *table_for_loop=new genstruct::HashTable<String, int> ;
	genstruct::HashTable<String, int> *table_for_fun=new genstruct::HashTable<String, int> ;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Heptane_PowerPC);
	try { 
		fw=manager.load(PATH, props);
		
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
		
		// compute each function
		TEST_OUT(cout << ">>Setting assignment for fun\n");
		reader("#FunReader#",table_for_fun);
		FunReader fr(table_for_fun, info);
		fr.processAST(ast);
		TEST_OUT(cout << ">>OK for setting assignment for fun\n");
		
		// Compute each AB times
		TEST_OUT(cout << ">>Timing the AB\n");
		TrivialAstBlockTime tabt(5, info);
		tabt.processAST(ast);
		TEST_OUT(cout << ">>OK for Timing the AB\n");
		
		// assignment for each loop
		TEST_OUT(cout << ">>Setting assignment for loop\n");
		reader("#FlowFactReader#",table_for_loop);
		FlowFactReader ffr(table_for_loop, info);
		ffr.processAST(ast);
		TEST_OUT(cout << ">>OK for setting assignment for loop\n");
		
		// compute wcet
		TEST_OUT(cout << ">>Computing the AST\n");
		WCETComputation wcomp(info);
		wcomp.processAST(ast);
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


/*
 * read in a file and put in a hashtable each key with its value.
 * @param file_name		File to read.
 * @param hash_table	HashTable to complete.
 */
void reader(String file_name, genstruct::HashTable<String, int> *hash_table){
	InFileStream f(file_name.toCString());
	if(!f.isReady()){
		TRACE; 
		throw IOException(String("Cannot open the file : "+ file_name));
	}
	else{
		int buf = f.read();
		if (buf == elm::io::InStream::ENDED)
			TEST_OUT(cout << file_name << " est vide.\n");
		else{
			while ((buf!=elm::io::InStream::ENDED)&&(buf!=elm::io::InStream::FAILED)){ //eof
				StringBuffer line_buf;
				while ((buf != '\n')&&(buf !=elm::io::InStream::FAILED)&&(buf!=elm::io::InStream::ENDED)){ //'\n'
					line_buf.put(buf);
					buf = f.read();
				}
				if (buf == elm::io::InStream::FAILED){
					TRACE;
					throw io::IOException(String("Probleme du matériel !"));
				}
				String line=line_buf.toString();
				int i=0;
				while ((i<line.length())&&(line.charAt(i)!=' ')&&(line.charAt(i)!='\t')){
					i++;
				}
				if (i==line.length()){
					TRACE;
					throw io::IOException(String("une ligne ne respecte pas la norme \"key [	] value\" !"));
				}
				String key=line.substring(0,i);
				while ((i<line.length())&&((line.charAt(i)==' ')||(line.charAt(i)=='\t')))
					i++;
				String value = line.substring(i,line.length()-i);
				int int_value=stringToInt(value);
				hash_table->put(key,int_value);
				buf = f.read();
			}
			if (buf == elm::io::InStream::FAILED){
				TRACE;
				throw io::IOException(String("Probleme du matériel !"));
			}
		}
		f.close();
	}
}


/*
 * Convert string to int, if each chararacter is a number,
 * else thrown IOException.
 * @param str	String to convert.
 */
	int stringToInt(String str){
		char c;
		int r = 0;
  		for (int i = 0; i < str.length(); i++) {
  			c=str.charAt(i);
  			if ((c<'0')||(c>'9')){
    			TRACE;
      			throw io::IOException (String("Pas un nombre !"));
    		}
    		r = 10 * r + c - '0';
  		}
  		return r;
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
		cout << "SEQUENCE " << ast->toSeq()->use<int>(ETS::ID_WCET)<<'\n';
		outputSeq(ast, ind + 1);
		break;
	case AST_If:
		cout << "IF "<<ast->toIf()->use<int>(ETS::ID_WCET)<<'\n';
		outputAST(ast->toIf()->condition(), ind);
		outputAST(ast->toIf()->thenPart(), ind);
		outputAST(ast->toIf()->elsePart(), ind);
		break;
	case AST_While:
		cout << "WHILE "<<"("<< ast->toWhile()->use<int>(ETS::ID_LOOP_COUNT)<<" iterations)"<<'\n';
		outputAST(ast->toWhile()->condition(), ind);
		outputAST(ast->toWhile()->body(), ind);
		break;
	case AST_DoWhile:
		cout << "DOWHILE "<<"("<< ast->toDoWhile()->use<int>(ETS::ID_LOOP_COUNT)<<" iterations)"<<'\n';
		outputAST(ast->toDoWhile()->body(), ind);
		outputAST(ast->toDoWhile()->condition(), ind);
		break;
	case AST_For:
		cout << "FOR "<<"("<< ast->toFor()->use<int>(ETS::ID_LOOP_COUNT)<<" iterations)"<<'\n';
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


	
