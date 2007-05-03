/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/FlowFactLoader.h -- FlowFactLoader class interface.
 */
 
#ifndef OTAWA_ETS_FLOWFACTLOADER_H
#define OTAWA_ETS_FLOWFACTLOADER_H

#include <otawa/proc/ASTProcessor.h>
#include <otawa/util/FlowFactLoader.h>
#include <elm/genstruct/HashTable.h>

namespace otawa { namespace ets {

// FlowFactLoader class	
class FlowFactLoader: public ASTProcessor, private otawa::FlowFactLoader {
	public :
		genstruct::HashTable<address_t, int> loop_table;
		
		inline FlowFactLoader(WorkSpace *fw);
		
		// ASTProcessor overload
		void processAST(WorkSpace *fw, AST *ast);
		
	protected:
		// FlowFactLoader overload
		virtual void onError(const char *fmt, ...);
		virtual void onWarning(const char *fmt, ...);
		virtual void onLoop(address_t addr, int count);
};

//inlines
inline FlowFactLoader::FlowFactLoader(WorkSpace *fw) {
	run(fw);
}

} } // otawa::ets

#endif // OTAWA_ETS_FLOWFACTLOADER_H

