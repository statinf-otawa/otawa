/*
 *	$Id$
 *	exegraph module interface
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
#ifndef LI_EXECUTIONGRAPH_BBTIME_H
#define LI_EXECUTIONGRAPH_BBTIME_H

#include "ExeGraphBBTime.h"
#include "LiExeGraph.h"
#include <otawa/ipet.h>



/*#define DO_LOG
#if defined(NDEBUG) || !defined(DO_LOG)
#	define LOG(c)
#else
#	define LOG(c) c
#	define OUT	elm::cerr
#endif*/


namespace otawa {
	
using namespace elm::genstruct; 
	
class LiExeGraphBBTime: public ExeGraphBBTime<LiExeGraph> {
	private:
	public:
		LiExeGraphBBTime(const PropList& props = PropList::EMPTY);
	
		void buildPrologueList(	BasicBlock * bb,
										ExeSequence<ExeNode> * prologue, 
										int capacity, 
										elm::genstruct::DLList<ExeSequence<ExeNode> *> * prologue_list);
		void buildEpilogueList(	BasicBlock * bb,
				       				ExeSequence<ExeNode> * epilogue, 
				       				int capacity, 
				       				elm::genstruct::DLList<ExeSequence<ExeNode> *>* epilogue_list,
				       				int start_index);
		void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb);
		int processSequence(WorkSpace *fw,
									ExeSequence<ExeNode> * prologue,
									ExeSequence<ExeNode> * body,
									ExeSequence<ExeNode> * epilogue,
									int capacity ) ;
		class InstIterator : public LiExeGraph::InstIterator {
			public:
				inline InstIterator(const ExeSequence<ExeNode> * sequence)
					: LiExeGraph::InstIterator(sequence) {}
		};
		class SeqIterator : public elm::genstruct::DLList<ExeSequence<ExeNode> *>::Iterator {
			public:
				inline SeqIterator(const elm::genstruct::DLList<ExeSequence<ExeNode> *>& list)
					: elm::genstruct::DLList<ExeSequence<ExeNode> *>::Iterator(list) {}
		};

};



} //otawa

#endif // LiExeGraphBBTime_H
