
#include <stdio.h>
#include <elm/io.h>
#include "FunctionBlock.h"
#include "FunctionBlockBuilder.h"
#include <otawa/ilp.h>
#include <otawa/ipet.h>
#include <otawa/cfg/Edge.h>
#include <otawa/util/Dominance.h>
#include <otawa/cfg.h>
#include <otawa/util/LoopInfoBuilder.h>
#include <otawa/hard/Platform.h>
#include <elm/assert.h>

#include "ISPProblem.h"


using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;

/**
 * @class ISPProblem
 * 
 * Problem for computing the Abstract Instruction Scratchpas State.
 * This implements Ferdinand's May analysis.
 * 
 */

namespace otawa {




//   address_t const ISP::PF_TOP = 0;
//   address_t const ISP::PF_BOTTOM = -1;

// ========================================================================
  void ISPProblem::update(Domain& out, const Domain& in, BasicBlock *bb) {
#ifdef TRACE_ISP_PROBLEM
    elm::cout << "==== Entering update for bb" << bb->number() << "\n";
    in.dump(elm::cout,"\tInput abstract state:");
    elm::cout << "\tupdate: in contains " << in.count() << " states\n";
#endif 
    //out = in;
    out.clear();
    Domain tmp = in;
    FunctionBlock *new_fb = FUNCTION_BLOCK(bb);
    if (new_fb){
       assert(CFG_SIZE(new_fb->cfg()) <= _size) ;
      if (!tmp.isEmpty()){
	size_t fb_size = CFG_SIZE(new_fb->cfg()); 
	for (AbstractISPState::Iterator fl(tmp); fl; fl++){
	  if (!fl->contains(new_fb)){
	    while (fl->size() + fb_size > _size)
	      fl->remove();
	    fl->add(new_fb);
	    assert(fl->size() <= _size);
	  }
	  out.add(fl.item());
	}
      }
      else {
	FunctionList *new_fl = new FunctionList;
	new_fl->add(new_fb);
	assert(new_fb);
	assert(new_fl->size() <= _size);
	out.add(new_fl);
     }
    }
    else
      out = in;

#ifdef TRACE_ISP_PROBLEM
    out.dump(elm::cout, "\tOutput abstract state:");
#endif
  }

  // ===================================================================================
  void ISPProblem::lub(Domain &a, const Domain &b) const {
    if (!b.isEmpty()){
      for (elm::genstruct::DLList<FunctionList *>::Iterator flb(b); flb; flb++){
	bool found = false;
	if (!a.isEmpty()){
	  for (AbstractISPState::Iterator fla(a); fla && !found; fla++){
	    if (*(fla.item()) == *(flb.item())){
	      found = true;
	    }
	  }
	}
	if (!found){
	  FunctionList *new_list = new FunctionList(*(flb.item()));
	  a.addLast(new_list);
	}
      }
    }
  }

  // =================================================================
  void AbstractISPState::contains(FunctionBlock *block, bool *may, bool *must){
    *may = false;
    *must = true;
     if (isEmpty())
      *must = false;
    else {
      for (Iterator fl(*this); fl; fl++){
	bool found = false;
	for (FunctionList::Iterator fb(*(fl.item())) ; fb ; fb++){
	  if (fb->cfg() == block->cfg()){
	    *may = true;
	    found = true;
	  }
	}
	if (!found){
	  *must = false;
	}
      }
    }
  }
 
} // otawa

