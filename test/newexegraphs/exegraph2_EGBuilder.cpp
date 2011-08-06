/*
 *	$Id$
 *	Implementation of the EGBuilder class.
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

#include "EGBuilder.h"

#define TRACE(x) x

using namespace otawa;
using namespace otawa::exegraph2;


EGBuilder::EGBuilder(WorkSpace * ws,
		EGNodeFactory *node_factory,
		EGEdgeFactory *edge_factory)
: _node_factory(node_factory), _edge_factory(edge_factory){
}

EGGenericBuilder::EGGenericBuilder(WorkSpace * ws,
		EGNodeFactory *node_factory,
		EGEdgeFactory *edge_factory)
	: EGBuilder(ws, node_factory, edge_factory)
{
	_ws = ws;

	// instruction cache?
	const otawa::hard::Cache *icache  = otawa::hard::CACHE_CONFIGURATION(ws)->instCache();
	_has_icache = (bool) icache;
	if (icache){
		_icache_line_size = icache->blockSize();
		_icache_hit_latency = 1;
		_icache_miss_latency = icache->missPenalty();
	}
	else {
		_has_icache = false;
	}
	if (_node_factory == NULL)
		_node_factory = new EGGenericNodeFactory();
	if (_edge_factory == NULL)
		_edge_factory = new EGGenericEdgeFactory();
	// data cache?
	// FIXME not implemented
	const otawa::hard::Cache *dcache = otawa::hard::CACHE_CONFIGURATION(ws)->dataCache();
	_has_dcache = false;
	// memory
	_memory = otawa::hard::MEMORY(ws);
	_has_memory = (bool) _memory;
	// branch predictor?
	// FIXME not implemented
	_branch_penalty = 2;
}

void EGGenericBuilder::build(ExecutionGraph *graph){
	_graph = graph;
	_inst_seq = _graph->instSeq();
	_proc = _graph->proc();
	createNodes();
	findDataDependencies();

	addEdgesForPipelineOrder();
	addEdgesForFetch();
	addEdgesForProgramOrder();

	addEdgesForMemoryOrder();
	addEdgesForDataDependencies();

	addEdgesForQueues();
}

// ----------------------------------------------------------------

void EGGenericBuilder::createNodes() {

    // consider every instruction
    for (EGInstSeq::InstIterator inst(_inst_seq) ; inst ; inst++)  {
		// consider every pipeline stage
		for (EGPipeline::StageIterator stage(_proc->pipeline()) ; stage ; stage++) {

			// create node
			EGNode *node;
			if (stage->category() != EGStage::EXECUTE) {
				node = _node_factory->newEGNode(_graph, stage, inst);
				inst->addNode(node);
				stage->addNode(node);
				if (stage->category() == EGStage::FETCH) {
					inst->setFetchNode(node);
					if (_has_icache){
						LBlock *lb = LBLOCK(inst->inst());
						if (lb){
							switch (CATEGORY(lb)){
							case ALWAYS_MISS:
								node->setLatency(_icache_miss_latency);
								TRACE(
										elm::cerr << "[createNodes] node " << node->name() << "is ALWAYS_MISS\n";
								)
								break;
							case ALWAYS_HIT:
								node->setLatency(_icache_hit_latency);
								TRACE(
										elm::cerr << "[createNodes] node " << node->name() << "is ALWAYS_HIT\n";
								)
								break;
							default:
								node->setLatency(_icache_miss_latency);
								_graph->addUnknownNode(node);
								TRACE(
										if (CATEGORY(lb) == FIRST_MISS)
											elm::cout << "[createNodes] adding unknown (FIRST_MISS) node " << node->name()
												<< " (header=b" << CATEGORY_HEADER(lb)->number() << ") {" << node << "}\n";
										else
											elm::cout << "[createNodes] adding unknown (NOT_CLASSIFIED) node " << node->name() << " {" << node << "}\n";
								)
								break;
							}
						}
					}
					else
						if (_has_memory)
							node->setLatency(_memory->get(inst->inst()->address())->latency());
				}
				if (!_graph->firstNode())
					_graph->setFirstNode(node);
				if (inst->part() == PREDECESSOR)  // FIXME
					_graph->setLastPredNode(node);
				if (!_graph->firstBlockNode() && (inst->part() == BLOCK) )
					_graph->setFirstBlockNode(node);
				_graph->setLastNode(node);
			}
			else {
				// add FU nodes
				EGPipeline *fu = stage->findFU(inst->inst()->kind());
				ASSERTP(fu, "cannot find FU for instruction " << inst->inst()->address() << " " << inst->inst());
				int index = 0;

				for(EGPipeline::StageIterator fu_stage(fu); fu_stage; fu_stage++) {
					EGNode *fu_node = _node_factory->newEGNode(_graph, fu_stage, inst);
					inst->addNode(fu_node);
					fu_stage->addNode(fu_node);
					if (index == 0)
						inst->setExecNode(fu_node);
					index++;
				}
				// if inst is a load, set the latency of the last exec node
				if (inst->inst()->isLoad()){
					//if (_has_dcache) // FIXME not supported yet
					//fu_node->setLatency(_dcache_hit_latency)
				}
			}

		} // endfor each pipeline stage

    } // endfor each instruction

}


// ----------------------------------------------------------------

void EGGenericBuilder::findDataDependencies() {

    otawa::hard::Platform *pf = _ws->platform();
    AllocatedTable<rename_table_t> rename_tables(pf->banks().count());
    int reg_bank_count = pf->banks().count();
    for(int i = 0; i <reg_bank_count ; i++) {
		rename_tables[i].reg_bank = (otawa::hard::RegBank *) pf->banks()[i];
		rename_tables[i].table =
			new AllocatedTable<EGNode *>(rename_tables[i].reg_bank->count());
		for (int j=0 ; j<rename_tables[i].reg_bank->count() ; j++)
			rename_tables[i].table->set(j,NULL);
    }

    // consider every instruction
    for (EGInstSeq::InstIterator inst(_inst_seq) ; inst ; inst++)  {
		EGNode *first_fu_node = NULL, *last_fu_node = NULL;
		for (EGInst::NodeIterator node(inst); node ; node++){
			if (node->stage()->category() == EGStage::FU){
				if (!first_fu_node)
					first_fu_node = node;
				last_fu_node = node;
			}
		}
		// check for data dependencies
		const elm::genstruct::Table<otawa::hard::Register *>& reads = first_fu_node->inst()->inst()->readRegs();
		for(int i = 0; i < reads.count(); i++) {
			for (int b=0 ; b<reg_bank_count ; b++) {
				if (rename_tables[b].reg_bank == reads[i]->bank()) {
					EGNode *producer = rename_tables[b].table->get(reads[i]->number());
					if (producer != NULL) {
						first_fu_node->addProducer(producer);
					}
				}
			}
		}
		// fu_node is the last FU node
		const elm::genstruct::Table<otawa::hard::Register *>& writes = last_fu_node->inst()->inst()->writtenRegs();
		for(int i = 0; i < writes.count(); i++) {
			for (int b=0 ; b<reg_bank_count ; b++) {
				if (rename_tables[b].reg_bank == writes[i]->bank()) {
					rename_tables[b].table->set(writes[i]->number(),last_fu_node);
				}
			}
		}

    } // endfor each instruction

    // Free rename tables
    for(int i = 0; i <reg_bank_count ; i++)
		delete rename_tables[i].table;
}

// ----------------------------------------------------------------
void EGGenericBuilder::addEdgesForPipelineOrder(){
    for (EGInstSeq::InstIterator inst(_inst_seq) ; inst ; inst++)  {
		for (int i=0 ; i<inst->numNodes()-1 ; i++){
			_edge_factory->newEGEdge(inst->node(i), inst->node(i+1), EGEdge::SOLID);
		}
    }
}


/**
 * Add edge for fetch blocking, that is, edges ensuring that instruction in the same
 * block are fetched in the same and that instructions in sequence owned by different blocks
 * require two fetches to be obtained.
 *
 * For example, for a block size of 16 with fixed size instructions of 4, the instruction
 * sequence is marked with fetches bounds:
 * @li start of basic block
 * @li 0x100C	fetch
 * @li 0x1010	fetch
 * @li 0x1014
 * @li 0x1018
 * @li 0x101C
 * @li 0x1010	fetch
 * @li 0x1014
 */
