/*
 *	$Id$
 *	gensim module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-08, IRIT UPS.
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
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/gensim/GenericState.h>
#include <otawa/gensim/SimulatedInstruction.h>
#include <otawa/program.h>
#include <otawa/otawa.h>
#include <otawa/gensim/debug.h>
#include <otawa/hard/Processor.h>
#include <otawa/hard/Platform.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/sim/AbstractCacheDriver.h>
#include <otawa/gensim/SimulatorFactory.h>

int sc_main(int argc, char *argv[]) {
	int err = dup(2);
	close(2);
	sc_core::sc_elab_and_sim(argc, argv);
	dup2(err, 2);
	close(err);
	return 0;
}

namespace otawa {
namespace gensim {

//Identifier<int> DEGREE("otawa::gensim::degree", 1);  ---------- A REMETTRE

/**
 * @class Exception
 * Exception thrown by the OGenSim module.
 */


/**
 * Build a simulator exception.
 * @param message	Message of the exception.
 */
Exception::Exception(const string& message): otawa::Exception(message) {
}


/**
 * Instruction execution time. Default to 5.
 */
// Identifier<int> INSTRUCTION_TIME("otawa::gensim::instruction_time"); ----------- A REMETTRE


/**
 * @class GenericSimulator
 * The Generic simulator simulates a generic processor.
 */

/**
 * Build an Generic simulator.
 */
GenericSimulator::GenericSimulator(void) :
	Simulator("gensim", Version(0, 3, 0), OTAWA_SIMULATOR_VERSION) {
}

