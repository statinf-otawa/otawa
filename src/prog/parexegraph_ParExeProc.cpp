/*
 *	$Id$
 *	ParExeProc methods
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


#include <otawa/parexegraph/ParExeProc.h>

namespace otawa {

 ParExeProc::ParExeProc(const hard::Processor *proc) {
	 ASSERT(proc);

    // Create queues
    const Table<hard::Queue *>& oqueues = proc->getQueues(); 
    for(int i = 0; i < oqueues.count(); i++)
      addQueue(oqueues[i]->getName(),1 << oqueues[i]->getSize());
    
    // Create stages
    const Table<hard::Stage *>& ostages = proc->getStages();
    for(int i = 0; i < ostages.count(); i++) {
      hard::Stage * hstage = ostages[i];
 
      ParExeStage::pipeline_stage_category_t category;
      ParExeStage::order_t policy = ParExeStage::IN_ORDER;
      int latency = hstage->getLatency();
      switch(hstage->getType()) {
      case hard::Stage::FETCH:
	category = ParExeStage::FETCH;
	break;
      case hard::Stage::LAZY:
	category = ParExeStage::DECODE;
	break;
      case hard::Stage::EXEC:
	policy = ostages[i]->isOrdered() ? ParExeStage::IN_ORDER : ParExeStage::OUT_OF_ORDER;
	category = ParExeStage::EXECUTE;
	latency = 0;
	break;
      case hard::Stage::COMMIT:
	category = ParExeStage::COMMIT;
	break;
      default:
    	  ASSERT(0);
    	  break;
      }
      // Link the stages
      ParExeQueue *source_queue = NULL;
      ParExeQueue *destination_queue = NULL;
      for(int j = 0; j < oqueues.count(); j++) {
	if(oqueues[j]->getInput() == hstage)
	  destination_queue = queue(j);
	if(oqueues[j]->getOutput() == hstage)
	  source_queue = queue(j);
      }
      
      // Create the stage
      ParExeStage *stage = new ParExeStage(category, latency, hstage->getWidth(), policy, source_queue, destination_queue, hstage->getName(), _pipeline.numStages(), hstage);
      _pipeline.addStage(stage);
      
      if (category == ParExeStage::FETCH)
	setFetchStage(stage);
      
      // Add functional units if required
      if (category == ParExeStage::EXECUTE) {
	setExecStage(stage);
	// Build the FUs
	const Table<hard::FunctionalUnit *>& fus = hstage->getFUs();
	for(int j = 0; j < fus.count(); j++) {
	  hard::FunctionalUnit *fu = fus[j];
	  stage->addFunctionalUnit(fu->isPipelined(), fu->getLatency(), fu->getWidth(), fu->getName(), fu);
	}
	
	// Build the bindings
	const Table<hard::Dispatch *>& dispatch = hstage->getDispatch();
	for(int j = 0; j < dispatch.count(); j++) {
	  bool found = false;
	  for(int k = 0; k < fus.count(); k++)
	    if(fus[k] == dispatch[j]->getFU()) {
		stage->addBinding(dispatch[j]->getType(), stage->fu(k));
	      found = true;
	    }
	  ASSERT(found);
	}
      }
    }
  }

/**
 * @class ParExeQueue
 * Representation of a hardware instruction queue to be used to build a ParExeGraph.
 * @ingroup peg
 * @see peg
 */

/**
 * @fn ParExeQueue::ParExeQueue(elm::String name, int size);
 * Constructor.
 * @param name	Name of the queue (used for debugging purposes)
 * @param size 	Capacity (number of instructions) of the queue.
 */

/**
 * @fn elm::String ParExeQueue::name(void);
 * @return	Name of the queue. 
 */

/**
 * @fn int ParExeQueue::size(void);
 * @return	Size (capacity) of the queue. 
 */

/**
 * @fn ParExeStage * ParExeQueue::fillingStage(void);
 * @return	Pointer to the pipeline stage (ParExeStage) that puts instructions into the queue. 
 */

