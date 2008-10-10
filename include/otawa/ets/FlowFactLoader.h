/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/FlowFactLoader.h -- FlowFactLoader class interface.
 */
 
#ifndef OTAWA_ETS_FLOWFACTLOADER_H
#define OTAWA_ETS_FLOWFACTLOADER_H

#include <otawa/ast/ASTProcessor.h>

namespace otawa { namespace ets {

// FlowFactLoader class	
class FlowFactLoader: public ASTProcessor {
public:
	FlowFactLoader(void);
	void processAST(WorkSpace *fw, AST *ast);
};

} } // otawa::ets

#endif // OTAWA_ETS_FLOWFACTLOADER_H

