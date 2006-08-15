#ifndef _ARMPROCESSOR_H_
#define _ARMPROCESSOR_H_

#include <systemc.h>
#include <otawa/otawa.h>
#include <stdlib.h>
#include <elm/io.h>
#include <elm/genstruct/VectorQueue.h>
#include <emul.h>
#include <iss_include.h>


#define MAX_STAGES 20
#define MAX_WIDTH 16

#define MIN(x,y) (x<y?x:y)

namespace otawa {
	class ARMState;
}

class SimulatedInstruction {
	private:
		otawa::Inst * instruction;
		code_t binary_code;
		instruction_t * emulated_inst;
	public:
		inline SimulatedInstruction(otawa::Inst* inst, code_t code, instruction_t* emul_inst);
		inline otawa::Inst * inst();
		inline instruction_t * emulatedInst();
};

inline SimulatedInstruction::SimulatedInstruction(otawa::Inst* inst, code_t code, instruction_t* emul_inst) :
	instruction(inst), binary_code(code), emulated_inst(emul_inst) {
	}
inline otawa::Inst * SimulatedInstruction::inst() {
	return instruction;
}

inline instruction_t * SimulatedInstruction::emulatedInst() {
	return emulated_inst;
}


class InstructionQueue {
	int head;
	int tail;
	int cap;
	SimulatedInstruction** buffer;
	public :
		inline InstructionQueue(int cap);
		inline ~InstructionQueue();
		inline int capacity();
		inline int size();
		inline bool isEmpty();
		inline void flush();
		inline void put(SimulatedInstruction *inst);
		inline SimulatedInstruction* get();
		inline SimulatedInstruction* read(int index);
};

inline InstructionQueue::InstructionQueue(int capacity): 
head(0), tail(0), cap(1<<capacity), buffer(new SimulatedInstruction*[cap]) {
	assert(cap>0);
}

inline InstructionQueue::~InstructionQueue() {
	delete [] buffer;
}

inline int InstructionQueue::capacity() {
	return cap;
}

inline int InstructionQueue::size() {
	if (head == tail)
		return 0;
	if (head < tail)
		return (tail - head);
	return ( (cap - head) + tail ) ;
}

inline bool InstructionQueue::isEmpty() {
	return (head == tail);
}

inline void InstructionQueue::flush() {
	head = 0;
	tail = 0;
}

inline void InstructionQueue::put(SimulatedInstruction *inst) {
	elm::cout << "fetch_queue->put() begins : h=" << head << ", t=" << tail << ", c=" << cap << "\n";
	int new_tail = (tail+1) & (cap-1);
	elm::cout << "fetch_queue->put() begins : h=" << head << ", newt=" << new_tail << "\n";
	assert (new_tail != head);
	buffer[tail] = inst;
	tail = new_tail;
	elm::cout << "fetch_queue->put() ends : h=" << head << ", t=" << tail << "\n";
}

inline SimulatedInstruction* InstructionQueue::get() {
	elm::cout << "fetch_queue->get() begins : h=" << head << ", t=" << tail << "\n";
	assert(head!=tail);
	int res=head;
	head = (head+1) & (cap-1);
	return buffer[head];
	elm::cout << "fetch_queue->get() ends : h=" << head << ", t=" << tail << "\n";
}
		
inline SimulatedInstruction* InstructionQueue::read(int index){
	elm::cout << "fetch_queue->read() begins : h=" << head << ", t=" << tail << "\n";
	return buffer[ (head + index) & (cap - 1) ];
	elm::cout << "fetch_queue->read() ends : h=" << head << ", t=" << tail << "\n";
}


SC_MODULE(FetchStage) {
	sc_in<bool> clock;
	sc_out<void *> fetched_instruction[MAX_WIDTH];
	sc_in<int>number_of_accepted_instructions;
	sc_out<int>number_of_leaving_instructions;
		
	otawa::ARMState *sim_state;
	state_t * emulated_state;
	otawa::address_t pc;
	int width;
	int number_of_fetched_instructions;
	InstructionQueue* fetch_queue;
	sc_signal<bool>control_request;
	otawa::Inst* next_inst;
	
	void fetch();
	
	inline void init(otawa::ARMState *state) {
		sim_state = state;
		fetch_queue->flush();
	}
	
	inline void configure(int width_value, int fq_size) {
		width = width_value;
		fetch_queue = new InstructionQueue(fq_size);  // real size equals 2^fq_size
	}

	inline void control() {
		elm::cout << "fetch_control() begins\n";
		number_of_leaving_instructions = MIN(number_of_fetched_instructions, 
											number_of_accepted_instructions);
		elm::cout << "fetch_control() ends\n";
	}
	
	inline otawa::address_t PC() {
		return(pc);
	}
	
	inline void setPC(otawa::address_t new_pc) {
		pc = new_pc;
	}
	
	inline bool isEmpty() {
		return fetch_queue->isEmpty();
	}
	
	SC_CTOR(FetchStage) {
		SC_METHOD(fetch);
		sensitive_pos << clock;
		SC_METHOD(control);
		sensitive << control_request;
		sensitive << number_of_accepted_instructions;
		
	}
};

