#ifndef _CCGOBJECTFUNCTION_H_
#define _CCGOBJECTFUNCTION_H_


#include <assert.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prop/Identifier.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/hard/Cache.h>


namespace otawa {
class LBlockSet;
class CFG;
class CCGNode;
class CCGObjectFunction: public CFGProcessor {
	FrameWork *fw;
	
	
public:
	inline CCGObjectFunction(FrameWork *framework);
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg );
	
};
inline CCGObjectFunction::CCGObjectFunction(FrameWork *framework)
 : fw(framework) {
		assert(fw);
}

}	// otawa


#endif //_CCGOBJECTFUNCTION_H_
