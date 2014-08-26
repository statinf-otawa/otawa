/*
 *	PotentialValue class implementation
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
#include "PotentialValue.h"

PotentialValue::PotentialValue(void) {
}

PotentialValue& PotentialValue::operator=(const PotentialValue& a) {
	clear() ;
	for (PotentialValue::Iterator ita(a); ita; ita++)
		insert(*ita) ;
	return *this ;
}

PotentialValue operator+(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res ;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		for(PotentialValue::Iterator itb(b); itb; itb++)
			res.insert(*ita + *itb ) ;
	return res ;
}

PotentialValue operator-(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res ;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		for(PotentialValue::Iterator itb(b); itb; itb++)
			res.insert(*ita - *itb ) ;
	return res ;
}

PotentialValue operator>>(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res ;
	for(PotentialValue::Iterator ita(a); ita; ita++) {
		for(PotentialValue::Iterator itb(b); itb; itb++) {
			res.insert(*ita >> *itb ) ;
		}
	}
	return res ;
}

PotentialValue operator<<(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res ;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		for(PotentialValue::Iterator itb(b); itb; itb++)
			res.insert(*ita << *itb ) ;
	return res ;
}

PotentialValue operator||(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res ;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		for(PotentialValue::Iterator itb(b); itb; itb++)
			res.insert(*ita +*itb ) ;
	return res ;
}

PotentialValue merge(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		res.insert(*ita);
	for ( PotentialValue::Iterator itb(b) ; itb ; itb++) {
		res.insert(*itb);
	}
	return res ;
}

bool operator==(const PotentialValue& a, const PotentialValue& b) {
	for(PotentialValue::Iterator isa(a); isa; isa++)
		if(!b.contains(*isa))
			return false;
	for(PotentialValue::Iterator isb(b); isb; isb++)
		if(!a.contains(*isb))
			return false ;
	return true ;
}

Output& operator<<(Output& o, PotentialValue const& pv) {
	o << "{ " ;
	for(PotentialValue::Iterator i(pv); i; i++)
		o << "0x" << hex(*i) << " " ;
	o << "}" ;
	return o  ;
}
