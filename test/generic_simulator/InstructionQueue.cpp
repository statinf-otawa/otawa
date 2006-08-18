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
	int new_tail = (tail+1) & (cap-1);
	assert (new_tail != head);
	buffer[tail] = inst;
	tail = new_tail;
}

inline SimulatedInstruction* InstructionQueue::get() {
	assert(head!=tail);
	int res=head;
	head = (head+1) & (cap-1);
	return buffer[head];
}
		
inline SimulatedInstruction* InstructionQueue::read(int index){
	return buffer[ (head + index) & (cap - 1) ];
}

inline int InstructionQueue::size() {
	if (head == tail)
		return 0;
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
	elm::cout << "\tin_number_of_ins=" << in_number_of_ins.read() << "\n";
	for (int i=0 ; i<in_number_of_ins.read() ; i++) {
		assert(size() < cap);
		put(in_instruction[i].read());
	}
	deliverOutputs();
	int accepted = cap - size() + out_number_of_outs.read();
	if (accepted > in_ports)
		accepted = in_ports;
	out_number_of_accepted_ins.write(accepted);	
	elm::cout << "\tout_number_of_accepted_ins=" << accepted << "\n";
}
	
void InstructionQueue::deliverOutputs() {
	int outs = size();
	if (outs > out_ports)
		outs = out_ports;
	for (int i=0 ; i<outs ; i++)
		out_instruction[i].write(read(i));
	out_number_of_outs.write(outs);
	
}

void InstructionBuffer::deliverOutputs() {
	int outs = size();
	if (outs > out_ports)
		outs = out_ports;
	int i = 0;
	while ((i<outs) && (read(i)->state() == TERMINATED)) {
		out_instruction[i].write(read(i));
		i++;
 	}
	out_number_of_outs.write(i);
	elm::cout << "\tout_number_of_outs=" << i << "\n";
	
}
