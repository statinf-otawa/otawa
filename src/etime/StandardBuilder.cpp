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

#include <otawa/etime/AbstractTimeBuilder.h>

namespace otawa { namespace etime {

/**
 * @class Builder
 * TODO
 * @ingroup etime
 */


Builder::Builder(const Monitor& mon)
:	Monitor(mon),
	_ws(nullptr),
	_processor(nullptr),
	_resources(nullptr),
	_factory(nullptr),
	_explicit(false)
{
}

/**
 */
Builder::~Builder(void) {
}

/**
 * @fn ParExeGraph *Builder::build(ParExeSequence *seq);
 * TODO
 */

/**
 * @fn ParExeProc *Builder::processor(void) const;
 * TODO
 */

/**
 * @fn resources_t *Builder::resources(void) const;
 * TODO
 */

/**
 * @fn Factory *Builder::factory(void) const;
 * TODO
 */

/**
 * @fn void Builder::setProcessor(ParExeProc *processor);
 * TODO
 */

/**
 * @fn void Builder::setResources(resources_t *resources);
 * TODO
 */

/**
 * @fn void Builder::setFactory(Factory *factory);
 * TODO
 */


/**
 * @class StandardBuilder
 * TODO
 */
class StandardBuilder: public Builder {
public:

	StandardBuilder(const Monitor& mon): Builder(mon) {
	}

