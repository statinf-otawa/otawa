#include <otawa/proc/Processor.h>
#include <otawa/cache/cat2/CAT2Builder.h>
#include <otawa/cache/cat2/CAT2NCBuilder.h>
#include <otawa/util/Dominance.h>
#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/cat2/ACSMayBuilder.h>
#include <otawa/cache/FirstLastBuilder.h>
#include <otawa/util/LBlockBuilder.h>

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
CAT2NCBuilder::CAT2NCBuilder(void): CAT2Builder(reg) {
}

Registration<CAT2NCBuilder> CAT2NCBuilder::reg(
	"otawa::CAT2NCBuilder",
	Version(1, 0, 1),
	p::base, &CAT2Builder::reg,
	p::require, &DOMINANCE_FEATURE,
	p::require, &LOOP_HEADERS_FEATURE,
	p::require, &LOOP_INFO_FEATURE,
	p::require, &COLLECTED_LBLOCKS_FEATURE,
	p::require, &ICACHE_ACS_FEATURE,
	p::require, &ICACHE_ACS_MAY_FEATURE,
	p::require, &ICACHE_FIRSTLAST_FEATURE,
	p::provide, &ICACHE_CATEGORY_FEATURE,
	p::end
);


}	// otawa
