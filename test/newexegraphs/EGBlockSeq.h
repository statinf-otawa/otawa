/*
 *	$Id$
 *	Interface to the EGSeqBuilder classes.
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

#ifndef _EGBlockSeqList_H_
#define _EGBlockSeqList_H_

#include <otawa/cfg.h>
#include "EGProc.h"

namespace otawa{

namespace exegraph2 {

using namespace elm::genstruct;

class EGBlockSeq{
private:
	elm::genstruct::SLList<BasicBlock *> _bb_list;
	int _num_insts;
	int _num_bbs;
	BasicBlock * _bb;
	Edge * _edge;
public:
	EGBlockSeq(BasicBlock *bb);
	EGBlockSeq(const EGBlockSeq& ctxt);
	~EGBlockSeq();
	void addBlock(BasicBlock * new_bb, Edge * edge);
	inline int numInsts()
		{ return _num_insts;}
	inline int numBlocks()
		{ return _num_bbs;}
	inline BasicBlock* lastBlock()
		{ return _bb_list.last();}
	inline BasicBlock* mainBlock()
		{ return _bb;}
	inline Edge * edge     ()
		{ return _edge;}
	void dump(io::Output& output) ;

	class BasicBlockIterator: public elm::genstruct::SLList<BasicBlock *>::Iterator {
	public:
		inline BasicBlockIterator(const EGBlockSeq& ctxt)
			: elm::genstruct::SLList<BasicBlock *>::Iterator(ctxt._bb_list) {}
	};
};

class EGBlockSeqList{
private:
	elm::genstruct::SLList<EGBlockSeq *> _list;
	uint32_t _min_pred_length;
	void build(EGBlockSeq *seq);
public:
	EGBlockSeqList(BasicBlock *bb, uint32_t min_pred_length);
	~EGBlockSeqList();
	elm::genstruct::SLList<EGBlockSeq *> * getList()
		{return &_list;}

	class SeqIterator: public elm::genstruct::SLList<EGBlockSeq *>::Iterator {
		public:
			inline SeqIterator(const EGBlockSeqList& list)
				: elm::genstruct::SLList<EGBlockSeq *>::Iterator(list._list) {}
		};
};

class EGBlockSeqListFactory{
public:
	EGBlockSeqList * newEGBlockSeqList(BasicBlock *bb, uint32_t min_pred_length)
		{return new EGBlockSeqList(bb, min_pred_length);}
};

} // namespace exegraph2
} // namespace otawa

#endif // _EGBlockSeqList_H_
