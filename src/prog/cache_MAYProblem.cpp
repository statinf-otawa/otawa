
#include <stdio.h>
#include <elm/io.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/ilp.h>
#include <otawa/ipet.h>
#include <otawa/cfg/Edge.h>
#include <otawa/util/Dominance.h>
#include <otawa/cfg.h>
#include <otawa/util/LoopInfoBuilder.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>


#include <otawa/cache/cat2/MAYProblem.h>
#include <otawa/cache/FirstLastBuilder.h>


using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;



/**
 * @class MAYProblem
 * 
 * Problem for computing the cache MAY ACS.
 * This implements Ferdinand's MAY analysis.
 * 
 */

namespace otawa {

	
MAYProblem::MAYProblem(const int _size, LBlockSet *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A, bool _unrolling) 
	: callstate(_size, _A), unrolling(_unrolling), line(lbset->line()), cache(_cache), bot(_size, _A), ent(_size, _A), lbset(_lbset),  fw(_fw) {


		ent.empty();
	
}
	
MAYProblem::~MAYProblem() {
	
}
const MAYProblem::Domain& MAYProblem::bottom(void) const {
		return bot;
}
const MAYProblem::Domain& MAYProblem::entry(void) const {
		return ent;
}
	
void MAYProblem::setEntry(const MAYProblem::Domain &entry) {
	assign(ent, entry);
}
	
void MAYProblem::update(Domain& out, const Domain& in, BasicBlock* bb) {
	assign(out, in);
	LBlock *lblock = LAST_LBLOCK(bb)[line];
    if (lblock != NULL)
    	out.inject(lblock->cacheblock());		
}
	
elm::io::Output& operator<<(elm::io::Output& output, const MAYProblem::Domain& dom) {
	dom.print(output);
	return output;
}

}
