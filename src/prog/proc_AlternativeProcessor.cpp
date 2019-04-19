/*
 *	AlternativeProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2015, IRIT UPS.
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

#include <otawa/proc/AlternativeProcessor.h>
#include <otawa/proc/AbstractFeature.h>

namespace otawa {

/**
 * @class AlternativeProcessor
 *
 * With OTAWA classic scheduler, the list of required feature if fixed in the registration data structure.
 * Yet, sometimes, it could be convenient to launch analyzes according the current state of the workspace
 * (hardware configuration, program state, etc). This processor can help the developer doing in a controlled manner:
 * the processor, as usual, provide a list of initially required features and a list of requirement alternatives.
 * Then, once the initial requirements has been fulfilled, a special method select() allows choosing between
 * the alternative requirements and they are applied, letting the application use the current workspace to take
 * the decision.
 *
 * Below is an example of AlternativeProcessor usage to select instruction cache analysis!
 * @code
 * class InstructionCacheSupport: public AlternativeProcessor {
 * public:
 * 		static p::declare reg;
 *
 * 		InstructionCacheSupport(void): AlternativeProcessor(reg, alts) { }
 * 			no_alt,
 * 			lru_alt(Alternative(*this).require(LRU_ICACHE_FEATURE)),
 * 			fifo_alt(Alternative(*this).require(FIFO_ICACHE_FEATURE)),
 * 			plru_alt(Alternative(*this).require(PLRU_ICACHE_FEATURE)),
 * 			random_alt(Alternative(*this).require(RANDOM_ICACHE_FEATURE))
 * 		{
 * 		}
 *
 *	protected:
 *		const Alternative& select(WorkSpace *ws) {
 *			const CacheConfiguration& cache = **hard::CACHE_CONFIGURATION(ws);
 *			if(!cache.instCache())
 *				return no_alt;
 *			else
 *				switch(cache.instCache()->replacementPolicy()) {
 *				case hard::Cache::LRU:		return lru_alt;
 *				case hard::Cache::FIFO:		return fifo_alt;
 *				case hard::Cache::PLRU:		return plru_alt;
 *				case hard::Cache::RANDOM:	return random_alt;
 *				}
 *		}
 *
 *	private:
 *		static p::alt no_alt, lru_alt, fifo_alt, plru_alt, random_alt;
 *		static p::altset alts;
 * };
 *
 * p::declare InstructionCacheSupport::reg =
 * 				p::init("InstructionCacheSupport", Version(1, 0, 0), )
 * 				.require(hard::CACHE_CONFIGURATION_FEATURE);
 * p::alt InstructionCacheSupport::no_alt;
 * p::alt InstructionCacheSupport::lru_alt 		= p::alt().require(LRU_ICACHE_FEATURE);
 * p::alt InstructionCacheSupport::fifo_alt 	= p::alt().require(FIFO_ICACHE_FEATURE);
 * p::alt InstructionCacheSupport::plru_alt 	= p::alt().require(PLRU_ICACHE_FEATURE);
 * p::alt InstructionCacheSupport::random_alt	= p::alt().require(RANDOM_ICACHE_FEATURE);
 * p::altset InstructionCacheSupport::alts 		= p::altset().add(no_alt).add(lru_alt).add(fifo_alt).add(plru_alt).add(random_alt);
 * @endcode
 */


/**
 * @class AlternativeProcessor::Alternative
 * Represent an alternetive in the requirement of an AlternativeProcessor, i.e., a list of required or used features.
 */

/**
 * @fn AlternativeProcessor::Alternative& AlternativeProcessor::Alternative::require(const AbstractFeature& f);
 * Add a requirement to the alternative.
 * @param f		Required feature.
 * @return		Alternative itself (for chaining).
 */

/**
 * @fn AlternativeProcessor::Alternative& AlternativeProcessor::Alternative::use(const AbstractFeature& f);
 * Add a use to the alternative.
 * @param f		Used feature.
 * @return		Alternative itself (for chaining).
 */

/**
 * @fn const Vector<FeatureUsage> AlternativeProcessor::Alternative::features(void) const;
 * Get the list of requirements of this alternative.
 * @return		List of requirements.
 */

/**
 * @class AlternativeProcessor::AlternativeSet;
 * Set of alternatives for an AlternativeProcessor.
 */

/**
 * @fn AlternativeProcessor::AlternativeSet& AlternativeProcessor::AlternativeSet::add(const AlternativeProcessor::Alternative& alt);
 * Add an alternative to the set.
 * @param alt	Added alternative.
 * @return		Set itself (for chaining).
 */


/**
 * Build an alternative processor. The alternatives are checked and recorded in the processor.
 * The check will ensure that no requirement in the alternative is provided and invalidated by
 * the current processor. Otherwise, an assertion failure is raised.
 * @param alts	Set ofg alternatives.
 * @param r		Registration.
 */
AlternativeProcessor::AlternativeProcessor(AlternativeSet& alts, AbstractRegistration& r): Processor(r) {
	set = alts;
	for(int i = 0; i < set.length(); i++) {
		const Alternative& alt = *set[i];
		for(Vector<FeatureUsage>::Iter a(alt.features()); a(); a++) {
			IN_ASSERT(const AbstractFeature& f = (*a).feature());
			ASSERTP(!r.requires(f) || !r.requires(f), "feature " << f.name() << " required in registration and in alternative");
		}
	}
}


/**
 * @fn const AlternativeProcessor::Alternative& AlternativeProcessor::select(WorkSpace *ws);
 * This method is called to select an alternative. It is just called after
 * the registration requirement has been fulfilled and before launching the processor
 * computation.
 * @param ws	Current workspace.
 * @return		Alternative to use.
 */

}	// otawa

