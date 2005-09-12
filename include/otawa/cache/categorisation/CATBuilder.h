#ifndef _CATBUILDER_H_
#define _CATBUILDER_H_
#include <assert.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prop/Identifier.h>
#include <string>
using std::string;

namespace otawa {
class LBlockSet;
class CFG;
class LBlock;
class CATBuilder: public CFGProcessor {
	FrameWork *fw;
	static Identifier ID_In;
	static Identifier ID_Out;	
public:
	inline CATBuilder(FrameWork *framework);
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg );
	};
inline CATBuilder::CATBuilder(FrameWork *framework)
 : fw(framework) {
		assert(fw);
}

}	// otawa

#endif //_CATBUILDER_H_
