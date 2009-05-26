/*
 *	$Id$
 *	CFGSizeComputer processor implementation
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

#include "CFGSizeComputer.h"
#include <elm/checksum/Fletcher.h>
#include <otawa/cfg/CFG.h>
#include <otawa/prog/WorkSpace.h>

using namespace elm;

namespace otawa {

#define MIN_ADDR 0
#define MAX_ADDR 0xFFFFFFFF

/**
 * @class CFGSizeComputer
 * Computes the size, lower address and higher address of a CFG.
 * @author	Christine Rochange <rochange@irit.fr>
 *
 * @par Required Features
 * @li @ref COLLECTED_CFG_FEATURE
 *
 * @par Provided Features
 * @li @ref CFG_SIZE_FEATURE
 */


/**
 * Build a new size computer.
 */
CFGSizeComputer::CFGSizeComputer(void): ContextualProcessor("otawa::CFGSizeComputer", Version(1, 0, 0)) {
  provide(CFG_SIZE_FEATURE);
  _current_level = -1;
}

/**
 */

  void CFGSizeComputer::enteringCall(WorkSpace *ws, CFG *cfg, BasicBlock *caller, BasicBlock *callee){
    CFG *inlined_cfg = NULL;
    for (BasicBlock::InIterator in_edge(callee) ; in_edge ; in_edge++){
      if (in_edge->source() == caller){
	assert(inlined_cfg == NULL);
	inlined_cfg = CALLED_CFG(in_edge);
      }
    }
    assert(inlined_cfg);
    _current_level++;
    _min_addr.add((address_t) MAX_ADDR); 
    _max_addr.add((address_t) MIN_ADDR);
    _inlined_cfg.add(inlined_cfg);

  }


/**
 */

  void CFGSizeComputer::leavingCall(WorkSpace *ws, CFG *cfg, BasicBlock *bb){
    CFG* icfg = _inlined_cfg[_current_level];
    assert(icfg);
    CFG_SIZE(icfg) = (size_t) (_max_addr[_current_level] - _min_addr[_current_level]);
    CFG_HIGHER_ADDR(icfg) = _max_addr[_current_level];
    CFG_LOWER_ADDR(icfg) = _min_addr[_current_level];
    assert(CFG_SIZE(icfg));
    elm::cout << "computing size for cfg " << icfg->label() << ": " << CFG_SIZE(icfg) << "\n";
       _min_addr.pop();
    _max_addr.pop();
    _inlined_cfg.pop();
    _current_level--;
  }

/**
 */
   void CFGSizeComputer::avoidingRecursive(WorkSpace *ws, CFG *cfg, BasicBlock *caller, BasicBlock *callee){
  }

/**
 */
  void CFGSizeComputer::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb){
    if (!bb->isEnd() && !bb->isEntry()){
      for (BasicBlock::InstIterator inst(bb); inst ; inst++){
	address_t addr = inst->address();
	if (addr < _min_addr[_current_level])
	  _min_addr[_current_level] = addr;
	if (addr > _max_addr[_current_level])
	  _max_addr[_current_level] = addr;    
      }
    }
  }
  
static SilentFeature::Maker<CFGSizeComputer> maker;
/**
 * This feature ensures that each CFG has hooked a checksum allowing
 * to check binary modifications between launch of an OTAWA application.
 *
 * @par Properties
 * @li @ref CHECKSUM
 */
SilentFeature CFG_SIZE_FEATURE("otawa::CFG_SIZE_FEATURE", maker);


/**
 * This property hooked on a CFG provides a checksum build
 * on the instruction of the CFG.
 */
  Identifier<size_t > CFG_SIZE("otawa::CFG_SIZE", 0);
  Identifier<size_t > CFG_CUMULATIVE_SIZE("otawa::CFG_CUMULATIVE_SIZE", 0);
  Identifier<address_t> CFG_LOWER_ADDR("otawa::CFG__LOWER_ADDR",0);
  Identifier<address_t> CFG_HIGHER_ADDR("otawa::CFG_HIGHER_ADDR",0);
  Identifier<address_t> CFG_CUMULATIVE_LOWER_ADDR("otawa::CFG__CUMULATIVE_LOWER_ADDR",0);
  Identifier<address_t> CFG_CUMULATIVE_HIGHER_ADDR("otawa::CFG_CUMULATIVE_HIGHER_ADDR",0);
  
} // otawa
