/*
 *	PotentialValue class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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
#ifndef OTAWA_DYNBRANCH_POTENTIAL_VALUE_H
#define OTAWA_DYNBRANCH_POTENTIAL_VALUE_H

#include "Set.h"
#include <elm/io/OutStream.h>
#include <elm/genstruct/SLList.h>
#include <otawa/prop/Identifier.h>

using namespace elm;
using namespace elm::io;
using namespace elm::genstruct;

namespace otawa { namespace dynbranch {

#ifdef SAFE_MEM_ACCESS
class PotentialValueMem;
typedef SLList<PotentialValueMem*> potential_value_list_t;
#else
class PotentialValue;
typedef SLList<PotentialValue*> potential_value_list_t;
#endif

class PotentialValue: public Set<elm::t::uint32> {
	friend Output& operator<<(Output& o, PotentialValue const& pv);
public:
	PotentialValue(void);
	PotentialValue(const PotentialValue& cpv);
	PotentialValue& operator=(const PotentialValue& obj);
	~PotentialValue(void);
	// for FastState:
	typedef PotentialValue t;
	static PotentialValue bot;
	static PotentialValue top;
	static PotentialValue DEFAULT;
	inline void dump(Output& o, PotentialValue &pv) { o << pv; }

	inline bool equals(PotentialValue &a, PotentialValue &b) { return a == b; }
	static unsigned int MAGIC;
	unsigned int magic;
	static potential_value_list_t potentialValueCollector;
#ifdef SAFE_MEM_ACCESS
	PotentialValueMem* pvm;
#endif
};

PotentialValue operator&(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator+(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator|(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator^(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator~(const PotentialValue& a);
PotentialValue operator-(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator>>(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator<<(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator||(const PotentialValue& a, const PotentialValue& b);
bool operator==(const PotentialValue& a, const PotentialValue& b);
PotentialValue merge(const PotentialValue& a, const PotentialValue& b);
PotentialValue logicalShiftRight(const PotentialValue& a, const PotentialValue& b);

#ifdef SAFE_MEM_ACCESS
class PotentialValueMem {
public:
	PotentialValue* pv;
	bool status;
};
#endif

//extern Identifier<potential_value_list_t* > DYNBRANCH_POTENTIAL_VALUE_LIST;
}}
#endif	// OTAWA_DYNBRANCH_POTENTIAL_VALUE_H