/**
 */
  sim::State *GenericSimulator::instantiate(WorkSpace *fw, const PropList& props) {
    static GenericState* state;
    static bool initialized = false;
    if (!initialized) {
      state = new GenericState(fw);
      TRACE_FILE_PATH(state) = TRACE_FILE_PATH(props);
      TRACE_LEVEL(state) = TRACE_LEVEL(props);
      ICACHE(state) = ICACHE(props);
      DCACHE(state) = DCACHE(props);
      state->init(FACTORY(props));
      initialized = true;
    }
    assert(fw == state->fw);
    return state;
  }

  void GenericState::init(SimulatorFactory *factory) {
    ProcessorConfiguration conf;

    // Get the processor description
    const hard::Processor *oproc = fw->platform()->processor();
    //    const hard::CacheConfiguration& cache = fw->platform()->cache();
    if (!oproc)
      throw LoadException("no processor description available.");

    // Build the queues
    elm::genstruct::Vector<InstructionQueueConfiguration *> queues;
    const elm::genstruct::Table<hard::Queue *>& oqueues = oproc->getQueues();
    for (int i = 0; i< oqueues.count(); i++) {
      /**
       * !!TODO!! Fix it according last processing stage
       */
      simulated_instruction_state_t condition = NONE;
      if (oqueues[i]->getOutput() && oqueues[i]->getOutput()->getType() == hard::Stage::EXEC) {
	if (oqueues[i]->getOutput()->isOrdered())
	  condition = READY;
	else
	  condition = EXECUTED;
      }
      if (oqueues[i]->getIntern())
	condition = EXECUTED;
      InstructionQueueConfiguration *queue =
	new InstructionQueueConfiguration(
					  &oqueues[i]->getName(),
					  oqueues[i]->getSize(),
					  condition); // Fix it according output stage.
      conf.addInstructionQueue(queue);
      queues.add(queue);
    }

    // Build the stages
    const elm::genstruct::Table<hard::Stage *>& stages = oproc->getStages();
    hard::Stage *exec_stage = 0;
    for (int i = 0; i< stages.count(); i++) {

      // Compute in and out queues
      InstructionQueueConfiguration *inqueue = 0, *outqueue = 0;
      for (int j = 0; j < oqueues.count(); j++) {
	if (oqueues[j]->getInput() == stages[i])
	  outqueue = queues[j];
	if (oqueues[j]->getOutput() == stages[i])
	  inqueue = queues[j];
	const elm::genstruct::Table<hard::Stage *>& intern =
	  oqueues[j]->getIntern();
	if (intern)
	  for (int k = 0; k < intern.count(); k++)
	    if (intern[k] == stages[i])
	      outqueue = inqueue = queues[j];
      }

      // Compute the type
      pipeline_stage_t type;
      switch (stages[i]->getType()) {
      case hard::Stage::FETCH:
	type = FETCH;
	break;
      case hard::Stage::LAZY:
	type = LAZYIQIQ;
	break;
      case hard::Stage::EXEC:
	if (stages[i]->isOrdered())
	  type = EXECUTE_IN_ORDER;
	else
	  type = EXECUTE_OUT_OF_ORDER;
	exec_stage = stages[i];
	break;
      case hard::Stage::COMMIT:
	type = COMMIT;
	break;
      case hard::Stage::DECOMP:
	type = DECOMP;
	break;
      default:
	assert(0);
      }

      PipelineStageConfiguration *stage;
      stage = new PipelineStageConfiguration(
					     &stages[i]->getName(),
					     type,
					     inqueue,
					     outqueue,
					     stages[i]->getWidth(),
					     0);
      conf.addPipelineStage(stage);
    }

    // Build functional units
    ASSERTP(exec_stage, "no execution stage !");
    const elm::genstruct::Table<hard::FunctionalUnit *>& fus =
      exec_stage->getFUs();
    elm::genstruct::AllocatedTable<FunctionalUnitConfiguration *> *fu_confs =
      new elm::genstruct::AllocatedTable<FunctionalUnitConfiguration *>(fus.count());
    for (int i = 0; i < fus.count(); i++) {

      // Configure the FU
      FunctionalUnitConfiguration *fu = new FunctionalUnitConfiguration(fus[i]->getName(),
									fus[i]->isPipelined(),
									fus[i]->getLatency(),
									fus[i]->getWidth());
      (*fu_confs)[i] = fu;
      // Build the FU
      conf.addFunctionalUnit(fu);
    }
    const Table<hard::Dispatch *>& dispatch = exec_stage->getDispatch();
    for (int j = 0; j < dispatch.count(); j++) {
      bool found = false;
      for (int k = 0; k < fus.count(); k++)
	if (fus[k] == dispatch[j]->getFU()) {
	  conf.addBinding(dispatch[j]->getType(), (*fu_confs)[k]);
	  found = true;
	}
      assert(found);
    }

    // Create the processor
    elm::String file_name = TRACE_FILE_PATH(this);
    processor = new GenericProcessor(
    	"GenericProcessor",
    	&conf,
    	this,
    	fw->platform(),
    	file_name.toCString(),
    	TRACE_LEVEL(this),
    	factory);
	
    // build the caches
    hard::Platform *pf = fw->platform();
    const hard::CacheConfiguration &caches = pf->cache();
    if (ICACHE(this))
      icache_driver = ICACHE(this);
    else {
      if(caches.hasInstCache())
	icache_driver = sim::AbstractCacheDriver::lookup(caches.instCache());
      else
	icache_driver = &(sim::CacheDriver::ALWAYS_MISS);
    }
    if (DCACHE(this))
      dcache_driver = DCACHE(this);
    else {
      if(caches.hasDataCache()) {
	if(caches.isUnified())
	  dcache_driver = icache_driver;
	else
	  dcache_driver = sim::AbstractCacheDriver::lookup(caches.dataCache());
      }
      else
	dcache_driver  = &(sim::CacheDriver::ALWAYS_MISS);
    }
  }

  void GenericState::step(void) {
    processor->step();
    _cycle ++;
    running = !processor->isEmpty();
  }

void GenericState::resetProc(void){
	processor->Flush();
	processor->_ended = false;
}

}
} // otawa::gensim


/**
 * Plugin hook.
 */
otawa::gensim::GenericSimulator OTAWA_SIMULATOR_HOOK;

/**
 * The entry point to use generic simulators.
 */
otawa::sim::Simulator& gensim_simulator= OTAWA_SIMULATOR_HOOK;

