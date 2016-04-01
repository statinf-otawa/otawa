/*
 *	$Id$
 *	InlinedCFGAdapter class interface
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
#ifndef OTAWA_DISPLAY_INLINEDCFGADAPTER_H_
#define OTAWA_DISPLAY_INLINEDCFGADAPTER_H_

#include <otawa/cfg.h>
#include <otawa/display/CFGAdapter.h>

namespace otawa { namespace display {

// InlinedCFGAdapter class
class InlinedCFGAdapter: public CFGAdapter {
public:

	// DiGraph concept
	class Vertex: public CFGAdapter::Vertex {
	public:
		inline Vertex(Block *_b): CFGAdapter::Vertex(_b), b(_b) { }
		inline int index(void) { return b->id(); }
		Block *b;
	};

	class Edge: public CFGAdapter::Edge {
	public:
		inline Edge(otawa::Edge *_edge): CFGAdapter::Edge(_edge), edge(_edge) { }
		inline Vertex source(void) const { return Vertex(edge->source()); }
		inline Vertex sink(void) const {
			if(edge->target()->isSynth()) { // the edge is between the caller and callee
				if(edge->target()->toSynth()->callee()) // the edge is between the caller and callee
					return Vertex(edge->target()->toSynth()->callee()->entry());
				else
					return Vertex(edge->target());
			}
			else // the edge is between Blocks of the same CFG
				return Vertex(edge->target());
		}
		otawa::Edge *edge;
	};
	
	class Successor: public PreIterator<Successor, Edge> {
	public:
		inline Successor(const InlinedCFGAdapter& ad, Vertex source): i(source.b->outs()) {
			if(source.b->isExit()) {
				for(CFG::CallerIter cfgci = source.b->cfg()->callers(); cfgci; cfgci++) {
					i = cfgci->outs();
					slli.push(i);
				}
				if(slli.count())
					slli.pop();
			}
		}

		inline bool ended(void) const { return i.ended(); }
		inline Edge item(void) const { return Edge(*i); }
		inline void next(void) {
			i.next();
			if(!i)
				if(slli.count())
					i = slli.pop();
		}
	private:
		Block::EdgeIter i;
		SLList<Block::EdgeIter> slli;
	};
	
	// Collection concept
	class Iterator: public PreIterator<Iterator, Vertex> {
	public:
		inline Iterator(const InlinedCFGAdapter& adapter): i(adapter.cfg->blocks()), stop(false) { }
		inline bool ended(void) const { return i.ended(); }
		inline Vertex item(void) const { return Vertex(*i); }
		inline void next(void)
		{
			// in the inlining mode, the Synth block will not be shown, and we have to follow to the entry node of the callee
			// we use a SLList to track the stack when following and exiting on an function call
			// when reaching the last block
			if(i->cfg()->count()-1 == i->index()) {
				if (vbi.count()) { // check if it is at the end of the stack
					i = vbi.pop(); // pop the call stack, go to next block after the caller block
					// this is the case if the block obtained is also a synth block
					// i.e. BB6->BB7, both BB6 and BB7 are synth blocks. When cfg of BB6 finishes, BB7 is loaded
					// however we don't want synth block in the CFG hence we go into the synth block right away
					while (i->isSynth() && i->toSynth()->callee()) { // when entering a function
						if(!travledCFG.contains(i->toSynth()->callee())) {
							travledCFG.add(i->toSynth()->callee());
							CFG::BlockIter j(i->toSynth()->callee()->blocks()); // j is just for temp
							i.next(); // consume the synth block
							vbi.push(i);
							i = j; // goes into the call
						}
						else
							i.next();
					}
					return; // return (1) the next block after the caller block, or (2) the entry block of the cfg if the block in (1) is a synth block
				}
				else { // when there is no more call stack to pop, and reaches the last block
					stop = true;
				}
			} // if reaches the last block

			i.next();

			if(stop)
				return;

			while(i->isSynth() && i->toSynth()->callee()) { // when entering a function
				if(!travledCFG.contains(i->toSynth()->callee())) {
					travledCFG.add(i->toSynth()->callee());
					CFG::BlockIter j(i->toSynth()->callee()->blocks()); // j is just for temp
					i.next(); // consume the synth block
					vbi.push(i);
					i = j;
				}
				else // ignore the SynthBlock because already traversed
					i.next();
			}
		}
	private:
		CFG::BlockIter i;
		SLList<CFG::BlockIter> vbi;
		bool stop;
		SLList<CFG*> travledCFG;		// each CFG only gets through once
	};
	
	// DiGraphWithVertexMap concept
	template <class T>
	class VertexMap {
	public:
		inline VertexMap(const InlinedCFGAdapter& adapter)
			: size(adapter.count()), vals(new T[size]), bAddrs(new t::uint64[size]) {
			// to make sure the bAddrs are all cleared
			for(int i = 0; i < size; i++)
				bAddrs[i] = 0;
			currIndex = 0;
		}

		inline ~VertexMap() {
			delete[] vals;
			delete[] bAddrs;
		}

		inline const T& get(const Vertex& vertex) const
		{
			bool found = false;
			int index = -1;
			for(int i = 0; i < size; i++) {
				if((void*)bAddrs[i] == (void*)vertex.b) {
					index = i;
					found = true;
					break;
				}
			}
			if(found)
				return vals[index];
			else {
				elm::cerr << "not found BB " << vertex.b->cfg()->index() << "-" << vertex.b->index() << " (id=" << vertex.b->id() << ")" << " @ " << (void*)vertex.b << " [" << index << "]" << io::endl;
				ASSERTP(false, "Element not found in the Vertex Map");
				return vals[0];
			}
		}

		inline void put(const Vertex& vertex, const T& val)
		{
			bool found = false;
			int index = -1;
			for(int i = 0; i < size; i++) { // find the index
				if((void*)bAddrs[i] == (void*)vertex.b) {
					index = i;
					found = true;
					break;
				}
			}
			if(found) {
				vals[index] = val;
			}
			else {
				vals[currIndex] = val;
				bAddrs[currIndex] = (t::uint64)vertex.b;
				currIndex++;
			}
		}
	private:
		int size; // the size of the table
		int currIndex; // the current index
		T *vals;
		t::uint64 *bAddrs; // the key of the table, which is the address of the Block
	};
	
	inline InlinedCFGAdapter(CFG *_cfg, WorkSpace *_ws = 0):
			CFGAdapter(_cfg, _ws),
			cfg(_cfg), ws(_ws) { }
	inline int count(void) const { // getting the number of the block by traversing through the iterator
		int num = 0;
		for(Iterator iterator(*this); iterator; iterator++)
			num++;
		return num;
	}

	CFG *cfg;
	WorkSpace *ws;
}; // end of class InlinedCFGAdapter

} } // otawa::display

#endif // OTAWA_DISPLAY_INLINED_CFGAdapter_H_
