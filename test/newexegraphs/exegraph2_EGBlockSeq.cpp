/*
 *	$Id$
 *	Implementation of the EGSeqBuilder classes.
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

#include "EGBlockSeq.h"

using namespace otawa;
using namespace exegraph2;


EGBlockSeq::EGBlockSeq(BasicBlock *bb){
	_bb_list.addFirst(bb);
	_num_insts = bb->countInstructions();
	_num_bbs = 1;
	_bb = bb;
	_edge = NULL;
}

EGBlockSeq::EGBlockSeq(const EGBlockSeq& seq){
		for (elm::genstruct::SLList<BasicBlock *>::Iterator block(seq._bb_list) ; block ; block++)
			_bb_list.addLast(block);
		_num_insts = seq._num_insts;
		_num_bbs = seq._num_bbs;
		_bb = seq._bb;
		_edge = seq._edge;
	}

EGBlockSeq::~EGBlockSeq(){
	_bb_list.clear();
}

void EGBlockSeq::addBlock(BasicBlock * new_bb, Edge * edge){
	_bb_list.addFirst(new_bb);
	_num_insts += new_bb->countInstructions();
	_num_bbs += 1;
	if (_num_bbs == 1)
		_bb = new_bb;
	if (_num_bbs == 2)
		_edge = edge;
}

void EGBlockSeq::dump(io::Output& output) {
	for (elm::genstruct::SLList<BasicBlock *>::Iterator bb(_bb_list) ; bb ; bb++){
		output << "b" << bb->number() << "-";
	}
}

EGBlockSeqList::EGBlockSeqList(BasicBlock * bb, uint32_t min_pred_length){

	EGBlockSeq * seq = new EGBlockSeq(bb);
	_min_pred_length = min_pred_length;
	build(seq);
}

void EGBlockSeqList::build(EGBlockSeq *seq){
	BasicBlock *bb = seq->lastBlock();
	uint32_t num_preds = 0;
	for(BasicBlock::InIterator edge(bb); edge; edge++) {
		BasicBlock *pred = edge->source();
		if (!pred->isEntry() && !pred->isExit()) {
			num_preds++;
			EGBlockSeq *new_seq = new EGBlockSeq(*seq);
			new_seq->addBlock(pred, edge);
			if (new_seq->numInsts() >= _min_pred_length)
				_list.addLast(new_seq);
			else
				build(new_seq);
		}
	}
	if (num_preds == 0){
		_list.addLast(seq);
	}
	else
		delete seq;
}

EGBlockSeqList::~EGBlockSeqList(){
	_list.clear();
}

