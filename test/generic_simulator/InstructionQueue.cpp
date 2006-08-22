#include <InstructionQueue.h>

InstructionQueueConfiguration::InstructionQueueConfiguration(CString name, int capacity) :
	queue_name(name), cap(capacity), number_of_write_ports(0), number_of_read_ports(0) {

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


InstructionQueue::InstructionQueue(sc_module_name name, InstructionQueueConfiguration * configuration) {
	conf = configuration;
	is_full = false;
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

inline void InstructionQueue::put(SimulatedInstruction *inst) {
	assert (is_full == false);
	int new_tail = (tail+1) & (cap-1);
	buffer[tail] = inst;
	tail = new_tail;
	is_full = (tail == head);
}

inline SimulatedInstruction* InstructionQueue::get() {
	assert((head!=tail) || is_full);
	int res=head;
	head = (head+1) & (cap-1);
	is_full = false;
	return buffer[head];
}
		
inline SimulatedInstruction* InstructionQueue::read(int index){
	return buffer[ (head + index) & (cap - 1) ];
}

inline int InstructionQueue::size() {
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
	return (head == tail);
}
	
void InstructionQueue::action() {
	elm::cout << this->name() << "->action():\n";
	elm::cout << "\tin_number_of_accepted_outs=" << in_number_of_accepted_outs.read() << "\n";
	for (int i=0 ; i<out_number_of_outs.read() ; i++)
		get();
	elm::cout << "\tin_number_of_ins=" << in_number_of_ins.read() << " (cap=" << cap << ", size=" << size() << ")\n";
	for (int i=0 ; i<in_number_of_ins.read() ; i++) {
		assert(!is_full);
		put(in_instruction[i].read());
	}
	deliverOutputs();
	int accepted = cap - size() + number_of_outs;
	if (accepted > in_ports)
		accepted = in_ports;
	out_number_of_accepted_ins.write(accepted);	
	elm::cout << "\tout_number_of_accepted_ins=" << accepted << " (cap=" << cap << ", size=" << size() << ")\n";
}
	
void InstructionQueue::deliverOutputs() {
	number_of_outs = in_number_of_accepted_outs.read();
	elm::cout << "\tcomputing number of outs: \n";
	elm::cout << "\t\tin_number_of_accepted_outs.read()=" << in_number_of_accepted_outs.read() << "\n";
	elm::cout << "\t\tsize()=" << size() << "\n";
	if (number_of_outs > size())
		number_of_outs = size();
	elm::cout << "\t\tnumber_of_outs=" << number_of_outs << "\n";
	for (int i=0 ; i<number_of_outs ; i++)
		out_instruction[i].write(read(i));
	out_number_of_outs.write(number_of_outs);
	elm::cout << "\tout_number_of_outs=" << number_of_outs << "\n";
}

void InstructionBuffer::deliverOutputs() {
	number_of_outs = in_number_of_accepted_outs.read();
	if (number_of_outs > size())
		number_of_outs = size();
	int i = 0;
	while ((i<number_of_outs) && (read(i)->state() == TERMINATED)) {
		out_instruction[i].write(read(i));
		i++;
 	}
 	number_of_outs = i;
	out_number_of_outs.write(number_of_outs);
	elm::cout << "\tout_number_of_outs=" << number_of_outs << "\n";
	
}
