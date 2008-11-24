/*
 *	$Id$
 *	gensim module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_GENSIM_CACHE_H
#define OTAWA_GENSIM_CACHE_H

#include <otawa/otawa.h>

namespace otawa {
namespace gensim {

// External class
class GenericState;

class Cache : public sc_module {
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

}
} // otawa::gensim

#endif // OTAWA_GENSIM_CACHE_H
