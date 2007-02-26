/*
 * $Id$
 * Copyright (c) 2006, IRIT-UPS
 *
 * otawa/gensim/Fetch.h -- FetchStage class interface
 */
#ifndef OTAWA_GENSIM_CACHE_H
#define OTAWA_GENSIM_CACHE_H

#include <otawa/otawa.h>

namespace otawa { namespace gensim {

// External class
class GenericState;

class Cache: public sc_module {
	public:
		// signals
		sc_in<bool> in_clock;
		sc_in<address_t> in_address;
		sc_in<int> in_requested_bytes;
		sc_out<bool> out_hit;
		sc_out<int> out_delivered_bytes;

	public:

		Cache(sc_module_name name);

		SC_HAS_PROCESS(Cache);	
		void action();

};

} } // otawa::gensim

#endif // OTAWA_GENSIM_CACHE_H
