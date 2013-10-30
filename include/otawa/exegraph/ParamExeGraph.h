/*
 *	$Id$
 *	exegraph module implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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

#ifndef _PARAM_EXEGRAPH_H_
#define _PARAM_EXEGRAPH_H_

#include <otawa/exegraph/ExeGraph.h>
#include <otawa/util/Trace.h>
#include <elm/util/BitVector.h>

namespace otawa { 

#define INFINITE_TIME 0x0FFFFFF

class ParamExeNode;

class ParamExeGraph : public ExeGraph<ParamExeNode> {
 public:
  typedef class ParamExeNode Node;
  typedef enum {STAGE, FU, QUEUE, REG, EXTERNAL_CONFLICT, INTERNAL_CONFLICT, BLOCK_START, RES_TYPE_NUM} resource_type_t;
  typedef struct input_t {
    hard::RegBank * reg_bank;
    elm::genstruct::AllocatedTable<bool> *_is_input;
    elm::genstruct::AllocatedTable<int> * _resource_index;
  } input_t;
  class Resource {
  private:
    elm::String _name;
    resource_type_t _type;
    int _index;
    int _offset; 
  public:
    inline Resource(elm::String name, resource_type_t type, int index, int offset)
      : _name(name), _type(type), _index(index), _offset(offset) {}
    inline elm::String name()
    {return _name;}
    inline resource_type_t type()
    {return _type;}
    inline int index()
    {return _index;}
    inline int offset()
    {return _offset;}
  };	
  class StartResource : public Resource {
  public:
    inline StartResource(elm::String name, int index, int offset)
      :Resource(name,BLOCK_START, index, offset) {}
  };
  class StageResource : public Resource {
  private:
    PipelineStage<ParamExeNode> *_stage;
    int _slot;
  public:
    inline StageResource(elm::String name,
			 PipelineStage<ParamExeNode> *stage,
			 int slot,
			 int index,
			 int offset)
      :	Resource(name,STAGE, index, offset),
      _stage(stage), _slot(slot)  {}
    inline PipelineStage<ParamExeNode> * stage()
    {return _stage;}
    inline int slot()
    {return _slot;}
  };
  class FuResource : public Resource {
  private:
    PipelineStage<ParamExeNode>::FunctionalUnit *_fu;
    int _slot;
  public:
    inline FuResource(elm::String name,
		      PipelineStage<ParamExeNode>::FunctionalUnit * fu,
		      int slot,
		      int index,
		      int offset)
      :	Resource(name,FU,index, offset), _fu(fu), _slot(slot) {}
    inline PipelineStage<ParamExeNode>::FunctionalUnit * fu()
    {return _fu;}
    inline int slot()
    {return _slot;}
  };
  class QueueResource : public Resource {
  private: 
    Queue<ParamExeNode> *_queue;
    int _slot;
    StageResource * _upper_bound;
    int _upper_bound_offset;
  public:
    inline QueueResource(elm::String name,
			 Queue<ParamExeNode> * queue,
			 int slot,
			 int index,
			 int offset,
			 StageResource * upper_bound,
			 int upper_bound_offset)
      :	Resource(name,QUEUE, index, offset),
      _queue(queue),
      _slot(slot),
      _upper_bound(upper_bound),
      _upper_bound_offset(upper_bound_offset)
      {}
    inline Queue<ParamExeNode> * queue()
    {return _queue;}
    inline int slot()
    {return _slot;}
    StageResource * upperBound() 
    {return _upper_bound;}
    int upperBoundOffset()
    {return _upper_bound_offset;}
  };
  class RegResource : public Resource {
  private:
    elm::genstruct::Vector<ExeInst<ParamExeNode> *> _using_instructions;
    otawa::hard::RegBank * _reg_bank;
    int _reg_index;
  public:
    inline RegResource(elm::String name,
		       otawa::hard::RegBank * reg_bank,
		       int reg_index,
		       int index,
		       int offset)
      :	Resource(name,REG, index, offset),
      _reg_bank(reg_bank),
      _reg_index(reg_index)
      {}
    inline otawa::hard::RegBank * regBank()
    {return _reg_bank;}
    inline int regIndex()
    {return _reg_index;}
    inline void addUsingInst(ExeInst<ParamExeNode> * inst) {
      if (! _using_instructions.contains(inst) )
	_using_instructions.add(inst);
    }
    class UsingInstIterator : public elm::genstruct::Vector<ExeInst<ParamExeNode> *>::Iterator {
    public:
    UsingInstIterator(const RegResource * res)
      : elm::genstruct::Vector<ExeInst<ParamExeNode> *>::Iterator(res->_using_instructions) {}
    };
  };
  class ExternalConflictResource : public Resource {
  private:
    ExeInst<ParamExeNode> * _instruction;
  public:
  ExternalConflictResource(elm::String name,
			   ExeInst<ParamExeNode> * instruction,
			   int index,
			   int offset)
    :	Resource(name,EXTERNAL_CONFLICT, index, offset),
      _instruction(instruction)
      {}
    ExeInst<ParamExeNode> * instruction()
      {return _instruction;}
  };
  class InternalConflictResource : public Resource {
  private:
    ExeInst<ParamExeNode> * _instruction;
    ParamExeNode * _node;
  public:
  InternalConflictResource(elm::String name,
			   ExeInst<ParamExeNode> * instruction,
			   int index,
			   int offset)
    :	Resource(name,INTERNAL_CONFLICT, index, offset),
      _instruction(instruction)
      {}
    ExeInst<ParamExeNode> * instruction()
      {return _instruction;}
    void setNode(ParamExeNode *node)
    {_node = node;}
    ParamExeNode *node()
    {return _node;}

  };
 private:
  Trace *_trace;

 protected:
  bool _times_changed;
  int _resource_count;	
  int _capacity;
  elm::genstruct::Vector<Resource *> _resources;
  elm::genstruct::Vector<Resource *> _internal_conflicts;
  elm::genstruct::Vector<Resource *> _external_conflicts;
  int _latest_resource_index; // index of the resource that stands for the last pipeline stage
  int _last_resource_index; // index of the last created resource
  bool _has_pred;
  ExeSequence<ParamExeNode> *_sequence;
  elm::genstruct::Vector<ParamExeNode *> _contending_nodes;
  bool res_dep[RES_TYPE_NUM];
  int res_dep_impact[RES_TYPE_NUM];

 public:
  ParamExeGraph(WorkSpace *fw,  Microprocessor<ParamExeNode> * microprocessor, ExeSequence<ParamExeNode> *sequence, bool has_pred,
		Trace *_trace);
  virtual void propagate();
  virtual void initDelays();
  virtual int Delta(ParamExeNode *a, Resource *res);
  virtual void analyzeContentions();
  inline void clearTimesChanged() 
  {_times_changed = false	;}
  inline void setTimesChanged() 
  {_times_changed = true;}
  inline bool timesChanged()
  {return _times_changed;}
  void dump(elm::io::Output& dotFile);
  void dumpSimple(elm::io::Output& dotFile);
  inline int resourceCount()
  {return _resources.length() + _internal_conflicts.length() + _external_conflicts.length();}
  inline int instructionCount()
  {return _sequence->length();}
  int analyze();
  virtual int cost();
  int latestResourceIndex()
  {return _latest_resource_index;}
  elm::String delayName(int delay);
  class ResourceIterator : public elm::genstruct::Vector<Resource *>::Iterator {
  public:
  ResourceIterator(const ParamExeGraph * graph)
    : elm::genstruct::Vector<Resource *>::Iterator(graph->_resources) {}
  };
  class InternalConflictIterator : public elm::genstruct::Vector<Resource *>::Iterator {
  public:
  InternalConflictIterator(const ParamExeGraph * graph)
    : elm::genstruct::Vector<Resource *>::Iterator(graph->_internal_conflicts) {}
  };
  class ExternalConflictIterator : public elm::genstruct::Vector<Resource *>::Iterator {
  public:
  ExternalConflictIterator(const ParamExeGraph * graph)
    : elm::genstruct::Vector<Resource *>::Iterator(graph->_external_conflicts) {}
  };
};


 class ParamExeNode : public ExeGraph<ParamExeNode>::ExeNode {
 private:
  ParamExeGraph * _graph;
  elm::genstruct::AllocatedTable<int> * _d[ExeGraph<ParamExeNode>::BOUNDS];
  //elm::genstruct::AllocatedTable<bool> * _e;
  int _max_contention_delay;
  int _min_contention_delay;
  int _stage_number;
  elm::genstruct::AllocatedTable<bool> * _contention_dep;
  elm::genstruct::AllocatedTable<int> * _delay[ExeGraph<ParamExeNode>::BOUNDS];
  int _internal_delay[ExeGraph<ParamExeNode>::BOUNDS];
  int _contention_delay[ExeGraph<ParamExeNode>::BOUNDS];
  elm::genstruct::SLList<ParamExeGraph::InternalConflictResource *> _contenders_list;
  elm::BitVector * _possible_contenders;
  elm::genstruct::SLList<elm::BitVector *> _contenders_masks_list;
  int _late_contenders;

 public:
  inline  ParamExeNode(ExeGraph<ParamExeNode> *graph, PipelineStage<ParamExeNode> *stage, ExeInst<ParamExeNode> *inst) ;
  elm::genstruct::SLList<elm::BitVector *>* contendersMasksList()
    {return &_contenders_masks_list;}
  elm::BitVector * possibleContenders()
    {return _possible_contenders;}
  inline int lateContenders()
  {return _late_contenders;}
  inline void setLateContenders(int num)
  {_late_contenders = num;}
  inline void initContenders(int size)
  {_possible_contenders = new BitVector(size);}
  inline bool isContender(int index)
  {return(_possible_contenders[index]);}
  inline void setContender(int index)
  {_possible_contenders->set(index);}
  inline int contendersNumber()
  {return(_possible_contenders->countBits());}
  void buildContendersMasks();
  inline void addConflict(ParamExeGraph::InternalConflictResource *conflict)
  {_contenders_list.addLast(conflict);}
  inline elm::genstruct::SLList<ParamExeGraph::InternalConflictResource *>* conflictList()
  {return &_contenders_list;}
  inline void setStageNumber(int number)
  {_stage_number = number;}
  inline int stageNumber()
  {return _stage_number;}
  inline bool hasContenders()
  {return (_contenders.length() != 0);}
  inline int d(int index, ExeGraph<ParamExeNode>::time_bound_t bound)
  {return (*_d[bound])[index];}
  inline int d(int index)
  {return (*_d[ExeGraph<ParamExeNode>::MIN])[index];} 
  inline bool e(int index)
  {/*return (*_e)[index];*/
    return ((*_d[ExeGraph<ParamExeNode>::MIN])[index]>=0);}
  inline void setD(int index, int value, ExeGraph<ParamExeNode>::time_bound_t bound)
  {(*_d[bound])[index] = value;}
  inline void setD(int index, int value)
  {(*_d[ExeGraph<ParamExeNode>::MIN])[index] = value;} 
  inline void setE(int index, bool value)
  { /*(*_e)[index] = value;*/
    if (value) {
      if ((*_d[ExeGraph<ParamExeNode>::MIN])[index]<0) {
	(*_d[ExeGraph<ParamExeNode>::MIN])[index] = 0;
      }
    }
    else
      (*_d[ExeGraph<ParamExeNode>::MIN])[index] = -1;
  }
  inline int maxContentionDelay()
  {return _max_contention_delay;}
  inline void setMaxContentionDelay(int value)
  {_max_contention_delay = value;}
  inline int minContentionDelay()
  {return _min_contention_delay;}
  inline void setMinContentionDelay(int value)
  {_min_contention_delay = value;}
  inline int delay(int index,  ExeGraph<ParamExeNode>::time_bound_t bound)
  {return (*_delay[bound])[index];}
  inline bool contentionDep(int index)
  {return (*_contention_dep)[index];}
  inline int internalDelay(ExeGraph<ParamExeNode>::time_bound_t bound)
  {return _internal_delay[bound];}
  inline int contentionDelay(ExeGraph<ParamExeNode>::time_bound_t bound)
  {return _contention_delay[bound];}
  inline void setDelay(int index, ExeGraph<ParamExeNode>::time_bound_t bound, int delay) {
    if ( (*_delay[bound])[index] != delay ) {
      (*_delay[bound])[index] = delay;
      _graph->setTimesChanged();
    }
  }
  inline void setContentionDep(int index) {
    (*_contention_dep)[index] = true;

  }		
  inline void setInternalDelay(ExeGraph<ParamExeNode>::time_bound_t bound, int delay) {
    if ( _internal_delay[bound] != delay ) {
      _internal_delay[bound] = delay;
      _graph->setTimesChanged();
    }
  }
  inline void setContentionDelay(ExeGraph<ParamExeNode>::time_bound_t bound, int delay) {
    if ( _contention_delay[bound] != delay ) {
      _contention_delay[bound] = delay;
      _graph->setTimesChanged();
    }
  }



};


 inline  ParamExeNode::ParamExeNode(
				    ExeGraph<ParamExeNode> *graph,
				    PipelineStage<ParamExeNode> *stage,
				    ExeInst<ParamExeNode> *inst) 
   :	ExeNode(graph, stage, inst), _graph((ParamExeGraph *)graph) {
   _max_contention_delay = 0;
   _min_contention_delay = 0;
   //  _round = 1;
   for (int j=0 ; j<ExeGraph<ParamExeNode>::BOUNDS ; j++) {
     _delay[j] = new elm::genstruct::AllocatedTable<int>(_graph->resourceCount());
     for (int k=0 ; k<_graph->resourceCount() ; k++) {
       (*_delay[j])[k] = -INFINITE_TIME;
     }	
     _internal_delay[j] = 0;

   }
   for (int j=0 ; j<ExeGraph<ParamExeNode>::BOUNDS ; j++) {
     _d[j] = new elm::genstruct::AllocatedTable<int>(_graph->resourceCount());
   }
   //_e = new elm::genstruct::AllocatedTable<bool>(_graph->resourceCount());
   for (int k=0 ; k<_graph->resourceCount() ; k++) {
     for (int j=0 ; j<ExeGraph<ParamExeNode>::BOUNDS ; j++) 
       (*_d[j])[k] = /*0*/-1;
     //(*_e)[k] = false;
   }
   _contention_dep =  new elm::genstruct::AllocatedTable<bool>(_graph->instructionCount());
   for (int k=0 ; k<_graph->instructionCount() ; k++) {
     (*_contention_dep)[k] = false;
   }

   _contention_delay[ExeGraph<ParamExeNode>::MIN] = 0;
   if (pipelineStage()->orderPolicy() == PipelineStage<ParamExeNode>::OUT_OF_ORDER)
     _contention_delay[ExeGraph<ParamExeNode>::MAX] = 1;
   else
     _contention_delay[ExeGraph<ParamExeNode>::MAX] = 0;
 }




 inline ParamExeGraph::ParamExeGraph(
				     WorkSpace *fw,
				     Microprocessor<ParamExeNode> * microprocessor, 
				     ExeSequence<ParamExeNode> *sequence,
				     bool has_pred,
				     Trace *trace) 
   : ExeGraph<ParamExeNode>::ExeGraph() {
   _trace = trace;

   int resource_index = 0;
   _has_pred = has_pred;
   _sequence = sequence;  
   {
     StringBuffer buffer;
     buffer << "start";
     StartResource * new_resource = new StartResource(buffer.toString(), resource_index++, 0);
     _resources.add(new_resource);
   }

   int _last_stage_index = 0;
   int _operand_producing_stage_index;
   for (PipelineIterator stage(microprocessor) ; stage ; stage++) {
     if (stage == microprocessor->operandProducingStage())
       _operand_producing_stage_index = _last_stage_index;
     _last_stage_index++;
   }
   _last_stage_index--;
   int _stage_index = 0;
   for (PipelineIterator stage(microprocessor) ; stage ; stage++) {
     if (! stage->usesFunctionalUnits()) {

       for (int i=0 ; i<stage->width() ; i++) {
	 StringBuffer buffer;
	 buffer << stage->name() << "[" << i << "]";
	 StageResource * new_resource = new StageResource(buffer.toString(), stage, i, resource_index++, _last_stage_index - _stage_index);
	 _resources.add(new_resource);
       }
     }
     else { // stage uses functional units
       if (stage->orderPolicy() == PipelineStage<ParamExeNode>::IN_ORDER) {
	 for(FunctionalUnitIterator fu(stage); fu; fu++) {
	   for (int i=0 ; i<fu->width() ; i++) {
	     StringBuffer buffer;
	     buffer << fu->name() << "[" << i << "]";
	     FuResource * new_resource = new FuResource(buffer.toString(), fu, i, resource_index++, _last_stage_index - _stage_index);
	     _resources.add(new_resource);
	   }
	 }
       }
     }
     _latest_resource_index = _resources.length() - 1;
     _stage_index++;
   }
   for (Microprocessor<ParamExeNode>::QueueIterator queue(microprocessor) ; queue ; queue++) {
     int num = queue->size();
     if (num > sequence->count())
       num = sequence->count();
     for (int i=0 ; i<num ; i++) {
       StringBuffer buffer;
       buffer << queue->name() << "[" << i << "]";
       int _i = 0, _empty_i;
       for (Microprocessor<ParamExeNode>::PipelineIterator stage(microprocessor) ; stage ; stage++) {
	 if (stage == queue->emptyingStage())
	   _empty_i = _i;
	 _i++;
       }
       StageResource * upper_bound;
       int upper_bound_offset;
       for (elm::genstruct::Vector<Resource *>::Iterator resource(_resources) ; resource ; resource++) {
	 if (resource->type() == STAGE) {
	   if (((StageResource *)(*resource))->stage() == queue->emptyingStage()) {
	     if (i < queue->size() - ((StageResource *)(*resource))->stage()->width() - 1) {
	       if (((StageResource *)(*resource))->slot() == ((StageResource *)(*resource))->stage()->width()-1) {
		 upper_bound = (StageResource *) (*resource);
		 upper_bound_offset = (queue->size() - i) / ((StageResource *)(*resource))->stage()->width();
	       }
	     }
	     else {
	       if (((StageResource *)(*resource))->slot() == i - queue->size() + ((StageResource *)(*resource))->stage()->width()) {
		 upper_bound = (StageResource *) (*resource);
		 upper_bound_offset = 0;
	       }
	     }
	   }
	 }
       }
       ASSERT(upper_bound);
       QueueResource * new_resource = new QueueResource(buffer.toString(), queue, i, resource_index++, 
							_last_stage_index - _empty_i, upper_bound, upper_bound_offset);
       _resources.add(new_resource);
     }
   }
   otawa::hard::Platform *pf = fw->platform();
   AllocatedTable<input_t> inputs(pf->banks().count());
   int reg_bank_count = pf->banks().count();
   for(int i = 0; i <reg_bank_count ; i++) {
     inputs[i].reg_bank = (otawa::hard::RegBank *) pf->banks()[i];
     inputs[i]._is_input = 
       new AllocatedTable<bool>(inputs[i].reg_bank->count());
     inputs[i]._resource_index =
       new AllocatedTable<int>(inputs[i].reg_bank->count());
     for (int j=0 ; j<inputs[i].reg_bank->count() ; j++) {
       inputs[i]._is_input->set(j,true);
       inputs[i]._resource_index->set(j,-1);
     }
   }
   for (InstIterator inst(sequence) ; inst ; inst++) {
     const elm::genstruct::Table<hard::Register *>& reads = inst->inst()->readRegs();
     for(int i = 0; i < reads.count(); i++) {
       for (int b=0 ; b<reg_bank_count ; b++) {
	 if (inputs[b].reg_bank == reads[i]->bank()) {
	   if (inputs[b]._is_input->get(reads[i]->number()) == true) {
	     if (inputs[b]._resource_index->get(reads[i]->number()) == -1) {
	       //new input coming from outside the sequence
	       StringBuffer buffer;
	       buffer << reads[i]->bank()->name() << reads[i]->number();
	       RegResource * new_resource = new RegResource(buffer.toString(), reads[i]->bank(), reads[i]->number(), resource_index++,
							    _last_stage_index - _operand_producing_stage_index);
	       _resources.add(new_resource);
	       new_resource->addUsingInst(inst);
	       inputs[b]._resource_index->set(reads[i]->number(), _resources.length()-1);
	     }
	     else {
	       ((RegResource *)_resources[inputs[b]._resource_index->get(reads[i]->number())])->addUsingInst(inst);
	     }
	   }
	 }
       }
     }
     const elm::genstruct::Table<hard::Register *>& writes = inst->inst()->writtenRegs();
     for(int i = 0; i < writes.count(); i++) {
       for (int b=0 ; b<reg_bank_count ; b++) {
	 if (inputs[b].reg_bank == writes[i]->bank()) {
	   inputs[b]._is_input->set(writes[i]->number(), false);
	 }
       }
     }	
   }

   int i = 0;
   for (InstIterator inst(sequence) ; inst ; inst++) {
     StringBuffer buffer;
     buffer << "extconf[" << i << "]";
     ExternalConflictResource * new_resource = new ExternalConflictResource(buffer.toString(), inst, resource_index++, 1);	
     //_resources.add(new_resource);
     _external_conflicts.add(new_resource);
     StringBuffer another_buffer;
     another_buffer << "intconf[" << i << "]";
     InternalConflictResource * another_new_resource = new InternalConflictResource(another_buffer.toString(), inst, resource_index++, 1);
     //_resources.add(another_new_resource);
     _internal_conflicts.add(another_new_resource);
     i++;
   }
   _last_resource_index = resource_index - 1 ;
 }

#define EPSILON 10
 inline elm::String ParamExeGraph::delayName(int delay) {
   StringBuffer _buffer;
   if (delay < -INFINITE_TIME+EPSILON)	
     _buffer << "-INF";
   else {
     if (delay > INFINITE_TIME-EPSILON)
       _buffer << "-INF";
     else
       _buffer << delay;
   }
   return _buffer.toString();
 }


}; // namespace otawa

#endif //_PARAM_EXEGRAPH_H_
