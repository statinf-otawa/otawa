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
		
		FlowFactReader(genstruct::HashTable<String, int> *hash_table);
		
		// ASTProcessor overload
		void processAST(AST *ast);
};

} } // otawa::ets

#endif // OTAWA_ETS_FLOWFACTREADER_H