void EGGenericBuilder::addEdgesForFetch(){

	address_t current_cache_line, cache_line;

    EGStage *fetch_stage = _proc->fetchStage();

    // traverse all fetch nodes
    EGNode * first_cache_line_node = fetch_stage->firstNode();
    if (_has_icache)
    	current_cache_line =
    			fetch_stage->firstNode()->inst()->inst()->address().address() /  _icache_line_size;
    for(int i=0 ; i<fetch_stage->numNodes()-1 ; i++) {
		EGNode *node = fetch_stage->node(i);
		EGNode *next = fetch_stage->node(i+1);

		// taken banch ?
		if (node->inst()->inst()->topAddress() != next->inst()->inst()->address()){
			EGEdge * edge = _edge_factory->newEGEdge(node, next, EGEdge::SOLID, _branch_penalty);
			edge = _edge_factory->newEGEdge(first_cache_line_node, next, EGEdge::SOLID, _branch_penalty);
		}
		else {
			_edge_factory->newEGEdge(node, next, EGEdge::SLASHED);
		}

		// new cache line?
		if (_has_icache){
			cache_line = next->inst()->inst()->address().address() /  _icache_line_size;
			if ( cache_line != current_cache_line){
				_edge_factory->newEGEdge(first_cache_line_node, next, EGEdge::SOLID);
				_edge_factory->newEGEdge(node, next, EGEdge::SOLID);
				first_cache_line_node = next;
				current_cache_line = cache_line;
			}
		}
    }
}


