/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS <casse@irit.fr>
 *
 *	LBlockBuilder class interface
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
#ifndef OTAWA_UTIL_FUNCTIONBLOCKBUILDER_H
#define OTAWA_UTIL_FUNCTIONBLOCKBUILDER_H

#include "FunctionBlock.h"
#include <otawa/proc/Feature.h>
#include <elm/genstruct/Vector.h> 
#include <elm/genstruct/Table.h>
#include <otawa/proc/ContextualProcessor.h>
#include "CFGSizeComputer.h"

namespace otawa {


// FunctionBlockBuilder class
class FunctionBlockBuilder: public ContextualProcessor {

  private:
    elm::genstruct::SLList<CFG *> _call_stack;

protected:
	virtual void enteringCall(WorkSpace *ws, CFG *cfg, BasicBlock *caller, BasicBlock *callee);
	virtual void leavingCall(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
	virtual void avoidingRecursive(WorkSpace *ws, CFG *cfg, BasicBlock *caller, BasicBlock *callee);
	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
	

public:
	FunctionBlockBuilder();
};

  //Identifiers
  extern Identifier<FunctionBlock *> FUNCTION_BLOCK;
  extern Identifier<elm::genstruct::Vector<FunctionBlock *> *> FUNCTION_BLOCKS;
  extern Identifier<size_t> ISP_SIZE;
// Features
extern Feature<FunctionBlockBuilder> COLLECTED_FUNCTIONBLOCKS_FEATURE;

} // otawa

#endif // OTAWA_UTIL_FUNCTIONBLOCKBUILDER_H
