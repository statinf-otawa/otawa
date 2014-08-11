#include <otawa/proc/Processor.h>
#include <otawa/cache/cat2/CAT2Builder.h>
#include <otawa/cache/cat2/CAT2NCBuilder.h>
#include <otawa/util/Dominance.h>
#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/cat2/ACSMayBuilder.h>
#include <otawa/cache/FirstLastBuilder.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/cache/categories.h>

namespace otawa {
	
/**
 * @class CAT2NCBuilder
 *
 * This processor produces categorization information for each l-block.
 * It is essentially the same processor as CAT2Builder, except it differentiates the ALWAYS_MISS
 * and NOT_CLASSIFIED categories by using the MAY ACS.
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 * @li @ref ICACHE_ACS_FEATURE
 * @li @ref ICACHE_FIRSTLAST_FEATURE
 *
 * @par Provided features
 * @li @ref ICACHE_CATEGORY2_FEATURE
 * 
 * @par Statistics
 * none
 */

/**
 */
CAT2NCBuilder::CAT2NCBuilder(p::declare& r): CAT2Builder(r) {
}

p::declare CAT2NCBuilder::reg = p::init("otawa::CAT2NCBuilder", Version(1, 0, 1))
	.maker<CAT2NCBuilder>()
	.base(CAT2Builder::reg)
	.require(DOMINANCE_FEATURE)
	.require(LOOP_HEADERS_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(COLLECTED_LBLOCKS_FEATURE)
	.require(ICACHE_ACS_FEATURE)
	.require(ICACHE_ACS_MAY_FEATURE)
	.require(ICACHE_FIRSTLAST_FEATURE)
	.provide(otawa::cache::ICACHE_CATEGORY_FEATURE);

}	// otawa