// ----------------------------------------------------------------

void EGGenericBuilder::addEdgesForProgramOrder(elm::genstruct::SLList<EGStage *> *list_of_stages){

    elm::genstruct::SLList<EGStage *> *list;
    if (list_of_stages != NULL)
		list = list_of_stages;
    else {
		// if no list of stages was provided, built the default list that includes all IN_ORDER stages
		list = new  elm::genstruct::SLList<EGStage *>;
		for (EGPipeline::StageIterator stage(_proc->pipeline()) ; stage ; stage++) {
			if (stage->orderPolicy() == EGStage::IN_ORDER){
				if (stage->category() != EGStage::FETCH){
					list->add(stage);
				}
				if (stage->category() == EGStage::EXECUTE){
					for (int i=0 ; i<stage->numFus() ; i++){
						EGStage *fu_stage = stage->fu(i)->firstStage();
						if (fu_stage->hasNodes()){
							list->add(fu_stage);
						}
					}
				}
			}
		}
    }

    for (elm::genstruct::SLList<EGStage *>::Iterator stage(*list) ; stage ; stage++) {
		int count = 1;
		int prev = 0;
		for (int i=0 ; i<stage->numNodes()-1 ; i++){
			EGNode *node = stage->node(i);
			EGNode *next = stage->node(i+1);
			if (stage->width() == 1){
				_edge_factory->newEGEdge(node, next, EGEdge::SOLID);
			}
			else {
				new EGEdge(node, next, EGEdge::SLASHED);
				if (count == stage->width()){
					EGNode *previous = stage->node(prev);
					_edge_factory->newEGEdge(previous,next,EGEdge::SOLID);
					prev++;
				}
				else
					count++;
			}
		}
    }
}


// ----------------------------------------------------------------

void EGGenericBuilder::addEdgesForMemoryOrder(){

    EGStage *stage = _proc->execStage();
    for (int i=0 ; i<stage->numFus() ; i++) {
		EGStage *fu_stage = stage->fu(i)->firstStage();
		EGNode * previous_load = NULL;
		EGNode * previous_store = NULL;
		for (int j=0 ; j<fu_stage->numNodes() ; j++){
			EGNode *node = fu_stage->node(j);
			if (node->inst()->inst()->isLoad()) {
				if (previous_store) {// memory access are executed in order
					_edge_factory->newEGEdge(previous_store, node, EGEdge::SOLID);
				}
				for (EGInst::NodeIterator last_node(node->inst()); last_node ; last_node++){
					if (last_node->stage()->category() == EGStage::FU)
						previous_load = last_node;
				}
			}
			if (node->inst()->inst()->isStore()) {
				if (previous_store) {// memory access are executed in order
					_edge_factory->newEGEdge(previous_store, node, EGEdge::SOLID);
				}
				if (previous_load) {// memory access are executed in order
					_edge_factory->newEGEdge(previous_load, node, EGEdge::SOLID);
				}
				for (EGInst::NodeIterator last_node(node->inst()); last_node ; last_node++){
					if (last_node->stage()->category() == EGStage::FU){
						previous_store = last_node;
					}
				}
			}
		}
    }
}

// ----------------------------------------------------------------

/**
 * Add edges for data dependencies, that is, if an instruction (a)
 * produces content of a register and instruction (b) uses this register value
 * create a solid edge between their execute stages.
 */
void EGGenericBuilder::addEdgesForDataDependencies(){
    EGStage *exec_stage = _proc->execStage();
    for (int j=0 ; j<exec_stage->numFus() ; j++) {
		EGStage *fu_stage = exec_stage->fu(j)->firstStage();
		for (int k=0 ; k<fu_stage->numNodes() ; k++) {
			EGNode *node = fu_stage->node(k);
			for (int p=0 ; p<node->numProducers(); p++) {
				EGNode *producer = node->producer(p);
				_edge_factory->newEGEdge(producer, node, EGEdge::SOLID);
			}
		}
    }
}

// ----------------------------------------------------------------

void EGGenericBuilder::addEdgesForQueues(){

 /*   // build edges for queues with limited capacity */
    for (EGPipeline::StageIterator stage(_proc->pipeline()) ; stage ; stage++) {
		EGStage * prod_stage;
		if (stage->sourceQueue() != NULL) {
			EGQueue *queue = stage->sourceQueue();
			int size = queue->size();
			prod_stage = queue->fillingStage();
			for (int i=0 ; i<stage->numNodes() - size ; i++) {
				assert(i+size < prod_stage->numNodes());
				_edge_factory->newEGEdge(stage->node(i), prod_stage->node(i + size), EGEdge::SLASHED);
			}
		}
    }
}

