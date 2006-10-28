/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/test/exegraph/Microprocessor.h -- interface for Microprocessor class.
 */
#ifndef OTAWA_MICROPROCESSOR_H
#define OTAWA_MICROPROCESSOR_H

#include <elm/io.h>
#include <elm/genstruct/Vector.h>
#include <elm/string/String.h>
#include <elm/genstruct/DLList.h>
#include <stdio.h>
#include <otawa/otawa.h>

namespace otawa {

using namespace elm;
using namespace elm::genstruct;

class PipelineStage;
class FunctionalUnit;
class Microprocessor;
class ExecutionNode;
namespace hard {
	class Microprocessor;
} // hard

typedef enum instruction_category_t {	
	IALU = 0,
	FALU = 1,
	MEMORY = 2,
	CONTROL = 3,
	MUL = 4,
	DIV = 5,
	INST_CATEGORY_NUMBER   // must be the last value
} instruction_category_t;	

instruction_category_t instCategory(Inst *inst);
 


// ----------------------------------------------------------------------
// Processor queue class
// ----------------------------------------------------------------------

class Queue {
	private:
		elm::String queue_name;
		int queue_size;
		PipelineStage * filling_stage;
		PipelineStage * emptying_stage;
	public:
		inline Queue(elm::String name, int size);
		inline elm::String name(void) const;
		inline int size(void) const;
		inline PipelineStage * fillingStage(void);
		inline PipelineStage * emptyingStage(void);
		inline void setFillingStage(PipelineStage * stage);
		inline void setEmptyingStage(PipelineStage * stage);
};

inline Queue::Queue(elm::String name, int size)
: queue_name(name), queue_size(size) {
}

inline elm::String Queue::name(void) const {
	return queue_name;
}

inline int Queue::size(void) const {
	return queue_size;
}

inline PipelineStage * Queue::fillingStage(void) {
	return filling_stage;
}

inline PipelineStage * Queue::emptyingStage(void) {
	return emptying_stage;
}

inline void Queue::setFillingStage(PipelineStage * stage) {
	filling_stage = stage;
}

inline void Queue::setEmptyingStage(PipelineStage * stage) {
	emptying_stage = stage;
}

// ----------------------------------------------------------------------
// Pipeline stage class
// ----------------------------------------------------------------------


// Pipeline stage class

class PipelineStage {
public:
	typedef enum order_policy_t {
		NO_POLICY = 0,
		IN_ORDER = 1,
		OUT_OF_ORDER = 2
	} order_policy_t;
	
	typedef enum pipeline_stage_category_t {
		NO_CATEGORY = 0,
		FETCH = 1,
		DECODE = 2,
		EXECUTE = 3,
		WRITE = 4,
		COMMIT=5,
		DELAY=6
	} pipeline_stage_category_t;

	typedef struct pipeline_info_t {
		order_policy_t order_policy;
		int stage_width;
		elm::String stage_name;
		elm::String stage_short_name;
		pipeline_stage_category_t stage_category;
		Queue *source_queue;
		Queue *destination_queue;	
		int min_latency;
		int max_latency;
	} pipeline_info_t;
	
	
	
	class FunctionalUnit {
		public :
			typedef struct fu_info_t {
			elm::String name;
			elm::String short_name;
			bool is_pipelined;
			int min_latency; // overrides pipeline stage latency
			int max_latency;
			int width;	
			order_policy_t order_policy;
		} fu_info_t;
		
		private:
			fu_info_t information;
			elm::genstruct::Vector<PipelineStage *> pipeline;
			Microprocessor *processor;
			
		public:
			inline FunctionalUnit(fu_info_t& info, PipelineStage *user_stage, Microprocessor *proc);
			inline elm::String name(void) const;
			inline elm::String shortName(void) const;
			inline bool isPipelined(void) const;
			inline int minLatency(void) const;
			inline int maxLatency(void) const;
			inline int width(void) const;
			inline PipelineStage * firstStage();
			
