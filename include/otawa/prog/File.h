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
//#include <elm/Collection.h>
//#include <elm/datastruct/Map.h>
#include <otawa/instruction.h>
#include <otawa/prog/Symbol.h>
#include <otawa/prog/Segment.h>

#include <elm/genstruct/Vector.h>
#include <elm/genstruct/HashTable.h>

namespace otawa {

using namespace elm::genstruct;

// Defined classes
class ProgItem;

// File class
class File: public PropList {
	String _name;
	Vector<Segment *> segs;
	typedef HashTable<String, Symbol *> syms_t;
	syms_t syms;
	

public:
	inline File(String name): _name(name) { }
	inline CString name(void) { return _name.toCString(); }
	Inst *findInstAt(address_t address);
	ProgItem *findItemAt(address_t address);
	
	// Segment management
	inline void addSegment(Segment *seg) { segs.add(seg); }
	class SegIter: public Vector<Segment *>::Iterator {
	public:
		inline SegIter(const File *file)
			: Vector<Segment *>::Iterator(file->segs) { }
		inline SegIter(const SegIter& iter)
			: Vector<Segment *>::Iterator(iter) { }
	};
	
	// Symbol management
	inline void addSymbol(Symbol *sym) { syms.put(sym->name(), sym); }
	inline void removeSymbol(Symbol *sym) { syms.remove(sym->name()); }
	address_t findLabel(const String& label);
	Symbol *findSymbol(String name);
	class SymIter: public syms_t::Iterator {
	public:
		inline SymIter(const File *file): syms_t::Iterator(file->syms) { }
		inline SymIter(const SymIter& iter): syms_t::Iterator(iter) { }
	};

	// Deprecated
	inline Inst *findByAddress(address_t address) { return findInstAt(address); }

protected:
	friend class Process;
	~File(void);
};

// Properties
extern Identifier<String> LABEL;
extern Identifier<String> FUNCTION_LABEL;

}	// otawa

#endif // OTAWA_PROG_FILE_H
