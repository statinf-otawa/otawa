/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/FlowFactReader.h -- FlowFactReader class interface.
 */
 
#ifndef OTAWA_ETS_FLOWFACTREADER_H
#define OTAWA_ETS_FLOWFACTREADER_H

#include <otawa/proc/ASTProcessor.h>
#include <elm/genstruct/HashTable.h>
#include <elm/string/String.h>

namespace otawa { namespace ets {

// FlowFactReader class	
class FlowFactReader: public ASTProcessor  {
	public :
		genstruct::HashTable<String, int> *loop_table ;
		ASTInfo *ast_info;
		
		inline FlowFactReader(genstruct::HashTable<String, int> *hash_table, ASTInfo *info);
		
		// ASTProcessor overload
		void processAST(AST *ast);
};


//inlines
inline FlowFactReader::FlowFactReader(genstruct::HashTable<String, int> *hash_table, ASTInfo *info){
	ast_info = info;
	loop_table = hash_table;
}

} } // otawa::ets

#endif // OTAWA_ETS_FLOWFACTREADER_H

