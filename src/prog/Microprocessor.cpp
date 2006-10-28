/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	test/exegraph/Microprocessor.cpp -- implementation of Cache class.
 */

#include <otawa/exegraph/Microprocessor.h>
#include <otawa/hard/Register.h>
#include <otawa/hard/Processor.h>

using namespace elm::genstruct;

namespace otawa { 

static const unsigned long MUL_MASK = Inst::IS_MUL | Inst::IS_INT;
static const unsigned long DIV_MASK = Inst::IS_DIV | Inst::IS_INT;
	
instruction_category_t instCategory(Inst *inst) {
	Inst::kind_t kind = inst->kind();
	if(kind & Inst::IS_FLOAT)
		return FALU;
	else if(kind & Inst::IS_MEM)
		return MEMORY;
	else if(kind & Inst::IS_CONTROL)
		return CONTROL;
	else if((kind & MUL_MASK) == MUL_MASK)
		return MUL;
	else if((kind & DIV_MASK) == DIV_MASK)
		return MUL;
	else
		return IALU;
}


/**
 * Build an empty microprocessor.
 */
Microprocessor::Microprocessor(void)
:	pipeline_stage_index(0),
	operand_reading_stage(0),
	operand_producing_stage(0)
{
}


/**
 * Build a microprocessor for ExeGraph from the abstract definition provided
 * by OTAWA.
 * @param proc	OTAWA abstract processor definition.
 */
Microprocessor::Microprocessor(const hard::Processor *proc)
:	pipeline_stage_index(0), 
	operand_reading_stage(0),
	operand_producing_stage(0)
{
	assert(proc);
	
	// Create queues
	Vector<Queue *> queues;
	const Table<hard::Queue *>& oqueues = proc->getQueues(); 
	for(int i = 0; i < oqueues.count(); i++)
		queues.add(addQueue(
			oqueues[i]->getName(),
			1 << oqueues[i]->getSize()));
	
	// Create stages
	const Table<hard::Stage *>& ostages = proc->getStages();
	for(int i = 0; i < ostages.count(); i++) {
		PipelineStage::pipeline_info_t info;
		
		// Common initialization
		info.stage_name = ostages[i]->getName();
		info.stage_short_name = ostages[i]->getName();
		info.stage_width = ostages[i]->getWidth();
		info.min_latency = info.max_latency = ostages[i]->getLatency();
		
		// Initialization according type of the stage
		bool is_exec = false;
		int exec_stage_index = 0;
		switch(ostages[i]->getType()) {
		case hard::Stage::FETCH:
			info.order_policy = PipelineStage::IN_ORDER;
			info.stage_category = PipelineStage::FETCH;
			break;
		case hard::Stage::LAZY:
			info.order_policy = PipelineStage::IN_ORDER;
			info.stage_category = PipelineStage::DECODE;
			break;
 		case hard::Stage::EXEC:
 			is_exec = true;
 			exec_stage_index = i;
			info.order_policy = ostages[i]->isOrdered()
				? PipelineStage::IN_ORDER : PipelineStage::OUT_OF_ORDER;
			info.stage_category = PipelineStage::EXECUTE;
			break;
 		case hard::Stage::COMMIT:
			info.order_policy = PipelineStage::IN_ORDER;
			info.stage_category = PipelineStage::COMMIT;
			break;
 		default:
 			assert(0);
		}

		// Link the stages
		info.source_queue = 0;
		info.destination_queue = 0;
		for(int j = 0; j < oqueues.count(); j++) {
			if(oqueues[j]->getInput() == ostages[i])
				info.destination_queue = queues[j];
			if(oqueues[j]->getOutput() == ostages[i])
				info.source_queue = queues[j];
		}
		
		// Create the stage
		PipelineStage *stage = addPipelineStage(info);
		
		// Add functional units if required
		if(is_exec) {
			setOperandReadingStage(stage);
			setOperandProducingStage(stage);
			// Build the FU
			const Table<hard::FunctionalUnit *>& fus = ostages[i]->getFUs();
			for(int j = 0; j < fus.count(); j++) {
				PipelineStage::FunctionalUnit::fu_info_t info;
				info.name = fus[j]->getName();
				info.short_name = fus[j]->getName();
				info.is_pipelined = fus[j]->isPipelined();
				info.max_latency = fus[j]->getLatency();
				info.min_latency = fus[j]->getLatency();
				info.order_policy = ostages[exec_stage_index]->isOrdered() 
					? PipelineStage::IN_ORDER : PipelineStage::OUT_OF_ORDER;
				info.width = fus[j]->getWidth();
				stage->addFunctionalUnit(info);
			}
			
			// Build the bindings
			const Table<hard::Dispatch *>& dispatch = ostages[i]->getDispatch();
			for(int j = 0; j < dispatch.count(); j++) {
				bool found = false;
				for(int k = 0; k < fus.count(); k++)
					if(fus[k] == dispatch[j]->getFU()) {
						stage->addBinding(dispatch[j]->getType(), stage->getFUs()[k]);
						found = true;
					}
				assert(found);
			}
		}
	}
	
}


/**
 * Dump the description of the microprocessor.
 * @param out_stream	Used output stream.
 */
void Microprocessor::dump(elm::io::Output& out_stream) {
	out_stream << "---------------------------------------------\n";
	out_stream << "Microprocessor configuration: \n";
	out_stream << "---------------------------------------------\n";
	out_stream << "\tpipeline stages:\n";
	for(PipelineIterator stage(this); stage; stage++)
	{
		out_stream << "\t\t" << stage->name() << " (" ;
		out_stream << stage->shortName() << "): category=" << stage->categoryString();
		out_stream << " - policy=" << stage->orderPolicyString();
		out_stream << " - width=" << stage->width();
		if (stage->sourceQueue() != NULL)
			out_stream << " - source_queue=" << stage->sourceQueue()->name();
		if (stage->destinationQueue() != NULL)
			out_stream << " - destination_queue=" << stage->destinationQueue()->name();
		out_stream << "\n";
		if (stage->usesFunctionalUnits()) {
			out_stream << "\t\t\tuses functional units:\n";
			//for (int i=0 ; i<PipelineStage::INST_CATEGORY_NUMBER ; i++)
			for (int i=0 ; i<INST_CATEGORY_NUMBER ; i++) {
				out_stream << "\t\t\t\tinstruction category " << i << " processed by unit ";
				/*if (stage->functionalUnit(i) != NULL)
				{
					out_stream << stage->functionalUnit(i)->name();
					out_stream << " (latency=" << stage->functionalUnit(i)->minLatency();
					out_stream << " to " << stage->functionalUnit(i)->maxLatency();
					if (stage->functionalUnit(i)->isPipelined())
						out_stream << " /pipelined/ ";
					out_stream << " - width=" << stage->functionalUnit(i)->width() << ")";
					out_stream << "\n\t\t\t\t\tstages: ";
					for(PipelineStage::FunctionalUnit::PipelineIterator fu_stage(stage->functionalUnit(i)); fu_stage; fu_stage++) {
						out_stream << fu_stage->shortName() << "(" << fu_stage->minLatency();
						out_stream << "/" <<  fu_stage->maxLatency() << ") - ";
					}
				}*/
				out_stream << "\n";
			}
			
		}
	}
	out_stream << "\n";
	out_stream << "\tqueues:\n";
	for(QueueIterator queue(this); queue; queue++)
	{
		out_stream << "\t\t" << queue->name() << ": size=" << queue->size();
		out_stream << " - filling_stage=" << queue->fillingStage()->name();
		out_stream << " - emptying_stage=" << queue->emptyingStage()->name() << "\n";
	}
	out_stream << "\n";
}

} // otawa
