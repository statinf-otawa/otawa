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
 * Problem for computing the MUST and PERS ACS together (since the optimized PERS take into account the MUST).
 * 
 * MUST ACS information:
 * - The ACS lists the ID of each cache block with MUST be in the cache, with its oldest possible age
 *   (noted mustACS[cacheblockID] = age). Age is between [0..A-1],age ==  represents an absent block
 * 
 * MUST ACS computation:
 * - Initialization to Empty ACS (mustACS[*] == -1)
 * - The Update function injects the newly accessed blocks in the ACS:
 *     mustACS2=update(mustACS,newBlock)
 * 	   In all cases, mustACS2[newBlock] = 0. Furthermore,
 *     if the accessed block was not in the ACS:
 *       foreach cb in mustACS, mustACS2[cb] = mustACS[cb] (+) 1
 *     otherwise:
 *       foreach cb in mustACS older or equal to newBlock: mustACS2[cb]=mustACS[cb]
 * 		 foreach cb in mustACS younger than newBlock: mustACS2[cb] = mustACS[cb] (+) 1
 *  where: a (+) b == if (a+b < A) then (a+b) else -1
 * This implements Ferdinand's Persistence analysis.
 * 
 * - Join: join(acs1,acs2) is the intersection of ACS, taking the older age when
 * a block is in both ACS.
 * 
 * PERS ACS information:
 * - The ACS lists the ID of each cache block with MAY be in the cache, with its oldest possible age
 *   (noted mustACS[cacheblockID] = age). Age is between [0..A],age ==  represents an absent block
 *   Virtual age A represents a block which have been wiped from the cache.
 * 
 * MUST ACS computation:
 * - Initialization to Empty ACS (persACS[*] == -1)
 * - The Update function injects the newly accessed blocks in the ACS:
 *     persACS2=update(persACS,newBlock)
 * 	   In all cases, persACS2[newBlock] = 0. Furthermore,
 *     if the accessed block was not in the corresponding_MUST_ ACS:
 *       foreach cb in persACS, persACS2[cb] = persACS[cb] (+) 1
 *     otherwise:
 *       foreach cb in persACS older or equal to newBlock in the MUST: persACS2[cb]=persACS[cb]
 * 		 foreach cb in persACS younger than newBlock in the MUST: persACS2[cb] = persACS[cb] (+) 1
 *  where: a (+) b == if (a+b <= A) then (a+b) else -1
 * 
 * - Join: join(acs1,acs2) is the union of ACS, taking the older age when
 * a block is in both ACS.
 * 
 * Multi-Level Persistence Computation:
 * For the Multi-Level persistence, the Multi-Level Pers ACS, mlpersACS, is a 
 * stack (Vector<>) of standard pers ACS. 
 * Each item of the stack correspond to a loop: 
 * in other words, an ACS computed before a basic block BB, which is in N levels
 * of nested loops, will be a stack of length N, with each item containing
 * a standard pers ACS taking into account only block replacement occuring
 * in the N-th loop (starting from outer-most loop).
 * 
 * To implement this, the Update() is modified to push
 * an empty pers ACS into the stack when we enter a loop, and pop it when we exit the loop.
 * The Join() is also modified to Join() corresponding pers ACS in the stack.
 * 
 */

namespace otawa {

	
MUSTPERS::MUSTPERS(const int _size, LBlockSet *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A) 
:	mustProb(_size, _lbset, _fw, _cache, _A),
	persProb(_size, _lbset, _fw, _cache, _A),
	bot(_size, _A),
	ent(_size, _A),
	line(_lbset->line())
{
		
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
