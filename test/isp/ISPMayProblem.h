
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

using namespace otawa::dfa;

namespace otawa {

  class ISP {
  public:
    static address_t const PF_BOTTOM;
    static address_t const PF_TOP;
  };


  // ?
  class AMS {
  public:
    
    BitSet P; /* Prefetch buffer */
    BitSet T; /* Trail buffer */
    BitSet L; /* Latch, trucs en cours de préchargement */
    inline AMS(int size) : P(size), T(size), L(size){
    }
  };

  class ASS {
    inline ASS(){
    }
  }


  class ISPMAYProblem {
  public:
    typedef ASS Domain;
    inline ISPMayProblem(int size) : bot(size + 1), ent(size + 1),  INVALID_BLOCK(size) { 

      /* bottom */
      /* 		bot.P.empty(); */
      /* 		bot.T.empty(); */
      /* 		bot.L.empty(); */
		
      /* entry */
      /* 		ent.P.empty(); */
      /* 		ent.T.empty(); */
      /* 		ent.L.empty();		 */
      /* 		ent.P.add(INVALID_BLOCK); */
      /* 		ent.T.add(INVALID_BLOCK); */
      /* 		ent.L.add(INVALID_BLOCK); */
    }
	
    inline const Domain& bottom(void) const { 
      return bot; 
    } 
    inline const Domain& entry(void) const { 
      return ent; 
    } 
    inline void lub(Domain &a, const Domain &b) const {
      a.P.add(b.P);
      a.T.add(b.T);
      a.L.add(b.L);
    }
    inline void assign(Domain &a, const Domain &b) const { 
      a = b; 
    }
    inline bool equals(const Domain &a, const Domain &b) const { 
      return ((a.P == b.P) && (a.T == b.T) && (a.L == b.L) ) ;
    }

    void update(Domain& out, const Domain& in, BasicBlock* bb); 
    void updateFunctionBlock(Domain &dom, FunctionBlock *fb, bool seq, bool branch); 
    inline void enterContext(Domain &dom, BasicBlock *header, otawa::util::hai_context_t&) { }
    inline void leaveContext(Domain &dom, BasicBlock *header, otawa::util::hai_context_t&) { }


  private:
    Domain bot; /* Bottom state */
    Domain ent; /* Entry state */
    const int INVALID_BLOCK;

  };

} // otawa

#endif // ISP_PROBLEM_H_
