/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/FunReader.h -- FunReader class interface.
 */
 
#ifndef OTAWA_ETS_FUNREADER_H
#define OTAWA_ETS_FUNREADER_H

#include <otawa/proc/ASTProcessor.h>
#include <elm/string/String.h>
#include <elm/genstruct/HashTable.h>

namespace otawa { namespace ets {
	
class FunReader: public ASTProcessor  {
	public :
		genstruct::HashTable<String, int> *fun_table ;
		ASTInfo *ast_info;
		
		inline FunReader(genstruct::HashTable<String, int> *hash_table_fun, ASTInfo *info);
		
		void processAST(AST *ast);
};

//inlines
inline FunReader::FunReader(genstruct::HashTable<String, int> *hash_table, ASTInfo *info){
	ast_info=info;
	fun_table=hash_table;
}

} } // otawa::ets

#endif // OTAWA_ETS_FUNREADER_H
