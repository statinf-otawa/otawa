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

using namespace elm ;
using namespace elm::io ;
using namespace elm::genstruct ;

class PotentialValue : public Set<int> {
  friend Output& operator<< (Output& o, PotentialValue const& pv) ;
  public :
      PotentialValue(void);
      PotentialValue& operator=(const PotentialValue& obj) ;
  private :
};

PotentialValue operator+(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator-(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator>>(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator<<(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator||(const PotentialValue& a, const PotentialValue& b);
bool operator==(const PotentialValue& a, const PotentialValue& b) ;
PotentialValue merge(const PotentialValue& a, const PotentialValue& b) ;

#endif	// OTAWA_DYNBRANCH_POTENTIAL_VALUE_H

