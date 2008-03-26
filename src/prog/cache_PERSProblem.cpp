#include <stdio.h>
#include <elm/io.h>
#include <elm/genstruct/Vector.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/ilp.h>
#include <otawa/ipet.h>
#include <otawa/util/Dominance.h>
#include <otawa/cfg.h>
#include <otawa/util/LoopInfoBuilder.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>


#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/cat2/PERSProblem.h>
#include <otawa/cache/cat2/MUSTPERS.h>
#include <otawa/cache/FirstLastBuilder.h>


using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;
using namespace elm;

namespace otawa {
	
/**
 * @class PERSProblem
 * 
 * Problem for computing the PERS ACS of L-blocks. 
 * This implements Ferdinand's Persistence analysis.
 */


	
PERSProblem::PERSProblem(const int _size, LBlockSet *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A) 
:	callstate(_size, _A),
	lbset(_lbset),
	fw(_fw),
	cache(_cache),
	bot(_size, _A),
	ent(_size, _A),
	line(lbset->line())
{
		bot.setToBottom();
		ent.empty();
#ifdef PERFTEST
		bot.enterContext();
		ent.enterContext();
#endif

}
	
PERSProblem::~PERSProblem() {

}
const PERSProblem::Domain& PERSProblem::bottom(void) const {
		return bot;
}

const PERSProblem::Domain& PERSProblem::entry(void) const {
		return ent;
}

void PERSProblem::update(Domain& out, const Domain& in, BasicBlock* bb)  {
		cerr << "FATAL: PERSProblem is not to be used directly, use MUSTPERS instead.\n";
		ASSERT(false);
}
	

elm::io::Output& operator<<(elm::io::Output& output, const PERSProblem::Domain& dom) {
	dom.print(output);
	return output;
}

}

