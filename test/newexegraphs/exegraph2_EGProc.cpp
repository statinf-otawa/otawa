/*
 *	$Id$
 *	 EGQueue, EGProc, EGStage, EGPipeline classes.
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


#include "EGProc.h"

using namespace otawa;
using namespace otawa::exegraph2;

 EGProc::EGProc(const otawa::hard::Processor *proc) {
    assert(proc);

    // Create queues
    const Table<otawa::hard::Queue *>& oqueues = proc->getQueues();
    for(int i = 0; i < oqueues.count(); i++)
      addQueue(oqueues[i]->getName(),1 << oqueues[i]->getSize());

    // Create stages
    const Table<otawa::hard::Stage *>& ostages = proc->getStages();
    for(int i = 0; i < ostages.count(); i++) {
      otawa::hard::Stage * hstage = ostages[i];

      EGStage::pipeline_stage_category_t category;
      EGStage::order_t policy = EGStage::IN_ORDER;
      int latency = hstage->getLatency();
      switch(hstage->getType()) {
      case otawa::hard::Stage::FETCH:
	category = EGStage::FETCH;
	break;
      case otawa::hard::Stage::LAZY:
	category = EGStage::DECODE;
	break;
      case otawa::hard::Stage::EXEC:
	policy = ostages[i]->isOrdered() ? EGStage::IN_ORDER : EGStage::OUT_OF_ORDER;
	category = EGStage::EXECUTE;
	latency = 0;
	break;
      case otawa::hard::Stage::COMMIT:
	category = EGStage::COMMIT;
	break;
      default:
	assert(0);
      }
      // Link the stages
      EGQueue *source_queue = NULL;
      EGQueue *destination_queue = NULL;
      for(int j = 0; j < oqueues.count(); j++) {
	if(oqueues[j]->getInput() == hstage)
	  destination_queue = queue(j);
	if(oqueues[j]->getOutput() == hstage)
	  source_queue = queue(j);
      }

      // Create the stage
      EGStage *stage = new EGStage(category, latency, hstage->getWidth(), policy, source_queue, destination_queue, hstage->getName(), _pipeline.numStages());
      _pipeline.addStage(stage);

      if (category == EGStage::FETCH)
	setFetchStage(stage);

      // Add functional units if required
      if (category == EGStage::EXECUTE) {
	setExecStage(stage);
	// Build the FUs
	const Table<otawa::hard::FunctionalUnit *>& fus = hstage->getFUs();
	for(int j = 0; j < fus.count(); j++) {
	  otawa::hard::FunctionalUnit *fu = fus[j];
	  stage->addFunctionalUnit(fu->isPipelined(), fu->getLatency(), fu->getWidth(), fu->getName());
	}

	// Build the bindings
	const Table<otawa::hard::Dispatch *>& dispatch = hstage->getDispatch();
	for(int j = 0; j < dispatch.count(); j++) {
	  bool found = false;
	  for(int k = 0; k < fus.count(); k++)
	    if(fus[k] == dispatch[j]->getFU()) {
		stage->addBinding(dispatch[j]->getType(), stage->fu(k));
	      found = true;
	    }
	  assert(found);
	}
      }
    }
  }

/**
 * @class EGQueue
 * Representation of a hardware instruction queue to be used to build a ParExeGraph.
 */

/**
 * @fn EGQueue::EGQueue(elm::String name, int size);
 * Constructor.
 * @param name	Name of the queue (used for debugging purposes)
 * @param size 	Capacity (number of instructions) of the queue.
 */

/**
 * @fn elm::String EGQueue::name(void);
 * @return	Name of the queue.
 */

/**
 * @fn int EGQueue::size(void);
 * @return	Size (capacity) of the queue.
 */

/**
 * @fn EGStage * EGQueue::fillingStage(void);
 * @return	Pointer to the pipeline stage (EGStage) that puts instructions into the queue.
 */

