/*
 *	$Id$
 *	Interface to the xxxResource classes.
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


#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include <otawa/parexegraph/ParExeProc.h>
//#include <otawa/parexegraph/ParExeGraph.h>
#include <otawa/hard/Register.h>

namespace otawa { 

  class ParExeInst;
	
  class Resource {
  public:
    typedef enum {STAGE, QUEUE, REG, EXTERNAL_CONFLICT, INTERNAL_CONFLICT, BLOCK_START, RES_TYPE_NUM} resource_type_t;
    typedef struct input_t {
    hard::RegBank * reg_bank;
    elm::genstruct::AllocatedTable<bool> *_is_input;
    elm::genstruct::AllocatedTable<int> * _resource_index;
  } input_t;
  private:
    elm::String _name;
    resource_type_t _type;
    int _index;
  public:
    inline Resource(elm::String name, resource_type_t type, int index)
      : _name(name), _type(type), _index(index) {}
    inline elm::String name()
      {return _name;}
    inline resource_type_t type()
      {return _type;}
    inline int index()
    	{return _index;}
   };
  
  class StartResource : public Resource {
  public:
    inline StartResource(elm::String name, int index)
      :Resource(name,BLOCK_START, index) {}
  };

  class StageResource : public Resource {
  private:
    ParExeStage *_stage;
    int _slot;
  public:
    inline StageResource(elm::String name, ParExeStage *stage, int slot, int index)
	: Resource(name,STAGE, index), _stage(stage), _slot(slot)  {}
    inline ParExeStage * stage()
      {return _stage;}
    inline int slot()
      {return _slot;}
  };

  class QueueResource : public Resource {
  private: 
    ParExeQueue *_queue;
    int _slot;
    StageResource * _upper_bound;
  public:
    inline QueueResource(elm::String name, ParExeQueue * queue, int slot, int index, StageResource * upper_bound)
	: Resource(name,QUEUE, index), _queue(queue), _slot(slot), _upper_bound(upper_bound) {}
    inline ParExeQueue * queue()
      {return _queue;}
    inline int slot()
      {return _slot;}
    StageResource * upperBound() 
      {return _upper_bound;}
  };

  class RegResource : public Resource {
  private:
    elm::genstruct::Vector<ParExeInst *> _using_instructions;
    otawa::hard::RegBank * _reg_bank;
    int _reg_index;
  public:
    inline RegResource(elm::String name, otawa::hard::RegBank * reg_bank, int reg_index, int index)
	: Resource(name,REG, index), _reg_bank(reg_bank), _reg_index(reg_index) {}
    inline ~RegResource() {
      _using_instructions.clear();
    }
    inline otawa::hard::RegBank * regBank()
      {return _reg_bank;}
    inline int regIndex()
      {return _reg_index;}
    inline void addUsingInst(ParExeInst * inst) {
      if (! _using_instructions.contains(inst) )
	_using_instructions.add(inst);
    }
    class UsingInstIterator : public elm::genstruct::Vector<ParExeInst *>::Iterator {
    public:
      UsingInstIterator(const RegResource * res)
	: elm::genstruct::Vector<ParExeInst *>::Iterator(res->_using_instructions) {}
    };
  };

  class ExternalConflictResource : public Resource {
  private:
    ParExeInst * _instruction;
  public:
    ExternalConflictResource(elm::String name, ParExeInst * instruction, int index)
	: Resource(name,EXTERNAL_CONFLICT, index), _instruction(instruction) {}
    ParExeInst * instruction()
      {return _instruction;}
  };

  class InternalConflictResource : public Resource {
  private:
    ParExeInst * _instruction;
    ParExeNode * _node;
  public:
    InternalConflictResource(elm::String name, ParExeInst * instruction, int index)
	: Resource(name,INTERNAL_CONFLICT, index), _instruction(instruction) {}
    ParExeInst* instruction()
      {return _instruction;}
    void setNode(ParExeNode *node)
      {_node = node;}
    ParExeNode *node()
      {return _node;}
    
  };


} // namespace otawa

#endif //_PARAM_EXEGRAPH_H_
