#include <elm/assert.h>
#include <elm/io.h>
#include <otawa/util/DefaultListener.h>
#include <otawa/util/HalfAbsInt.h>
#include "FunctionBlockBuilder.h"
#include "ISPCATBuilder.h"
#include "FunctionBlock.h"
#include "ISPMayProblem.h"

using namespace elm;
using namespace otawa;
using namespace otawa::util;

namespace otawa {

  ISPCATBuilder::ISPCATBuilder(void) : CFGProcessor("otawa::ISPCATBuilder", Version(1, 0, 0)) {
    require(DOMINANCE_FEATURE);
    require(LOOP_HEADERS_FEATURE);
    require(LOOP_INFO_FEATURE);
    require(COLLECTED_FUNCTIONBLOCKS_FEATURE);
    provide(ISP_CAT_FEATURE);

  }
  
  void ISPCATBuilder::configure (const PropList &props) {
    CFGProcessor::configure(props);
  }

  void ISPCATBuilder::processCFG(WorkSpace *ws, CFG *cfg) {
    elm::genstruct::Vector<FunctionBlock *> *function_blocks = FUNCTION_BLOCKS(ws);
    ISPMAYProblem problem(/*size??*/);
    DefaultListener<ISPMAYProblem> listener(ws, problem);
    DefaultFixPoint<DefaultListener<ISPMAYProblem> > fixpoint(listener);
    HalfAbsInt<DefaultFixPoint<DefaultListener<ISPMAYProblem> > > halfabsint(fixpoint, *ws);
    halfabsint.solve();
	
    for(CFG::BBIterator bb(cfg); bb; bb++) {
      if (bb->isEntry() || bb->isExit())
	continue;
      bool seq = false;
      bool branch = false;
      for (BasicBlock::InIterator inedge(bb); inedge; inedge++) {
	if (inedge->kind() == Edge::NOT_TAKEN) 
	  seq = true;
	else 
	  branch = true;
      }		
      FunctionBlock *fb = FUNCTION_BLOCK(bb);
      ASSERT(fb);
      int j = 1; 
      ISPMAYProblem::Domain dom(*listener.results[cfg->number()][bb->number()]); 
      if ((dom.P.contains(id) && (dom.P.count() == 1)) || ( dom.T.contains(id) && dom.T.count() == 1)) {
	ISP_CATEGORY(fb) = ISP_ALWAYS_HIT;
      } 
      else {
	if (!dom.P.contains(id) && !dom.T.contains(id)) {
	  ISP_CATEGORY(fb) = ISP_ALWAYS_MISS;
	} 
	else {
	  ISP_CATEGORY(fb) = ISP_NOT_CLASSIFIED; 
	}
	prob.updateMBlock(dom, fb, seq, branch);
	seq = true;
	branch = false;
      }
    }
  }
  
Identifier<isp_category_t> ISP_CATEGORY("otawa::isp_category", ISP_ALWAYS_HIT);
 Feature<ISPCATBuilder> ISP_CAT_FEATURE("otawa::isp_cat_feature");

} // otawa
