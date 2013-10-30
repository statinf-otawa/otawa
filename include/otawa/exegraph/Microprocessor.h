/*
 *	$Id$
 *	exegraph module interface
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
#ifndef OTAWA_MICROPROCESSOR_H
#define OTAWA_MICROPROCESSOR_H

#include <elm/io.h>
#include <elm/genstruct/Vector.h>
#include <elm/string/String.h>
#include <elm/genstruct/DLList.h>
#include <stdio.h>
#include <otawa/otawa.h>
#include <otawa/hard/Processor.h>

namespace otawa {

using namespace elm;
using namespace elm::genstruct;

template <class N> class PipelineStage;
class FunctionalUnit;
template <class N> class Microprocessor;

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

template <class N>
class Queue {
 private:
  elm::String _name;
  int _size;
  PipelineStage<N> * _filling_stage;
  PipelineStage<N> * _emptying_stage;
 public:
  inline Queue(elm::String name, int size)
    : _name(name), _size(size) {}
  inline elm::String name(void)
    {return _name;}
  inline int size(void) 
    {return _size;}
  inline PipelineStage<N> * fillingStage(void)
    {return _filling_stage;}
  inline PipelineStage<N> * emptyingStage(void)
    {return _emptying_stage;}
  inline void setFillingStage(PipelineStage<N> * stage)
    {_filling_stage = stage;}
  inline void setEmptyingStage(PipelineStage<N> * stage)
    {_emptying_stage = stage;}
};


// ----------------------------------------------------------------------
// Pipeline stage class
// ----------------------------------------------------------------------


// Pipeline stage class

template <class N>
class PipelineStage {
 public:
  typedef enum order_policy_t {
    NO_POLICY = 0,
    IN_ORDER = 1,
    OUT_OF_ORDER = 2,
    NUMBER_OF_POLICIES=3
  } order_policy_t;
  typedef enum pipeline_stage_category_t {
    NO_CATEGORY = 0,
    FETCH = 1,
    DECODE = 2,
    EXECUTE = 3,
    WRITE = 4,
    COMMIT=5,
    DELAY=6,
    NUMBER_OF_CATEGORIES=7  // must be the last value
  } pipeline_stage_category_t;
  
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
    fu_info_t _info;
    elm::genstruct::Vector<PipelineStage<N> *> _pipeline;
    Microprocessor<N> * _processor;   
  public:
    inline FunctionalUnit(fu_info_t& info, PipelineStage<N> *user_stage, Microprocessor<N> *proc);
    inline elm::String name(void)
      {return _info.name;}
    inline elm::String shortName(void)
      {return _info.short_name;}
    inline bool isPipelined(void)
      {return _info.is_pipelined;}
    inline order_policy_t orderPolicy()
      {return _info.ordder_policy;}
    inline int minLatency(void)
      {return _info.min_latency;}
    inline int maxLatency(void)
      {return _info.max_latency;}
    inline int width(void)
      {return _info.width;}
    inline PipelineStage<N> * firstStage()
      {return _pipeline.get(0);}
    class PipelineIterator: public elm::genstruct::Vector<PipelineStage<N> *>::Iterator {
    public:
      inline PipelineIterator(const FunctionalUnit *fu)
	: elm::genstruct::Vector<PipelineStage<N> *>::Iterator(fu->_pipeline) {}
    };
  };
  
  typedef struct pipeline_info_t {
    order_policy_t order_policy;
    int stage_width;
    elm::String stage_name;
    elm::String stage_short_name;
    pipeline_stage_category_t stage_category;
    Queue<N> *source_queue;
    Queue<N> *destination_queue;	
    int min_latency;
    int max_latency;
  } pipeline_info_t;
  
 private:
  pipeline_info_t _info;
  bool _uses_fus;
  Vector<Pair<Inst::kind_t, FunctionalUnit *> > bindings;
  elm::genstruct::Vector<FunctionalUnit *> fus;
  int _index;
  Microprocessor<N> * _processor;
  elm::genstruct::Vector<N *> _nodes;
  StringBuffer _category_name[NUMBER_OF_CATEGORIES];
  StringBuffer _order_name[NUMBER_OF_POLICIES];
 public:
  inline PipelineStage(pipeline_info_t& info, Microprocessor<N>* proc);
  inline order_policy_t orderPolicy(void)
    {return _info.order_policy;}
  inline int width(void) const
    {return _info.stage_width;}
  inline elm::String name(void) 
    {return _info.stage_name;}
  inline elm::String shortName(void)
    {return _info.stage_short_name;}
  inline pipeline_stage_category_t category(void)
    {return _info.stage_category;}
  inline elm::String categoryString(void)
    {return _category_name[_info.stage_category].toString();}
  inline elm::String orderPolicyString(void)
    {return _order_name[_info.order_policy].toString();}
  inline Queue<N> * sourceQueue(void)
    {return _info.source_queue;}
  inline Queue<N> * destinationQueue(void)
    {return _info.destination_queue;}
  inline int index(void)
    {return _index;}
  inline int minLatency(void)
    {return _info.min_latency;}
  inline int maxLatency(void)
    {return _info.max_latency;}
  FunctionalUnit * addFunctionalUnit(typename PipelineStage<N>::FunctionalUnit::fu_info_t& fu_info);
  inline bool usesFunctionalUnits(void)
    {return _uses_fus;}
  inline void addNode(N * node)
    {_nodes.add(node);}
  inline void deleteNodes()
    {_nodes.clear();}
  inline int numberOfNodes()
    {return _nodes.length();}
  inline N * node(int index) {
    if (index >= _nodes.length())
      return NULL;
    return _nodes[index];
  }
  elm::genstruct::Vector<FunctionalUnit *>& getFUs(void) 
    {return fus;}
  inline void addBinding(Inst::kind_t kind, FunctionalUnit *fu)
    {bindings.add(pair(kind, fu));}
  inline FunctionalUnit *findFU(Inst::kind_t kind) {
    for(int i = 0; i < bindings.length(); i++) {
      Inst::kind_t mask = bindings[i].fst;
      if((kind & mask) == mask) {
	return bindings[i].snd;
      }
    }
    cerr << "Unsupported instruction kind : " << io::hex(kind) << io::endl;
    ASSERT(0);
  }
  class ExeNodeIterator: public elm::genstruct::Vector<N *>::Iterator {
  public:
    inline ExeNodeIterator(const PipelineStage<N> *stage)
      : elm::genstruct::Vector<N *>::Iterator(stage->_nodes) {}
  };
};

template <class N>
inline PipelineStage<N>::FunctionalUnit::FunctionalUnit(fu_info_t& fu_info, PipelineStage<N> *user_stage, Microprocessor<N> *proc)
   : _info(fu_info), _processor(proc) {
   typename PipelineStage<N>::pipeline_info_t pipeline_info;	
   PipelineStage<N> *stage;
   elm::StringBuffer *number;
   elm::String number_string;
   
   user_stage->fus.add(this);
   //pipeline_info.order_policy = user_stage->orderPolicy();
   pipeline_info.order_policy = fu_info.order_policy;
   pipeline_info.stage_width = fu_info.width;
   pipeline_info.stage_name = fu_info.name;
   pipeline_info.stage_short_name = fu_info.short_name;
   if ( ! fu_info.is_pipelined) {
     pipeline_info.source_queue = user_stage->_info.source_queue;
     pipeline_info.destination_queue = user_stage->_info.destination_queue;
     pipeline_info.min_latency = fu_info.min_latency;
     pipeline_info.max_latency = fu_info.max_latency;
     stage = new PipelineStage(pipeline_info,_processor);
     this->_pipeline.add(stage);
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
	 pipeline_info.source_queue = user_stage->_info.source_queue;
	 pipeline_info.order_policy = fu_info.order_policy;
       }
       if (i==fu_info.min_latency) {
	 pipeline_info.destination_queue = user_stage->_info.destination_queue;
	 pipeline_info.max_latency = fu_info.max_latency - fu_info.min_latency + 1;	
       }
       stage = new PipelineStage(pipeline_info, _processor);
       this->_pipeline.add(stage);	
     }
   }
 }
 


// Microprocessor class
template <class N>
class Microprocessor {
 private:
  elm::genstruct::Vector<Queue<N> *> _queues;
  elm::genstruct::Vector<PipelineStage<N> *> _pipeline;
  int _pipeline_stage_index;
  PipelineStage<N> * _operand_reading_stage;
  PipelineStage<N> * _operand_producing_stage;
  
 public:
  Microprocessor(void);
  Microprocessor(const hard::Processor *proc);
  inline PipelineStage<N> * addPipelineStage(typename PipelineStage<N>::pipeline_info_t& info) ;
  inline Queue<N> * addQueue(elm::String name, int size);
  void dump(elm::io::Output& out_stream) ;
  inline int getPipelineStageIndex()
    {return _pipeline_stage_index++;}
  inline void setOperandReadingStage(PipelineStage<N> * stage)
    {_operand_reading_stage = stage;}
  inline void setOperandProducingStage(PipelineStage<N> * stage)
    {_operand_producing_stage = stage;}
  inline PipelineStage<N> * operandReadingStage(void)
    {return _operand_reading_stage;}
  inline PipelineStage<N> *operandProducingStage(void)
    {return _operand_producing_stage;}
  inline bool isLastStage(PipelineStage<N> *stage)
    {return (_pipeline.get(_pipeline.length()-1) == stage);}
  
  class PipelineIterator: public elm::genstruct::Vector<PipelineStage<N> *>::Iterator {
  public:
    inline PipelineIterator(const Microprocessor<N> *processor)
      : elm::genstruct::Vector<PipelineStage<N> *>::Iterator(processor->_pipeline) {}
  };
  
  class QueueIterator: public elm::genstruct::Vector<Queue<N> *>::Iterator {
  public:
    inline QueueIterator(const Microprocessor<N> *processor)
      : elm::genstruct::Vector<Queue<N> *>::Iterator(processor->_queues) {}
  };
  
};

template <class N>
inline PipelineStage<N> * Microprocessor<N>::addPipelineStage(typename PipelineStage<N>::pipeline_info_t& info) {
  PipelineStage<N> *stage = new PipelineStage<N>(info, this);
  _pipeline.add(stage);
  if ( stage->sourceQueue() != NULL)
    stage->sourceQueue()->setEmptyingStage(stage);
  if ( stage->destinationQueue() != NULL)
    stage->destinationQueue()->setFillingStage(stage);
  return stage;
}

template <class N>
inline Queue<N> * Microprocessor<N>::addQueue(elm::String name, int size) {
  Queue<N> *queue = new Queue<N>(name, size);
  _queues.add(queue);
  return queue;
}

template <class N>
inline PipelineStage<N>::PipelineStage(PipelineStage<N>::pipeline_info_t & info, Microprocessor<N> * proc)
    : _info(info), _uses_fus(false), _processor(proc) {
    _index = proc->getPipelineStageIndex();
    _category_name[NO_CATEGORY] << "NONE";
    _category_name[FETCH] << "FETCH";
    _category_name[DECODE] << "DECODE";
    _category_name[EXECUTE] << "EXECUTE";
    _category_name[WRITE] << "WRITE";
    _category_name[COMMIT] << "COMMIT";
    _category_name[DELAY] << "DELAY";
    _order_name[NO_POLICY] << "NONE";
    _order_name[IN_ORDER] << "IN_ORDER";
    _order_name[OUT_OF_ORDER] << "OUT_OF_ORDER";
  }
  
template <class N>
typename PipelineStage<N>::FunctionalUnit * PipelineStage<N>::addFunctionalUnit(typename PipelineStage<N>::FunctionalUnit::fu_info_t& fu_info) {	
	FunctionalUnit *fu = new FunctionalUnit(fu_info, this, _processor);
	_uses_fus = true;
	return fu;
}
  

/**
 * Build an empty microprocessor.
 */
