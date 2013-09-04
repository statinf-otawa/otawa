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

#ifndef OTAWA_PAREXEPROC_H
#define OTAWA_PAREXEPROC_H

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

  class ParExeStage;
  class ParExePipeline;
  class ParExeProc;
  class ParExeNode;

  namespace hard {
    class Microprocessor;
  } // hard



  //  instruction_category_t instCategory(Inst *inst);

  /*
   * class ParExeQueue
   *
   */

  class ParExeQueue {
  private:
    elm::String _name;
    int _size;
    ParExeStage * _filling_stage;
    ParExeStage * _emptying_stage;
  public:
    inline ParExeQueue(elm::String name, int size)
      : _name(name), _size(size) {}
    inline elm::String name(void)
      {return _name;}
    inline int size(void) 
      {return _size;}
    inline ParExeStage * fillingStage(void)
      {return _filling_stage;}
    inline ParExeStage * emptyingStage(void)
      {return _emptying_stage;}
    inline void setFillingStage(ParExeStage * stage)
      {_filling_stage = stage;}
    inline void setEmptyingStage(ParExeStage * stage)
      {_emptying_stage = stage;}
  };


  /*
   * class ParExeStage
   *
   */

  class ParExeStage {
  public:
    typedef enum order_t {IN_ORDER, OUT_OF_ORDER} order_policy_t;
    typedef enum pipeline_stage_category_t {FETCH, DECODE, EXECUTE, COMMIT, DELAY, FU} pipeline_stage_category_t;
  
  private:
    const hard::PipelineUnit *_unit;
    pipeline_stage_category_t _category;
    int _latency;
    int _width;
    order_t _order_policy;
    ParExeQueue *_source_queue;
    ParExeQueue *_destination_queue;	
    elm::String _name;
    int _index;
    Vector<Pair<Inst::kind_t, ParExePipeline *> > _bindings;
    elm::genstruct::Vector<ParExePipeline *> _fus;
    elm::genstruct::Vector<ParExeNode *> _nodes;
  public:
    inline ParExeStage(pipeline_stage_category_t category, int latency, int width, order_t policy, ParExeQueue *sq, ParExeQueue *dq, elm::String name, int index=0, const hard::PipelineUnit *unit = 0);

	inline void addNode(ParExeNode * node)
		{ _nodes.add(node); }
	inline void removeNode(ParExeNode *node)
		{ _nodes.remove(node); }

	inline const hard::PipelineUnit *unit(void) const { return _unit; }
	inline order_t orderPolicy(void) {return _order_policy;}
	inline int width(void) const {return _width;}
	inline elm::String name(void)  {return _name;}
	inline int index(void) {return _index;}
	inline pipeline_stage_category_t category(void) {return _category;}
	inline ParExeQueue * sourceQueue(void) {return _source_queue;}
	inline ParExeQueue * destinationQueue(void) {return _destination_queue;}
	inline int latency(void) {return _latency;}
	inline bool isFuStage(void) {return (_category == FU);}
	inline void addFunctionalUnit(bool pipelined, int latency, int width, elm::String name, const hard::PipelineUnit *unit);
	inline int numFus(void) {return _fus.length();}
	inline ParExePipeline *fu(int index) {return _fus[index];}
	inline ParExeNode* firstNode(void) {return _nodes[0];}
	inline ParExeNode* lastNode(void) {return _nodes[_nodes.length()-1];}
	inline bool hasNodes(void) {return (_nodes.length() != 0);}
	inline void deleteNodes(void) {if (_nodes.length() != 0) _nodes.clear();}
	inline int numNodes(void) {return _nodes.length();}
	inline ParExeNode * node(int index)
		{ if (index >= _nodes.length()) return NULL; else return _nodes[index]; }
	inline void addBinding(Inst::kind_t kind, ParExePipeline *fu) {_bindings.add(pair(kind, fu));}

     inline ParExePipeline *findFU(Inst::kind_t kind) {
		for(int i = 0; i < _bindings.length(); i++) {
			Inst::kind_t mask = _bindings[i].fst;
			if((kind & mask) == mask)
				return _bindings[i].snd;
		}
		return 0;
	}

    class NodeIterator: public elm::genstruct::Vector<ParExeNode *>::Iterator {
    public:
      inline NodeIterator(const ParExeStage *stage)
	: elm::genstruct::Vector<ParExeNode *>::Iterator(stage->_nodes) {}
    };
    
  };


  /*
   * class ParExePipeline
   *
   */

  class ParExePipeline { 
  protected:
    elm::genstruct::Vector<ParExeStage *> _stages;
  private:
    ParExeStage * _last_stage;
    ParExeStage * _first_stage;
  public:
    ParExePipeline()
      : _first_stage(NULL){    }
    ~ParExePipeline() {    }
    inline ParExeStage *lastStage()
    {return _last_stage;}
    inline ParExeStage *firstStage()
    {return _first_stage; }
    inline void addStage(ParExeStage *stage);
    inline int numStages()
    {return _stages.length();}

    class StageIterator: public elm::genstruct::Vector<ParExeStage *>::Iterator {
    public:
      inline StageIterator(const ParExePipeline *pipeline)
	: elm::genstruct::Vector<ParExeStage *>::Iterator(pipeline->_stages) {}
    };
  };


  inline void ParExePipeline::addStage(ParExeStage *stage){
    _stages.add(stage);
    if (_first_stage == NULL)
      _first_stage = stage;
    _last_stage = stage;
    if ( stage->sourceQueue() != NULL)
      stage->sourceQueue()->setEmptyingStage(stage);
    if ( stage->destinationQueue() != NULL)
      stage->destinationQueue()->setFillingStage(stage);
   }

  /*
   * class ParExeProc
   *
   */

  class ParExeProc {
  private:
    elm::genstruct::Vector<ParExeQueue *> _queues;
    ParExePipeline _pipeline;
    ParExeStage * _fetch_stage;
    ParExeStage * _exec_stage;
 
  public:
      typedef enum instruction_category_t {	
	IALU = 0,
	FALU = 1,
	MEMORY = 2,
	CONTROL = 3,
	MUL = 4,
	DIV = 5,
	INST_CATEGORY_NUMBER   // must be the last value
      } instruction_category_t;	


    ParExeProc(const hard::Processor *proc);
    inline void addQueue(elm::String name, int size){
        _queues.add(new ParExeQueue(name, size));
    }
    inline ParExeQueue * queue(int index){
      return _queues[index];
    }
    inline void setFetchStage(ParExeStage * stage)
      {_fetch_stage = stage;}
    inline ParExeStage * fetchStage(void)
      {return _fetch_stage;}
    inline void setExecStage(ParExeStage * stage)
      {_exec_stage = stage;}
    inline ParExeStage * execStage(void)
      {return _exec_stage;}
    inline ParExeStage * lastStage(void)
    {return _pipeline.lastStage() ;}
    inline bool isLastStage(ParExeStage *stage)
     {return (_pipeline.lastStage() == stage);}
    inline ParExePipeline *pipeline()
     { return &_pipeline;}
    
    class QueueIterator: public elm::genstruct::Vector<ParExeQueue *>::Iterator {
    public:
      inline QueueIterator(const ParExeProc *processor)
	: elm::genstruct::Vector<ParExeQueue *>::Iterator(processor->_queues) {}
    };

  };



 
  inline ParExeStage::ParExeStage(pipeline_stage_category_t category, int latency, int width, order_t policy, ParExeQueue *sq, ParExeQueue *dq, elm::String name, int index, const hard::PipelineUnit *unit)
  : _unit(unit), _category(category), _latency(latency), _width(width), _order_policy(policy),
    _source_queue(sq), _destination_queue(dq), _name(name), _index(index) {}

  inline void ParExeStage::addFunctionalUnit(bool pipelined, int latency, int width, elm::String name, const hard::PipelineUnit *unit) {
	  ParExePipeline *fu = new ParExePipeline();
	  if ( !pipelined) {
		  ParExeStage * stage = new ParExeStage(FU, latency, width, _order_policy, _source_queue, _destination_queue, name, 0, unit);
		  fu->addStage(stage);
	  }
	  else {
		  ParExeStage * stage;

		  // first_stage
		  stage = new ParExeStage(FU, 1, width, _order_policy, _source_queue, NULL, name + "1", 0, unit);
		  fu->addStage(stage);

		  // intermediate stages
		  for (int i=2 ; i<latency ; i++) {
			  stage = new ParExeStage(FU, 1, width, IN_ORDER, NULL, NULL, _ << name << i, 0, unit);
			  fu->addStage(stage);
		  }

		  // last stage
		  stage = new ParExeStage(FU, 1, width, IN_ORDER, NULL, _destination_queue, _ << name << latency, 0, unit);
		  fu->addStage(stage);
	  }
	  _fus.add(fu);
  }
  



} // otawa

#endif // OTAWA_MICROPROCESSOR_H
