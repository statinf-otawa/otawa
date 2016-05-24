#include "otawa/oslice/oslice.h"

namespace otawa { namespace oslice {
Identifier<interested_instructions_t*> INTERESTED_INSTRUCTIONS("", 0);
Identifier<String> SLICED_CFG_OUTPUT_PATH("otawa::oslice::SLICED_CFG_OUTPUT_PATH", "");
Identifier<String> SLICING_CFG_OUTPUT_PATH("otawa::oslice::SLICING_CFG_OUTPUT_PATH", "");
Identifier<InstSet*> SET_OF_REMAINED_INSTRUCTIONS("otawa::oslice::SET_OF_REMAINED_INSTRUCTIONS", 0);
Identifier<int> SLICE_DEBUG_LEVEL("otawa::oslice::DEBUG_LEVEL", 0);
Identifier<bool> CFG_OUTPUT("otawa::oslice::CFG_OUTPUT", false);
}}
