
#ifndef ISP_MAYPROBLEM_H_
#define ISP_MAYPROBLEM_H_

#include <otawa/util/HalfAbsInt.h>
#include <elm/assert.h>
#include <elm/io.h>
#include <elm/util/Pair.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/dfa/BitSet.h>
#include "FunctionBlock.h"
#include <elm/Collection.h>

using namespace otawa::dfa;

namespace otawa {

  // == Function List
  class FunctionList: public elm::genstruct::DLList<FunctionBlock *>{
  public:
    inline FunctionList(){}
    inline FunctionList& operator=(const FunctionList& list);
    inline bool operator==(FunctionList& list);
  };

  inline FunctionList& FunctionList::operator=(const FunctionList& list) {
    clear();
    for (elm::genstruct::DLList<FunctionBlock *>::Iterator fb(list); fb; fb++)
      addLast(fb);
    return *this;
  }
  
  inline bool FunctionList::operator==(FunctionList& list) {
    bool equ = true;
    elm::genstruct::DLList<FunctionBlock *>::Iterator fa(*this), fb(list);
    while (equ && fa && fb){
      if (fa.item() != fb.item())
	equ = false;
      else {
	fa++;
	fb++;
      }
    }
    if ((fa && !fb) || (!fa && fb))
      equ = false;
    return equ;
  }

  // == AbstractISPState
  class AbstractISPState: public elm::genstruct::DLList<FunctionList *>{
  public:
    inline AbstractISPState(){}
    inline AbstractISPState& operator=(const AbstractISPState& list);
    inline bool operator==(AbstractISPState& list);
  };

  inline AbstractISPState& AbstractISPState::operator=(const AbstractISPState& list) {
    while (!isEmpty()){
      delete last();
      removeLast();
    }
    for (elm::genstruct::DLList<FunctionList *>::Iterator fl(list); fl; fl++){
      FunctionList *new_list = new FunctionList();
      new_list = fl;
      addLast(new_list);
    }
    return *this;
  }
  
  inline bool AbstractISPState::operator==(AbstractISPState& state) {
    bool equ = true;
    elm::genstruct::DLList<FunctionList *>::Iterator fla(*this), flb(state);
    while (equ && fla && flb){
      if (!(fla.item() == flb.item()))
	equ = false;
      else {
	fla++;
	flb++;
      }
    }
    if ((fla && !flb) || (!fla && flb))
      equ = false;
    return equ;
  }
  

  class ISPMayProblem {
  public:
    typedef AbstractISPState Domain;

    inline ISPMayProblem() {}	
    inline const Domain& bottom(void) const { 
      return _bottom; 
    } 
    inline const Domain& entry(void) const { 
      return _entry; 
    } 
    inline void lub(Domain &a, const Domain &b) const;
    inline void assign(Domain &a, const Domain &b) const {
      a = b;
    }
    inline bool equals(const Domain &a, const Domain &b) const { 
      return (a==b);
    }

    void update(Domain& out, const Domain& in, BasicBlock* bb); 
    void updateFunctionBlock(Domain &dom, FunctionBlock *fb, bool seq, bool branch); 
    inline void enterContext(Domain &dom, BasicBlock *header, otawa::util::hai_context_t&) { }
    inline void leaveContext(Domain &dom, BasicBlock *header, otawa::util::hai_context_t&) { }


  private:
    Domain _bottom; /* Bottom state */
    Domain _entry; /* Entry state */
 
  };

  inline void ISPMayProblem::lub(Domain &a, const Domain &b) const {
    for (elm::genstruct::DLList<FunctionList *>::Iterator flb(b); flb; flb++){
      bool found = false;
      // check if list already contained in a
      for (elm::genstruct::DLList<FunctionList *>::Iterator fla(a); fla && !found; fla++){
	if (fla.item() == flb.item())
	  found = true;
      }
      if (!found){
	// add list to a
	FunctionList *new_list = new FunctionList();
	new_list = flb;
	a.addLast(new_list);
      }
    }
  }

} // otawa

#endif // ISP_PROBLEM_H_
