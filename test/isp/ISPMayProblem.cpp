
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
	  // check whether list already contains the function block
	  if (!fl->contains(new_fb)){
	    //elm::cout << "update: fl->size()=" << fl->size() << " and CFG_SIZE(new_fb->cfg())=" << CFG_SIZE(new_fb->cfg()) << "\n";
	    while (fl->size() + fb_size > _size)
	      fl->remove();
	    //elm::cout << "update2: fl->size()=" << fl->size() << " and CFG_SIZE(new_fb->cfg())=" << CFG_SIZE(new_fb->cfg()) << "\n";
	    fl->add(new_fb);
	    //elm::cout << "update: fl->size()=" << fl->size() << "\n";
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
    //   elm::cout << "==== Exiting update with:\n";
    out.dump(elm::cout, "\tOutput abstract state:");
#endif
  }

  // ===================================================================================
  void ISPMayProblem::lub(Domain &a, const Domain &b) const {
#ifdef TRACE_ISP_PROBLEM
//     elm::cout << "\t==Entering lub function with:\n";
//     a.dump(elm::cout, "\t\ta=");
//     b.dump(elm::cout, "\t\tb=");
#endif
    if (!b.isEmpty()){
 //      elm::cout << "** lub: b is not empty\n";
//       elm::cout << "** lub: b contains " << b.count() << " lists\n";
      for (elm::genstruct::DLList<FunctionList *>::Iterator flb(b); flb; flb++){
// 	elm::cout << "** lub: considering list of b ";
// 	flb->dump(elm::cout);
	bool found = false;
	// check if list already contained in a
	if (!a.isEmpty()){
// 	  elm::cout << "** lub: a is not empty: ";
// 	  a.dump(elm::cout,"");
	  for (AbstractISPState::Iterator fla(a); fla && !found; fla++){
	    if (*(fla.item()) == *(flb.item())){
	      found = true;
#ifdef TRACE_ISP_PROBLEM
// 	      elm::cout << "** lub: this list is already contained in a: ";
//  	      fla->dump(elm::cout);
#endif	      
	    }
	  }
	}
// 	else
// 	  elm::cout << "** lub: a is empty\n";
	if (!found){
	  //	  elm::cout << "** lub: not found!\n";
	  // add list to a
	  FunctionList *new_list = new FunctionList(*(flb.item()));
	  //	  elm::cout << "New FunctionList: \n";
	  //	  elm::cout << "\tmodel: ";
	  //	  flb->dump(elm::cout);
	  //	  elm::cout << "\n";
	  //	  elm::cout << "\tnew_list: ";
	  //	  new_list->dump(elm::cout);
	  //	  elm::cout << "\n";
	  a.addLast(new_list);
#ifdef TRACE_ISP_PROBLEM
 // 	  elm::cout << "** lub: a now contains ";
//  	  a.dump(elm::cout,"");
// 	  elm::cout << "** lub: a now contains " << a.count() << " lists\n";
#endif	      
	}
      }
    }
 #ifdef TRACE_ISP_PROBLEM
//     elm::cout << "\t==Exiting lub function with:";
//     if (!a.isEmpty())
//       a.dump(elm::cout, "a=");
#endif      
  }

  // =================================================================
  void AbstractISPState::contains(FunctionBlock *block, bool *may, bool *must){
    *may = false;
    *must = true;
    //elm::cout << "contains: looking for fb " << block->cfg()->label() << "\n";
    if (isEmpty())
      *must = false;
    else {
      for (Iterator fl(*this); fl; fl++){
	//elm::cout << "\tconsidering function list:  ";
	//fl->dump(elm::cout);
	bool found = false;
	for (FunctionList::Iterator fb(*(fl.item())) ; fb ; fb++){
	  if (fb->cfg() == block->cfg()){
	    *may = true;
	    found = true;
	  }
	}
	if (!found){
	  *must = false;
	  //elm::cout << " DOES NOT CONTAIN BLOCK";
	}
	//elm::cout << "\n";
      }
    }
  }
 
} // otawa

