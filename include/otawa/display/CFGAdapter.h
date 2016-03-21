/*
 *	$Id$
 *	CFGAdapter class interface
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
#ifndef OTAWA_DISPLAY_CFGADAPTER_H_
#define OTAWA_DISPLAY_CFGADAPTER_H_

#include <otawa/cfg.h>

namespace otawa { namespace display {

// CFGAdapter class
class CFGAdapter {
public:

	// DiGraph concept
	class Vertex {
	public:
		inline Vertex(Block *_b): b(_b) { }
		inline int index(void) { return b->id(); }
		inline bool hasCallee(void) { return (b->isSynth() && b->toSynth()->callee()); }
		inline Vertex calleeEntry(void) { return Vertex(b->toSynth()->callee()->entry()); }
		inline Vertex calleeExit(void) { return Vertex(b->toSynth()->callee()->exit()); }
		Block *b;
		inline CFG* cfg(void) { return b->cfg(); }
	};

	class Edge {
	public:
		inline Edge(otawa::Edge *_edge): edge(_edge) { }
		inline Vertex source(void) const { return Vertex(edge->source()); }
		inline Vertex sink(void) const { return Vertex(edge->target()); }
		otawa::Edge *edge;
	};
	
	class Successor: public PreIterator<Successor, Edge> {
	public:
		inline Successor(const CFGAdapter& ad, Vertex source): i(source.b->outs()), inlining(ad.inlining) { }
		inline bool ended(void) const { return i.ended(); }
		inline Edge item(void) const { return Edge(*i); }
		inline void next(void) { i.next(); }
	private:
		Block::EdgeIter i;
		bool inlining;
	};
	
	// Collection concept
	class Iterator: public PreIterator<Iterator, Vertex> {
	public:
		inline Iterator(const CFGAdapter& adapter): i(adapter.cfg->blocks()), inlining(adapter.inlining), stop(false) { }
		inline bool ended(void) const { return i.ended(); }
		inline Vertex item(void) const { return Vertex(*i); }
		inline void next(void)
		{
			// in the inlining mode, the Synth block will not be shown, and we have to follow to the entry node of the callee
			// we use a SLList to track the stack when following and exiting on an function call
			if(inlining) {
				// when reaching the last block
				if(i->cfg()->count()-1 == i->index()) {
					if (vbi.count()) { // check if it is at the end of the stack
						i = vbi.pop(); // pop the call stack, go to next block after the caller block
						// this is the case if the block obtained is also a synth block
						// i.e. BB6->BB7, both BB6 and BB7 are synth blocks. When cfg of BB6 finishes, BB7 is loaded
						// however we don't want synth block in the CFG hence we go into the synth block right away
						if (i->isSynth()) { // when entering a function
							if (i->toSynth()->callee()) {
								CFG::BlockIter j(i->toSynth()->callee()->blocks()); // j is just for temp
								i.next(); // consume the synth block
								vbi.push(i);
								i = j;
							}
						}
						return; // return (1) the next block after the caller block, or (2) the entry block of the cfg if the block in (1) is a synth block
					}
					else { // when there is no more call stack to pop, and reaches the last block
						stop = true;
					}
				}

				i.next();

				if(stop)
					return;

				if (i->isSynth()) { // when entering a function
					if (i->toSynth()->callee()) {
						CFG::BlockIter j(i->toSynth()->callee()->blocks()); // j is just for temp
						i.next(); // consume the synth block
						vbi.push(i);
						i = j;
					}
				}
			}
			else // non-inlining case
				i.next();
		}
	private:
		CFG::BlockIter i;
		SLList<CFG::BlockIter> vbi;
		bool inlining;
		bool stop;
	};
	
	// DiGraphWithVertexMap concept
	template <class T>
	class VertexMap {
	public:
		inline VertexMap(const CFGAdapter& adapter)
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
			if(found)
				vals[index] = val;
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
	
	inline CFGAdapter(CFG *_cfg, bool _inlining = false, WorkSpace *_ws = 0): cfg(_cfg), ws(_ws), inlining(_inlining) { }
	inline int count(void) const { // getting the number of the block by traversing through the iterator
		int num = 0;
		for(Iterator iterator(*this); iterator; iterator++)
			num++;
		return num;
	}

	CFG *cfg;
	WorkSpace *ws;
	bool inlining;
}; // end of class CFGAdapter

// AUX functions for the map
inline bool operator==(const CFGAdapter::Vertex& lhs, const CFGAdapter::Vertex& rhs) {
	return (lhs.b == rhs.b);
}

inline bool operator>(const CFGAdapter::Vertex& lhs, const CFGAdapter::Vertex& rhs) {
	return (lhs.b > rhs.b);
}

} } // otawa::display

#endif // OTAWA_DISPLAY_CFGADAPTER_H_
