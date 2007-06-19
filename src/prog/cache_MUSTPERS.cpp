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
#include <otawa/cache/cat2/PERSProblem.h>
#include <otawa/cache/cat2/MUSTPERS.h>
#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/FirstLastBuilder.h>

using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;


/**
 * @class MUSTProblem
 * 
 * Problem for computing the MUST and PERS ACS together.
 * This implements Ferdinand's Persistence analysis.
 */

namespace otawa {

	
MUSTPERS::MUSTPERS(const int _size, LBlockSet *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A) 
	: persProb(_size, _lbset, _fw, _cache, _A), mustProb(_size, _lbset, _fw, _cache, _A), bot(_size, _A), ent(_size, _A), line(_lbset->line()) {
		
		persProb.assign(bot.pers, persProb.bottom());
		mustProb.assign(bot.must, mustProb.bottom());
		
		persProb.assign(ent.pers, persProb.entry());
		mustProb.assign(ent.must, mustProb.entry());
}
	

const MUSTPERS::Domain& MUSTPERS::bottom(void) const {
		return bot;
}
const MUSTPERS::Domain& MUSTPERS::entry(void) const {
		return ent;
}
		
void MUSTPERS::update(Domain& out, const Domain& in, BasicBlock* bb) {
		LBlock *lblock;
				                
	    assign(out, in);
	    lblock = LAST_LBLOCK(bb)[line];
        if (lblock != NULL)
        	out.inject(lblock->cacheblock());

}
	
elm::io::Output& operator<<(elm::io::Output& output, const MUSTPERS::Domain& dom) {	
	dom.print(output);
	return(output);
	
}

}