/**
 * @fn EGStage * EGQueue::emptyingStage(void);
 * @return	Pointer to the pipeline stage (EGStage) that gets instructions from the queue.
 */

/**
 * @fn void EGQueue::SetFillingStage(EGStage *stage);
 * Specifies the pipeline stage that puts instructions into the queue.
 * @param	Pointer to the pipeline stage (EGStage) that puts instructions into the queue.
 */

/**
 * @fn void EGQueue::SetEmptyingStage(EGStage *stage);
 * Specifies the pipeline stage that gets instructions from the queue.
 * @param	Pointer to the pipeline stage (EGStage) that gets instructions from the queue.
 */

/**
 * @class EGStage
 * Representation of a pipeline stage to be used to build a ParExeGraph.
 */

/**
 * @fn EGStage::EGStage(pipeline_stage_category_t category, int latency, int width, order_t policy, EGQueue *sq, EGQueue *dq, elm::String name, int index=0);
 * Constructor.
 * @param category Stage category
 * @param latency Latency (i.e. number of cycles required to process an instruction)
 * @param width Number of instructions that can be processed in parallel
 * @param policy Policy used to schedule instructions (only applies to EXECUTE stages)
 * @param sq Source queue (i.e. queue from which instructions to be processed are read - NULL if instructions directly come from the previous pipeline stage)
 * @param dq Destination queue (i.e. queue into which instructions will be inserted when they have been processed - NULL if instructions directly go to the next pipeline stage)
 * @param name Name of the pipeline stage (used for debugging purposes)
 * @param index Position of the stage in the pipeline (0 is for the first stage)
 */

/**
 * @fn order_t EGStage::orderPolicy(void);
 * @return	Instruction scheduling policy (relevant for EXECUTE stages only).
 */

/**
 * @fn int EGStage::width(void);
 * @return	Stage width.
 */

/**
 * @fn elm::String EGStage::name(void);
 * @return	Stage name.
 */

/**
 * @fn int EGStage::index(void);
 * @return	Stage index (its position in the pipeline - 0 is for the first stage).
 */


/**
 * @fn pipeline_stage_category_t EGStage::category(void);
 * @return	Stage category.
 */

/**
 * @fn EGQueue * EGStage::destinationQueue(void);
 * @return	Pointer to the queue into which processed instructions are to be inserted into (NULL if instructions go directly to the next pipeline stage.
 */

/**
 * @fn int EGStage::latency(void);
 * @return	Stage latency (i.e. number of cycles required to process an instruction.
 */

/**
 * @fn bool EGStage::isFuStage(void);
 * @return	True if the stage is part of a functional unit.
 */

/**
 * @fn void EGStage::addFunctionalUnit(bool pipelined, int latency, int width, elm::String name);
 * Adds a functional unit (should be used with EXECUTE stages only).
 * @param pipelined True if the functional unit is pipelined.
 * @param latency Latency of the functional unit. If the functional unit is pipelined, it will have as many stages as the value of its latency, each one with a single-cycle latency. If the functional unit is not pipelined, it will have a single stage with the specified latency.
 * @param width Number of identical functional units to add. In other words, number of instructions that can be processed in parallel par the functional unit.
 * @param name Name of the functional unit (used for debugging purposes).
 */

/**
 * @fn int EGStage::numFus(void);
 * @return	Number of functional units attached to the stage (relevant for EXECUTE stages only).
 */

/**
 * @fn EGPipeline * EGStage::fu(int index);
 * This function returns a pointer to a functional unit pipeline.
 * @param index index of the functional unit (functional units are numbered in the order they have been added to the EXECUTE stage)
 * @return Pointer to the indexed functional unit.
 */

/**
 * @fn void EGStage::addNode(ParExeNode * node);
 * This function adds an execution graph (ParExeGraph) node to the list of nodes related to the pipeline stage.
 * @param node Pointer to the graph node.
 */

/**
 * @fn ParExeNode * EGStage::firstNode(void);
 * @return Pointer to the first graph node related to the stage.
 */

