
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


#include <otawa/cache/cat2/MUSTProblem.h>
#include <otawa/cache/FirstLastBuilder.h>


using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;



/**
 * @class MUSTProblem
 * 
 * Problem for computing the cache MUST ACS.
 * This implements Ferdinand's Must analysis.
 * 
 */

namespace otawa {

	
MUSTProblem::MUSTProblem(const int _size, LBlockSet *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A) 
:	lbset(_lbset),
	fw(_fw),
	line(lbset->line()),
	cache(_cache), 
	bot(_size, _A),
	ent(_size, _A),
	callstate(_size, _A)
{
		ent.empty();	
}
	
MUSTProblem::~MUSTProblem() {
	
}
const MUSTProblem::Domain& MUSTProblem::bottom(void) const {
		return bot;
}
const MUSTProblem::Domain& MUSTProblem::entry(void) const {
		return ent;
}

void MUSTProblem::update(Domain& out, const Domain& in, BasicBlock* bb) {
	assign(out, in);
	LBlock *lblock = LAST_LBLOCK(bb)[line];
    if (lblock != NULL)
    	out.inject(lblock->cacheblock());		
}
	
elm::io::Output& operator<<(elm::io::Output& output, const MUSTProblem::Domain& dom) {
	dom.print(output);
	return output;
}

}
