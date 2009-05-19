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

#include <otawa/proc/BBProcessor.h>
#include "FunctionBlock.h"
#include <otawa/proc/Feature.h>
#include <elm/genstruct/Vector.h> 
#include <elm/genstruct/Table.h>


namespace otawa {


// FunctionBlockBuilder class
class FunctionBlockBuilder: public BBProcessor {

  private:
    elm::genstruct::Vector<FunctionBlock *> * _fb_set;
    int _current_function;
    address_t _min_addr, _max_addr;
    BasicBlock * _entry_bb;

protected:
	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
	virtual void cleanup(WorkSpace *ws);
	virtual void setup(WorkSpace *ws);

public:
	FunctionBlockBuilder(void);
};

  //Identifiers
  extern Identifier<FunctionBlock *> FUNCTION_BLOCK;
  extern Identifier<elm::genstruct::Vector<FunctionBlock *> *> FUNCTION_BLOCKS;

// Features
extern Feature<FunctionBlockBuilder> COLLECTED_FUNCTIONBLOCKS_FEATURE;

} // otawa

#endif // OTAWA_UTIL_FUNCTIONBLOCKBUILDER_H