			class PipelineIterator: public elm::genstruct::Vector<PipelineStage *>::Iterator {
				public:
					inline PipelineIterator(const FunctionalUnit *fu);
			};
			
	};
	
	
private:
	pipeline_info_t information;
	bool uses_functional_units;
	//FunctionalUnit * functional_unit[INST_CATEGORY_NUMBER];
	Vector<Pair<Inst::kind_t, FunctionalUnit *> > bindings;
	elm::genstruct::Vector<FunctionalUnit *> fus;
	int stage_index;
	Microprocessor *processor;
	elm::genstruct::DLList<ExecutionNode *> _nodes;
		
public:
	inline PipelineStage(const pipeline_info_t& info, Microprocessor* proc);
	inline order_policy_t orderPolicy(void) const;
	inline int width(void) const;
	inline elm::String name(void) const;
	inline elm::String shortName(void) const;
	inline pipeline_stage_category_t category(void) const;
	inline elm::String categoryString(void) const;
	inline elm::String orderPolicyString(void) const;
	inline Queue * sourceQueue(void) const;
	inline Queue * destinationQueue(void) const;
	inline int index(void) const;
	inline int minLatency(void) const;
	inline int maxLatency(void) const;
	inline FunctionalUnit * addFunctionalUnit(FunctionalUnit::fu_info_t& fu_info);
	//inline void bindFunctionalUnit(FunctionalUnit * fu, instruction_category_t category);
	inline bool usesFunctionalUnits(void) const;
	//inline FunctionalUnit * functionalUnit(int category);
	inline void addNode(ExecutionNode * node);
	inline void deleteNodes();
	inline int numberOfNodes();
	
	const elm::genstruct::Vector<FunctionalUnit *>& getFUs(void) const {
		return fus;
	}
	
	inline void addBinding(Inst::kind_t kind, FunctionalUnit *fu) {
		bindings.add(pair(kind, fu));
	}
	
	inline FunctionalUnit *findFU(Inst::kind_t kind) {
		for(int i = 0; i < bindings.length(); i++) {
			Inst::kind_t mask = bindings[i].fst;
			if((kind & mask) == mask)
				return bindings[i].snd;
		}
		cerr << "Unsupported instruction kind : " << io::hex(kind) << io::endl;
		assert(0);
	}
	
	class ExecutionNodeIterator: public elm::genstruct::DLList<ExecutionNode *>::Iterator {
	public:
		inline ExecutionNodeIterator(const PipelineStage *stage);
	};
	
};


// Microprocessor class
class Microprocessor {
	
private:
	elm::genstruct::Vector<Queue *> queues;
	elm::genstruct::Vector<PipelineStage *> pipeline;
	int pipeline_stage_index;
	PipelineStage * operand_reading_stage;
	PipelineStage * operand_producing_stage;

public:
	Microprocessor(void);
	Microprocessor(const hard::Processor *proc);
	inline PipelineStage * addPipelineStage(PipelineStage::pipeline_info_t& info) ;
	inline Queue * addQueue(elm::String name, int size);
	void dump(elm::io::Output& out_stream) ;
	inline int getPipelineStageIndex();
	inline void setOperandReadingStage(PipelineStage * stage);
	inline void setOperandProducingStage(PipelineStage * stage);
	inline PipelineStage * operandReadingStage(void);
	inline PipelineStage *operandProducingStage(void);
	inline bool isLastStage(PipelineStage *stage);
	
	class PipelineIterator: public elm::genstruct::Vector<PipelineStage *>::Iterator {
	public:
		inline PipelineIterator(const Microprocessor *processor);
	};
	
