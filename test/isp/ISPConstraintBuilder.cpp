/*
 *	$Id: ISPConstraintBuilder.cpp
 *	MemConstraintBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
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

#include <stdio.h>
#include <elm/io.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/ilp.h>
#include <otawa/ipet.h>
#include <otawa/cfg.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/ipet/TrivialInstCacheManager.h>
#include <otawa/cache/categorisation/CATBuilder.h>

#include "ISPConstraintBuilder.h"
#include "ISPCATBuilder.h"
#include "FunctionBlockBuilder.h"

using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;

namespace otawa {
	

Identifier<ilp::Var *> ISP_HIT_VAR("otawa::isp_hit_var", 0);
Identifier<ilp::Var *> ISP_MISS_VAR("otawa::isp_miss_var", 0);
Identifier<int> RAM_PENALTY("otawa::ram_penalty", 0);

/**
 * @class ISPConstraintBuilder
 *
 * This processor produces constraints for the ISP
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref ASSIGNED_VARS_FEATURE
 * @li @ref COLLECTED_FUNCTIONBLOCKS_FEATURE
 * @li @ref ISP_CAT_FEATURE
 * @li @ref ILP_SYSTEM_FEATURE
 *
 * @par Provided features
 * @li @ref ISP_SUPPORT_FEATURE
 * 
 * @par Statistics
 * none
 */



ISPConstraintBuilder::ISPConstraintBuilder(void) : Processor("otawa::ISPConstraintBuilder", Version(1, 0, 0)), _explicit(false) {
	require(ASSIGNED_VARS_FEATURE);
	require(ISP_CAT_FEATURE);
	require(COLLECTED_FUNCTIONBLOCKS_FEATURE);
	require(ILP_SYSTEM_FEATURE);
	provide(ISP_SUPPORT_FEATURE);
}

void ISPConstraintBuilder::configure(const PropList& props) {
        Processor::configure(props);
	_explicit = EXPLICIT(props);
	penalty = RAM_PENALTY(props);
}

void ISPConstraintBuilder::setup(otawa::WorkSpace *fw) {
}

                
void ISPConstraintBuilder::processWorkSpace(otawa::WorkSpace *ws) {
  ilp::System *system = SYSTEM(ws);

  //  elm::genstruct::Vector<FunctionBlock *> *function_blocks = FUNCTION_BLOCKS(ws);
  for (CFGCollection::Iterator cfg(INVOLVED_CFGS(ws)); cfg; cfg++) {
    for (CFG::BBIterator bb(cfg); bb ; bb++) {
      FunctionBlock *fb = FUNCTION_BLOCK(bb);
      if (fb) {
	// create variable for the number of function misses for this bb
	ilp::Var *fmiss;
	if(!_explicit)
	  fmiss = system->newVar();
	else {
	  StringBuffer buffer;
	  buffer << "xISP_miss_bb" << bb->number() << "_" << fb->cfg()->label();
	  String name = buffer.toString();
	  fmiss = system->newVar(name);
	}
	ISP_MISS_VAR(bb) = fmiss;
	Constraint *cons;
	switch(ISP_CATEGORY(bb)) {
	case ISP_ALWAYS_HIT: 
	  /* xfmiss = 0 */
	  cons = system->newConstraint("ISP ALWAYS HIT",Constraint::EQ);
	  cons->addLeft(1, ISP_MISS_VAR(bb));
	  break;	
	case ISP_ALWAYS_MISS:
	  /* xfmiss = x */
	  cons = system->newConstraint("ISP ALWAYS MISS", Constraint::EQ);
	  cons->addLeft(1, ISP_MISS_VAR(bb));
	  cons->addRight(1, VAR(bb));
	  break;				
	case ISP_NOT_CLASSIFIED:
	  /* xfmiss <= x */
	  cons = system->newConstraint("ISP NOT CLASSIFIED", Constraint::LE);
	  cons->addLeft(1, ISP_MISS_VAR(bb));
	  cons->addRight(1, VAR(bb));
	  break;
	case ISP_PERSISTENT:{
	  /* xfmiss = x_in_header */
	  cons = system->newConstraint("ISP PERSISTENT", Constraint::LE);
	  BasicBlock *header = ISP_HEADER(bb);
	  cons->addLeft(1, ISP_MISS_VAR(bb));
	  for (BasicBlock::InIterator edge(header) ; edge ; edge++){
	    if (!Dominance::isBackEdge(edge)){  // edge thats enters the loop
	      cons->addRight(1, VAR(edge));
	    }
	  }}
	  break;
	default:
	  ASSERTP(false, "Unknown category");
	  break;
	}
	// Add x_miss*penalty to object function
	size_t function_size = CFG_SIZE(fb->cfg());
	int function_load_penalty = (function_size / 4) * (penalty+3);
	if (function_size % 4 != 0)
	  function_load_penalty += penalty + (function_size % 4 - 1);
	system->addObjectFunction(function_load_penalty, ISP_MISS_VAR(bb));
      }          
    }    
  }
}
Feature<ISPConstraintBuilder> ISP_SUPPORT_FEATURE("otawa::isp_support_feature");
}
