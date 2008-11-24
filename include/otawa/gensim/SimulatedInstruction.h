/*
 *	$Id$
 *	gensim module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
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
#ifndef OTAWA_GENSIM_SIMULATED_INSTRUCTION_H
#define OTAWA_GENSIM_SIMULATED_INSTRUCTION_H

#include <otawa/otawa.h>
#include <otawa/hard/Register.h>
#include <elm/genstruct/Table.h>

extern unsigned long Mem_Base_Read_First;
extern unsigned long Mem_Base_Write_First;
extern unsigned long Mem_Base_Read_Last;
extern unsigned long Mem_Base_Write_Last;

namespace otawa {
namespace gensim {

typedef enum {NONE, WAITING, READY, EXECUTING, EXECUTED, NOTIFIED}
		simulated_instruction_state_t; // ordered set

typedef enum {PENDING, ACCESSING_MEMORY, TERMINATED}
		simulated_instruction_in_fu_state_t; // ordered set
typedef enum {
	COND_BRANCH,
	UNCOND_BRANCH,
	TRAP,
	CALL,
	RETURN,
	LOAD,
	STORE,
	IALU,
	FALU,
	MUL,
	DIV,
	OTHER
} instruction_type_t;
#define INST_TYPE_NUMBER 12
// FIXME : should be read from framework
instruction_type_t convertInstType(Inst::kind_t kind);

class SimulatedInstruction;

typedef struct rename_table_t {
	otawa::hard::RegBank * reg_bank;
	elm::genstruct::AllocatedTable<SimulatedInstruction *> *table;
} rename_table_t;

class SimulatedInstruction {
private:
  elm::genstruct::SLList<SimulatedInstruction *> * active_instructions;
  otawa::Inst * instruction;
  simulated_instruction_state_t instruction_state;
  simulated_instruction_in_fu_state_t instruction_fu_state;
  elm::genstruct::SLList<SimulatedInstruction *> source_instructions;
  int time_to_finish_execution;
  Inst::kind_t _type;
  bool _is_control;
  int _index;
  otawa::address_t Mem_Read_First;
  otawa::address_t Mem_Read_Last;
  otawa::address_t Mem_Write_First;
  otawa::address_t Mem_Write_Last;
  inline void addSourceInstruction(SimulatedInstruction * source_inst) {
    source_instructions.addLast(source_inst);
  }
  inline void removeSourceInstruction(SimulatedInstruction * source_inst) {
    source_instructions.remove(source_inst);
  }

 public:
  inline SimulatedInstruction(otawa::Inst* inst, int index,
			      elm::genstruct::SLList<SimulatedInstruction *> * _active_instructions)
    : active_instructions(_active_instructions), instruction(inst), instruction_state(READY), _index(index), 
    Mem_Read_First(Mem_Base_Read_First), Mem_Read_Last(Mem_Base_Read_Last),
    Mem_Write_First(Mem_Base_Write_First), Mem_Write_Last(Mem_Base_Write_Last) {
      active_instructions->addLast(this);
      _type = inst->kind();
      _is_control = inst->isControl();
    }
  inline ~SimulatedInstruction() {
    active_instructions->remove(this);
  }
  inline otawa::Inst * inst() {
    return instruction;
  }
  inline int getIndex(){
    return _index;
  }
  inline void setState(simulated_instruction_state_t new_state) {
    instruction_state = new_state;
  }
  inline simulated_instruction_state_t state() {
    return instruction_state;
  }
  inline void setFuState(simulated_instruction_in_fu_state_t new_state) {
    instruction_fu_state = new_state;
  }
  inline simulated_instruction_in_fu_state_t fuState() {
    return instruction_fu_state;
  }
   inline elm::genstruct::SLList<SimulatedInstruction *> * sourceInstructions() {
    return &source_instructions;
  }
  inline void renameOperands(
			     elm::genstruct::AllocatedTable<rename_table_t> * rename_tables);
  inline void notifyResult(
			   elm::genstruct::AllocatedTable<rename_table_t> * rename_tables);
  inline int timeToFinish() {
    return time_to_finish_execution;
  }
  inline void setTimeToFinish(int time) {
    time_to_finish_execution = time;
  }
  inline int decrementTimeToFinish() {
    time_to_finish_execution--;
    assert(time_to_finish_execution >= 0);
    return time_to_finish_execution;
  }
  inline void dump(elm::io::Output& out_stream);
  inline Inst::kind_t type() {
    return _type;
  }
  inline bool isControl() {
    return _is_control;
  }
  inline String dump();
  inline String typeName();
  inline String stateName();
  inline otawa::address_t getReadMemAddr() {
    //elm::cout << "get readadr "<< Mem_Read_First <<"\n";
    return Mem_Read_First;
  }

  inline otawa::address_t getWriteMemAddr() {
    //elm::cout << "get writeadr "<< Mem_Write_First <<"\n";
    return Mem_Write_First;
  }

  inline int getReadMemSize() {
    //elm::cout << "get read size "<< Mem_Read_Last - Mem_Read_First + 1 <<"\n";
    return (Mem_Read_Last - Mem_Read_First + 1);
  }

  inline int getWriteMemSize() {
    //elm::cout << "get write size "<< Mem_Write_Last - Mem_Write_First + 1 <<"\n";
    return (Mem_Write_Last - Mem_Write_First + 1);
  }

};

 inline void SimulatedInstruction::renameOperands(
						  elm::genstruct::AllocatedTable<rename_table_t> * rename_tables) {
   const elm::genstruct::Table<otawa::hard::Register *> &reads = this->inst()->readRegs();
   for (int i = 0; i < reads.count(); i++) {
     for (int b=0; b<rename_tables->count() ; b++) {
       if ((*rename_tables)[b].reg_bank == reads[i]->bank()) {
	 SimulatedInstruction * producing_inst = (*rename_tables)[b].table->get(reads[i]->number());
	 if (producing_inst != NULL) {
	   this->removeSourceInstruction(producing_inst);
	   this->addSourceInstruction(producing_inst);
	   /* 	  dump(elm::cout); */
	   /* 	  elm::cout << " depends on "; */
	   /* 	  producing_inst->dump(elm::cout); */
	   /* 	  elm::cout << " for " << reads[i]->bank()->name() << "[" << reads[i]->number() << "]\n"; */
	   this->setState(WAITING);
	 }
       }
     }
   }
   const elm::genstruct::Table<otawa::hard::Register *>& writes = this->inst()->writtenRegs();
   for (int i = 0; i < writes.count(); i++) {
     for (int b=0; b<rename_tables->count() ; b++) {
       if ((*rename_tables)[b].reg_bank == writes[i]->bank()) {
	 (*rename_tables)[b].table->set(writes[i]->number(), this);
       }
     }
   }
 }

 inline void SimulatedInstruction::notifyResult(
						elm::genstruct::AllocatedTable<rename_table_t> * rename_tables) {
   for (elm::genstruct::SLList<SimulatedInstruction *>::Iterator
	  inst(*active_instructions); inst; inst++) {
     if (inst->state() == WAITING) {
       inst->sourceInstructions()->remove(this);
       if (inst->sourceInstructions()->isEmpty()) {
	 inst->setState(READY);
       }
     }
   }
   this->setState(EXECUTED);
   const elm::genstruct::Table<otawa::hard::Register *>& writes = this->inst()->writtenRegs();
   for (int i = 0; i < writes.count(); i++) {
     for (int b=0; b<rename_tables->count() ; b++) {
       if ((*rename_tables)[b].reg_bank == writes[i]->bank()) {
	 if ((*rename_tables)[b].table->get(writes[i]->number())
	     == this)
	   (*rename_tables)[b].table->set(writes[i]->number(),
					  NULL);
       }
     }
   }
 }

 inline String SimulatedInstruction::dump() {
   StringBuffer buffer;
   buffer << "[i" << _index << "] @" << inst()->address() << ": ";
   Output output(buffer.stream());
   inst()->dump(output);
   buffer << " (" << stateName() << ") - ";
   if (instruction_state == WAITING) {
     /*  buffer << " - waits for: "; */
     /*      for (elm::genstruct::SLList<SimulatedInstruction *>::Iterator inst(source_instructions) ; inst ; inst++) { */
     /*         buffer << inst->inst()->address() << ", "; */
     /*      } */
   }
   return (buffer.toString());
 }
 inline void SimulatedInstruction::dump(elm::io::Output& out_stream) {
   out_stream << inst()->address() << ": ";
   inst()->dump(out_stream);
   out_stream << " - " << stateName();
   if (instruction_state == WAITING) {
     out_stream << " - waits for ";
     for (elm::genstruct::SLList<SimulatedInstruction *>::Iterator
	    inst(source_instructions); inst; inst++) {
       out_stream << inst->inst()->address() << ", ";
     }
   }
   out_stream << "\n";
 }

 inline String SimulatedInstruction::stateName() {
   StringBuffer buffer;
   switch (instruction_state) {
   case NONE:
     buffer << "NONE";
     break;
   case WAITING:
     buffer << "WAITING";
     break;
   case READY:
     buffer << "READY";
     break;
   case EXECUTING:
     buffer << "EXECUTING";
     break;
   case EXECUTED:
     buffer << "EXECUTED";
     break;
   case NOTIFIED:
     buffer << "NOTIFIED";
     break;
   default:
     buffer << "UNKNOWN";
     break;
   }
   return (buffer.toString());
 }

 inline String SimulatedInstruction::typeName() {
   StringBuffer buffer;
   switch (_type) {
   case COND_BRANCH:
     buffer << "COND_BRANCH";
     break;
   case UNCOND_BRANCH:
     buffer << "UNCOND_BRANCH";
     break;
   case TRAP:
     buffer << "TRAP";
     break;
   case CALL:
     buffer << "CALL";
     break;
   case RETURN:
     buffer << "RETURN";
     break;
   case LOAD:
     buffer << "LOAD";
     break;
   case STORE:
     buffer << "STORE";
     break;
   case IALU:
     buffer << "IALU";
     break;
   case FALU:
     buffer << "FALU";
     break;
   case MUL:
     buffer << "MUL";
     break;
   case DIV:
     buffer << "DIV";
     break;
   case OTHER:
     buffer << "OTHER";
     break;
   default:
     buffer << "UNKNOWN";
     break;
   }
   return (buffer.toString());
 }

}
} // otawa::gensim

#endif // OTAWA_GENSIM_SIMULATED_INSTRUCTION_H