	class QueueIterator: public elm::genstruct::Vector<Queue *>::Iterator {
	public:
		inline QueueIterator(const Microprocessor *processor);
	};
		
};


// Inlines

inline PipelineStage::PipelineStage(const pipeline_info_t& info, Microprocessor* proc)
: information(info), uses_functional_units(false), processor(proc) {
	/*for (int i=0 ; i<INST_CATEGORY_NUMBER ; i++)
		functional_unit[i] = NULL;*/
	stage_index = proc->getPipelineStageIndex();
	
}

inline void PipelineStage::addNode(ExecutionNode * node) {
	_nodes.addLast(node);
}

inline void PipelineStage::deleteNodes() {
	_nodes.clear();
}

inline int PipelineStage::numberOfNodes() {
	return _nodes.count();
}


inline PipelineStage::ExecutionNodeIterator::ExecutionNodeIterator(const PipelineStage *stage):
	elm::genstruct::DLList<ExecutionNode *>::Iterator(stage->_nodes) {
}

inline PipelineStage::order_policy_t PipelineStage::orderPolicy(void) const {
	return information.order_policy;
}

inline int PipelineStage::width(void) const {
	return information.stage_width;
}

inline elm::String PipelineStage::name(void) const {
	return information.stage_name;
}

inline elm::String PipelineStage::shortName(void) const {
	return information.stage_short_name;
}

inline PipelineStage::pipeline_stage_category_t PipelineStage::category(void) const {
	return information.stage_category;
}

inline Queue * PipelineStage::sourceQueue(void) const {
	return information.source_queue;
}

inline Queue * PipelineStage::destinationQueue(void) const {
	return information.destination_queue;
}

inline elm::String PipelineStage::categoryString(void) const {
	switch(information.stage_category)
	{
		case FETCH:
			return("FETCH");
		case DECODE:
			return("DECODE");
		case EXECUTE:
			return("EXECUTE");
		case WRITE:
			return("WRITE");
		case COMMIT:
			return("COMMIT");
		case DELAY:
			return("DELAY");
		default:
			return("NONE");
	}
}

inline elm::String PipelineStage::orderPolicyString(void) const {
	switch(information.order_policy)
	{
		case IN_ORDER:
			return("IN_ORDER");
		case OUT_OF_ORDER:
			return("OUT_OF_ORDER");
		default:
			return("NONE");
	}
}

inline bool PipelineStage::usesFunctionalUnits(void) const {
	return uses_functional_units;
}

/*inline PipelineStage::FunctionalUnit * PipelineStage::functionalUnit(int category) {
	return functional_unit[category];
}*/

inline int PipelineStage::index(void) const {
	return stage_index;
}

inline int PipelineStage::minLatency(void) const {
	return information.min_latency;
}

inline int PipelineStage::maxLatency(void) const {
	return information.max_latency;
}

inline PipelineStage::FunctionalUnit * PipelineStage::addFunctionalUnit(FunctionalUnit::fu_info_t& fu_info) {
	PipelineStage::FunctionalUnit *fu = new PipelineStage::FunctionalUnit(fu_info, this, processor);
	uses_functional_units = true;
	return fu;
}

/*inline void PipelineStage::bindFunctionalUnit(FunctionalUnit * fu, instruction_category_t category) {
		functional_unit[category] = fu;
}*/


//Functional Unit class


inline PipelineStage::FunctionalUnit::FunctionalUnit(fu_info_t& fu_info, PipelineStage *user_stage, Microprocessor *proc)
: information(fu_info), processor(proc) {
	PipelineStage::pipeline_info_t pipeline_info;	
	PipelineStage *stage;
	elm::StringBuffer *number;
	elm::String number_string;
	
	user_stage->fus.add(this);
	//pipeline_info.order_policy = user_stage->orderPolicy();
	pipeline_info.order_policy = fu_info.order_policy;
	pipeline_info.stage_width = fu_info.width;
	pipeline_info.stage_name = fu_info.name;
	pipeline_info.stage_short_name = fu_info.short_name;
	if ( ! fu_info.is_pipelined) {
		pipeline_info.source_queue = user_stage->information.source_queue;
		pipeline_info.destination_queue = user_stage->information.destination_queue;
		pipeline_info.min_latency = fu_info.min_latency;
		pipeline_info.max_latency = fu_info.max_latency;
		stage = new PipelineStage(pipeline_info,processor);
		this->pipeline.add(stage);
	}
	else {
		for (int i=1 ; i<=fu_info.min_latency ; i++) {
			number = new elm::StringBuffer;
			*number << i;
			number_string = number->toString();
			pipeline_info.stage_name = fu_info.name.concat(number_string); 
			pipeline_info.stage_short_name = fu_info.short_name.concat(number_string); 
			delete number;
			pipeline_info.min_latency = 1;
			pipeline_info.max_latency = 1;
			pipeline_info.order_policy = PipelineStage::IN_ORDER;
			
			if (i==1) {
				pipeline_info.source_queue = user_stage->information.source_queue;
				pipeline_info.order_policy = fu_info.order_policy;
			}
			if (i==fu_info.min_latency) {
				pipeline_info.destination_queue = user_stage->information.destination_queue;
				pipeline_info.max_latency = fu_info.max_latency - fu_info.min_latency + 1;	
			}
			stage = new PipelineStage(pipeline_info, processor);
			this->pipeline.add(stage);	
		}
	}
}

inline elm::String PipelineStage::FunctionalUnit::name(void) const {
	return information.name;
}

inline elm::String PipelineStage::FunctionalUnit::shortName(void) const {
	return information.short_name;
}

inline PipelineStage * PipelineStage::FunctionalUnit::firstStage() {
	return pipeline.get(0);
}

inline bool PipelineStage::FunctionalUnit::isPipelined(void) const {
	return information.is_pipelined;
}

inline int PipelineStage::FunctionalUnit::minLatency(void) const {
	return information.min_latency;
}

inline int PipelineStage::FunctionalUnit::maxLatency(void) const {
	return information.max_latency;
}

inline int PipelineStage::FunctionalUnit::width(void) const {
	return information.width;
}

inline PipelineStage::FunctionalUnit::PipelineIterator::PipelineIterator(const PipelineStage::FunctionalUnit *fu):
elm::genstruct::Vector<PipelineStage *>::Iterator(fu->pipeline) {
}

// Microprocessor::PipelineIterator class
inline Microprocessor::PipelineIterator::PipelineIterator(const Microprocessor *processor):
elm::genstruct::Vector<PipelineStage *>::Iterator(processor->pipeline) {
}

inline Microprocessor::QueueIterator::QueueIterator(const Microprocessor *processor):
elm::genstruct::Vector<Queue *>::Iterator(processor->queues) {
}

inline PipelineStage * Microprocessor::addPipelineStage(PipelineStage::pipeline_info_t& info) {
	PipelineStage *stage = new PipelineStage(info, this);
	pipeline.add(stage);
	if ( stage->sourceQueue() != NULL)
		stage->sourceQueue()->setEmptyingStage(stage);
	if ( stage->destinationQueue() != NULL)
		stage->destinationQueue()->setFillingStage(stage);
	return stage;
}

inline Queue * Microprocessor::addQueue(elm::String name, int size) {
	Queue *queue = new Queue(name, size);
	queues.add(queue);
	return queue;
}

inline int Microprocessor::getPipelineStageIndex() {
	return pipeline_stage_index++;
}

inline void Microprocessor::setOperandReadingStage(PipelineStage * stage) {
	operand_reading_stage = stage;
}

inline void Microprocessor::setOperandProducingStage(PipelineStage * stage) {
	operand_producing_stage = stage;
}

inline PipelineStage * Microprocessor::operandReadingStage(void) {
	return operand_reading_stage;
}

inline PipelineStage * Microprocessor::operandProducingStage(void) {
	return operand_producing_stage;
}

inline bool Microprocessor::isLastStage(PipelineStage *stage) {
	return (pipeline.get(pipeline.length()-1) == stage);
}


} // otawa

#endif // OTAWA_MICROPROCESSOR_H