/**
 * @fn ParExeStage * ParExeQueue::emptyingStage(void);
 * @return	Pointer to the pipeline stage (ParExeStage) that gets instructions from the queue. 
 */

/**
 * @fn void ParExeQueue::SetFillingStage(ParExeStage *stage);
 * Specifies the pipeline stage that puts instructions into the queue.
 * @param	Pointer to the pipeline stage (ParExeStage) that puts instructions into the queue. 
 */

/**
 * @fn void ParExeQueue::SetEmptyingStage(ParExeStage *stage);
 * Specifies the pipeline stage that gets instructions from the queue.
 * @param	Pointer to the pipeline stage (ParExeStage) that gets instructions from the queue. 
 */

/**
 * @class ParExeStage
 * Representation of a pipeline stage to be used to build a ParExeGraph.
 * @ingroup peg
 * @see peg
 */

/**
 * @fn ParExeStage::ParExeStage(pipeline_stage_category_t category, int latency, int width, order_t policy, ParExeQueue *sq, ParExeQueue *dq, elm::String name, int index=0);
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
 * @fn order_t ParExeStage::orderPolicy(void);
 * @return	Instruction scheduling policy (relevant for EXECUTE stages only). 
 */

/**
 * @fn int ParExeStage::width(void);
 * @return	Stage width. 
 */

/**
 * @fn elm::String ParExeStage::name(void);
 * @return	Stage name. 
 */

/**
 * @fn int ParExeStage::index(void);
 * @return	Stage index (its position in the pipeline - 0 is for the first stage). 
 */


/**
 * @fn pipeline_stage_category_t ParExeStage::category(void);
 * @return	Stage category. 
 */

/**
 * @fn ParExeQueue * ParExeStage::destinationQueue(void);
 * @return	Pointer to the queue into which processed instructions are to be inserted into (NULL if instructions go directly to the next pipeline stage. 
 */

/**
 * @fn int ParExeStage::latency(void);
 * @return	Stage latency (i.e. number of cycles required to process an instruction.
 */

/**
 * @fn bool ParExeStage::isFuStage(void);
 * @return	True if the stage is part of a functional unit.
 */

/**
 * @fn void ParExeStage::addFunctionalUnit(bool pipelined, int latency, int width, elm::String name);
 * Adds a functional unit (should be used with EXECUTE stages only).
 * @param pipelined True if the functional unit is pipelined.
 * @param latency Latency of the functional unit. If the functional unit is pipelined, it will have as many stages as the value of its latency, each one with a single-cycle latency. If the functional unit is not pipelined, it will have a single stage with the specified latency.
 * @param width Number of identical functional units to add. In other words, number of instructions that can be processed in parallel par the functional unit.
 * @param name Name of the functional unit (used for debugging purposes).
 */

/**
 * @fn int ParExeStage::numFus(void);
 * @return	Number of functional units attached to the stage (relevant for EXECUTE stages only).
 */

/**
 * @fn ParExePipeline * ParExeStage::fu(int index);
 * This function returns a pointer to a functional unit pipeline.
 * @param index index of the functional unit (functional units are numbered in the order they have been added to the EXECUTE stage)
 * @return Pointer to the indexed functional unit.	
 */

/**
 * @fn void ParExeStage::addNode(ParExeNode * node);
 * This function adds an execution graph (ParExeGraph) node to the list of nodes related to the pipeline stage.
 * @param node Pointer to the graph node.	
 */

/**
 * @fn ParExeNode * ParExeStage::firstNode(void);
 * @return Pointer to the first graph node related to the stage.	
 */

/**
 * @fn ParExeNode * ParExeStage::lastNode(void);
 * @return Pointer to the lastgraph node related to the stage.	
 */

/**
 * @fn bool ParExeStage::hasNodes(void);
 * @return  True if the list of nodes is not empty (pipeline stages belonging to functional units might have no nodes if they are not used by any instruction in the graph).
 */

