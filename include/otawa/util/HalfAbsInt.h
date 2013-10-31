/*
 *	"Half" abstract interpretation class interface.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_UTIL_HALFABSINT_H
#define OTAWA_UTIL_HALFABSINT_H

#include <otawa/dfa/hai/HalfAbsInt.h>
#include <otawa/util/DefaultListener.h>
#include <otawa/util/DefaultFixPoint.h>

namespace otawa {

using namespace dfa::hai;

namespace util { using namespace dfa::hai; }

// backward compatibility declaration
/*template <class F>
class HalfAbsInt: public dfa::hai::HalfAbsInt<F> {
public:
	inline HalfAbsInt(F& _fp, WorkSpace& _fw):
		dfa::hai::HalfAbsInt<F>(_fp, _fw) { }
};

typedef dfa::hai::hai_context_t hai_context_t;
enum util_context_t {
	CTX_LOOP = dfa::hai::CTX_LOOP,
	CTX_FUNC = dfa::hai::CTX_FUNC
};
inline bool operator==(hai_context_t v1, util_context_t v2) { return v1 == hai_context_t(v2); }
inline bool operator!=(hai_context_t v1, util_context_t v2) { return v1 != hai_context_t(v2); }
inline bool operator==(util_context_t v1, hai_context_t v2) { return v2 == hai_context_t(v1); }
inline bool operator!=(util_context_t v1, hai_context_t v2) { return v2 != hai_context_t(v1); }

} // util*/

} // otawa

#endif // OTAWA_UTIL_HALFABSINT_H

