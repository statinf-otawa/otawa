#include <otawa/gensim/PipelineStage.h>

PipelineStageConfiguration::PipelineStageConfiguration(CString name, pipeline_stage_t type,
	InstructionQueueConfiguration * inqueue, InstructionQueueConfiguration * outqueue,
	int in_width, int out_width) :
	stage_name(name), stage_type(type), input_queue(inqueue), output_queue(outqueue),
	in_stage_width(in_width), out_stage_width(out_width) {
		assert(inqueue);
		inqueue->setNumberOfReadPorts(in_width);
		assert(outqueue);
		outqueue->setNumberOfWritePorts(out_width);
}

PipelineStageConfiguration::PipelineStageConfiguration(CString name, pipeline_stage_t type,
	InstructionQueueConfiguration * inqueue, InstructionQueueConfiguration * outqueue,
	int width) :
	stage_name(name), stage_type(type), input_queue(inqueue), output_queue(outqueue), stage_width(width) {
		if (inqueue)
			inqueue->setNumberOfReadPorts(width);
		if (outqueue)
			outqueue->setNumberOfWritePorts(width);
}


pipeline_stage_t PipelineStageConfiguration::type() {
	return stage_type;
}

CString PipelineStageConfiguration::name() {
	return stage_name;
}

InstructionQueueConfiguration * PipelineStageConfiguration::inputQueue() {
	return input_queue;
}

InstructionQueueConfiguration * PipelineStageConfiguration::outputQueue() {
	return output_queue;
}

int PipelineStageConfiguration::width() {
	return stage_width;
}

LazyStageIQIQ::LazyStageIQIQ(sc_module_name name, int width) {
	stage_width = width;
	in_instruction = new sc_in<SimulatedInstruction *>[width];
	out_instruction = new sc_out<SimulatedInstruction *>[width];
	
	SC_METHOD(action);
	sensitive_pos << in_clock;
}

void LazyStageIQIQ::action() {
	elm::cout << "LazyStage->action(): \n";
	elm::cout << "\tin_number_of_ins=" << in_number_of_ins.read() << "\n";
	for (int i=0 ; i<in_number_of_ins.read() ; i++) {
		out_instruction[i].write(in_instruction[i].read());
	}
	out_number_of_outs.write(in_number_of_ins.read());
	
	int accepted = in_number_of_accepted_outs.read();
	if (accepted > stage_width)
		accepted = stage_width;
	elm::cout << "\tout_number_of_accepted_ins=" << accepted << "\n";
	out_number_of_accepted_ins.write(accepted);
}

