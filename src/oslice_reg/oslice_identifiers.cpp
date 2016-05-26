#include "otawa/oslice_reg/oslice_reg.h"

namespace otawa { namespace oslice_reg {
Identifier<interested_instructions_t*> INTERESTED_INSTRUCTIONS("", 0);
Identifier<String> SLICED_CFG_OUTPUT_PATH("otawa::oslice_reg::SLICED_CFG_OUTPUT_PATH", "");
Identifier<String> SLICING_CFG_OUTPUT_PATH("otawa::oslice_reg::SLICING_CFG_OUTPUT_PATH", "");
Identifier<InstSet*> SET_OF_REMAINED_INSTRUCTIONS("otawa::oslice_reg::SET_OF_REMAINED_INSTRUCTIONS", 0);
Identifier<int> SLICE_DEBUG_LEVEL("otawa::oslice_reg::DEBUG_LEVEL", 0);
Identifier<bool> CFG_OUTPUT("otawa::oslice_reg::CFG_OUTPUT", false);
}}
