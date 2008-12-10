/*
 *	$Id$
 *	exegraph module implementation
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

#include <otawa/gensim/GenericProcessor.h>
#include <otawa/gensim/GenericState.h>
#include <otawa/gensim/debug.h>
#include <otawa/gensim/SimulatorFactory.h>

namespace otawa {
  namespace gensim {

    Identifier<String> TRACE_FILE_PATH("trace file path", "");
    Identifier<int> TRACE_LEVEL("trace level", 6);

    void ProcessorStats::dump(elm::io::Output& out_stream) {
      out_stream << "\n---- Simulation statistics ----\n";
      out_stream << "-------------------------------\n";
      out_stream << "Execution time: " << _time << " cycles\n";
    }

    void ProcessorConfiguration::dump(elm::io::Output& out_stream) {
      out_stream << "---- Processor configuration ----\n";
      out_stream << " Instruction queues:\n";
      for (elm::genstruct::SLList<InstructionQueueConfiguration *>::Iterator
	     iqc(instruction_queues); iqc; iqc++) {
	iqc->dump(out_stream);
      }
      out_stream << " Pipeline stages:\n";
      for (elm::genstruct::SLList<PipelineStageConfiguration *>::Iterator
	     psc(pipeline_stages); psc; psc++) {
	psc->dump(out_stream);
      }
      out_stream << "---- end of configuration ----\n";
    }

    void GenericProcessor::build(
    	ProcessorConfiguration * conf,
		GenericState * sim_state,
		otawa::hard::Platform *pf,
		int trace_level,
		SimulatorFactory *factory)
    {
      int iports=0, oports=0;
      InstructionQueue * input_queue = NULL;
      InstructionQueue * output_queue = NULL;
      bool found = false;
      sc_signal<int> * nb;

      sim_state->addStats(&stats);

      // Init rename tables
      rename_tables = new elm::genstruct::AllocatedTable<rename_table_t>(pf->banks().count());
      int reg_bank_count = pf->banks().count();
      for (int i = 0; i <reg_bank_count; i++) {
	(*rename_tables)[i].reg_bank
	  = (otawa::hard::RegBank *) pf->banks()[i];
	(*rename_tables)[i].table
	  = new elm::genstruct::AllocatedTable<SimulatedInstruction *>((*rename_tables)[i].reg_bank->count());
	for (int j=0; j<(*rename_tables)[i].reg_bank->count() ; j++)
	  (*rename_tables)[i].table->set(j, NULL);
      }

      for (elm::genstruct::SLList<InstructionQueueConfiguration *>::Iterator
	     queue_conf(*(conf->instructionQueuesList())); queue_conf; queue_conf++) {
	Trace *trace = new Trace(*_trace_stream, trace_level, (elm::String) queue_conf->name());

	InstructionQueue * new_queue = new InstructionQueue((sc_module_name) (queue_conf->name()), *queue_conf, trace);
	instruction_queues.addLast(new_queue);
	new_queue->in_clock(clock);
      }

      Trace *mem_trace = new Trace(*_trace_stream, trace_level, (elm::String) "MemorySystem");
      MemorySystem * memory = factory->makeMemory("memory", sim_state, mem_trace, &pf->memory());
    	  // !!TODO!! new MemorySystem("memory", sim_state, mem_trace, &pf->memory());
      memory->in_clock(clock);
      for (elm::genstruct::SLList<PipelineStageConfiguration *>::Iterator
	     stage_conf(*(conf->pipelineStagesList())); stage_conf; stage_conf++) {
      }
      for (elm::genstruct::SLList<PipelineStageConfiguration *>::Iterator
	     stage_conf(*(conf->pipelineStagesList())); stage_conf; stage_conf++) {
	switch (stage_conf->type()) {
	case FETCH: {
	  assert(stage_conf->outputQueue());
	  found = false;
	  for (elm::genstruct::SLList<InstructionQueue *>::Iterator iq(
								       instruction_queues); iq; iq++) {
	    if (iq->configuration() == stage_conf->outputQueue()) {
	      oports = iq->configuration()->numberOfWritePorts();
	      output_queue = iq;
	      found = true;
	    }
	  }
	  assert(found);
	  Trace *trace = new Trace(*_trace_stream, trace_level, (elm::String) "FetchStage");

	  FetchStage
	    * fetch_stage =
	    new FetchStage((sc_module_name) (stage_conf->name()),
			   oports, sim_state, rename_tables, &active_instructions, &_ended, trace);
	  pipeline_stages.addLast(fetch_stage);
	  fetch_stage->in_clock(clock);
	  for (int i=0; i<oports; i++) {
	    sc_signal<SimulatedInstruction *> * fetched_instruction =
	      new sc_signal<SimulatedInstruction *>;
	    fetch_stage->out_fetched_instruction[i](*fetched_instruction);
	    output_queue->in_instruction[i](*fetched_instruction);
	  }
	  nb = new sc_signal<int>;
	  nb->write(0);
	  fetch_stage->out_number_of_fetched_instructions(*nb);
	  output_queue->in_number_of_ins(*nb);
	  nb = new sc_signal<int>;
	  nb->write(0);
	  fetch_stage->in_number_of_accepted_instructions(*nb);
	  output_queue->out_number_of_accepted_ins(*nb);

	  sc_signal<address_t> * inst_addr = new sc_signal<address_t>;
	  memory->in_inst_address(*inst_addr);
	  fetch_stage->out_address(*inst_addr);
	  sc_signal<bool> * inst_req = new sc_signal<bool>;
	  memory->in_inst_request(*inst_req);
	  fetch_stage->out_request(*inst_req);
	  sc_signal<bool> * inst_wait = new sc_signal<bool>;
	  memory->out_inst_wait(*inst_wait);
	  fetch_stage->in_wait(*inst_wait);

	}
	  break;
	case LAZYIQIQ: {
	  assert(stage_conf->inputQueue());
	  found = false;
	  for (elm::genstruct::SLList<InstructionQueue *>::Iterator iq(
								       instruction_queues); iq; iq++) {
	    if (iq->configuration() == stage_conf->inputQueue()) {
	      iports = iq->configuration()->numberOfReadPorts();
	      input_queue = iq;
	      found = true;
	    }
	  }
	  assert(found);
	  assert(stage_conf->outputQueue());
	  found = false;
	  for (elm::genstruct::SLList<InstructionQueue *>::Iterator iq(
								       instruction_queues); iq; iq++) {
	    if (iq->configuration() == stage_conf->outputQueue()) {
	      oports = iq->configuration()->numberOfWritePorts();
	      output_queue = iq;
	      found = true;
	    }
	  }
	  assert(found);
	  StringBuffer buffer;
	  buffer << stage_conf->name() << " stage";
	  Trace *trace = new Trace(*_trace_stream, trace_level, buffer.toString());
	  LazyStageIQIQ * lazy_stage = new LazyStageIQIQ((sc_module_name) (stage_conf->name()), stage_conf->width(), sim_state, trace);
	  pipeline_stages.addLast(lazy_stage);

	  lazy_stage->in_clock(clock);

	  for (int i=0; i<iports; i++) {
	    sc_signal<SimulatedInstruction *> * instruction =
	      new sc_signal<SimulatedInstruction *>;
	    lazy_stage->in_instruction[i](*instruction);
	    input_queue->out_instruction[i](*instruction);
	  }

	  for (int i=0; i<oports; i++) {
	    sc_signal<SimulatedInstruction *> * instruction =
	      new sc_signal<SimulatedInstruction *>;
	    lazy_stage->out_instruction[i](*instruction);
	    output_queue->in_instruction[i](*instruction);
	  }
	  nb = new sc_signal<int>;
	  nb->write(0);
	  lazy_stage->in_number_of_ins(*nb);
	  input_queue->out_number_of_outs(*nb);
	  nb = new sc_signal<int>;
	  nb->write(0);
	  lazy_stage->out_number_of_accepted_ins(*nb);
	  input_queue->in_number_of_accepted_outs(*nb);
	  nb = new sc_signal<int>;
	  nb->write(0);
	  lazy_stage->out_number_of_outs(*nb);
	  output_queue->in_number_of_ins(*nb);
	  nb = new sc_signal<int>;
	  nb->write(0);
	  lazy_stage->in_number_of_accepted_outs(*nb);
	  output_queue->out_number_of_accepted_ins(*nb);
	}
	  break; /*LAZY */

	case EXECUTE_IN_ORDER: {
	  found = false;
	  InstructionQueue * rob = NULL;
	  for (elm::genstruct::SLList<InstructionQueue *>::Iterator iq(
								       instruction_queues); iq; iq++) {
	    if (iq->configuration() == stage_conf->instructionBuffer()) {
	      rob = iq;
	      found = true;
	    }
	  }
	  assert(found);
	  Trace *trace = new Trace(*_trace_stream, trace_level, "ExecuteStage");
	  ExecuteOOOStage * execute_stage = new ExecuteOOOStage((sc_module_name) (stage_conf->name()), (stage_conf->width()), true, rob, sim_state,
								rename_tables, conf->functionalUnitsList(), &(conf->fu_bindings), trace);
	  pipeline_stages.addLast(execute_stage);
	  execute_stage->in_clock(clock);
	  sc_signal<address_t> * data_addr = new sc_signal<address_t>;
	  memory->in_data_address(*data_addr);
	  execute_stage->out_address(*data_addr);
	  sc_signal<int> * data_size = new sc_signal<int>;
	  memory->in_data_size(*data_size);
	  execute_stage->out_size(*data_size);
	  sc_signal<otawa::sim::CacheDriver::action_t> * data_access_type =
	    new sc_signal<otawa::sim::CacheDriver::action_t>;
	  memory->in_data_access_type(*data_access_type);
	  execute_stage->out_access_type(*data_access_type);
	  sc_signal<bool> * data_req = new sc_signal<bool>;
	  memory->in_data_request(*data_req);
	  execute_stage->out_request(*data_req);
	  sc_signal<bool> * data_wait = new sc_signal<bool>;
	  memory->out_data_wait(*data_wait);
	  execute_stage->in_wait(*data_wait);
	}
	  break;

	case EXECUTE_OUT_OF_ORDER: {
	  found = false;
	  InstructionQueue * rob = NULL;
	  for (elm::genstruct::SLList<InstructionQueue *>::Iterator iq(
								       instruction_queues); iq; iq++) {
	    if (iq->configuration() == stage_conf->instructionBuffer()) {
	      rob = iq;
	      found = true;
	    }
	  }
	  assert(found);
	  Trace *trace = new Trace(*_trace_stream, trace_level, "ExecuteStage");
	  ExecuteOOOStage * execute_stage = new ExecuteOOOStage((sc_module_name) (stage_conf->name()), (stage_conf->width()), false, rob, sim_state,
								rename_tables, conf->functionalUnitsList(), &(conf->fu_bindings), trace);
	  pipeline_stages.addLast(execute_stage);
	  execute_stage->in_clock(clock);

	  sc_signal<address_t> * data_addr = new sc_signal<address_t>;
	  memory->in_data_address(*data_addr);
	  execute_stage->out_address(*data_addr);
	  sc_signal<int> * data_size = new sc_signal<int>;
	  memory->in_data_size(*data_size);
	  execute_stage->out_size(*data_size);
	  sc_signal<otawa::sim::CacheDriver::action_t> * data_access_type =
	    new sc_signal<otawa::sim::CacheDriver::action_t>;
	  memory->in_data_access_type(*data_access_type);
	  execute_stage->out_access_type(*data_access_type);
	  sc_signal<bool> * data_req = new sc_signal<bool>;
	  memory->in_data_request(*data_req);
	  execute_stage->out_request(*data_req);
	  sc_signal<bool> * data_wait = new sc_signal<bool>;
	  memory->out_data_wait(*data_wait);
	  execute_stage->in_wait(*data_wait);

	}
	  break;

	case COMMIT: {
	  assert(stage_conf->inputQueue());
	  found = false;
	  for (elm::genstruct::SLList<InstructionQueue *>::Iterator iq(
								       instruction_queues); iq; iq++) {
	    if (iq->configuration() == stage_conf->inputQueue()) {
	      iports = iq->configuration()->numberOfReadPorts();
	      input_queue = iq;
	      found = true;
	    }
	  }
	  assert(found);
	  Trace *trace = new Trace(*_trace_stream, trace_level, "CommitStage");
	  CommitStage * commit_stage = new CommitStage((sc_module_name) (stage_conf->name()), iports, sim_state, trace);
	  pipeline_stages.addLast(commit_stage);
	  commit_stage->in_clock(clock);

	  for (int i=0; i<iports; i++) {
	    sc_signal<SimulatedInstruction *> * instruction =
	      new sc_signal<SimulatedInstruction *>;
	    commit_stage->in_instruction[i](*instruction);
	    input_queue->out_instruction[i](*instruction);
	  }
	  nb = new sc_signal<int>;
	  nb->write(0);
	  commit_stage->in_number_of_ins(*nb);
	  input_queue->out_number_of_outs(*nb);
	  nb = new sc_signal<int>;
	  nb->write(0);
	  commit_stage->out_number_of_accepted_ins(*nb);
	  input_queue->in_number_of_accepted_outs(*nb);
	}
	  break;
	default:
	  break;
	}
      }
      clock.write(0);
    }

    GenericProcessor::GenericProcessor(
    	sc_module_name name,
    	ProcessorConfiguration * conf, GenericState * sim_state,
    	otawa::hard::Platform *pf,
    	SimulatorFactory *factory)
    {
      _ended = false;

      build(conf, sim_state, pf, -1, factory);
    }

    GenericProcessor::GenericProcessor(
    	sc_module_name name,
    	ProcessorConfiguration * conf,
    	GenericState * sim_state,
    	otawa::hard::Platform *pf,
    	String trace_file_path,
    	int trace_level,
    	SimulatorFactory *factory)
    {
      _ended = false;
      if (!trace_file_path.isEmpty()) {
	_trace_stream = new io::OutFileStream(trace_file_path);
      } else
	_trace_stream = &elm::io::OutStream::null;
      _trace = new Trace(*_trace_stream, 1, (elm::String) "Processor");

      build(conf, sim_state, pf, trace_level, factory);
    }

    bool GenericProcessor::isEmpty() {
      return ( (active_instructions.count() == 0) && _ended);
    }

    void GenericProcessor::step() {
      clock.write(1);
      sc_start(0.5);
      clock.write(0);
      sc_start(0.5);
      stats.incrementTime();
    }

    void GenericProcessor::Flush() {
      for(int i = 0; i <rename_tables->count(); i++) {
        for (int j=0; j<(*rename_tables)[i].reg_bank->count(); j++)
          (*rename_tables)[i].table->set(j,NULL);
      }
    }

  }} // otawa::gensim
