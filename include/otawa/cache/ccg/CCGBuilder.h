#ifndef _CCGBUILDER_H_
#define _CCGBUILDER_H_

#include <assert.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prop/Identifier.h>
#include <string>
using std::string;

namespace otawa {
class LBlockSet;
class CFG;
class LBlock;
class CCGBuilder: public CFGProcessor {
	FrameWork *fw;
	static Identifier ID_In;
	static Identifier ID_Out;	
public:
	inline CCGBuilder(FrameWork *framework);
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg );
	};
inline CCGBuilder::CCGBuilder(FrameWork *framework)
 : fw(framework) {
		assert(fw);
}

}	// otawa

#endif //_CCGBUILDER_H_
