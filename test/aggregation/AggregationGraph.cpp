/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	AggregationGraph.cpp -- AggregationGraph class implementation.
 */
#include "AggregationGraph.h"
#include "Header.h"
#include "SequenceBasicBlock.h"
#include <assert.h>
#include <otawa/util/Dominance.h>
#include <otawa/ilp.h>
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/HashTable.h>

#include <iostream>

using namespace otawa::ilp;
using namespace otawa::ipet;
using namespace elm::genstruct;


namespace otawa { namespace ipet {

/**
 * @author G. Cavaignac
 * @class AggregationGraph
 * This class creates an aggregation graph from a cfg:
 * Basic blocks are grouped together when they can be executed
 * sequencially. These sequences are duplicated when the path
 * lengthen, and two path can be made (when the current basic block
 * have two targets)
 */


/**
 * Creates an AggregationGraph from the given inlined CFG.
 * See the GenericIdentifiers for the properties
 */
AggregationGraph::AggregationGraph(CFG *_cfg, PropList &props)
: cfg(_cfg), _made(false){
	assert(cfg);
	assert(cfg->isInlined());
	configure(props);
}


/**
 * Main algorithm. Initialize the AggregationGraph, if it hasn't been
 * initialized yet.
 */
void AggregationGraph::makeGraph(){
	if(_made){
		return;
	}
	
	// ensure that cfg has loop headers marked
	Dominance::ensure(cfg);
	
	Vector<Pair<BBPath*, BBPath*>*> sequences;
	HashTable<void*, Header*> headers; // <BasicBlock*, Header*>
	BasicBlock *nextBB;
	BBPath *entry_path;
	BBPath *exit_path;
	BBPath *nextBB_path;
	BBPath *hdr_path;
	BBPath *cur_path;
	BBPath *next_path;
	BBPath *father;
	Header *hdr;
	Pair<BBPath*, BBPath*> *pair;
	
	// Initialize graph with Entry and Exit
	entry_path = BBPath::getBBPath(&_entry);
	exit_path  = BBPath::getBBPath(&_exit); 
	
	// Find the only basic block next to Entry
	nextBB = cfg->entry();
	for(BasicBlock::OutIterator edge(nextBB); edge; edge++)
		nextBB = edge->target();

	// Create the header just after the Entry
	hdr = new Header(nextBB);
	headers.put(nextBB, hdr);
	hdr_path = BBPath::getBBPath(hdr);
	
	// N' = N' U Entry
	elts.add(entry_path);
	// N' = N' U hdr
	elts.add(hdr_path);
	
	// E' = E' U (Entry, hdr)
	link(entry_path, hdr_path);
	
	// Initialise the stack of path to process, with the first basic block
	nextBB_path = BBPath::getBBPath(nextBB);
	pair = new Pair<BBPath*, BBPath*>(hdr_path, nextBB_path);
	sequences.add(pair);
	
	// main loop
	while(!sequences.isEmpty()){
		pair = sequences.pop();
		father = pair->fst;
		cur_path = pair->snd;
		delete pair;
		
		Vector<BBPath*> *next_paths = cur_path->nexts();
		int l = next_paths->length();
		
		// iterate for each out edge
		for(int i = 0; i < l; i++){
			next_path = next_paths->get(i);
			nextBB = next_path->tail();
			
			if(isHeader(cur_path, next_path)){
				// N' = N' U cur_path
				if(!elts.contains(cur_path))
					elts.add(cur_path);
				
				// E' = E' U (father, cur_path)
				link(father, cur_path);
				
				hdr = headers.get(nextBB, 0);
				if(!hdr){
					hdr = new Header(nextBB);
					headers.put(nextBB, hdr);
					hdr_path = BBPath::getBBPath(hdr);
					
					// N' = N' U hdr
					elts.add(hdr_path);
					
					// Push this path for reuse
					nextBB_path = BBPath::getBBPath(nextBB);
					pair = new Pair<BBPath*, BBPath*>(hdr_path, nextBB_path);
					sequences.add(pair);
				}
				// E' = E' U (cur_path, hdr)
				hdr_path = BBPath::getBBPath(hdr);
				link(cur_path, hdr_path);
			} // end if isHeader
			else if(isBoundary(cur_path, next_path)){
				// N' = N' U cur_path
				if(!elts.contains(cur_path)){
					elts.add(cur_path);
				}
				
				// E' = E' U (father, cur_path)
				link(father, cur_path);
				
				if(nextBB->isExit()){
					link(cur_path, exit_path);
				}
				else {
					nextBB_path = BBPath::getBBPath(nextBB);
					pair = new Pair<BBPath*, BBPath*>(cur_path, nextBB_path);
					sequences.add(pair);
				}
			} // end if isBoundary
			else {
				pair = new Pair<BBPath*, BBPath*>(father, next_path);
				sequences.add(pair);
			} // end else
		} // end for
	} // end while
	elts.add(exit_path);
	_made = true;
}


/**
 * configure the AggregationGraph
 */
void AggregationGraph::configure(PropList &props){
}


/**
 * This function analyses if the first path need to be cutted, or if the
 * the algorithm can use the second path.
 * @return true if there is a boundary between the two given paths
 */
bool AggregationGraph::isBoundary(BBPath *cur_bbpath, BBPath *next_bbpath) {
	BasicBlock *next_bb = next_bbpath->tail();
	bool bound = next_bb->isExit();
	return bound;
}


/**
 * @return true if we need to put a header between the two given paths
 */
bool AggregationGraph::isHeader(BBPath *cur_bbpath, BBPath *next_bbpath){
	BasicBlock *bb = next_bbpath->tail();
	bool header = Dominance::isLoopHeader(bb);
	// ?idea? -> header = header || ID_Has_Header(bb);
	return header;
}

/**
 * Link the two paths: Put an edge between them and stores the edge
 */
void AggregationGraph::link(BBPath *src, BBPath *dst){
	int l = edges.length();
	for(int i = 0; i < l; i++){
		Pair<BBPath*, BBPath*> &tmp = *edges[i];
		// compare only addresses, because there is only 1 instance of every BBPath
		if(tmp.fst == src && tmp.snd == dst)
			return;
	}
	edges.add(new Pair<BBPath*, BBPath*>(src, dst));
}




/**
 * Creates the list of basic blocks. Initialize the CFG
 */
void AggregationGraph::scan(){
	makeGraph();
	
	HashTable<void*, BasicBlock*> basic_blocks; // <BBPath*, SequenceBasicBlock*>
	BBPath *entry_path;
	BBPath *exit_path;
	int l;
	int nb_paths;
	int sum_length;
	
	// stats. Initialise statistics
	length_of_longer_path = 0;
	sum_length = 0;
	nb_paths = 0;
	
	// Initialise Entry and Exit
	entry_path = BBPath::getBBPath(&_entry);
	exit_path  = BBPath::getBBPath(&_exit);
	
	// Add Basic blocks to _bbs
	l = elts.length();
	for(int i = 0; i < l; i++){
		BBPath *path;
		BasicBlock *bb;
		
		path = elts[i];
		
		// stats
		int path_length = path->length(); 
		if(path_length > length_of_longer_path)
			length_of_longer_path = path_length;
		
		if(path == entry_path){
			bb = &_entry;
		}
		else if(path == exit_path){
			bb = &_exit;
		}
		else {
			bb = new SequenceBasicBlock(&path->bbs());
			
			// stats
			// If it is only a loop header, don't calc mean length
			if(!dynamic_cast<Header*>(path->head())){
				sum_length += path_length;
				nb_paths++;
			}
		}
		basic_blocks.put(path, bb);
		_bbs.add(bb);
	}
	
	// stats
	mean_length = (double)sum_length / nb_paths;
	
	// Add Edges
	l = edges.length();
	for(int i = 0; i < l; i++){
		BBPath *path_src;
		BBPath *path_dest;
		BasicBlock *bb_src;
		BasicBlock *bb_dest;
		Edge::kind_t kind;
		
		path_src  = edges[i]->fst;
		path_dest = edges[i]->snd;
		bb_src  = basic_blocks.get(path_src);
		bb_dest = basic_blocks.get(path_dest);
		
		kind = Edge::VIRTUAL;
		new Edge(bb_src, bb_dest, kind);
	}
	
	flags |= FLAG_Scanned;
	
	// number the basic blocks
	numberBB();
}




/**
 * Creates a name for the given BBPath
 */
String AggregationGraph::pathName(BBPath *path){
	if(path->length() == 1)
		return bbName(path->head());
	StringBuffer buf;
	buf << '[';
	IteratorInst<BasicBlock*> *iter = path->bbs().visit();
	bool first = true;
	while(!iter->ended()){
		if(first)
			first = false;
		else
			buf << ", ";
		buf << 'x' << iter->item()->number();
		iter->next();
	}
	buf << ']';
	return buf.toString();
}




/**
 * Creates an explicit name for the fiven BasicBlock
 * that can be an Entry, Exit, Header, SequenceBasicBlock,
 * or a simple BasicBlock.
 */
String AggregationGraph::bbName(BasicBlock *bb){
	if(bb->isEntry())
		return "Entry";
	if(bb->isExit())
		return "Exit";
	Header *hdr = dynamic_cast<Header*>(bb); 
	if(hdr){
		StringBuffer buf;
		//buf << "Header_of_";
		buf << '*';
		buf << hdr->child()->number(); 
		return buf.toString();
	}
	SequenceBasicBlock *sbb = dynamic_cast<SequenceBasicBlock*>(bb);
	if(sbb){
		return pathName(sbb->getBBPath());
	}
	StringBuffer buf;
	buf << 'x';
	buf << bb->number();
	return buf.toString();
}




/**
 * Dump to the given output the correspondance between var names
 * of the ILP system, and the path drawn in the Dot graph
 */
void AggregationGraph::printEquivalents(elm::io::Output &out){
	makeGraph();
	for(int i = 0; i < _bbs.length(); i++){
		out << 'x' << _bbs[i]->number() << " = " << bbName(_bbs[i]) << '\n';
	}
}




/**
 * Dumps some stats to the given output
 */
void AggregationGraph::printStats(elm::io::Output &out){
	makeGraph();
	out << elts.length()
		<< " nodes and "
		<< edges.length()
		<< " edges are created.\n";
	
	out << "Source CFG have " << cfg->bbs().count() << " nodes.\n";
	
	int instructions_to_simul = 0;
	for(int i = 0; i < elts.length(); i++)
		instructions_to_simul += elts[i]->countInsts();
	out << "If simulator state doesn't support cloning, "
		<< instructions_to_simul
		<< " instructions will be simulated.\n";
	
	out << "Actually, "
		<< BBPath::instructions_simulated
		<< " instructions have been simulated.\n";
	
	out << "The longer path contains "
		<< length_of_longer_path
		<< " basic blocks\n";
	
	out << "The mean length is " << mean_length << '\n';
	
}




/**
 * Dumps the dot data to the given output, in order to create a dot graph
 */
void AggregationGraph::toDot(io::Output &out){
	makeGraph();
	out << "digraph " << cfg->label() << "{\n";
	int l = edges.length();
	for(int i = 0; i < l; i++){
		Pair<BBPath*, BBPath*> *pair = edges[i];
		BBPath *src = pair->fst;
		BBPath *dst = pair->snd;
		BBPath *tst = src->sub(1,1);
		out << "    \"" << pathName(src) << "\" -> \"" << pathName(dst) << "\";\n";
	}
	out << "}\n";
	return; 
}

} } // otawa::ipet
