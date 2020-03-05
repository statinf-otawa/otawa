/*
 *	StandardBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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

#include <otawa/etime/StandardXGraphBuilder.h>

namespace otawa { namespace etime {

/**
 * @class XGraphBuilder
 * TODO
 * @ingroup etime
 */


XGraphBuilder::XGraphBuilder(Monitor& mon): Monitor(mon), _atb(nullptr) {
}


/**
 */
XGraphBuilder::~XGraphBuilder(void) {
}

/**
 * Get the current processor description.
 * @return	Processor description.
 */
ParExeProc *XGraphBuilder::processor(void) const {
	return _atb->_proc;
}

/**
 * Get the resources for the current processor.
 * @return	Processor resources.
 */
resources_t& XGraphBuilder::resources(void) const {
	return _atb->_resources;
}

/**
 * Get the current factory to build the graph.
 * @return	Current factory.
 */
Factory *XGraphBuilder::factory(void) const {
	return _atb->solver()->getFactory();
}

/**
 * Test if explicit node and edge naming is required.
 * @return	True if explicit naming is required, false else.
 */
bool XGraphBuilder::isExplicit(void) const {
	return _atb->_explicit;
}


/**
 * Called with the same configuration properties passed
 * to AbstractTimebuilder to let the builder configure
 * itself. Override this function to customize the
 * configuration.
 * @param props	Configuration properties.
 */
void XGraphBuilder::configure(const PropList& props) {
}


/**
 * Find a stage or a functional unit by its name.
 * @param name	Name of looked stage.
 * @return		Found stage or null pointer.
 */
ParExeStage *XGraphBuilder::findStage(cstring name) const {
	for(ParExePipeline::StageIterator s(processor()->pipeline()); s(); s++) {
		if(s->name() == name)
			return *s;
		if(s->category() == ParExeStage::EXECUTE)
			for(int i = 0; i < s->numFus(); i++)
				for(auto ss(s->fu(i)->stages()); ss(); ss++)
					if(ss->name() == name)
						return *ss;
	}
	return nullptr;
}


/**
 * @fn ParExeGraph *XGraphBuilder::build(ParExeSequence *seq);
 * This function is used to build a ParExeGraph from a sequence of
 * instructions. It will use fhe factory provided by XGraphSolver from
 * AbstractTimeBuilder it is used with.
 *
 * This function is usually overridden to specialize the way ParExeGraph
 * are built. Although there is now way to enforce this in C++, it is
 * advised to use the provided factory.
 *
 * @param seq	Sequence to build graph for.
 */


/**
 * @class StandardXGraphBuilder
 * ParExeGraph builder using the standard resources and microprocessor
 * description to build the graph.
 *
 * @ingroup etime
 */

///
StandardXGraphBuilder::StandardXGraphBuilder(Monitor& mon): XGraphBuilder(mon) {
}


///
ParExeGraph *StandardXGraphBuilder::build(ParExeSequence *seq) {

	// build the graph
	PropList props;
	ParExeGraph *g = factory()->make(processor(), &resources(), seq);

	// populate it
	init();
	g->createSequenceResources();
	createNodes(g, seq);
	addEdgesForPipelineOrder(g, seq);
	addEdgesForFetch();
	addEdgesForProgramOrder();
	addEdgesForMemoryOrder();
	addEdgesForDataDependencies();
	addEdgesForQueues();
	return g;
}


/**
 * TODO
 */
ParExeNode *StandardXGraphBuilder::makeNode(ParExeGraph *g, ParExeInst *i, ParExeStage *stage) {
	return factory()->makeNode(g, i, stage);
}


/**
 * TODO
 */
ParExeEdge *StandardXGraphBuilder::makeEdge(ParExeNode *src, ParExeNode *snk,
ParExeEdge::edge_type_t_t type, int latency, string name) {
	return factory()->makeEdge(src, snk, type, latency, name);
}


/**
 * Creates nodes in the graph: one node for each (instruction/pipeline_stage) pair.
 * For the execution stage, creates as many nodes as stages in the pipeline of the required functional unit.
 * TODO
 */
void StandardXGraphBuilder::createNodes(ParExeGraph *g, ParExeSequence *seq) {
	ParExeNode *last_node = nullptr;

	ParExeNode *node;
	for(ParExeGraph::InstIterator inst(seq); inst(); inst++)  {
		for(ParExePipeline::StageIterator stage(processor()->pipeline()) ; stage() ; stage++) {

			// generic case
			if(stage->category() != ParExeStage::EXECUTE) {
				node = makeNode(g, *inst, *stage);
				// register the new node to the related instruction and pipeline stage
				inst->addNode(node);
				stage->addNode(node);
				last_node = node;
			}

			// EXECUTE stage => expand functional unit's pipeline
			else {

				// no FU
				if(stage->numFus() == 0) {
					ParExeNode *node = makeNode(g, *inst, *stage);
					inst->addNode(node);
					stage->addNode(node);
					inst->setFirstFUNode(node);
					inst->setLastFUNode(node);
				}

				// multiple FUs: select one
				else {
					ParExePipeline *fu = stage->findFU(inst->inst()->kind());
					ParExeNode *first = nullptr, *last = nullptr;
					ASSERTP(fu != nullptr,
						"cannot find FU for kind " << inst->inst()->getKind() << " at " << inst->inst()->address());
					for(ParExePipeline::StageIterator fu_stage(fu); fu_stage(); fu_stage++) {
						ParExeNode *fu_node = makeNode(g, *inst, *fu_stage);
						if (!first)
							first = fu_node;
						last = fu_node;
						inst->addNode(fu_node);
						fu_stage->addNode(fu_node);
					}
					inst->setFirstFUNode(first);
					inst->setLastFUNode(last);
					// !!HUX!! Ensuring in-order for execute stage
					if(stage->orderPolicy() == ParExeStage::IN_ORDER)
						stage->addNode(first);
				}
			}
		}
   }

	g->setLastNode(last_node);
}


