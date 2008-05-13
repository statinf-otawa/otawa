
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
 * MAY ACS information:
 * - The ACS lists the ID of each cache block with MAY be in the cache, with its oldest possible age
 *   (noted mayACS[cacheblockID] = age). Age is between [0..A-1],age ==  represents an absent block
 *
 * MAY ACS computation:
 * - Initialization to Empty ACS (ACS[*] == -1) (XXX FIXME: This is wrong)
 * - The Update function injects the newly accessed blocks in the ACS:
 *     mayACS2=update(mayACS,newBlock)
 * 	   In all cases, mayACS2[newBlock] = 0. Furthermore,
 *     if the accessed block was not in the ACS:
 *       foreach cb in mayACS, mayACS2[cb] = mayACS[cb] (+) 1
 *     otherwise:
 *       foreach cb in mayACS older than newBlock: mayACS2[cb]=mayACS[cb]
 * 		 foreach cb in mayACS younger or equal to newBlock: mayACS2[cb] = mayCS[cb] (+) 1
 *  where: a (+) b == if (a+b < A) then (a+b) else -1
 * 
 * - Join: join(acs1,acs2) is the union of ACS, taking the younger age when
 * a block is in both ACS.
 */

namespace otawa {

	
MAYProblem::MAYProblem(const int _size, LBlockSet *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A) 
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
	
MAYProblem::~MAYProblem() {
	
}
const MAYProblem::Domain& MAYProblem::bottom(void) const {
		return bot;
}
const MAYProblem::Domain& MAYProblem::entry(void) const {
		return ent;
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
