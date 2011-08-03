/*
 *	$Id$
 *	Interface to the EGQueue, EGProc, EGStage, EGPipeline classes.
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


#ifndef _EG_PROC_H
#define _EG_PROC_H

#include <elm/io.h>
#include <elm/genstruct/Vector.h>
#include <elm/string/String.h>
#include <elm/genstruct/DLList.h>
#include <stdio.h>
#include <otawa/otawa.h>
#include <otawa/hard/Processor.h>

namespace otawa { namespace exegraph2 {

  using namespace elm;
  using namespace elm::genstruct;

  class EGStage;
  class EGPipeline;
  class EGProc;
  class EGNode;

  namespace hard {
    class Microprocessor;
  } // hard

  /*
   * class EGQueue
   *
   */

  class EGQueue {
  private:
    elm::String _name;
    int _size;
    EGStage * _filling_stage;
    EGStage * _emptying_stage;
  public:
    inline EGQueue(elm::String name, int size)
      : _name(name), _size(size) {}
    // get information
    inline elm::String name(void)
      {return _name;}
    inline int size(void)
      {return _size;}
    inline EGStage * fillingStage(void)
      {return _filling_stage;}
    inline EGStage * emptyingStage(void)
      {return _emptying_stage;}
    // set information
    inline void setFillingStage(EGStage * stage)
      {_filling_stage = stage;}
    inline void setEmptyingStage(EGStage * stage)
      {_emptying_stage = stage;}
  };


  /*
   * class EGStage
   *
   */

  class EGStage {
  public:
    typedef enum order_t {IN_ORDER, OUT_OF_ORDER} order_policy_t;
    typedef enum pipeline_stage_category_t {FETCH, DECODE, EXECUTE, COMMIT, DELAY, FU} pipeline_stage_category_t;

  private:
    pipeline_stage_category_t _category;
    int _latency;
    int _width;
    order_t _order_policy;
    EGQueue *_source_queue;
    EGQueue *_destination_queue;
    elm::String _name;
    int _index;
    Vector<Pair<Inst::kind_t, EGPipeline *> > _bindings;
    elm::genstruct::Vector<EGPipeline *> _fus;
    elm::genstruct::Vector<EGNode *> _nodes;
  public:
    inline EGStage(pipeline_stage_category_t category, int latency, int width, order_t policy, EGQueue *sq, EGQueue *dq, elm::String name, int index=0);
    // get information
    inline pipeline_stage_category_t category(void)
		{return _category;}
    inline int latency(void)
		{return _latency;}
    inline int width(void) const
		{return _width;}
    inline order_t orderPolicy(void)
		{return _order_policy;}
    inline EGQueue * sourceQueue(void)
		{return _source_queue;}
    inline EGQueue * destinationQueue(void)
    	{return _destination_queue;}
    inline elm::String name(void)
    	{return _name;}
    inline int index()
    	{return _index;}
    inline bool isFuStage(void)
    	{return (_category == FU);}
    inline int numFus()
		{return _fus.length();}
    inline EGPipeline *fu(int index)
		{return _fus[index];}
    inline EGPipeline *findFU(Inst::kind_t kind) {
		for(int i = 0; i < _bindings.length(); i++) {
			Inst::kind_t mask = _bindings[i].fst;
			if((kind & mask) == mask)
				return _bindings[i].snd;
		}
		return 0;
	}
    inline EGNode* firstNode(void)
      {return _nodes[0];}
    inline EGNode* lastNode(void)
      {return _nodes[_nodes.length()-1];}
    inline bool hasNodes(void)
      {return (_nodes.length() != 0);}
    inline int numNodes(void)
      {return _nodes.length();}
    inline EGNode * node(int index) {
      if (index >= _nodes.length())
      	return NULL;
      return _nodes[index];
    }
    // set information
    // add/remove bindings
    inline void addBinding(Inst::kind_t kind, EGPipeline *fu)
    	{_bindings.add(pair(kind, fu));}
    // add/remove fus
    inline void addFunctionalUnit(bool pipelined, int latency, int width, elm::String name);
    // add/remove nodes
	inline void addNode(EGNode * node)
		{ _nodes.add(node); }
	inline void removeNode(EGNode *node)
		{ _nodes.remove(node); }
    inline void deleteNodes(void)
      {if (_nodes.length() != 0) _nodes.clear();}

    class NodeIterator: public elm::genstruct::Vector<EGNode *>::Iterator {
    public:
      inline NodeIterator(const EGStage *stage)
	: elm::genstruct::Vector<EGNode *>::Iterator(stage->_nodes) {}
    };

  };


  /*
   * class EGPipeline
   *
   */

  class EGPipeline {
  protected:
    elm::genstruct::Vector<EGStage *> _stages;
  private:
    EGStage * _last_stage;
    EGStage * _first_stage;
  public:
    EGPipeline()
      : _first_stage(NULL){    }
    ~EGPipeline() {    }
    inline EGStage *lastStage()
    {return _last_stage;}
    inline EGStage *firstStage()
    {return _first_stage; }
    inline void addStage(EGStage *stage);
    inline int numStages()
    {return _stages.length();}

    class StageIterator: public elm::genstruct::Vector<EGStage *>::Iterator {
    public:
      inline StageIterator(const EGPipeline *pipeline)
	: elm::genstruct::Vector<EGStage *>::Iterator(pipeline->_stages) {}
    };
  };


  inline void EGPipeline::addStage(EGStage *stage){
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
   * class EGProc
   *
   */

  class EGProc {
  private:
    elm::genstruct::Vector<EGQueue *> _queues;
    EGPipeline _pipeline;
    EGStage * _fetch_stage;
    EGStage * _exec_stage;

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


    EGProc(const otawa::hard::Processor *proc);
    inline void addQueue(elm::String name, int size){
        _queues.add(new EGQueue(name, size));
    }
    inline EGQueue * queue(int index){
      return _queues[index];
    }
    inline void setFetchStage(EGStage * stage)
      {_fetch_stage = stage;}
    inline EGStage * fetchStage(void)
      {return _fetch_stage;}
    inline void setExecStage(EGStage * stage)
      {_exec_stage = stage;}
    inline EGStage * execStage(void)
      {return _exec_stage;}
    inline EGStage * lastStage(void)
    {return _pipeline.lastStage() ;}
    inline bool isLastStage(EGStage *stage)
     {return (_pipeline.lastStage() == stage);}
    inline EGPipeline *pipeline()
     { return &_pipeline;}

    class QueueIterator: public elm::genstruct::Vector<EGQueue *>::Iterator {
    public:
      inline QueueIterator(const EGProc *processor)
	: elm::genstruct::Vector<EGQueue *>::Iterator(processor->_queues) {}
    };

  };




  inline EGStage::EGStage(pipeline_stage_category_t category, int latency, int width, order_t policy, EGQueue *sq, EGQueue *dq, elm::String name, int index)
  : _category(category), _latency(latency), _width(width), _order_policy(policy),
    _source_queue(sq), _destination_queue(dq), _name(name), _index(index) {}

  inline void EGStage::addFunctionalUnit(bool pipelined, int latency, int width, elm::String name) {
	  EGPipeline *fu = new EGPipeline();
	  if ( !pipelined) {
		  EGStage * stage = new EGStage(FU, latency, width, _order_policy, _source_queue, _destination_queue, name);
		  fu->addStage(stage);
	  }
	  else {
		  EGStage * stage;

		  // first_stage
		  stage = new EGStage(FU, 1, width, _order_policy, _source_queue, NULL, name + "1");
		  fu->addStage(stage);

		  // intermediate stages
		  for (int i=2 ; i<latency ; i++) {
			  stage = new EGStage(FU, 1, width, IN_ORDER, NULL, NULL, _ << name << i);
			  fu->addStage(stage);
		  }

		  // last stage
		  stage = new EGStage(FU, 1, width, IN_ORDER, NULL, _destination_queue, _ << name << latency);
		  fu->addStage(stage);
	  }
	  _fus.add(fu);
  }



} // exegraph2
} // otawa

#endif // _EG__H



