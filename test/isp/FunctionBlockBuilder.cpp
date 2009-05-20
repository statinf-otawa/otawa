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
  FunctionBlockBuilder::FunctionBlockBuilder()
  : ContextualProcessor("otawa::util::FunctionBlockBuilder", Version(1, 1, 0)) {
    require(COLLECTED_CFG_FEATURE);
    require(CFG_SIZE_FEATURE);
    provide(COLLECTED_FUNCTIONBLOCKS_FEATURE);
 
  }


  /**
   * processCFG ???
   */

  /**
   * Function called when a function call is found
   */
  
  void FunctionBlockBuilder::enteringCall(WorkSpace *ws, CFG *cfg, BasicBlock *caller, BasicBlock *callee){
    CFG *inlined_cfg = NULL;
    for (BasicBlock::InIterator in_edge(callee) ; in_edge ; in_edge++){
      if (in_edge->source() == caller){
	assert(inlined_cfg == NULL);
	inlined_cfg = CALLED_CFG(in_edge);
      }
    }
    assert(inlined_cfg);
    FunctionBlock *fb = new FunctionBlock(inlined_cfg);
    FUNCTION_BLOCK(callee) = fb;
    if (!FUNCTION_BLOCKS(ws))
      FUNCTION_BLOCKS(ws) = new elm::genstruct::Vector<FunctionBlock *>;
    FUNCTION_BLOCKS(ws)->add(fb);
  }

  /**
   * Function called when a function return is found
   */
  
  void FunctionBlockBuilder::leavingCall(WorkSpace *ws, CFG *cfg){
  }

  /**
   * Function called when a recursive function call is found
   */
  
  void FunctionBlockBuilder::avoidingRecursive(WorkSpace *ws, CFG *cfg, BasicBlock *caller, BasicBlock *callee){
  }

  /**
   * ProcessBB
   */
  void FunctionBlockBuilder::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
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
  Identifier<elm::genstruct::Vector<FunctionBlock *> *> FUNCTION_BLOCKS("otawa::FUNCTION_BLOCKS", NULL);

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