/**
 * TODO
 */
void StandardXGraphBuilder::addEdgesForPipelineOrder(ParExeGraph *g, ParExeSequence *seq) {
	static string pipeline_order("pipeline order");
	for(ParExeGraph::InstIterator inst(seq) ; inst() ; inst++){
		ParExeNode *previous = NULL;
		for (ParExeInst::NodeIterator node(*inst); node() ; node++){
			if (previous)
				makeEdge(previous, *node, ParExeEdge::SOLID, 0, comment(pipeline_order));
			previous = *node;
		}
	}

}


/**
 * @fn string StandardXGraphBuilder::comment(string com);
 * TODO
 */


/**
 * TODO
 */
void StandardXGraphBuilder::addEdgesForFetch(void) {
	static string cache_trans_msg = "cache", cache_inter_msg = "line", branch_msg = "branch";
	static string in_order = "in-order";

	ParExeNode * first_cache_line_node = fetch_stage->firstNode();

	// compute current cache line
	Address current_cache_line = 0;
	if(_cache_line_size != 0)
		current_cache_line = first_cache_line_node->inst()->inst()->address().offset() / _cache_line_size;

	ParExeNode *previous = nullptr;
	for (ParExeStage::NodeIterator node(fetch_stage) ; node() ; node++) {
		if (previous){

			// branch case
			if (previous->inst()->inst()->topAddress() != node->inst()->inst()->address()) {

				// look for the branch stage node
				ParExeNode *branching_node = nullptr;
				for(auto node = previous->inst()->nodes(); node(); node++) {
					if(node->stage() == branch_stage) {
						branching_node = *node;
						break;
					}
				}
				ASSERT(branching_node != nullptr);

				// create the edges
				factory()->makeEdge(branching_node, *node, ParExeEdge::SOLID, 0, comment(branch_msg));
				if(_cache_line_size != 0)
					makeEdge(first_cache_line_node, *node, ParExeEdge::SOLID, 0, comment(cache_inter_msg));
			}

			// no branch
			else {

				// no cache
				if(_cache_line_size == 0) {
					if(previous != nullptr)
						makeEdge(previous, *node, ParExeEdge::SOLID, 0, comment(in_order));
				}

				// cache bound edges
				else {
					Address cache_line = node->inst()->inst()->address().offset() / _cache_line_size;
					if(cache_line != current_cache_line) {
						factory()->makeEdge(first_cache_line_node, *node, ParExeEdge::SOLID, 0, comment(cache_trans_msg));
						if(first_cache_line_node != previous)
							makeEdge(previous, *node, ParExeEdge::SOLID, 0, comment(cache_inter_msg));
						first_cache_line_node = *node;
						current_cache_line = cache_line;
					}
					else
						makeEdge(previous, *node, ParExeEdge::SLASHED);
				}
			}
		}
		previous = *node;
	}
}


/**
 * Adds edges to reflect processing of instruction in the order of the program.
  * @param list_of_stages	List of stages that process nodes in order
 * 							(if an empty pointer is passed, one is created containing the in-order-scheduled stages).
 */
void StandardXGraphBuilder::addEdgesForProgramOrder(void){
	static string program_order("program order");

	// select list of in-order stages
	List<ParExeStage *> *list = _proc->listOfInorderStages();

	// create the edges
	for(ParExeGraph::StageIterator stage(list) ; stage() ; stage++) {
		int count = 1;
		ParExeNode *previous = NULL;
		int prev_id = 0;
		for(ParExeStage::NodeIterator node(*stage); node(); node++){
			if(previous){
				if(stage->width() == 1)
					makeEdge(previous, *node, ParExeEdge::SOLID, 0, program_order);
				else {
					factory()->makeEdge(previous, *node, ParExeEdge::SLASHED, 0, stage->name());
					if (count == stage->width()){		// when stage width is reached, add edges to show precedence
						ParExeNode *not_at_the_same_cycle = stage->node(prev_id);
						makeEdge(not_at_the_same_cycle,*node,ParExeEdge::SOLID, 0, stage->name());
						prev_id++;
					}
					else
						count++;
				}
			}
			previous = *node;
		}
	}
}


