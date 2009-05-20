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
 * Build a new check summer.
 */
CFGSizeComputer::CFGSizeComputer(void): CFGProcessor("otawa::CFGSizeComputer", Version(1, 0, 0)) {
  provide(CFG_SIZE_FEATURE);
}


/**
 */
void CFGSizeComputer::processCFG(WorkSpace *ws, CFG *cfg) {

  address_t min_addr = 0xFFFFFFFF;
  address_t max_addr = 0;
  for (CFG::BBIterator bb(cfg); bb ; bb++) {
    if (!bb->isEnd() && !bb->isEntry()){
      for (BasicBlock::InstIterator inst(bb); inst ; inst++){
	address_t addr = inst->address();
	if (addr < min_addr)
	  min_addr = addr;
	if (addr > max_addr)
	  max_addr = addr;    
      }
    }
  }
  CFG_LOWER_ADDR(cfg) = min_addr;
  CFG_HIGHER_ADDR(cfg) = max_addr;
  CFG_SIZE(cfg) = (size_t) (max_addr - min_addr);
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
  Identifier<address_t> CFG_LOWER_ADDR("otawa::CFG__LOWER_ADDR",0);
  Identifier<address_t> CFG_HIGHER_ADDR("otawa::CFG_HIGHER_ADDR",0);
  
} // otawa
