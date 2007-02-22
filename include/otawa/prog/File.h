/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	File class interface
 */
#ifndef OTAWA_PROG_FILE_H
#define OTAWA_PROG_FILE_H

#include <elm/inhstruct/DLList.h>
#include <elm/Collection.h>
#include <elm/datastruct/Map.h>
#include <otawa/prog/CodeItem.h>
#include <otawa/instruction.h>
#include <otawa/prog/Symbol.h>
#include <otawa/prog/Segment.h>

#include <elm/genstruct/Vector.h>
#include <elm/genstruct/HashTable.h>

namespace otawa {

using namespace elm::genstruct;

// Defined classes
class ProgItem;
class CodeItem;
class Data;

// File class
class File: public PropList {
	String _name;
	Vector<Segment *> segs;
	typedef HashTable<String, Symbol *> syms_t;
	syms_t syms;
	
protected:
	inline File(String name): _name(name) { }
	inline void addSegment(Segment *seg) { segs.add(seg); }
	inline void addSymbol(Symbol *sym) { syms.put(sym->name(), sym); }

public:
	inline CString name(void) { return _name.toCString(); }
	
	// Segment management
	class SegIter: public Vector<Segment *>::Iterator {
	public:
		inline SegIter(const File *file)
			: Vector<Segment *>::Iterator(file->segs) { }
		inline SegIter(const SegIter& iter)
			: Vector<Segment *>::Iterator(iter) { }
	};
	
	// Symbol management
	address_t findLabel(const String& label);
	Symbol *findSymbol(String name);
	class SymIter: public syms_t::ItemIterator {
	public:
		inline SymIter(const File *file): syms_t::ItemIterator(file->syms) { }
		inline SymIter(const SymIter& iter): syms_t::ItemIterator(iter) { }
	};
};

// Properties
extern Identifier<String> LABEL;
extern Identifier<String> FUNCTION_LABEL;

}	// otawa

#endif // OTAWA_PROG_FILE_H