/**
 * Adds edges to represent contention to access memory, basically, between FUs
 * of instructions performing memory access.
 */
void StandardXGraphBuilder::addEdgesForMemoryOrder(void) {
	static string memory_order = "memory order";

	// !!HUG!! Memory stage?
	ParExeStage *stage = _proc->execStage();

	// looking in turn each FU
	for(int i = 0; i< stage->numFus(); i++) {
		ParExeStage *fu_stage = stage->fu(i)->firstStage();
		ParExeNode * previous_load = nullptr;
		ParExeNode * previous_store = nullptr;

		// look for each node of this FU
		for(int j = 0; j < fu_stage->numNodes(); j++){
			ParExeNode *new_load = previous_load;
			ParExeNode *new_store = previous_store;
			ParExeNode *node = fu_stage->node(j);

			// found a load instruction
			if(node->inst()->inst()->isLoad()) {
				new_load = node;

				// if any, dependency on previous store
				if(previous_store)
					makeEdge(previous_store, node, ParExeEdge::SOLID, 0, memory_order);

				// current node becomes the new previous load
				for(ParExeGraph::InstNodeIterator last_node(node->inst()); last_node(); last_node++)
					if (last_node->stage()->category() == ParExeStage::FU)
						previous_load = *last_node;
			}

			// found a store instruction
			if(node->inst()->inst()->isStore()) {
				new_store = node;

				// if any, dependency on previous store
				if(previous_store)
					makeEdge(previous_store, node, ParExeEdge::SOLID, 0, memory_order);

				// if any, dependency on previous load
				if(previous_load)
					makeEdge(previous_load, node, ParExeEdge::SOLID, 0, memory_order);

				// current node becomes the new previous store
				for(ParExeGraph::InstNodeIterator last_node(node->inst()); last_node() ; last_node++)
					if (last_node->stage()->category() == ParExeStage::FU)
						previous_store = *last_node;
			}

			// update previous load and store
			previous_load = new_load;
			previous_store = new_store;
		}
	}
}


/**
 * Adds edges for data dependencies, that is, if an instruction (a)
 * produces content of a register and instruction (b) uses this register value
 * create a solid edge between their execute stages.
 */
void StandardXGraphBuilder::addEdgesForDataDependencies(void){
	static string data = "data";
	ParExeStage *exec_stage = _proc->execStage();

	// for each functional unit
	for(int j = 0; j < exec_stage->numFus(); j++) {
		ParExeStage *fu_stage = exec_stage->fu(j)->firstStage();

		// for each stage in the functional unit
		for (int k=0; k < fu_stage->numNodes(); k++) {
			ParExeNode *node = fu_stage->node(k);
			ParExeInst *inst = node->inst();

			// for each instruction producing a used data
			for(ParExeInst::ProducingInstIterator prod(inst); prod(); prod ++){

				// create the edge
				ParExeNode *producing_node;
				if(!prod->inst()->isLoad())
					producing_node = prod->lastFUNode();
				else {
					producing_node = nullptr;
					for(auto node = prod->nodes(); node(); node++)
						if(node->stage() == _proc->memStage())
							producing_node = *node;
					// TODO		propagate fix to parexegraph
					if(producing_node == nullptr)
						producing_node = prod->lastFUNode();
				}
				makeEdge(producing_node, node, ParExeEdge::SOLID, 0, comment(data));
			}
		}
	}
}

/**
 * Called to add edges representing contention on the different
 * queues of the microprocessor.
 */
void StandardXGraphBuilder::addEdgesForQueues(void){
	for (ParExePipeline::StageIterator stage(_proc->pipeline()) ; stage() ; stage++) {
		ParExeStage * prod_stage;
		if (stage->sourceQueue() != NULL) {
			ParExeQueue *queue = stage->sourceQueue();
			int size = queue->size();
			prod_stage = queue->fillingStage();
			for (int i=0 ; i<stage->numNodes() - size ; i++) {
				ASSERT(i+size < prod_stage->numNodes());
				makeEdge(stage->node(i), prod_stage->node(i + size), ParExeEdge::SLASHED, 0, queue->name());
			}
		}
	}
}

/**
 * Prepare data useful to build the graph.
 */
void StandardXGraphBuilder::init(void) {

	// initialize cache size
	if(_ws != workspace()) {
		_ws = workspace();
		const hard::CacheConfiguration *cache = hard::CACHE_CONFIGURATION_FEATURE.get(_ws);
		if (cache && cache->instCache())
			_cache_line_size = cache->instCache()->blockSize();
		else
			_cache_line_size = 0;
	}

	// initialize stages
	if(_proc != processor()) {
		_proc = processor();
		fetch_stage = _proc->fetchStage();
		ASSERTP(fetch_stage != nullptr, "no fetch stage");
		branch_stage = _proc->branchStage();
		ASSERTP(branch_stage != nullptr, "no stage designed for banching");
	}
}


/**
 * TODO
 */
XGraphBuilder *XGraphBuilder::make(Monitor& mon) {
	return new StandardXGraphBuilder(mon);
}

} }	// otawa::etime

