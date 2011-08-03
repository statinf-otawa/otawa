/*
 *	$Id$
 *	Interface to the ExecutionGraph, EGNode, EGEdge classes.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "ExecutionGraph.h"

using namespace  otawa;
using namespace otawa::exegraph2;

/**
 * Build a parametric execution graph.
 * @param inst_seq	Instruction sequence to consider.
 */
ExecutionGraph::ExecutionGraph(WorkSpace *ws, EGBlockSeq *block_seq){
	_proc = new EGProc(otawa::hard::PROCESSOR(ws));

	part_t part = PREDECESSOR;
	int index = 0;
	for (EGBlockSeq::BasicBlockIterator block(*block_seq) ; block ; block++){
		if (block == block_seq->mainBlock())
			part = BLOCK;
		for(BasicBlock::InstIterator inst(block); inst; inst++) {
			EGInst * new_inst = new EGInst(inst, block, part, index++);
			_inst_seq.add(new_inst);
		}
	}
}

ExecutionGraph::~ExecutionGraph() {
	for (EGInstSeq::InstIterator inst(&_inst_seq) ; inst ; inst++) {
		inst->deleteNodes();
	}
	delete _proc;
}





