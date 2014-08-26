/*
 *	GlobalAnalysisProblem class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_DYNBRANCH_GLOBAL_ANALYSIS_PROBLEM_H
#define OTAWA_DYNBRANCH_GLOBAL_ANALYSIS_PROBLEM_H

#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/sem.h>
#include <otawa/data/clp/ClpAnalysis.h>

#include "State.h"

#define GLOBAL_MEMORY_LOADER false

using namespace elm::genstruct ;
using namespace otawa ;

namespace global {

  typedef State Domain ;

  class GlobalAnalysisProblem {
    public  :
      typedef State Domain ;

      GlobalAnalysisProblem(WorkSpace* workspace, bool verbose, Domain entry) ;

      const Domain& bottom() ;

      const Domain& entry() ;

      void lub(Domain& a, const Domain& b) const ;

      void assign(Domain& a,const Domain &b) const;

      void widening(Domain &a,Domain b) const ; 

      void updateEdge(Edge *edge, Domain& d) ;

      bool equals(const Domain &a, const Domain &b) const;

      void update(Domain& out, const Domain& in, BasicBlock *bb) ;

      void enterContext(Domain &dom, BasicBlock *header,  util::hai_context_t ctx) {} ;
      void leaveContext(Domain &dom, BasicBlock *header,  util::hai_context_t ctx) {} ;

    private :
      WorkSpace* ws ;
      Domain bot ;
      Domain ent ;
      bool verbose ;
  };

  bool operator== (const Domain& a, const Domain& b) ; 

} // global

#endif	// OTAWA_DYNBRANCH_GLOBAL_ANALYSIS_PROBLEM_H
