/*
 *	$Id$
 *	VirtualizedCFGAdapter class interface
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
#ifndef OTAWA_DISPLAY_VIRTUALIZEDCFGADAPTER_H_
#define OTAWA_DISPLAY_VIRTUALIZEDCFGADAPTER_H_

#include <otawa/cfg.h>
#include <otawa/display/CFGAdapter.h>

namespace otawa { namespace display {

// VirtualizedCFGAdapter class
class VirtualizedCFGAdapter: public CFGAdapter {
public:

	/*
	 * The display::Vertex class wraps the otawa::Block (the member variable b)
	 */
	class Vertex: public CFGAdapter::Vertex {
	public:
		inline Vertex(otawa::Block *_b, otawa::Block* _c = 0, Block* _br = 0, otawa::Block* _r = 0, otawa::Edge* _er = 0):
			CFGAdapter::Vertex(_b), b(_b), c(_c), bR(_br), r(_r), eR(_er) { }
		inline int index(void) { return b->id(); }
		otawa::Block *b;	// the associated Block
		otawa::Block* c;	// the SynthBlock which associated with the CFG that b belongs to, (b, c) will be used as the key for the VertexMap
		otawa::Block *bR;	// the Block to return to when the call including b finishes
		otawa::Block* r;	// the SynthBlock which associated with the CFG that bR belongs to, (bR, r) will be used as the key for the VertexMap
		otawa::Edge *eR;	// the edge which has bR as the target()
	};

	/*
	 * The display::Edge class wraps the actual otawa::Edge (the member variable edge).
	 * @param _edge
	 */
	class Edge: public CFGAdapter::Edge {
	public:
		inline Edge(otawa::Edge *_edge, otawa::Block* _from = 0, otawa::Block* _to = 0, otawa::Edge* _eR = 0):
			CFGAdapter::Edge(_edge == 0?_eR:_edge), edge(_edge), associatedSynth(_from), associatedReturningSynth(_to), eR(_eR) {
			if(!edge) {
				returnEdge = true;
				edge = eR;
			}
			else
				returnEdge = false;
		}
		inline Vertex source(void) const {
			ASSERTP(false, "Need to implement this function.");
			return edge->source();
		}
		inline Vertex sink(void) const {
			if(returnEdge) // the edge is between the Exit Block of the CFG and the block to return in the caller CFG
				return Vertex(edge->target(), associatedReturningSynth);
			else if(edge->target()->isSynth()) // the edge is between the caller and callee
				return Vertex(edge->target()->toSynth()->callee()->entry(), (otawa::Block*)(edge->target()));
			else // the edge is between Blocks of the same CFG
				return Vertex(edge->target(), associatedSynth);
		}
		otawa::Edge *edge;						// the associated edge
		otawa::Block* associatedSynth;			// if the edge is between Blocks of the same CFG, both block will have the same bR, which we call associatedSynth here
		otawa::Block* associatedReturningSynth;	// if the edge is between Exit Block of the callee CFG and the block to return in the Caller CFG
		bool returnEdge;						// the flag indicates the edge is formed when returning from the Exit Block of callee. The constructor is called by making argument _edge to NULL
		otawa::Edge* eR;						// used to replace "edge" when returnEdge comes into effect
	};
	
	class Successor: public PreIterator<Successor, Edge> {
	public:
		inline Successor(const VirtualizedCFGAdapter& ad, Vertex source): i(source.b->outs()), associatedSynth(source.c), returnSynth(source.r), exitBlockProcessed(false), eR(source.eR) {
			if(source.b->isExit()) {
				if(source.bR)
					exitBlockProcessed = true;
			}
		}

		inline bool ended(void) const {
			if(exitBlockProcessed)
				return false;
			return i.ended();
		}
		inline Edge item(void) const {
			if(exitBlockProcessed)
				return Edge(0, associatedSynth /* not used */, returnSynth, eR);
			
			return Edge(*i, associatedSynth); // for edge between Blocks in the same CFG
		}
		inline void next(void) {
			if(exitBlockProcessed) {
				exitBlockProcessed = false;
				return;
			}
			i.next();
		}
	private:
		Block::EdgeIter i;				// the associated Edge iterator
		otawa::Block* associatedSynth; 	// used to provide information of the SynthBlock associated with the current CFG
		otawa::Block* returnSynth; 		// used to provide information of the SynthBlock associated with the caller CFG
		bool exitBlockProcessed;		// if the given vertex is a Exit Block, this flag is created so that successor will provide the edge leading back to the caller
		otawa::Edge* eR;				// Assumed that Successor is created by giving the vertex which has the valid edge back to the caller
	};
	
	// Collection concept
	class Iterator: public PreIterator<Iterator, Vertex> {
	public:
		inline Iterator(const VirtualizedCFGAdapter& adapter): i(adapter.cfg->blocks()), stop(false), currentSynth(0), returnSynth(0), blockToReturn(0), edgeToReturn(0) {
			//synthList.push(currentSynth);
		}
		inline bool ended(void) const { return i.ended(); }
		inline Vertex item(void) const { return Vertex(*i, currentSynth, blockToReturn, returnSynth, edgeToReturn); }
		inline void next(void)
		{
			// The Synth Blocks will not be shown, and we have to follow to the entry node of the callee
			// we use a SLList to track the stack when following and exiting on an function call
			// when reaching the last block
			if(i->cfg()->count()-1 == i->index()) {
				if (vbi.count()) { // check if it is at the end of the stack
					i = vbi.pop(); // pop the call stack, go to next otawa::Block after the caller block
					currentSynth = synthList.pop();
					if(synthList.count())
						returnSynth = synthList.last();
					else
						returnSynth = 0;
					blockToReturn = returnCallStack.pop();
					edgeToReturn = returnEdgeCallStack.pop();

					// this is the case if the otawa::Block obtained is also a synth block
					// i.e. BB6->BB7, both BB6 and BB7 are synth blocks. When cfg of BB6 finishes, BB7 is loaded
					// however we don't want synth otawa::Block in the CFG hence we go into the synth otawa::Block right away
					if (i->isSynth() && i->toSynth()->callee()) { // when entering a function
						returnSynth = currentSynth;
						synthList.push((otawa::Block*)currentSynth);
						currentSynth = (otawa::Block*)(*i);
						returnCallStack.push(blockToReturn);
						blockToReturn = i->outs()[0].target();
						returnEdgeCallStack.push(edgeToReturn);
						edgeToReturn = &(i->outs()[0]);

						CFG::BlockIter j(i->toSynth()->callee()->blocks()); // j is just for temp
						i.next(); // consume the synth block
						vbi.push(i);
						i = j;
					} // if (i->isSynth()) {
					return; // return (1) the next otawa::Block after the caller block, or (2) the entry otawa::Block of the cfg if the otawa::Block in (1) is a synth block
				} // if (vbi.count()) {
				else { // when there is no more call stack to pop, and reaches the last block
					stop = true;
				}
			} // if(i->cfg()->count()-1 == i->index()) {

			i.next();

			if(stop)
				return;

			if (i->isSynth() && i->toSynth()->callee()) { // when entering a function
				returnSynth = currentSynth;
				synthList.push((otawa::Block*)currentSynth);
				currentSynth = (otawa::Block*)(*i);
				returnCallStack.push(blockToReturn);
				blockToReturn = i->outs()[0].target();
				returnEdgeCallStack.push(edgeToReturn);
				edgeToReturn = &(i->outs()[0]);
				CFG::BlockIter j(i->toSynth()->callee()->blocks()); // j is just for temp
				i.next(); // consume the synth block
				vbi.push(i);
				i = j;
			} // end of if (i->isSynth()) {
		}
	private:
		CFG::BlockIter i;
		SLList<CFG::BlockIter> vbi;
		bool stop;
		SLList<otawa::Block*> synthList;
		otawa::Block* currentSynth;
		otawa::Block* returnSynth;
		SLList<Block*> returnCallStack;
		Block* blockToReturn;
		SLList<otawa::Edge*> returnEdgeCallStack;
		otawa::Edge* edgeToReturn;
	};
	
	// DiGraphWithVertexMap concept
	template <class T>
	class VertexMap {
	public:
		inline VertexMap(const VirtualizedCFGAdapter& adapter)
			: size(adapter.count()), vals(new T[size]), bAddrs(new otawa::Block*[size]), synthAddrs(new otawa::Block*[size]) {
			// to make sure the bAddrs are all cleared
			for(int i = 0; i < size; i++) {
				bAddrs[i] = 0;
				synthAddrs[i] = 0;
			}
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
				if(((void*)bAddrs[i] == (void*)vertex.b) && (synthAddrs[i] == vertex.c)) {
					index = i;
					found = true;
					break;
				}
			}
			if(found)
				return vals[index];
			else {
				elm::cout << "not found BB " << vertex.b->cfg()->index() << "-" << vertex.b->index() << " (id=" << vertex.b->id() << ")" << " @ " << (void*)vertex.b << " [" << index << "]" << ", c = " << (void*)vertex.c << ", r = " << (void*)vertex.r << ", rb = " << (void*)(vertex.bR) << io::endl;
				ASSERTP(false, "Element not found in the Vertex Map");
				return vals[0];
			}
		}

		inline void put(const Vertex& vertex, const T& val)
		{
			bool found = false;
			int index = -1;
			for(int i = 0; i < size; i++) { // find the index, need to match the address of the Block and the associated Synth Block
				if((bAddrs[i] == vertex.b) && (synthAddrs[i] == vertex.c)) {
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
				bAddrs[currIndex] = (otawa::Block*)vertex.b;
				synthAddrs[currIndex] = vertex.c;
				currIndex++;
			}
		}
	private:
		int size; 					// the size of the table
		int currIndex; 				// the current index
		T *vals;
		otawa::Block* *bAddrs; 		// the key of the table, which is the address of the Block
		otawa::Block* *synthAddrs;	// the synth block goes to CFG will be used as the "id" of the CFG
	};
	
	inline VirtualizedCFGAdapter(CFG *_cfg, WorkSpace *_ws = 0): CFGAdapter(_cfg, _ws), cfg(_cfg), ws(_ws) { }

	/*
	 * Getting the number of the Block by traversing through the iterator
	 */
	inline int count(void) const {
		int num = 0;
		for(Iterator iterator(*this); iterator; iterator++)
			num++;
		return num;
	}

	CFG *cfg;
	WorkSpace *ws;
}; // end of class VirtualizedCFGAdapter

} } // otawa::display

#endif // OTAWA_DISPLAY_CFGADAPTER_H_
