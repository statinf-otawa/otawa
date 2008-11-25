


#include <otawa/gensim/Memory.h>
#include <otawa/gensim/GenericState.h>

namespace otawa { namespace gensim {

Identifier<sim::CacheDriver*> ICACHE("otawa::gensim::icache",NULL);
Identifier<sim::CacheDriver*> DCACHE("otawa::gensim::dcache",NULL);


// int GenericState::getNumberOfInstructionAccesses(){
//   MemoryStats* memoryStats = NULL;
//   for (elm::genstruct::SLList<SimulationStats *>::Iterator stat(stats); stat; stat++) { 
//     // test de type de l'objet
//     memoryStats = dynamic_cast<MemoryStats*>(stat.item());
//     if (memoryStats != NULL)
//       return memoryStats->getNbrInstructionAccesses();
//   }
//   return(NULL);
// }

// int GenericState::getNumberOfInstructionCacheHits(){
//   MemoryStats* memoryStats = NULL;
//   for (elm::genstruct::SLList<SimulationStats *>::Iterator stat(stats); stat; stat++) { 
//     // test de type de l'objet
//     memoryStats = dynamic_cast<MemoryStats*>(stat.item());
//     if (memoryStats != NULL)
//       return memoryStats->getNbrInstructionCacheHits();
//   }
//   return(NULL);
// }

// int GenericState::getNumberOfInstructionCacheMisses(){
//   MemoryStats* memoryStats = NULL;
//   for (elm::genstruct::SLList<SimulationStats *>::Iterator stat(stats); stat; stat++) { 
//     // test de type de l'objet
//     memoryStats = dynamic_cast<MemoryStats*>(stat.item());
//     if (memoryStats != NULL)
//       return memoryStats->getNbrInstructionCacheMisses();
//   }
//   return(NULL);
// }

// int GenericState::getNumberOfDataAccesses(){
//   MemoryStats* memoryStats = NULL;
//   for (elm::genstruct::SLList<SimulationStats *>::Iterator stat(stats); stat; stat++) { 
//     // test de type de l'objet
//     memoryStats = dynamic_cast<MemoryStats*>(stat.item());
//     if (memoryStats != NULL)
//       return memoryStats->getNbrDataAccesses();
//   }
//   return(NULL);
// }

// int GenericState::getNumberOfDataReads(){
//   MemoryStats* memoryStats = NULL;
//   for (elm::genstruct::SLList<SimulationStats *>::Iterator stat(stats); stat; stat++) { 
//     // test de type de l'objet
//     memoryStats = dynamic_cast<MemoryStats*>(stat.item());
//     if (memoryStats != NULL)
//       return memoryStats->getNbrDataReads();
//   }
//   return(NULL);
// }

// int GenericState::getNumberOfDataWrites(){
//   MemoryStats* memoryStats = NULL;
//   for (elm::genstruct::SLList<SimulationStats *>::Iterator stat(stats); stat; stat++) { 
//     // test de type de l'objet
//     memoryStats = dynamic_cast<MemoryStats*>(stat.item());
//     if (memoryStats != NULL)
//       return memoryStats->getNbrDataWrites();
//   }
//   return(NULL);
// }

// int GenericState::getNumberOfDataCacheHits(){
//   MemoryStats* memoryStats = NULL;
//   for (elm::genstruct::SLList<SimulationStats *>::Iterator stat(stats); stat; stat++) { 
//     // test de type de l'objet
//     memoryStats = dynamic_cast<MemoryStats*>(stat.item());
//     if (memoryStats != NULL)
//       return memoryStats->getNbrDataCacheHits();
//   }	
//   return(NULL);
// }

// int GenericState::getNumberOfDataCacheMisses(){
//   MemoryStats* memoryStats = NULL;
//   for (elm::genstruct::SLList<SimulationStats *>::Iterator stat(stats); stat; stat++) { 
//     // test de type de l'objet
//     memoryStats = dynamic_cast<MemoryStats*>(stat.item());
//     if (memoryStats != NULL)
//       return memoryStats->getNbrDataCacheMisses();
//   }
//   return(NULL);
// }


/**
 */
sim::State *GenericState::clone(void) {
	return new GenericState(*this);
}


/**
 */
void GenericState::run(sim::Driver& driver) {
	this->driver = &driver;
	if(!icache_driver)
		this->icache_driver = &(sim::CacheDriver::ALWAYS_HIT);
	running = true;
	while (running)
		step();
}


/**
 */
void GenericState::run(sim::Driver& driver, sim::CacheDriver* icache_driver,
		sim::CacheDriver* dcache_driver, sim::MemoryDriver *mem_driver) {
	// !!NOTE!! This method is not mandatory if we use properties : to remove. 
	this->driver = &driver;
	if (!icache_driver) {
	  if (ICACHE(this))
	    this->icache_driver = ICACHE(this);
	  else
	    this->icache_driver = &(sim::CacheDriver::ALWAYS_MISS);
	}
	else
	  this->icache_driver = icache_driver;
	if (!dcache_driver){
	  if (DCACHE(this))
	    this->dcache_driver = DCACHE(this);
	  else
	    this->dcache_driver = &(sim::CacheDriver::ALWAYS_MISS);
	}
	else
		this->dcache_driver = dcache_driver;
	if (!mem_driver)
		this->mem_driver = &(sim::MemoryDriver::ALWAYS_DATA_CACHE);
	else
		this->mem_driver = mem_driver;
	running = true;
	while (running)
		step();
}


/**
 */
void GenericState::stop(void) {
	running = false;
}


/**
 */
void GenericState::flush(void) {
}


/**
 */
int GenericState::cycle(void) {
	return _cycle;
}


/**
 */
void GenericState::reset(void) {
	_cycle = 0;
}


/**
 */
Process *GenericState::process(void) {
	return fw->process();
}

} } //otawa::sim

