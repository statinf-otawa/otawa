/*
 *	StepGraphBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2019, IRIT UPS.
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

#include <otawa/etime/StepGraphBuilder.h>

namespace otawa { namespace etime {


/**
 * @class StepGraphBuilder
 * X-graph builder using the Inst::execute() function of instructions.
 * In the opposite to StandardXGraphBuilder, the use of execution steps
 * is much more flexible and allow to model more complex pipelines.
 *
 * @ingroup etime
 */


///
StepGraphBuilder::~StepGraphBuilder() {
}


///
StepGraphBuilder::StepGraphBuilder(Monitor& mon): XGraphBuilder(mon) {
}


///
void StepGraphBuilder::nextNode(ParExeInst *inst, const hard::PipelineUnit *u) {
	prev = node;
	stage = &stages[u->index()];
	node = factory()->makeNode(graph, inst, stage->stage());
	inst->addNode(node);			// HUX: really useful?
	stage->stage()->addNode(node);	// HUX: really useful?*/
	last_node = node;
}


///
void StepGraphBuilder::managePipelineOrder() {
	static string pipeline_order_label("pipeline order");
	if(prev != nullptr)
		factory()->makeEdge(prev, node, ParExeEdge::SOLID, 0, comment(pipeline_order_label));
}


///
void StepGraphBuilder::manageFetchOrder() {
	static string cache_trans_label = "cache", cache_inter_label = "line", branch_label = "branch";
	static string in_order_label = "in-order";

	if(stage->get() == nullptr)
		first_in_line = node;
	else {
		ParExeNode *last = stage->get();

		// branch case
		if(last->inst()->inst()->topAddress() != node->inst()->inst()->address()) {
			factory()->makeEdge(last_branch, node, ParExeEdge::SOLID, 0, comment(branch_label));
			if(_icache_shift != 0)
				factory()->makeEdge(first_in_line, node, ParExeEdge::SOLID, 0, comment(cache_inter_label));
		}

		// no branch
		else {
			int p_line = last->inst()->inst()->address().offset() >> _icache_shift;
			int c_line = node->inst()->inst()->address().offset() >> _icache_shift;
			if(p_line == c_line)
				factory()->makeEdge(last, node, ParExeEdge::SLASHED);
			else {
				factory()->makeEdge(first_in_line, node, ParExeEdge::SOLID, 0, comment(cache_trans_label));
				if(last != first_in_line)
					factory()->makeEdge(last, node, ParExeEdge::SOLID, 0, comment(_icache_shift == 0 ? in_order_label : cache_inter_label));
				first_in_line = node;
			}
		}
	}
}


///
void StepGraphBuilder::manageInOrder() {
	static string program_order_label("program order");
	if(stage->get() != nullptr) {
		if(stage->stage()->width() == 1)
			factory()->makeEdge(stage->get(), node, ParExeEdge::SOLID, 0, program_order_label);
		else {
			factory()->makeEdge(stage->get(), node, ParExeEdge::SLASHED, 0);
			if(stage->last() != nullptr)
				factory()->makeEdge(stage->last(), node, ParExeEdge::SLASHED, 0);
		}
	}
	stage->put(node);
}


///
void StepGraphBuilder::manageBranch() {
	last_branch = node;
}


/*
 * REMARKS: concerning data dependencies
 * * internal renaming is assumed: only read-after-write produces edges,
 * 	 neither write-after-write, nor write-after-read
 * * externally dependency are not constrained -- but constrained to what?
 * * register produced by memory accesses are handled in the memory functions
 */

///
void StepGraphBuilder::manageRead(const hard::Register *r) {
	if(prods[r->id()] != nullptr) {
		static string label = "read-after-write";
		factory()->makeEdge(prods[r->id()], node, ParExeEdge::SOLID, 0, label);
	}
	else {
		RegResource *res = rres[r->id()];
		if(res == nullptr) {
			res = graph->newRegResource(r);
			rres[r->id()] = res;
		}
		res->addUsingInst(node->inst());
	}
}


///
void StepGraphBuilder::manageWrite(const hard::Register *r) {
	prods[r->id()] = node;
}


///
void StepGraphBuilder::manageLoadIssue(bool cached) {
	static string memory_order = "memory order";
	if(prev_store != nullptr)
		factory()->makeEdge(prev_store, node, ParExeEdge::SOLID, 0, memory_order);
	prev_load = node;
}


///
void StepGraphBuilder::manageStoreIssue(bool cached) {
	static string memory_order = "memory order";
	if(prev_store != nullptr)
		factory()->makeEdge(prev_store, node, ParExeEdge::SOLID, 0, memory_order);
	if(prev_load != nullptr)
		factory()->makeEdge(prev_load, node, ParExeEdge::SOLID, 0, memory_order);
	prev_store = node;
}


///
void StepGraphBuilder::manageLoadWait() {
	prev_load = node;
}


///
void StepGraphBuilder::manageStoreWait() {
	prev_store = node;
}


///
void StepGraphBuilder::manageResourceUse(const hard::Queue *q) {
	ParExeNode *last = queues[q->index()].last();
	if(last != nullptr)
		factory()->makeEdge(last, node, ParExeEdge::SOLID, 0, q->getName());
}


