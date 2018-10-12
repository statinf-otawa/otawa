/*
 *	util::SymAddress implementation
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

#include <otawa/cfg/CFGInfo.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/Process.h>
#include <otawa/prog/Symbol.h>
#include <otawa/util/SymAddress.h>

namespace otawa {

/**
 * @class SymAddress
 * Abstraction of a program address that may be expressed using several ways:
 * absolute address, label + offset, source file:line, etc.
 *
 * This type of address is handled either by user, or by external application
 * that generates human-readable output.
 */


/**
 * Parse a symbolic address from a string.
 * @param str				String to parse.
 * @return					Built symbolic address (deletion is delegated to the caller).
 * @throw otawa::Exception	Thrown if there is a parsing error.
 */
SymAddress *SymAddress::parse(string str) {
	str = str.trim();

	// empty string: nothing to do
	if(!str)
		throw otawa::Exception("invalid symbolic address");

	// look for '+' or '-'
	bool neg = false;
	int p = str.indexOf('+');
	if(p < 0) {
		neg = true;
		p = str.indexOf('-');
	}

	// process label + offset if any
	if(p >= 0) {

		// get parts
		string p1 = str.substring(0, p).trim();
		string p2 = str.substring(p + 1).trim();
		if(!p1 || !p2)
				throw otawa::Exception("malformed label+offset symbolic address");

		// get offset
		t::uint32 offset;
		if(!('0' <= p2[0] && p2[0] <= '9')) {
			string t = p2;
			p2 = p1;
			p1 = t;
		}
		try {
			p2 >> offset;
		}
		catch(IOException& e) {
			throw otawa::Exception("malformed label+offset symbolic address");
		}

		// build the address
		if(neg)
			offset = -offset;
		return new LabelAddress(p1, offset);
	}

	// is it an absolute address?
	if('0' <= str[0] && str[0] <= '9') {
		t::uint32 addr;
		try {
			str >> addr;
		}
		catch(IOException &exn) {
			throw otawa::Exception(exn.message());
		}
		return new AbsoluteAddress(addr);
	}

	// else it a simple label
	else
		return new LabelAddress(str);
}


/**
 */
SymAddress::~SymAddress(void) {
}


/**
 * @fn Address SymAddress::toAddress(WorkSpace *ws);
 * Expand the current symbolic address as an actual address
 * in the given workspace.
 * @param ws	Workspace to expand in.
 * @return		Actual address.
 */

/**
 * @fn CFG *SymAddress::cfg(WorkSpace *ws);
 * Get the CFG matching the given address. @ref CFG_INFO_FEATURE
 * should be available first.
 * @param ws	Workspace to look in.
 * @return		Found CFG or null.
 */


/**
 * @fn void SymAddress::print(io::Output& out);
 * Print the symbolic address.
 * @param out	Stream to output to.
 */

/**
 * @class AbsoluteAddress
 * Symbolic address as an absolute address.
 */

/**
 */
AbsoluteAddress::AbsoluteAddress(Address addr): _addr(addr) {
}

/**
 */
Address AbsoluteAddress::toAddress(WorkSpace *ws) {
	return _addr;
}


/**
 */
CFG *AbsoluteAddress::cfg(WorkSpace *ws) {
	/* TODO
	ASSERT(ws->isProvided(CFG_INFO_FEATURE));
	CFGInfo *info = CFGInfo::ID(ws);
	return info->findCFG(ws->findInstAt(_addr));*/
	return 0;
}


/**
 */
void AbsoluteAddress::print(io::Output& out) {
	out << "0x" << _addr;
}


/**
 * @class LabelAddress
 * A label address formed by a label and a possible signed offset.
 */

/**
 */
LabelAddress::LabelAddress(string label, t::int32 offset): _label(label), _offset(offset) {
}

/**
 */
Address LabelAddress::toAddress(WorkSpace *ws) {
	Symbol *sym = ws->process()->findSymbol(_label);
	if(!sym)
		return Address::null;
	else
		return sym->address() + _offset;
}

/**
 */
CFG *LabelAddress::cfg(WorkSpace *ws) {

	// get address
	Address addr = toAddress(ws);
	if(addr.isNull())
		return nullptr;

	// find CFG
	/*	TODO
	ASSERT(ws->isProvided(CFG_INFO_FEATURE));
	CFGInfo *info = CFGInfo::ID(ws);
	return info->findCFG(ws->findInstAt(addr));*/
	return 0;
}

/**
 */
void LabelAddress::print(io::Output& out) {
	out << _label;
	if(_offset) {
		if(_offset < 0)
			out << "-0x" << io::hex(-_offset);
		else
			out << "+0x" << io::hex(_offset);
	}
}

}	// otawa
