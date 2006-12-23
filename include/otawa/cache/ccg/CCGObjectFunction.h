#ifndef _CCGOBJECTFUNCTION_H_
#define _CCGOBJECTFUNCTION_H_


#include <assert.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prop/Identifier.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/hard/Cache.h>
#include <otawa/ipet/BasicObjectFunctionBuilder.h>


namespace otawa {
class LBlockSet;
class CFG;
class CCGNode;
class CCGObjectFunction: public Processor {
	FrameWork *fw;
	
	
public:
	inline CCGObjectFunction(FrameWork *framework);
	// CFGProcessor overload
	virtual void processFrameWork(FrameWork *fw );
	
};
inline CCGObjectFunction::CCGObjectFunction(FrameWork *framework)
 : fw(framework) {
		assert(fw);
		require(ipet::OBJECT_FUNCTION_FEATURE);
}

}	// otawa


#endif //_CCGOBJECTFUNCTION_H_
