/*
 *	util::SymAddress interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2015, IRIT - UPS.
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
#ifndef OTAWA_UTIL_SYMADDRESS_H_
#define OTAWA_UTIL_SYMADDRESS_H_

#include <elm/io.h>
#include <otawa/base.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {

using namespace elm;
class CFG;

class SymAddress {
public:
	static SymAddress *parse(string str);
	virtual ~SymAddress(void);
	virtual Address toAddress(WorkSpace *ws) = 0;
	virtual CFG *cfg(WorkSpace *ws) = 0;
	virtual void print(io::Output& out) = 0;
};

class AbsoluteAddress: public SymAddress {
public:
	AbsoluteAddress(Address addr);
	virtual Address toAddress(WorkSpace *ws);
	virtual CFG *cfg(WorkSpace *ws);
	virtual void print(io::Output& out);
private:
	Address _addr;
};

class LabelAddress: public SymAddress {
public:
	LabelAddress(string label, t::int32 offset = 0);
	virtual Address toAddress(WorkSpace *ws);
	virtual CFG *cfg(WorkSpace *ws);
	virtual void print(io::Output& out);
private:
	string _label;
	t::int32 _offset;
};

inline io::Output& operator<<(io::Output& out, SymAddress *addr) {
	addr->print(out);
	return out;
}

}	//otawa

#endif /* OTAWA_UTIL_SYMADDRESS_H_ */
