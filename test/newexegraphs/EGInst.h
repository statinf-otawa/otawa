/*
 *	$Id$
 *	Interface to the EGInst, EGInstSeq classes.
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

#ifndef _EG_INST_H_
#define _EG_INST_H_

#include <otawa/cfg/BasicBlock.h>
#include "ExecutionGraph.h"
/*
#include <otawa/graph/GenGraph.h>
#include <elm/string/StringBuffer.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/parexegraph/ParExeProc.h>
#include <otawa/parexegraph/Resource.h>
#include <otawa/hard/Platform.h>
#include <otawa/graph/PreorderIterator.h>
*/

namespace otawa { namespace exegraph2 {

class EGNode;
class EGEdge;
class ExecutionGraph;

typedef enum part_t {
	PREDECESSOR = 0,
	BLOCK = 1,
	SUCCESSOR = 2,
	PARTS_NUMBER  // should be the last value
} part_t;

	/*
	 * class EGInst
	 *
	 */

	class EGInst {
	private:
		Inst * _inst;
		BasicBlock *_bb;
		part_t _part;
		int _index;
		elm::genstruct::Vector<EGNode *> _nodes;
		EGNode * _fetch_node;
		EGNode *_exec_node;

	public:
		inline EGInst(Inst * inst, BasicBlock *bb, part_t part, int index)
		: _inst(inst), _bb(bb), _part(part), _index(index) {}
		// get information
		inline Inst * inst()  {return _inst;}
		inline part_t part()  {return _part;}
		inline BasicBlock * basicBlock()  {return _bb;}
		inline int index()  {return _index;}
		inline EGNode *node(int index) { return _nodes[index];}
		inline EGNode * firstNode() { return _nodes[0];}
		inline EGNode * lastNode() { return _nodes[_nodes.length()-1];}
		inline bool hasNodes() { return (_nodes.length() != 0);}
		inline int numNodes() { return _nodes.length();}
		inline EGNode * fetchNode() { return _fetch_node;}
		inline EGNode * execNode() { return _exec_node;}
		// set information
		inline void setIndex(int index) {_index=index;}
		inline void setFetchNode(EGNode *node) { _fetch_node = node;}
		inline void setExecNode(EGNode *node) { _exec_node = node;}
		// add/remove nodes
		inline void addNode(EGNode * node)  { _nodes.add(node);}
		inline void removeNode(EGNode *node) { _nodes.remove(node); }
		inline void addAfter(EGNode *pos, EGNode *node) {
			int p = _nodes.indexOf(pos);
			ASSERT(p >= 0);
			_nodes.insert(p + 1, node);
		}

		inline void deleteNodes() {
			if (_nodes.length() != 0)
				_nodes.clear();
		}

		class NodeIterator: public elm::genstruct::Vector<EGNode *>::Iterator {
		public:
			inline NodeIterator(const EGInst *inst):
				elm::genstruct::Vector<EGNode *>::Iterator(inst->_nodes){}
		};

	};

	/*
	 * class EGInstSeq
	 *
	 */

	class EGInstSeq : public elm::genstruct::Vector<EGInst *> {
	public:
		class InstIterator: public elm::genstruct::Vector<EGInst *>::Iterator {
		public:
			inline InstIterator(const EGInstSeq *seq):
				elm::genstruct::Vector<EGInst *>::Iterator(*seq){}
		};
		inline int length()
		{return elm::genstruct::Vector<EGInst *>::length();}
	};


} // namespace newexegrpah
} // namespace otawa

#endif //_EG_INST_H_








