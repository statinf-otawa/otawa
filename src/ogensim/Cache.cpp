#include <otawa/gensim/Fetch.h>
#include <otawa/gensim/GenericState.h>
#include <otawa/gensim/debug.h>

namespace otawa { namespace gensim {

Cache::Cache(sc_module_name name) {
	SC_METHOD(action);
	sensitive_pos << in_clock;
}

void Cache::action() {

}

} } // otawa::gensim