/**
 * @fn void ParExeStage::hasNodes(void);
 * Deletes all the nodes in the list.
 */

/**
 * @fn int ParExeStage::numNodes(void);
 * @return Number of nodes in the list.
 */

/**
 * @fn ParExeNode * ParExeStage::node(int index);
 * Returns a pointer to a graph node related to the stage. 
 * @param index Index of the node in the list (nodes are numbered in the order they have been added to the list).
 * @return Pointer to the indexed node.
 */

/**
 * @fn void ParExeStage::addBinding(Inst::kind_t kind, ParExePipeline *fu);
 * Binds a functional unit to an instruction kind. This binding will be used to determine which functional unit will process an instruction (accrding to its kind). See function findFU().
 * @param kind Instruction kind
 * @param fu Pointer to the functional unit pipeline.
 */

/**
 * @fn ParExePipeline * ParExeStage::findFU(Inst::kind_t kind);
 * @param kind Instruction kind
 * @return Pointer to the functional unit pipeline that handles that kind of instructions.
 */

/**
 * @class ParExeStage::NodeIterator
 * Iterator for the graph nodes related to the pipeline stage.
 */

/**
 * @class ParExePipeline
 * Representation of a pipeline (list of stages).
 * @ingroup peg
 * @see peg
 */

/**
 * @fn ParExeStage * ParExePipeline::lastStage();
 * @return Pointer to the last stage.
 */

/**
 * @fn ParExeStage * ParExePipeline::firstStage();
 * @return Pointer to the first stage.
 */

/**
 * @fn void ParExePipeline::addStage(ParExeStage *stage);
 * Adds a stage to the pipeline. Stages must be added in the pipeline order.
 * @param Pointer to the stage to be added.
 */

/**
 * @fn int ParExePipeline::numStages();
 * @return Number of stages.
 */

/**
 * @class ParExeStage::StageIterator
 * Iterator for the stages in the pipeline.
 */

/**
 * @class ParExeProc
 * Représentation of a processor (to be used to build a ParExeGraph). A processor is a pipeline, with a number of stages, among which a fetch stage and an execution stage, and a set of instruction queues.
 * @ingroup peg
 * @see peg
 */

/**
 * @fn ParExeProc::ParExeProc(const hard::Processor *proc);
 * Constructor.
 * @param Description of the processor.
 */

/**
 * @fn void ParExeProc::addQueue(elm::String name, int size);
 * Add an instruction queue to the processor.
 * @param name Name of the queue.
 * @param size Size (capacity) of the queue.
 */

/**
 * @fn ParExeQueue * ParExeProc::queue(int index);
 * Returns a pointer to the queue specified by index. Queues are numbered when they are added to the processor.
 * @param index Index of the queue.
 * @return Pointer to the indexed queue.
 */

/**
 * @fn void ParExeProc::setFetchStage(ParExeStage * stage);
 * Declares a stage as the fetch stage (it will undergo a special processing when building a ParExeGraph).
 * @param stage Pointer to the stage.
 */

/**
 * @fn ParExeStage * ParExeProc::fetchStage();
 * @return Pointer to the fetch stage (must have been set beforehand).
 */

/**
 * @fn void ParExeProc::setExecStage(ParExeStage * stage);
 * Declares a stage as the execution stage (it will undergo a special processing when building a ParExeGraph).
 * @param stage Pointer to the stage.
 */

/**
 * @fn ParExeStage * ParExeProc::execStage();
 * @return Pointer to the execution stage (must have been set beforehand).
 */

/**
 * @fn bool ParExeProc::isLastStage(ParExeStage *stage);
 * @param stage Pointer to a stage.
 * @return True if the stage is the last in the pipeline.
 */

/**
 * @fn ParExePipeline * ParExeProc::pipeline();
 * @return Pointer to the pipeline.
 */

/**
 * @class ParExeProc::QueueIterator
 * Iterator for the instruction queues.
 */

}	// otawa
