/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ipet_TrivialBBTime.cpp -- TrivialBBTime class implementation.
 */

#include <stdlib.h>
#include <otawa/otawa.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ipet/IPET.h>
#include <elm/genstruct/Vector.h>
#include <otawa/exegraph/ExecutionGraph.h>
#include <elm/genstruct/Table.h>
#include <elm/genstruct/DLList.h>
#include <otawa/hard/Register.h>
#include <otawa/exegraph/ExeGraphBBTime.h>

using namespace otawa;
using namespace otawa::hard;
using namespace elm;
using namespace elm::genstruct;
using namespace otawa::graph;
using namespace otawa::ipet;

namespace otawa { 

// ---------------------------------------------------
// buildPrologueList
// ---------------------------------------------------

void ExeGraphBBTime::buildPrologueList(BlockSequence *sequence, 
									int capacity, 
									elm::genstruct::DLList<BlockSequence *>& prologue_list) {
	BasicBlock *pred;
	int offset;
	BasicBlock *bb = sequence->first();
	
	// prologues are recursively built by considering preceeding nodes
	for(BasicBlock::InIterator edge(bb); edge; edge++) {
		pred = edge->source();
		if (pred->countInstructions() == 0) {
			// current sequence is terminated (not more instructions to add)
			// (end of recursive building)
			BlockSequence *new_sequence = new BlockSequence(sequence);
			prologue_list.addLast(new_sequence);
			continue;
		}	
		// build new sequence from pred
		offset = pred->countInstructions() - (capacity - sequence->instCount());
		if (offset < 0)
			offset = 0;
		BlockSequence *new_sequence = new BlockSequence(sequence);
		new_sequence->addBlockFirst(pred,offset);
		if (new_sequence->instCount() != capacity)
			buildPrologueList(new_sequence, capacity, prologue_list);
		else {
			prologue_list.addLast(new_sequence);
		}
	}
	delete sequence;
	
}

// ---------------------------------------------------
// buildEpilogueList
// ---------------------------------------------------

void ExeGraphBBTime::buildEpilogueList(BlockSequence *sequence, 
									int capacity, 
									elm::genstruct::DLList<BlockSequence *>& epilogue_list) {
	BasicBlock *succ;
	int offset;
	BasicBlock *bb = sequence->last();
	
	
	for(BasicBlock::OutIterator edge(bb); edge; edge++) {
		succ = edge->target();
		if (succ->countInstructions() == 0) {
			BlockSequence *new_sequence = new BlockSequence(sequence);
			epilogue_list.addLast(new_sequence);
			continue;
		}	
		offset = succ->countInstructions() - (capacity - sequence->instCount());
		if (offset < 0)
			offset = 0;
		BlockSequence *new_sequence = new BlockSequence(sequence);
		new_sequence->addBlockLast(succ,offset);
		if (new_sequence->instCount() != capacity)
			buildEpilogueList(new_sequence, capacity, epilogue_list);
		else {
			epilogue_list.addLast(new_sequence);
		}
	}
	delete sequence;
	
}


// ---------------------------------------------------
// processBB
// ---------------------------------------------------

void ExeGraphBBTime::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb) {
	elm::genstruct::DLList<Inst *> prologue, body, epilogue;
	elm::genstruct::DLList<BlockSequence *> prologue_list, epilogue_list;
	int offset;
	BasicBlock *pred, *succ;
	
	dumpFile << "================================================================\n";
	dumpFile << "Processing block b" << bb->number() << ":\n";	
	
	if (bb->countInstructions() == 0)
		return;

	// compute prologue/epilogue size
	int capacity = 0;
	for(Microprocessor::QueueIterator queue(microprocessor); queue; queue++){
		capacity += queue->size();
	}
	// build the list of possible prologues
	for(BasicBlock::InIterator edge(bb); edge; edge++) {
		pred = edge->source();
		if (pred->countInstructions() == 0)
			continue;
		offset = bb->countInstructions() - capacity;
		if (offset < 0)
			offset = 0;	
		BlockSequence *new_sequence = new BlockSequence();
		new_sequence->addBlockFirst(pred,offset);
		if (new_sequence->instCount() < capacity)
			buildPrologueList(new_sequence, capacity, prologue_list);
		else {
			prologue_list.addLast(new_sequence);
		}
	}
	// build the list of possible epilogues
	for(BasicBlock::OutIterator edge(bb); edge; edge++) {
		succ = edge->target();
		if (succ->countInstructions() == 0)
			continue;
		offset = bb->countInstructions() - capacity;
		if (offset < 0)
			offset = 0;	
		BlockSequence *new_sequence = new BlockSequence();
		new_sequence->addBlockLast(succ,offset);
		if (new_sequence->instCount() < capacity)
			buildEpilogueList(new_sequence, capacity, epilogue_list);
		else {
			epilogue_list.addLast(new_sequence);
		}

	}
	
	// build the list of body instructions	
	for(BasicBlock::InstIterator inst(bb); inst; inst++) {
		body.addLast(inst);
	}
	
	int maxExecTime = 0;
	int bbExecTime;
	
	// consider every possible prologue
	for (elm::genstruct::DLList<BlockSequence *>::Iterator seq1(prologue_list) ; seq1 ; seq1++) {
		// build the list of prologue instructions
		while (!prologue.isEmpty())
			prologue.removeFirst();
		for (elm::genstruct::DLList<BasicBlock *>::Iterator block(*(seq1->blockList())) ; block ; block++) {
			for(BasicBlock::InstIterator inst(block); inst; inst++) {
				prologue.addLast(inst);
			}	
		}
		
		// consider every possible epilogue
		for (elm::genstruct::DLList<BlockSequence *>::Iterator seq2(epilogue_list) ; seq2 ; seq2++) {
			// build the list of epilogue instructions
			while (!epilogue.isEmpty()) {
				epilogue.removeFirst();
			}
			for (elm::genstruct::DLList<BasicBlock *>::Iterator block(*(seq2->blockList())) ; block ; block++) {
				for(BasicBlock::InstIterator inst(block); inst; inst++) {
					epilogue.addLast(inst);
				}	
			}
			
			// set dump file
			/*elm::StringBuffer file_name;
			elm::String string_file_name, string_timed_file_name, extension, number, extension2;
			string_file_name = "./";
			string_timed_file_name = string_file_name;
			extension = ".dot";
			extension2 = "_times.dot";
			for (elm::genstruct::DLList<BasicBlock *>::Iterator block(*(seq1->blockList())) ; block ; block++) {
				file_name << block->number() << "-";
			}
			file_name << "--" << bb->number() << "---";
			for (elm::genstruct::DLList<BasicBlock *>::Iterator block(*(seq2->blockList())) ; block ; block++) {
				file_name << block->number() << "-";
			}
			number = file_name.toString();
			string_file_name = string_file_name.concat(number);
			string_file_name = string_file_name.concat(extension);
			elm::io::OutFileStream dotStream(string_file_name.toCString());
			elm::io::Output dotFile(dotStream);*/	
			
			// build the execution graph
			dumpFile << "\nBuilding execution graph for:\n\tprologue: ";
			seq1->dump(dumpFile);
			dumpFile << "\n\tbody: b" << bb->number()<< "\n\tepilogue: ";
			seq2->dump(dumpFile);
			dumpFile << "\n";
			ExecutionGraph execution_graph;
			execution_graph.build(fw, microprocessor, prologue, body, epilogue);
			execution_graph.dumpLight(dumpFile);
			// dump the execution graph in dot format
			//execution_graph.dotDump(dotFile,false);
			
			// shade prologue nodes that have a direct path to IF(I1)
			//execution_graph.shadeNodes(dumpFile);
			// compute ready/start.finish earliest and latest times
			bbExecTime = execution_graph.analyze(dumpFile);
			
			dumpFile << "Cost of block " << bb->number() << " is " << bbExecTime << "\n";
			/*string_timed_file_name = string_timed_file_name.concat(number);
			string_timed_file_name = string_timed_file_name.concat(extension2);
			elm::io::OutFileStream timedDotStream(string_timed_file_name.toCString());
			elm::io::Output timedDotFile(timedDotStream);*/	
			// dump the execution graph *with times* in dot format
			//execution_graph.dotDump(timedDotFile,true);		
			
		}
		// is the cost of the block for that prologue/epilogue pair the WCC of the block ?
		if (bbExecTime > maxExecTime)
			maxExecTime = bbExecTime;
	}
	
	dumpFile << "\nWCC of block " << bb->number() << " is " << maxExecTime << "\n";
	bb->set<int>(TIME, maxExecTime);

	// Free block sequences
	for(elm::genstruct::DLList<BlockSequence *>::Iterator bs(prologue_list); bs; bs++)
		delete bs;
	for(elm::genstruct::DLList<BlockSequence *>::Iterator bs(epilogue_list); bs; bs++)
		delete bs;
}


}  // otawa
