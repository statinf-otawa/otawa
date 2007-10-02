#include <otawa/proc/Processor.h>
#include <otawa/cache/cat2/CAT2Builder.h>
#include <otawa/cache/cat2/CAT2NCBuilder.h>
#include <otawa/util/Dominance.h>
#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/cat2/ACSMayBuilder.h>
#include <otawa/cache/FirstLastBuilder.h>
#include <otawa/util/LBlockBuilder.h>

namespace otawa {
	

CAT2NCBuilder::CAT2NCBuilder(void) : CAT2Builder() {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	require(ICACHE_ACS_FEATURE);
	require(ICACHE_ACS_MAY_FEATURE);
	require(ICACHE_FIRSTLAST_FEATURE);
	provide(ICACHE_CATEGORY_FEATURE);
}

}
