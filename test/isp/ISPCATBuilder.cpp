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
    ISPMayProblem problem(65536);
    DefaultListener<ISPMayProblem> listener(ws, problem);
    DefaultFixPoint<DefaultListener<ISPMayProblem> > fixpoint(listener);
    HalfAbsInt<DefaultFixPoint<DefaultListener<ISPMayProblem> > > halfabsint(fixpoint, *ws);
    halfabsint.solve();
	
    for(CFG::BBIterator bb(cfg); bb; bb++) {
      if (bb->isEntry() || bb->isExit())
	continue;
      FunctionBlock *fb = FUNCTION_BLOCK(bb);
      if (fb) {
	ISPMayProblem::Domain dom(*listener.results[cfg->number()][bb->number()]); 
	bool may, must;
	dom.contains(fb, &may, &must);
	if (must) {
	  ISP_CATEGORY(fb) = ISP_ALWAYS_HIT;
	} 
	else {
	  if (!may) {
	    ISP_CATEGORY(fb) = ISP_ALWAYS_MISS;
	  } 
	  else {
	    ISP_CATEGORY(fb) = ISP_NOT_CLASSIFIED; 
	  }
	}
      }
    }
  }
  
Identifier<isp_category_t> ISP_CATEGORY("otawa::isp_category", ISP_ALWAYS_HIT);
 Feature<ISPCATBuilder> ISP_CAT_FEATURE("otawa::isp_cat_feature");

} // otawa