//
void StepGraphBuilder::manageResourceRelease(const hard::Queue *q) {
	queues[q->index()].put(node);
}


///
ParExeGraph *StepGraphBuilder::build(ParExeSequence *seq) {

	// build the graph
	PropList props;
	graph = factory()->make(processor(), &resources(), seq);

	// populate the graph
	reset();
	Vector<hard::Step> steps;
    for(ParExeGraph::InstIterator inst(seq); inst(); inst++)  {
    	steps.clear();
    	_proc->processor()->execute(inst->inst(), steps);
    	prev = nullptr;
    	for(const auto s: steps) {
    		switch(s.kind()) {

    		case hard::Step::STAGE:
    			nextNode(*inst, s.stage());
    			managePipelineOrder();
	    		if(stage->stage() == _proc->fetchStage())
	    			manageFetchOrder();
	    		else if(stage->stage()->orderPolicy() == ParExeStage::IN_ORDER)
	    			manageInOrder();
    			break;

    		case hard::Step::READ:
    			manageRead(s.reg());
    			break;

    		case hard::Step::WRITE:
    			manageWrite(s.reg());
    			break;

    		case hard::Step::USE:
    			manageResourceUse(s.queue());
    			break;

    		case hard::Step::RELEASE:
    			manageResourceRelease(s.queue());
    			break;

    		case hard::Step::BRANCH:
    			manageBranch();
    			break;

    		case hard::Step::ISSUE_MEM:
				if(s.isLoad())
					manageLoadIssue(s.isCached());
				else
					manageStoreIssue(s.isCached());
				break;

    		case hard::Step::WAIT_MEM:
				if(s.isLoad())
					manageLoadWait();
				else
					manageStoreWait();
				break;

    		case hard::Step::WAIT:
    			node->setDefaultLatency(s.delay());
    			break;

			case hard::Step::NONE:
				break;

    		default:
    			ASSERT(false);
    		}
    	}
    }

	// complete and return graph
	graph->setLastNode(last_node);
	return graph;
}

void StepGraphBuilder::reset() {

	// new workspace?
	if(_ws != workspace()) {
		_ws = workspace();
		const hard::CacheConfiguration *cache = hard::CACHE_CONFIGURATION_FEATURE.get(_ws);
		if (cache && cache->instCache())
			_icache_shift = cache->instCache()->blockBits();
		else
			_icache_shift = 0;
		prods.setLength(_ws->process()->platform()->regCount());
		rres.setLength(_ws->process()->platform()->regCount());
	}

	// new processor?
	if(_proc != processor()) {
		_proc = processor();

		// record stages
		stages.setLength(_proc->processor()->unitCount());
		for(ParExePipeline::StageIterator s(_proc->pipeline()); s(); s++) {
			stages[s->unit()->index()].configure(*s);
			if(s->category() == ParExeStage::EXECUTE) {
				for(int i = 0; i < s->numFus(); i++) {
					ASSERTP(
						   s->unit()->getLatency() == 1
						|| !static_cast<const hard::FunctionalUnit *>(s->unit())->isPipelined(),
						"pipelined multi-cycle FU is deprecated");
					auto fup = s->fu(i);
					for(ParExePipeline::StageIterator fs(fup); fs(); fs++)
						stages[fs->unit()->index()].configure(*fs);
				}
			}
		}
		queues.setLength(_proc->processor()->queueCount());
		for(int i = 0; i < queues.count(); i++)
			queues[i].configure(_proc->queue(i));
	}

	// clear resources
	last_node = nullptr,
	last_branch = nullptr,
	first_in_line = nullptr,
	prev = nullptr;
	node = nullptr;
	prev_load = nullptr;
	prev_store = nullptr;
	stage = nullptr;
	for(int i = 0; i < stages.count(); i++)
		stages[i].reset();
	for(int i = 0; i < queues.count(); i++)
		queues[i].reset();
	for(int i = 0; i < prods.count(); i++) {
		prods[i] = nullptr;
		rres[i] = nullptr;
	}
}

///
StepGraphBuilder::CircularQueue::CircularQueue(const CircularQueue& q) {
	i = q.i;
	s = q.s;
	if(q.ns != nullptr) {
		ns = new ParExeNode *[s];
		array::copy(ns, q.ns, s);
	}
}


///
void StepGraphBuilder::CircularQueue::configure(int size) {
	s = size;
	ns = new ParExeNode *[s];
	reset();
}

///
void StepGraphBuilder::CircularQueue::reset() {
	i = 0;
	array::set(ns, s, static_cast<ParExeNode *>(nullptr));
}

///
StepGraphBuilder::Stage::Stage() {
}

///
StepGraphBuilder::Stage::Stage(const Stage& s): CircularQueue(s) {
	st = s.st;
}

///
void StepGraphBuilder::Stage::configure(ParExeStage *st) {
	this->st = st;
	CircularQueue::configure(st->width());
}

///
StepGraphBuilder::ProcQueue::ProcQueue() {
}

///
StepGraphBuilder::ProcQueue::ProcQueue(const ProcQueue& q): CircularQueue(q) {
	pq = q.pq;
}

///
void StepGraphBuilder::ProcQueue::configure(ParExeQueue *pq) {
	this->pq = pq;
	CircularQueue::configure(pq->size());
}

} }		// otawa::etime
