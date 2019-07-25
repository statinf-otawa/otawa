/*
 *	$Id$
 *	File class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-8, IRIT UPS.
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
#ifndef OTAWA_PROG_FILE_H
#define OTAWA_PROG_FILE_H

#include <elm/inhstruct/DLList.h>
#include <otawa/instruction.h>
#include <otawa/prog/Symbol.h>
#include <otawa/prog/Segment.h>

#include <elm/data/Vector.h>
#include <elm/data/HashMap.h>

namespace otawa {

// Defined classes
class ProgItem;

// File class
class File: public PropList {
	typedef HashMap<String, Symbol *> syms_t;
public:
	static rtti::Type& __type;

	inline File(String name): _name(name) { }
	inline CString name(void) { return _name.toCString(); }
	Inst *findInstAt(address_t address);
	ProgItem *findItemAt(address_t address);
	inline const Vector<Segment *>& segments(void) const { return segs; }
	inline const syms_t& symbols(void) const { return syms; }

	// Segment management
	inline void addSegment(Segment *seg) { segs.add(seg); }
	Segment *findSegmentAt(Address addr);
	class SegIter: public Vector<Segment *>::Iter {
	public:
		inline SegIter(const File *file): Vector<Segment *>::Iter(file->segs) { }
		inline SegIter(const SegIter& iter): Vector<Segment *>::Iter(iter) { }
	};

	// Symbol management
	inline void addSymbol(Symbol *sym) { syms.put(sym->name(), sym); }
	inline void removeSymbol(Symbol *sym) { syms.remove(sym->name()); }
	address_t findLabel(const String& label);
	Symbol *findSymbol(String name);
	class SymIter: public syms_t::Iter {
	public:
		inline SymIter(const File *file): syms_t::Iter(file->syms) { }
		inline SymIter(const SymIter& iter): syms_t::Iter(iter) { }
	};

protected:
	friend class Process;
	~File(void);

private:
	String _name;
	Vector<Segment *> segs;
	syms_t syms;
};

}	// otawa

#endif // OTAWA_PROG_FILE_H