/**
 * @fn ParExeNode * EGStage::lastNode(void);
 * @return Pointer to the lastgraph node related to the stage.
 */

/**
 * @fn bool EGStage::hasNodes(void);
 * @return  True if the list of nodes is not empty (pipeline stages belonging to functional units might have no nodes if they are not used by any instruction in the graph).
 */

/**
 * @fn void EGStage::hasNodes(void);
 * Deletes all the nodes in the list.
 */

/**
 * @fn int EGStage::numNodes(void);
 * @return Number of nodes in the list.
 */

/**
 * @fn ParExeNode * EGStage::node(int index);
 * Returns a pointer to a graph node related to the stage.
 * @param index Index of the node in the list (nodes are numbered in the order they have been added to the list).
 * @return Pointer to the indexed node.
 */

/**
 * @fn void EGStage::addBinding(Inst::kind_t kind, EGPipeline *fu);
 * Binds a functional unit to an instruction kind. This binding will be used to determine which functional unit will process an instruction (accrding to its kind). See function findFU().
 * @param kind Instruction kind
 * @param fu Pointer to the functional unit pipeline.
 */

/**
 * @fn EGPipeline * EGStage::findFU(Inst::kind_t kind);
 * @param kind Instruction kind
 * @return Pointer to the functional unit pipeline that handles that kind of instructions.
 */

/**
 * @class EGStage::NodeIterator
 * Iterator for the graph nodes related to the pipeline stage.
 */

/**
 * @class EGPipeline
 * Representation of a pipeline (list of stages).
 */

/**
 * @fn EGStage * EGPipeline::lastStage();
 * @return Pointer to the last stage.
 */

/**
 * @fn EGStage * EGPipeline::firstStage();
 * @return Pointer to the first stage.
 */

/**
 * @fn void EGPipeline::addStage(EGStage *stage);
 * Adds a stage to the pipeline. Stages must be added in the pipeline order.
 * @param Pointer to the stage to be added.
 */

/**
 * @fn int EGPipeline::numStages();
 * @return Number of stages.
 */

/**
 * @class EGStage::StageIterator
 * Iterator for the stages in the pipeline.
 */

/**
 * @class EGProc
 * Représentation of a processor (to be used to build a ParExeGraph). A processor is a pipeline, with a number of stages, among which a fetch stage and an execution stage, and a set of instruction queues.
 */

/**
 * @fn EGProc::EGProc(const hard::Processor *proc);
 * Constructor.
 * @param Description of the processor.
 */

/**
 * @fn void EGProc::addQueue(elm::String name, int size);
 * Add an instruction queue to the processor.
 * @param name Name of the queue.
 * @param size Size (capacity) of the queue.
 */

/**
 * @fn EGQueue * EGProc::queue(int index);
 * Returns a pointer to the queue specified by index. Queues are numbered when they are added to the processor.
 * @param index Index of the queue.
 * @return Pointer to the indexed queue.
 */

/**
 * @fn void EGProc::setFetchStage(EGStage * stage);
 * Declares a stage as the fetch stage (it will undergo a special processing when building a ParExeGraph).
 * @param stage Pointer to the stage.
 */

/**
 * @fn EGStage * EGProc::fetchStage();
 * @return Pointer to the fetch stage (must have been set beforehand).
 */

/**
 * @fn void EGProc::setExecStage(EGStage * stage);
 * Declares a stage as the execution stage (it will undergo a special processing when building a ParExeGraph).
 * @param stage Pointer to the stage.
 */

/**
 * @fn EGStage * EGProc::execStage();
 * @return Pointer to the execution stage (must have been set beforehand).
 */

/**
 * @fn bool EGProc::isLastStage(EGStage *stage);
 * @param stage Pointer to a stage.
 * @return True if the stage is the last in the pipeline.
 */

/**
 * @fn EGPipeline * EGProc::pipeline();
 * @return Pointer to the pipeline.
 */

/**
 * @class EGProc::QueueIterator
 * Iterator for the instruction queues.
 */