SC_MODULE(DecodeStage) {
	sc_in<bool> clock;
	sc_in<void *> fetched_instruction[MAX_WIDTH];
	sc_out<void *> decoded_instruction[MAX_WIDTH];
	sc_out<int>number_of_accepted_fetched_instructions;
	sc_in<int>number_of_fetched_instructions;
	sc_in<int>number_of_accepted_decoded_instructions;
	sc_out<int>number_of_leaving_instructions;
		
	sc_signal<bool>control_request;
	
	int number_of_decoded_instructions;
	int width;
	
	void decode() {	
		elm::cout << "decode() begins\n";
		
		SimulatedInstruction* inst;
		otawa::address_t pc;
		for (int i=0 ; i<number_of_fetched_instructions ; i++) {
			inst = (SimulatedInstruction *) fetched_instruction[i].read();
			pc = inst->inst()->address();
			elm::cout << "Decoding instruction at pc=" << pc << "\n";
			decoded_instruction[i] = fetched_instruction[i];
		}
		
		number_of_decoded_instructions = number_of_fetched_instructions;
		// recompute control signals
		control_request.write(!control_request.read());
		elm::cout << "decode() ends\n";
		
	}
	
	void control() {
		elm::cout << "decode_control() begins\n";
		
		number_of_leaving_instructions = MIN(number_of_decoded_instructions, 
											number_of_accepted_decoded_instructions);
		number_of_accepted_fetched_instructions = width - (number_of_decoded_instructions - number_of_leaving_instructions);
		elm::cout << "decode_control() ends\n";
	}
	
	inline void configure(int width_value) {
		width = width_value;
	}
	
	SC_CTOR(DecodeStage) {
		SC_METHOD(decode);
		sensitive_pos << clock;
		SC_METHOD(control);
		sensitive << control_request;
		sensitive << number_of_fetched_instructions << number_of_accepted_decoded_instructions;
		
	}
};

SC_MODULE(ExecuteStage) {
	sc_in<bool> clock;
	sc_in<void *> decoded_instruction[MAX_WIDTH];
	sc_out<int>number_of_accepted_decoded_instructions;
	sc_in<int>number_of_decoded_instructions;
		
	sc_signal<bool>control_request;
	int width;
	otawa::ARMState *sim_state;
	
	class PendingInstruction;
	class PendingInstruction {
		SimulatedInstruction * inst;
		int cycles_to_complete;
		PendingInstruction * next;
		public: 
			PendingInstruction(SimulatedInstruction* instruction, int cycles):
				inst(instruction), cycles_to_complete(cycles) {
			}
			PendingInstruction * getNext() {
				return next;
			}	
			void setNext(PendingInstruction * next_instruction) {
				next = next_instruction;
			}
	
			bool terminated() {
				cycles_to_complete --;
				return (cycles_to_complete == 0 );
			}
			SimulatedInstruction * instruction() {
				return inst;
			}
	};
	
	
	PendingInstruction * pending_instruction_list;
	
	void execute(); 
	
	void control() {
				elm::cout << "execute_control() begins\n";
		
		number_of_accepted_decoded_instructions = width;
				elm::cout << "execute_control() ends\n";
		
	}
	
	inline void configure(int width_value) {
		width = width_value;
	}
	
	inline void init(otawa::ARMState *state) {
		sim_state = state;
		pending_instruction_list = NULL;
	}
	
	SC_CTOR(ExecuteStage) {
		SC_METHOD(execute);
		sensitive_pos << clock;
		SC_METHOD(control);
		sensitive << control_request;
		sensitive << number_of_decoded_instructions; 
		
	}
};

SC_MODULE(ARMProcessor)
{
  sc_signal<void *> fetched_instruction[MAX_WIDTH];
  sc_signal<void *> decoded_instruction[MAX_WIDTH];
  
  sc_signal<int> number_of_fetched_instructions;
  sc_signal<int> number_of_accepted_fetched_instructions;
  sc_signal<int> number_of_decoded_instructions;
  sc_signal<int> number_of_accepted_decoded_instructions;
  sc_signal<bool> clock;
 
  FetchStage* fetch_stage;
  DecodeStage* decode_stage;
  ExecuteStage* execute_stage;
  
  otawa::ARMState *sim_state;

  void build(otawa::ARMState *state);
  otawa::address_t currentPC() {
  	return(fetch_stage->PC());
  }
  void step();
  void setPC(otawa::address_t pc) {
  	fetch_stage->setPC(pc);
  }
  void init() {
  	fetch_stage->init(sim_state);
  }
  
  bool isEmpty() {
  	return fetch_stage->isEmpty();
	}
	
  ARMProcessor(sc_module_name name);
  ~ARMProcessor()
    {
    }
};


#endif //_ARMPROCESSOR_H_
