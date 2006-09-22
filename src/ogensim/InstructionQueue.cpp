#include <otawa/gensim/InstructionQueue.h>
#include <otawa/gensim/debug.h>

InstructionQueueConfiguration::InstructionQueueConfiguration(CString name, int capacity, simulated_instruction_state_t condition) :
	queue_name(name), cap(capacity), number_of_write_ports(0), number_of_read_ports(0), 
	leaving_condition(condition) {

}
int InstructionQueueConfiguration::capacity() {
	return cap;
}

int InstructionQueueConfiguration::numberOfWritePorts() {
	return number_of_write_ports;
}

int InstructionQueueConfiguration::numberOfReadPorts() {
	return number_of_read_ports;
}

CString InstructionQueueConfiguration::name() {
	return queue_name;
}

void InstructionQueueConfiguration::setNumberOfWritePorts(int n) {
	assert(number_of_write_ports == 0); // should not have been modified by another pipeline stage
	number_of_write_ports = n;
}

void InstructionQueueConfiguration::setNumberOfReadPorts(int n) {
	assert(number_of_read_ports == 0); // should not have been modified by another pipeline stage
	number_of_read_ports = n;
}

simulated_instruction_state_t InstructionQueueConfiguration::leavingCondition() {
	return leaving_condition;
}

void InstructionQueueConfiguration::dump(elm::io::Output& out_stream) {
	out_stream << queue_name << ": ";
	out_stream << "capacity=" << cap << " - iports=" << number_of_write_ports;
	out_stream << " - oports=" << number_of_read_ports << " instructions leave when ";
	switch(leaving_condition) {
		case WAITING:
			out_stream << "WAITING\n";
			break;
		case READY:
			out_stream << "READY\n";
			break;
		case EXECUTED:
			out_stream << "EXECUTED\n";
			break;
		case NONE:
			out_stream << "NONE\n";
			break;
		default:
			out_stream << "ERROR\n";
			break;			
	}
}

InstructionQueue::InstructionQueue(sc_module_name name, InstructionQueueConfiguration * configuration) {
	conf = configuration;
	is_full = false;
	head = 0;
	tail = 0;
	cap = 1<<conf->capacity();
	assert(cap>0);
	in_ports = conf->numberOfWritePorts();
	assert(in_ports);
	in_instruction = new sc_in<SimulatedInstruction *>[in_ports];
	out_ports = conf->numberOfReadPorts();
	assert(out_ports);
	out_instruction = new sc_out<SimulatedInstruction *>[out_ports];
	buffer = new SimulatedInstruction*[cap]; 
	SC_METHOD(action);
	sensitive_neg << in_clock;
}

InstructionQueue::~InstructionQueue() {
	delete [] buffer;
}

void InstructionQueue::flush() {
	head = 0;
	tail = 0;
}

void InstructionQueue::put(SimulatedInstruction *inst) {
	assert (is_full == false);
	int new_tail = (tail+1) & (cap-1);
	buffer[tail] = inst;
	tail = new_tail;
	is_full = (tail == head);
}

SimulatedInstruction* InstructionQueue::get() {
	assert((head!=tail) || is_full);
	int res=head;
	head = (head+1) & (cap-1);
	is_full = false;
	return buffer[res];
}
		
SimulatedInstruction* InstructionQueue::read(int index){
	return buffer[ (head + index) & (cap - 1) ];
}

int InstructionQueue::size() {
	if (head == tail)
		if (is_full == false)
			return 0;
		else
			return cap;
	if (head < tail)
		return (tail - head);
	return ( (cap - head) + tail ) ;
}

InstructionQueueConfiguration * InstructionQueue::configuration() {
	return conf;
}

bool InstructionQueue::isEmpty() {
	return ((head == tail) && !is_full);
}
	
void InstructionQueue::action() {
	TRACE(elm::cout << this->name() << "->action():\n";)
	// remove instructions that were accepted by the consumer pipeline stage at the last rising edge
	TRACE(elm::cout << "\tin_number_of_accepted_outs=" << in_number_of_accepted_outs.read() << "\n";)
	int outs = 0;
	while ( !isEmpty() && (outs < in_number_of_accepted_outs.read()) ) {
		SimulatedInstruction * inst = get();
		TRACE(elm::cout << "\textracting " << inst->inst()->address() << "\n";)
		outs++;
	}
	// insert instructions that were submitted by the producer pipeline stage at the last rising edge
	TRACE(elm::cout << "\tin_number_of_ins=" << in_number_of_ins.read() << "\n";)
	int ins = 0;
	while (!is_full &&  (ins<in_number_of_ins.read())) {
		put(in_instruction[ins++].read()); 
	}
	out_number_of_accepted_ins.write(ins);
	TRACE(elm::cout << "\tout_number_of_accepted_ins=" << ins << "\n";)
	// submit instructions to the next stage (for the next rising edge)
	TRACE(elm::cout << "\tcontains: \n";
		for (int i=0 ; i<size() ; i++) {
			elm::cout << "\t\t";
			read(i)->dump(elm::cout);
		}
		elm::cout << "\n";)
	outs = 0;
	while ( (outs < size()) && (outs < out_ports) && (read(outs)->state() >= conf->leavingCondition())) {
		out_instruction[outs] = read(outs);	
		outs++;
	}
//	if ((outs < size()) && (outs < out_ports) )
//		elm::cout << "\tcannot send inst " << read(outs)->inst()->address() << " because its state is " << read(outs)->state() << "\n";
	out_number_of_outs.write(outs);
	TRACE(elm::cout << "\tout_number_of_outs=" << outs << "\n";)
	
}

	
