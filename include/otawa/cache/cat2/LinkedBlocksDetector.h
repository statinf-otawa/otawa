/*
 *	$Id$
 *	Processor performing the non-conflicting lblocks detection
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef CACHE_LINKEDBLOCKSDETECTOR_H_
#define CACHE_LINKEDBLOCKSDETECTOR_H_

#include <otawa/proc/Processor.h>
#include <otawa/cache/cat2/CAT2Builder.h>
#include <elm/genstruct/SortedSLList.h>
#include <elm/genstruct/Vector.h>
#include <otawa/cache/LBlock.h>
#include <otawa/prop/Identifier.h>

namespace otawa {

extern Identifier<genstruct::Vector<LBlock*> *> LINKED_BLOCKS;

class NumberOrder {
	public:
	bool greaterThan(LBlock *lb1, LBlock *lb2) {
		return(CATEGORY_HEADER(lb1)->number() < CATEGORY_HEADER(lb2)->number());
	}
};

typedef genstruct::SortedSLList<LBlock*, NumberOrder> LinkedBlockList;

class LinkedBlocksDetector : public otawa::Processor {
	bool _explicit;
	NumberOrder order;
	void recordBlocks(Vector<LBlock*> *equiv);
	public:
	LinkedBlocksDetector(void);
	virtual void processWorkSpace(otawa::WorkSpace*);
	virtual void configure(const PropList& props);
};

}

#endif