	ParExeGraph *build(ParExeSequence *seq) override {
		ASSERT(workspace() != nullptr);
		ASSERT(processor() != nullptr);
		ASSERT(resources() != nullptr);
		ASSERT(factory() != nullptr);

		// build the graph
		PropList props;
		ParExeGraph *g = factory()->make(processor(), resources(), seq);

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

private:

	/**
	 * Creates nodes in the graph: one node for each (instruction/pipeline_stage) pair.
	 * For the execution stage, creates as many nodes as stages in the pipeline of the required functional unit.
	 * TODO
	 */
	void createNodes(ParExeGraph *g, ParExeSequence *seq) {
		ParExeNode *last_node = nullptr;

		ParExeNode *node;
	    for(ParExeGraph::InstIterator inst(seq); inst; inst++)  {
			for(ParExePipeline::StageIterator stage(processor()->pipeline()) ; stage ; stage++) {

				// generic case
				if(stage->category() != ParExeStage::EXECUTE) {
					node = factory()->makeNode(g, inst, stage);
					// register the new node to the related instruction and pipeline stage
					inst->addNode(node);
					stage->addNode(node);
					last_node = node;
				}

				// EXECUTE stage => expand functional unit's pipeline
				else {
					ParExePipeline *fu = stage->findFU(inst->inst()->kind());
					ParExeNode *first = nullptr, *last = nullptr;
					ASSERTP(fu != nullptr,
						"cannot find FU for kind " << inst->inst()->getKind() << " at " << inst->inst()->address());
					for(ParExePipeline::StageIterator fu_stage(fu); fu_stage; fu_stage++) {
						ParExeNode *fu_node = factory()->makeNode(g, inst, fu_stage);
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

	    g->setLastNode(last_node);
	}

	/**
	 * TODO
	 */
	void addEdgesForPipelineOrder(ParExeGraph *g, ParExeSequence *seq) {
		static string pipeline_order("pipeline order");
	    for(ParExeGraph::InstIterator inst(seq) ; inst ; inst++){
	    	ParExeNode *previous = NULL;
	    	for (ParExeInst::NodeIterator node(inst); node ; node++){
	    		if (previous)
	    			factory()->makeEdge(previous, node, ParExeEdge::SOLID, 0, comment(pipeline_order));
	    		previous = node;
	    	}
	    }

	}

	/**
	 * TODO
	 */
	inline string comment(string com) {
		if(isExplicit())
			return com;
		else
			return "";
	}

	/**
	 * TODO
	 */
	void addEdgesForFetch(void) {
		static string cache_trans_msg = "cache", cache_inter_msg = "line", branch_msg = "branch";
		static string in_order = "in-order";

		ParExeNode * first_cache_line_node = fetch_stage->firstNode();

		// compute current cache line
		Address current_cache_line = 0;
		if(_cache_line_size != 0)
			current_cache_line = first_cache_line_node->inst()->inst()->address().offset() / _cache_line_size;

		ParExeNode *previous = nullptr;
		for (ParExeStage::NodeIterator node(fetch_stage) ; node ; node++) {
			if (previous){

				// branch case
				if (previous->inst()->inst()->topAddress() != node->inst()->inst()->address()) {

					// look for the branch stage node
					ParExeNode *branching_node = nullptr;
					for(auto node = previous->inst()->nodes(); node; node++)
						if(node->stage() == branch_stage) {
							branching_node = node;
							break;
						}
					ASSERT(branching_node != nullptr);

					// create the edges
					factory()->makeEdge(branching_node, node, ParExeEdge::SOLID, 0, comment(branch_msg));
					if(_cache_line_size != 0)
						factory()->makeEdge(first_cache_line_node, node, ParExeEdge::SOLID, 0, comment(cache_inter_msg));
				}

				// no branch
				else {

					// no cache
					if(_cache_line_size == 0) {
						if(previous != nullptr)
							factory()->makeEdge(previous, node, ParExeEdge::SOLID, 0, comment(in_order));
					}

					// cache bound edges
					else {
						Address cache_line = node->inst()->inst()->address().offset() / _cache_line_size;
						if(cache_line != current_cache_line) {
							factory()->makeEdge(first_cache_line_node, node, ParExeEdge::SOLID, 0, comment(cache_trans_msg));
							if(first_cache_line_node != previous)
								factory()->makeEdge(previous, node, ParExeEdge::SOLID, 0, comment(cache_inter_msg));
							first_cache_line_node = node;
							current_cache_line = cache_line;
						}
						else
							factory()->makeEdge(previous, node, ParExeEdge::SLASHED);
					}
				}
			}
			previous = node;
		}
	}

	/**
	 * Adds edges to reflect processing of instruction in the order of the program.
	  * @param list_of_stages	List of stages that process nodes in order
	 * 							(if an empty pointer is passed, one is created containing the in-order-scheduled stages).
	 */
	void addEdgesForProgramOrder(void){
		static string program_order("program order");

		// select list of in-order stages
		elm::genstruct::SLList<ParExeStage *> *list = _proc->listOfInorderStages();

		// create the edges
		for(ParExeGraph::StageIterator stage(list) ; stage ; stage++) {
			int count = 1;
			ParExeNode *previous = NULL;
			int prev_id = 0;
			for(ParExeStage::NodeIterator node(stage); node; node++){
				if(previous){
					if(stage->width() == 1)
						factory()->makeEdge(previous, node, ParExeEdge::SOLID, 0, program_order);
					else {
						factory()->makeEdge(previous, node, ParExeEdge::SLASHED, 0, stage->name());
						if (count == stage->width()){		// when stage width is reached, add edges to show precedence
							ParExeNode *not_at_the_same_cycle = stage->node(prev_id);
							factory()->makeEdge(not_at_the_same_cycle,node,ParExeEdge::SOLID, 0, stage->name());
							prev_id++;
						}
						else
							count++;
					}
				}
				previous = node;
			}
		}
	}


	/**
	 * Adds edges to represent contention to access memory, basically, between FUs
	 * of instructions performing memory access.
	 */
	void addEdgesForMemoryOrder(void) {
		static string memory_order = "memory order";

	    ParExeStage *stage = _proc->execStage();

	    // looking in turn each FU
	    for (int i=0 ; i<stage->numFus() ; i++) {
			ParExeStage *fu_stage = stage->fu(i)->firstStage();
			ParExeNode * previous_load = NULL;
			ParExeNode * previous_store = NULL;

			// look for each node of this FU
			for (int j=0 ; j<fu_stage->numNodes() ; j++){
				ParExeNode *node = fu_stage->node(j);

				// found a load instruction
				if (node->inst()->inst()->isLoad()) {

					// if any, dependency on previous store
					if(previous_store)
						factory()->makeEdge(previous_store, node, ParExeEdge::SOLID, 0, memory_order);

					// current node becomes the new previous load
					for(ParExeGraph::InstNodeIterator last_node(node->inst()); last_node ; last_node++)
						if (last_node->stage()->category() == ParExeStage::FU)
							previous_load = last_node;
				}

				// found a store instruction
				if (node->inst()->inst()->isStore()) {

					// if any, dependency on previous store
					if (previous_store)
						factory()->makeEdge(previous_store, node, ParExeEdge::SOLID, 0, memory_order);

					// if any, dependency on previous load
					if (previous_load)
						factory()->makeEdge(previous_load, node, ParExeEdge::SOLID, 0, memory_order);

					// current node becomes the new previous store
					for(ParExeGraph::InstNodeIterator last_node(node->inst()); last_node ; last_node++)
						if (last_node->stage()->category() == ParExeStage::FU)
							previous_store = last_node;
				}
			}
	    }
	}

	/**
	 * Adds edges for data dependencies, that is, if an instruction (a)
	 * produces content of a register and instruction (b) uses this register value
	 * create a solid edge between their execute stages.
	 */
	void addEdgesForDataDependencies(void){
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
				for(ParExeInst::ProducingInstIterator prod(inst); prod; prod ++){

					// create the edge
					ParExeNode *producing_node;
					if(!prod->inst()->isLoad())
						producing_node = prod->lastFUNode();
					else {
						producing_node = nullptr;
						for(auto node = prod->nodes(); node; node++)
							if(node->stage() == _proc->memStage())
								producing_node = node;
						// TODO		propagate fix to parexegraph
						if(producing_node == nullptr)
							producing_node = prod->lastFUNode();
					}
					factory()->makeEdge(producing_node, node, ParExeEdge::SOLID, 0, comment(data));
				}
			}
		}
	}

	/**
	 * Called to add edges representing contention on the different
	 * queues of the microprocessor.
	 */
	void addEdgesForQueues(void){
	    for (ParExePipeline::StageIterator stage(_proc->pipeline()) ; stage ; stage++) {
			ParExeStage * prod_stage;
			if (stage->sourceQueue() != NULL) {
				ParExeQueue *queue = stage->sourceQueue();
				int size = queue->size();
				prod_stage = queue->fillingStage();
				for (int i=0 ; i<stage->numNodes() - size ; i++) {
					ASSERT(i+size < prod_stage->numNodes());
					factory()->makeEdge(stage->node(i), prod_stage->node(i + size), ParExeEdge::SLASHED, 0, queue->name());
				}
			}
	    }
	}

	/**
	 * TODO
	 */
	void init(void) {

		// initialize cache size
		if(_ws != workspace()) {
			_ws = workspace();
			const hard::CacheConfiguration *cache = hard::CACHE_CONFIGURATION(_ws);
			if (cache && cache->instCache())
				_cache_line_size = cache->instCache()->blockSize();
			else
				_cache_line_size = 0;
		}

		// initialize stages
		if(_proc != processor()) {
			_proc = processor();
			fetch_stage = _proc->fetchStage();
			branch_stage = _proc->branchStage();
		}
	}

	WorkSpace *_ws = nullptr;
	int _cache_line_size = 0;
	ParExeProc *_proc = nullptr;
	ParExeStage *fetch_stage = nullptr;
	ParExeStage *branch_stage = nullptr;
};


/**
 * TODO
 */
Builder *Builder::make(const Monitor& mon) {
	return new StandardBuilder(mon);
}

} }	// otawa::etime

