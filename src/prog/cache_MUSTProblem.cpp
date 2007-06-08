
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
 * Problem for computing the CATegorization of l-blocks.
 */

namespace otawa {

	
MUSTProblem::MUSTProblem(const int _size, LBlockSet *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A, bool _unrolling) 
	: callstate(_size, _A), unrolling(_unrolling), line(lbset->line()), cache(_cache), bot(_size, _A), ent(_size, _A), lbset(_lbset),  fw(_fw) {

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
		
/**
 * @fn MUSTProblem::isFirst
 * 
 * Returns true if lblock is the first lblock of any BasicBlock, for the current line.
 *
 * @return boolean result
 */

	
#ifdef BLAHBLAH

void MUSTProblem::blockInterpreted(const FirstUnrollingFixPoint<MUSTProblem>* fp, BasicBlock* bb, const Domain& in, const Domain& out, CFG *cur_cfg) const {
		int bbnumber = bb->number() ;
		int cfgnumber = cur_cfg->number();
	
		lub(*results[cfgnumber][bbnumber], in);
		if (unrolling) {
			BasicBlock *header = NULL;
			bool loopStart = true;
			int i = 0;
			/*
		 	* Detects the header of the loop containing bb, if any
		 	*/
			if (Dominance::isLoopHeader(bb)) {
				header = bb;
			} else if (ENCLOSING_LOOP_HEADER(bb) != NULL)
				header = ENCLOSING_LOOP_HEADER(bb);
			while (header != NULL) {
				if (fp->getIter(header) != 1)
					loopStart = false;
				header = ENCLOSING_LOOP_HEADER(header);
				if (loopStart)
					lub(*unrolledResults[cfgnumber][bbnumber]->firstIter.get(i), in);
				else
					lub(*unrolledResults[cfgnumber][bbnumber]->otherIter.get(i), in);
				i++;
			}	
		}		
#ifdef DEBUG
		cout << "[TRACE] line " << line << " Block " << bbnumber << ": IN=" << in << " OUT=" << out << "\n";
#endif		
}

#endif	
	
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