template <class N>
Microprocessor<N>::Microprocessor(void)
:	_pipeline_stage_index(0),
	_operand_reading_stage(0),
	_operand_producing_stage(0)
{
}


/**
 * Build a microprocessor for ExeGraph from the abstract definition provided
 * by OTAWA.
 * @param proc	OTAWA abstract processor definition.
 */

template <class N>
Microprocessor<N>::Microprocessor(const hard::Processor *proc)
:	_pipeline_stage_index(0), 
	_operand_reading_stage(0),
	_operand_producing_stage(0)
{
	ASSERT(proc);
	
	// Create queues
	Vector<Queue<N> *> queues;
	const Table<hard::Queue *>& oqueues = proc->getQueues(); 
	for(int i = 0; i < oqueues.count(); i++)
		queues.add(addQueue(
			oqueues[i]->getName(),
			1 << oqueues[i]->getSize()));
	
	// Create stages
	const Table<hard::Stage *>& ostages = proc->getStages();
	for(int i = 0; i < ostages.count(); i++) {
		typename PipelineStage<N>::pipeline_info_t info;
		
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
			info.order_policy = PipelineStage<N>::IN_ORDER;
			info.stage_category = PipelineStage<N>::FETCH;
			break;
		case hard::Stage::LAZY:
			info.order_policy = PipelineStage<N>::IN_ORDER;
			info.stage_category = PipelineStage<N>::DECODE;
			break;
 		case hard::Stage::EXEC:
 			is_exec = true;
 			exec_stage_index = i;
			info.order_policy = ostages[i]->isOrdered()
				? PipelineStage<N>::IN_ORDER : PipelineStage<N>::OUT_OF_ORDER;
			info.stage_category = PipelineStage<N>::EXECUTE;
			break;
 		case hard::Stage::COMMIT:
			info.order_policy = PipelineStage<N>::IN_ORDER;
			info.stage_category = PipelineStage<N>::COMMIT;
			break;
 		default:
 			ASSERT(0);
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
		PipelineStage<N> *stage = addPipelineStage(info);
		
		// Add functional units if required
		if(is_exec) {
			setOperandReadingStage(stage);
			setOperandProducingStage(stage);
			// Build the FU
			const Table<hard::FunctionalUnit *>& fus = ostages[i]->getFUs();
			for(int j = 0; j < fus.count(); j++) {
				typename PipelineStage<N>::FunctionalUnit::fu_info_t info;
				info.name = fus[j]->getName();
				info.short_name = fus[j]->getName();
				info.is_pipelined = fus[j]->isPipelined();
				info.max_latency = fus[j]->getLatency();
				info.min_latency = fus[j]->getLatency();
				info.order_policy = ostages[exec_stage_index]->isOrdered() 
					? PipelineStage<N>::IN_ORDER : PipelineStage<N>::OUT_OF_ORDER;
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
				ASSERT(found);
			}
		}
	}
	
}


/**
 * Dump the description of the microprocessor.
 * @param out_stream	Used output stream.
 */
template <class N>
void Microprocessor<N>::dump(elm::io::Output& out_stream) {
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

#endif // OTAWA_MICROPROCESSOR_H
