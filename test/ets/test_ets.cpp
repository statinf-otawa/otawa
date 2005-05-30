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

//#define PATH	"/home/casse/Benchs/tests/simple/simple"
//#define PATH	"/home/casse/Benchs/standard/bs/bs"
//#define PATH	"/home/casse/Benchs/standard/fft1/fft1"
//#define PATH	"/home/casse/Benchs/standard/fibcall/fibcall"
//#define PATH	"/home/casse/Benchs/standard/insertsort/insertsort"
//#define PATH	"/home/casse/Benchs/standard/lms/lms"
//#define PATH	"/home/casse/Benchs/standard/select/select"
//#define PATH	"/home/casse/Benchs/standard/fft1k/fft1k"
//#define PATH	"/home/casse/Benchs/standard/matmul/matmul"
//#define PATH	"/home/casse/Benchs/standard/qurt/qurt"
//#define PATH	"/home/casse/Benchs/standard/minver/minver"
//#define PATH	"/home/casse/Benchs/standard/ludcmp/ludcmp"

//#define PATH	"/home/casse/Benchs/standard/crc/crc"
#define PATH	"/home/casse/Benchs/standard/fir/fir"
//#define PATH	"/home/casse/Benchs/standard/jfdctint/jfdctint"


using namespace otawa;
using namespace elm::io;
using namespace otawa::ets;

void reader (String file_name,genstruct::HashTable<String, int> *hash_table);
int stringToInt(String str);

int main(int argc, char **argv) {

	Manager manager;
	PropList props;
	FrameWork *fw;
	genstruct::HashTable<String, int> *hash_table=new genstruct::HashTable<String, int> ;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Heptane_PowerPC);
	try { 
		fw=manager.load(PATH, props);
		
		// Functions
		ASTInfo *info = fw->getASTInfo();
		/*for(Iterator<FunAST *> fun(info->functions()); fun; fun++)
			cout << "-> " << fun->name() << '\n';*/
		
		// Find main AST
		cout << ">>Looking for the main AST\n";
		Option< FunAST *> result = info->map().get("main");
		if(!result) {
			TRACE;
			throw IOException(String("Cannot find main !"));
		}
		AST *ast = (*result)->ast();
		ast->set<int>(ETS::ID_WCET,-1);
		
		// Compute each AB times
		cout << ">>Timing the AB\n";
		TrivialAstBlockTime ab(5);
		ab.processAST(ast);
		cout << ">>OK for Timing the AB\n";
		
		// assignment for each loop
		cout << ">>Setting assignment for loop\n";
		reader("#FlowFactReader#",hash_table);
		FlowFactReader ffr(hash_table);
		ffr.processAST(ast);
		cout << ">>OK for setting assignment for loop\n";
		
		// compute each function
		
		// compute wcet
		cout << ">>Computing the AST\n";
		WCETComputation wcomp;
		wcomp.processAST(ast);
		cout << ">>OK for Computing the AST\n";
		
		// Display the result
		cout << ">>SUCCESS\n|| WCET = " << ast->use<int>(ETS::ID_WCET) << '\n';
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
	}
	catch(IOException e){
		cout << "ERROR : " << e.message() << '\n';
	}
	return 0;
}

void reader(String file_name, genstruct::HashTable<String, int> *hash_table){
	InFileStream f(file_name.toCString());
	if(!f.isReady()){
		TRACE; 
		throw IOException(String("Cannot open the file : "+ file_name));
	}
	else{
		int buf = f.read();
		if (buf == elm::io::InStream::ENDED)
			cout << file_name << " est vide.\n";
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
				//cout << "key : !!" << key << "!!\n";
				while ((i<line.length())&&((line.charAt(i)==' ')||(line.charAt(i)=='\t')))
					i++;
				String value = line.substring(i,line.length()-i);
				//cout << "value : !!" << value <<"!!\n";
				int int_value=stringToInt(value);
				//cout << "int_value :" << int_value << '\n';
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

