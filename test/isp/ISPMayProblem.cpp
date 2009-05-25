
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

#include "ISPMayProblem.h"


using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;

/**
 * @class ISPMayProblem
 * 
 * Problem for computing the Abstract Instruction Scratchpas State.
 * This implements Ferdinand's May analysis.
 * 
 */

namespace otawa {

#define TRACE_ISP_PROBLEM 1

//   address_t const ISP::PF_TOP = 0;
//   address_t const ISP::PF_BOTTOM = -1;

// ========================================================================
  void ISPMayProblem::update(Domain& out, const Domain& in, BasicBlock *bb) {
#ifdef TRACE_ISP_PROBLEM
    elm::cout << "==== Entering update for bb" << bb->number() << "\n";
    in.dump(elm::cout,"Input abstract state:");
    out.dump(elm::cout,"Output abstract state:");
#endif
    FunctionBlock *new_fb = FUNCTION_BLOCK(bb);
    if (new_fb){
      elm::cout << "new_fb != 0\n";
      assert(CFG_SIZE(new_fb->cfg()) <= _size) ;
      if (!out.isEmpty()){
	elm::cout << "!out.isEmpty()\n";
	for (AbstractISPState::Iterator fl(out); fl; fl++){
	  // check whether list already contains the function block
	  if (!fl->contains(new_fb)){
	    elm::cout << "!fl->contains(new_fb))\n";
	    while (fl->size() + CFG_SIZE(new_fb->cfg()) > _size)
	      fl->remove();
	    fl->add(new_fb);
	  }
	}
      }
      else {
	elm::cout << "out.isEmpty()\n";
	FunctionList *new_fl = new FunctionList;
	new_fl->add(new_fb);
	assert(new_fb);
	out.addLast(new_fl);
     }
    }
    else {
      elm::cout << "new_fb == 0\n";
    }
#ifdef TRACE_ISP_PROBLEM
    elm::cout << "==== Exiting update with:\n";
    out.dump(elm::cout, "Output abstract state:");
#endif
  }

  // ===================================================================================
  void ISPMayProblem::lub(Domain &a, const Domain &b) const {
 #ifdef TRACE_ISP_PROBLEM
    elm::cout << "\t==Entering lub function with:\n";
    a.dump(elm::cout, "\t\ta=");
    b.dump(elm::cout, "\t\tb=");
#endif
    if (!b.isEmpty()){
      for (elm::genstruct::DLList<FunctionList *>::Iterator flb(b); flb; flb++){
	bool found = false;
	// check if list already contained in a
	if (!a.isEmpty()){
	  for (AbstractISPState::Iterator fla(a); fla && !found; fla++){
	    if (fla.item() == flb.item()){
	      found = true;
#ifdef TRACE_ISP_PROBLEM
	      elm::cout << "\t\t\tthis list is already contained in a: ";
	      fla->dump(elm::cout);
#endif	      
	    }
	  }
	}
	if (!found){
	  // add list to a
	  FunctionList *new_list = new FunctionList(*(flb.item()));
	  a.addLast(new_list);
#ifdef TRACE_ISP_PROBLEM
	  elm::cout << "\t\t\tadding this list to a: ";
	  flb->dump(elm::cout);
#endif	      
	}
      }
    }
 #ifdef TRACE_ISP_PROBLEM
    elm::cout << "\t==Exiting lub function with:";
    a.dump(elm::cout, "a=");
#endif      
  }


  void AbstractISPState::contains(FunctionBlock *block, bool *may, bool *must){
    *may = false;
    *must = true;
    for (Iterator fl(*this); fl && !may && must; fl++){
      bool found = false;
      for (FunctionList::Iterator fb(*(fl.item())) ; fb ; fb++){
	if (fb.item() == block){
	  *may = true;
	  found = true;
	}
      }
      if (!found)
	*must = false;
    }
  }
 
} // otawa

