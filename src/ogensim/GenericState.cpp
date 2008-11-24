


#include <otawa/gensim/Memory.h>
#include <otawa/gensim/GenericState.h>

namespace otawa {
namespace gensim {

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

}
}
