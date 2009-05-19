/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS <casse@irit.fr>
 *
 *	LBlockBuilder class implementation
 *	This file is part of OTAWA
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <elm/assert.h>
#include "FunctionBlockBuilder.h"
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/cfg.h>
#include <otawa/ilp.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/IPET.h>
#include <otawa/cfg/CFGCollector.h>
#include <elm/genstruct/Vector.h> 
#include <elm/genstruct/HashTable.h>
#include <elm/genstruct/Table.h>

namespace otawa {

  /**
   * @class FunctionBlockBuilder
   * This processor builds the list of function_blocks and stores it in the CFG.
   * 
   * @par Required Features
   * @li @ref INVOLVED_CFGS_FEATURE
   * 
   * @par Provided Features
   * @li @ref COLLECTED_FUNCTIONBLOCKS_FEATURE
   */


  /**
   * Build a new function_block builder.
   */
  FunctionBlockBuilder::FunctionBlockBuilder(void)
  : BBProcessor("otawa::util::FunctionBlockBuilder", Version(1, 1, 0)) {
    _current_function = -1;
    require(COLLECTED_CFG_FEATURE);
    provide(COLLECTED_FUNCTIONBLOCKS_FEATURE);
  }


  /**
   */
  void FunctionBlockBuilder::setup(WorkSpace *ws) {
    ASSERT(ws);

    _fb_set = new elm::genstruct::Vector<FunctionBlock *>;
    FUNCTION_BLOCKS(ws) = _fb_set;
  }


  /**
   */
  void FunctionBlockBuilder::cleanup(WorkSpace *ws) {
    ASSERT(ws);
	
    // Add end blocks ???
//     for(int i = 0; i < cache->rowCount(); i++)
//       new LBlock(lbsets[i], 0, 0, 0, -1);
	
  }



  /**
   */
  void FunctionBlockBuilder::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
    ASSERT(ws);
    ASSERT(cfg);
    ASSERT(bb);
	
    // Do not process entry and exit
    if (bb->isEnd() || bb->isEntry())
      return;

    if (bb->cfg()->number() != _current_function){
      if (_current_function != -1 /* value before start of process */){
	FunctionBlock *fb = new FunctionBlock(_entry_bb, _max_addr-_min_addr);
	FUNCTION_BLOCK(_entry_bb) = fb;
	_fb_set->add(fb);
      }
      // starting a new function_block
      _entry_bb = bb;
      _current_function = bb->cfg()->number();
      _min_addr = 0xFFFFFFFF;
      _max_addr = 0;
     }
    for (BasicBlock::InstIterator inst(bb); inst ; inst++){
      address_t addr = inst->address();
      if (addr < _min_addr)
	_min_addr = addr;
      if (addr > _max_addr)
	_max_addr = addr;
      
    }
  }


  /**
   * This property is set on the entry basic block of the function and points to the function_block
   *
   * @par Hooks
   * @li @ref BasicBlock
   */
  Identifier<FunctionBlock *> FUNCTION_BLOCK("otawa::FUNCTION_BLOCK");

  /**
   * This property is set on the workspace and points to the vector of function_blocks
   *
   * @par Hooks
   * @li @ref WorkSpace
   */
  Identifier<elm::genstruct::Vector<FunctionBlock *> *> FUNCTION_BLOCKS("otawa::FUNCTION_BLOCKS");

  /**
   * This feature ensures that the L-blocks of the current task has been
   * collected.
   * 
   * @par Properties
   * @li @ref LBLOCKS
   * @li @ref BB_LBLOCKS
   */
  Feature<FunctionBlockBuilder> COLLECTED_FUNCTIONBLOCKS_FEATURE("otawa::COLLECTED_FUNCTIONBLOCKS_FEATURE");



} // otawa
