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
#include <elm/data/Vector.h>
#include <elm/string/String.h>
#include <elm/data/BiDiList.h>
#include <elm/data/List.h>
#include <stdio.h>
#include <otawa/otawa.h>
#include <otawa/hard/Processor.h>

namespace otawa {

  using namespace elm;

  class ParExeStage;
  class ParExePipeline;
  class ParExeProc;
  class ParExeNode;

  namespace hard {
    class Microprocessor;
  } // hard



  /*
   * class ParExeQueue
   *
   */

	class ParExeQueue {
	public:
		inline ParExeQueue(elm::String name, int size)
			: _name(name), _size(size), _filling_stage(nullptr), _emptying_stage(nullptr)  {}
		inline elm::String name(void) {return _name;}
		inline int size(void) {return _size;}
		inline ParExeStage *fillingStage(void) {return _filling_stage;}
		inline ParExeStage *emptyingStage(void) {return _emptying_stage;}
		inline void setFillingStage(ParExeStage * stage) {_filling_stage = stage;}
		inline void setEmptyingStage(ParExeStage * stage) {_emptying_stage = stage;}
	private:
		elm::String _name;
		int _size;
		ParExeStage * _filling_stage;
		ParExeStage * _emptying_stage;
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
    Vector<ParExePipeline *> _fus;
    Vector<ParExeNode *> _nodes;
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
	inline ParExePipeline *addFunctionalUnit(bool pipelined, int latency, int width, elm::String name, const hard::PipelineUnit *unit);
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

    class NodeIterator: public Vector<ParExeNode *>::Iter {
    public:
      inline NodeIterator(const ParExeStage *stage)
	: Vector<ParExeNode *>::Iter(stage->_nodes) {}
    };
    
  };


  /*
   * class ParExePipeline
   *
   */

  class ParExePipeline { 
  	public:
    ParExePipeline()  : _first_stage(nullptr), _last_stage(nullptr) {}
    ~ParExePipeline() {    }
    inline ParExeStage *lastStage() {return _last_stage;}
    inline ParExeStage *firstStage() {return _first_stage; }
    inline void addStage(ParExeStage *stage);
    inline int numStages() {return _stages.length();}

    class StageIterator: public Vector<ParExeStage *>::Iter {
    public:
      inline StageIterator(const ParExePipeline *pipeline)
	: Vector<ParExeStage *>::Iter(pipeline->_stages) {}
    };

    inline StageIterator stages(void) const { return StageIterator(this); }

    protected:
      Vector<ParExeStage *> _stages;
    private:
      ParExeStage * _first_stage;
      ParExeStage * _last_stage;
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
	  inline const hard::Processor *processor(void) const { return _proc; }
	  inline void addQueue(elm::String name, int size){_queues.add(new ParExeQueue(name, size));}
	  inline ParExeQueue * queue(int index) {return _queues[index];}
	  inline int queueCount() const { return _queues.count(); }
	  inline void setFetchStage(ParExeStage * stage) {_fetch_stage = stage;}
	  inline ParExeStage *fetchStage(void) {return _fetch_stage;}
	  inline void setExecStage(ParExeStage * stage) {_exec_stage = stage;}
	  inline ParExeStage *execStage(void) {return _exec_stage;}
	  inline void setBranchStage(ParExeStage *stage) { _branch_stage = stage; }
	  inline ParExeStage *branchStage(void) const { return _branch_stage; }
	  inline void setMemStage(ParExeStage *stage) { _mem_stage = stage; }
	  inline ParExeStage *memStage(void) const { return _mem_stage; }
	  inline ParExeStage *lastStage(void) {return _pipeline.lastStage() ;}
	  inline bool isLastStage(ParExeStage *stage) {return (_pipeline.lastStage() == stage);}
	  inline ParExePipeline *pipeline() {return &_pipeline;}
	  inline List<ParExeStage *> *listOfInorderStages() {return &_inorder_stages;}

	  class QueueIterator: public Vector<ParExeQueue *>::Iter {
	  public:
		  inline QueueIterator(const ParExeProc *processor)
		  : Vector<ParExeQueue *>::Iter(processor->_queues) {}
	  };

	  private:
	  	  const hard::Processor *_proc;
		  Vector<ParExeQueue *> _queues;
		  ParExePipeline _pipeline;
		  List<ParExeStage *> _inorder_stages;
		  ParExeStage * _fetch_stage;
		  ParExeStage * _exec_stage;
		  ParExeStage *_branch_stage;
		  ParExeStage *_mem_stage;
  };

} // otawa

#endif // OTAWA_MICROPROCESSOR_H
