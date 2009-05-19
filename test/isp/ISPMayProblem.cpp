
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

  address_t const ISP::PF_TOP = 0;
  address_t const ISP::PF_BOTTOM = -1;

  void ISPMayProblem::update(Domain& out, const Domain& in, BasicBlock *bb) {
	
    bool seq = false;
    bool branch = false;
	
    for (BasicBlock::InIterator inedge(bb); inedge; inedge++) {
      if (inedge->kind() == Edge::NOT_TAKEN) 
	seq = true;
      else 
	branch = true;
    }

    assign(out, in);
	
    FunctionBlock *fb = FUNCTION_BLOCK(bb);
    if (!fb)
      return;
    
    updateFunctionBlock(out, fb, seq, branch);

    seq = true;
    branch = false;
    }
  }

  void ISPMayProblem::updateFunctionBlock(Domain &dom, FunctionBlock *fb, bool seq, bool branch) {
    Domain branchdom(bot), seqdom(bot);
    //   int id = mamblock->mamBlockID();
	
    assign(seqdom, dom);
    assign(branchdom, dom);		
    assign(dom, bot);
		 
    if (branch) {
      if (!seqdom.P.contains(id)) {
	seqdom.T.empty();
	seqdom.T.add(id);
      } 
      else {
	if ((seqdom.P.contains(id) && (seqdom.P.count() == 1)) || ( seqdom.T.contains(id) && seqdom.T.count() == 1)) {

	} 
	else {
	  seqdom.T.add(id);						
	}
      }	
      lub(dom, seqdom);	
    } 
    if (seq) {
      if (!branchdom.T.contains(id)) {			
	branchdom.P.empty();
	branchdom.P.add(id);
      } 
      else {
	if ((branchdom.P.contains(id) && (branchdom.P.count() == 1)) || ( branchdom.T.contains(id) && branchdom.T.count() == 1)) {
	} 
	else {
	  branchdom.P.add(id);						
	}
      }
      lub(dom, branchdom);
    }
	
	 
    dom.L.empty();
    dom.L.add(mamblock->next);


  }

} // otawa

