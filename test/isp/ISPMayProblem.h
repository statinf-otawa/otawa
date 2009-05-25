
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
#include <elm/genstruct/DLList.h>

using namespace otawa::dfa;

namespace otawa {

  // == Function List
  class FunctionList: private elm::genstruct::DLList<FunctionBlock *>{
  public:
    inline FunctionList(): _size(0){
    }
    inline FunctionList(const FunctionList& list);
    inline FunctionList& operator=(const FunctionList& list);
    inline bool operator==(FunctionList& list);
    inline void add(FunctionBlock *fb);
    inline void remove();
    inline bool contains(FunctionBlock *fb);
    inline size_t size(){
      return _size;
    }
    inline bool isEmpty(){
      return elm::genstruct::DLList<FunctionBlock *>::isEmpty();
    }
    inline void dump(elm::io::Output& out_stream);
    class Iterator:  public elm::genstruct::DLList<FunctionBlock *>::Iterator {
    public:
      inline Iterator(const FunctionList &fl)
	: elm::genstruct::DLList<FunctionBlock *>::Iterator(fl) {}
    };
  private:
    size_t _size;
  };

  inline FunctionList::FunctionList(const FunctionList& list){
    for (Iterator fb(list); fb ; fb++){
      add(fb.item());
    }    
  }

  inline FunctionList& FunctionList::operator=(const FunctionList& list) {
    clear();
    _size = 0;
    for (Iterator fb(list); fb; fb++){
      addLast(fb);
      _size += CFG_SIZE(fb->cfg());
    }
    return *this;
  }
  
  inline bool FunctionList::operator==(FunctionList& fl) {
    bool equ = true;
    if (_size != fl.size())
      equ = false;
    Iterator fa(*this), fb(fl);
    while (equ && fa && fb){
      if (fa.item() != fb.item())
	equ = false;
      else {
	fa++;
	fb++;
      }
    }
    return equ;
  }

  inline void FunctionList::add(FunctionBlock *fb){
    addLast(fb);
    _size += CFG_SIZE(fb->cfg());
  }
  inline void FunctionList::remove(){
    FunctionBlock *fb = first();
    _size -= CFG_SIZE(fb->cfg());
    removeFirst();
  }

  inline bool FunctionList::contains(FunctionBlock *fblock){
    if (isEmpty())
      return false;
    for (Iterator fb(*this); fb ; fb++){
      if (fb.item() == fblock)
	return true;
    }
    return false;
  }

  inline void FunctionList::dump(elm::io::Output& out_stream){
    if (isEmpty())
      out_stream << "empty\n";
    else {
      for (Iterator fb(*this); fb ; fb++){
	assert(fb.item());
	out_stream << fb->cfg()->label() << "-";
      }
      out_stream << "\n";
    }
  }

  // == AbstractISPState
  class AbstractISPState: public elm::genstruct::DLList<FunctionList *>{
  public:
    inline AbstractISPState(){}
    inline AbstractISPState(const AbstractISPState& state){
      *this = state;
    }
    inline AbstractISPState& operator=(const AbstractISPState& list);
    inline bool operator==(AbstractISPState& list);
    void contains(FunctionBlock *fb, bool *may, bool *must);
    inline void dump(elm::io::Output& out_stream, String header) const;
  
    class Iterator:  public elm::genstruct::DLList<FunctionList *>::Iterator {
    public:
      inline Iterator(const AbstractISPState &fl)
	: elm::genstruct::DLList<FunctionList *>::Iterator(fl) {}
    };
  };

  inline AbstractISPState& AbstractISPState::operator=(const AbstractISPState& state) {
    while (!isEmpty()){
      delete last();
      removeLast();
    }
    if (!state.isEmpty()){
      for (Iterator fl(state); fl; fl++){
	FunctionList *new_list = new FunctionList();
	new_list = fl;
	addLast(new_list);
      }
    }
    return *this;
  }
  
  inline bool AbstractISPState::operator==(AbstractISPState& state) {
    bool equ = true;
    Iterator fla(*this), flb(state);
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
 
  inline void AbstractISPState::dump(elm::io::Output& out_stream, String header) const{
    out_stream << header;
    if (isEmpty())
      out_stream << "\tempty!\n";
    else {
      for (Iterator fl(*this); fl ; fl++){
	out_stream << "\t";
	fl->dump(out_stream);
      }
    }
  }
 

  class ISPMayProblem {
  public:
    typedef AbstractISPState Domain;

    inline ISPMayProblem(size_t size) : _size(size){}	
    inline const Domain& bottom(void) const { 
      return _bottom; 
    } 
    inline const Domain& entry(void) const { 
      return _entry; 
    } 
    void lub(Domain &a, const Domain &b) const;
    inline void assign(Domain &a, const Domain &b) const {
      a = b;
    }
    inline bool equals(const Domain &a, const Domain &b) const { 
      return (a==b);
    }
    inline void enterContext(Domain &dom, BasicBlock *header, otawa::util::hai_context_t&) { }
    inline void leaveContext(Domain &dom, BasicBlock *header, otawa::util::hai_context_t&) { }
    void update(Domain& out, const Domain& in, BasicBlock* bb); 
 
  private:
    Domain _bottom; /* Bottom state */
    Domain _entry; /* Entry state */
    size_t _size;
  };



} // otawa

#endif // ISP_PROBLEM_H_
