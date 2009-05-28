#include <elm/assert.h>
#include <elm/io.h>
#include <otawa/util/DefaultListener.h>
#include <otawa/util/HalfAbsInt.h>
#include "FunctionBlockBuilder.h"
#include "ISPCATBuilder.h"
#include "FunctionBlock.h"
#include "ISPProblem.h"

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
    _isp_size = ISP_SIZE(props);
    elm::cerr << "ISP Size is " << _isp_size << "\n";
  }

  void ISPCATBuilder::processCFG(WorkSpace *ws, CFG *cfg) {
    ISPProblem problem(_isp_size);
    DefaultListener<ISPProblem> listener(ws, problem,true);
    DefaultFixPoint<DefaultListener<ISPProblem> > fixpoint(listener);
    HalfAbsInt<DefaultFixPoint<DefaultListener<ISPProblem> > > halfabsint(fixpoint, *ws);
    halfabsint.solve();
	
    for(CFG::BBIterator bb(cfg); bb; bb++) {
      if (bb->isEntry() || bb->isExit())
	continue;
      FunctionBlock *fb = FUNCTION_BLOCK(bb);
      if (fb) {
	ISPProblem::Domain dom(*listener.results[cfg->number()][bb->number()]); 
	bool may, must;
	dom.contains(fb, &may, &must);
	
	if (must) {
	  ISP_CATEGORY(bb) = ISP_ALWAYS_HIT;
	} 
	else {
	  if (!may) {
	    ISP_CATEGORY(bb) = ISP_ALWAYS_MISS;
	  } 
	  else {
	    BasicBlock *pers_header = NULL;
	    BasicBlock *loop_header = ENCLOSING_LOOP_HEADER(bb);
 	    while (loop_header){
	      bool pers_entering, pers_back = true;
 	      for (BasicBlock::InIterator edge(loop_header) ; edge && pers_back; edge++){
		BasicBlock * source = edge->source();
		ISPProblem::Domain source_out(*listener.results_out[cfg->number()][source->number()]); 
		bool pers_may, pers_must;
		source_out.contains(fb, &pers_may, &pers_must);
 		if (!Dominance::isBackEdge(edge)){  // edge thats enters the loop
		  pers_entering = pers_must;
		}
		else { // backedge
		  if (!pers_must)
		    pers_back = false;
		}
	      }
	      if (pers_back){
		pers_header = loop_header;
		if (pers_entering)
		  loop_header = ENCLOSING_LOOP_HEADER(loop_header);
		else
		  loop_header = NULL;
	      }
	      else
		loop_header = NULL;
	    }
		  
	    if (pers_header){
	      ISP_CATEGORY(bb) = ISP_PERSISTENT; 
	      ISP_HEADER(bb) = pers_header;
	    }
	    else
	      ISP_CATEGORY(bb) = ISP_NOT_CLASSIFIED; 
	  }
	}
      }
    }
  }
  
Identifier<isp_category_t> ISP_CATEGORY("otawa::isp_category", ISP_ALWAYS_HIT);
Identifier<BasicBlock *> ISP_HEADER("otawa::isp_header", NULL);

 Feature<ISPCATBuilder> ISP_CAT_FEATURE("otawa::isp_cat_feature");

} // otawa
