/*
 *	File class implementation
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <otawa/prog/File.h>
#include <otawa/prog/Symbol.h>
#include <otawa/prog/Process.h>
#include <otawa/prop/info.h>

namespace otawa {

/**
 * @class File
 * This class represents a file involved in the building of a process. A file
 * usually matches a program file on the system file system.
 * @ingroup prog
 */


/**
 * @fn File::File(String name)
 * Build a file with the given name.
 * @param name	Name of the file.
 */


/**
 * @fn void File::addSegment(Segment *seg)
 * Add the given segment to the file.
 * @param seg	Added segment.
 */


/**
 * @fn void File::addSymbol(Symbol *sym)
 * Add the given symbol to the file.
 * @param sym	Added symbol.
 */


/**
 * @fn CString File::name(void);
 * Get the name of the file. It is usually its absolute path.
 * @return Name of the file.
 */


/**
 * Find the address of the given label.
 * @param label Label to find.
 * @return	Address of the label or null if label is not found.
 */
address_t File::findLabel(const String& label) {
	Symbol *sym = syms.get(label, 0);
	if(!sym)
		return Address::null;
	else
		return sym->address();
}


/**
 * Find a symbol  by its name.
 * @param name	Symbol name.
 * @return		Found symbol or null.
 */
Symbol *File::findSymbol(String name) {
	return syms.get(name, 0);
}

/**
 * Find a symbol  by its address
 * @param name	Symbol address.
 * @return		Found symbol or null.
 */
Symbol *File::findSymbol(address_t addr) {
	Symbol *s = nullptr;
	for(auto sym : syms) {
		if(sym->address() == addr) {
			s = sym;
			break;
		}
	}
	return s;
}

/**
 * Find a symbol which contains an address
 * @param name	Symbol address.
 * @return		Found symbol or null.
 */
Symbol *File::findContainingSymbol(address_t addr) {
	Symbol *s = nullptr;
	for(auto sym : syms) {
		if(sym->address() <= addr && sym->address()+sym->size() > addr) {
			s = sym;
			break;
		}
	}
	return s;
}


/**
 * @class SegIter
 * Iterator for segments in a file.
 */


/**
 * @fn File::SegIter::SegIter(const File *file)
 * Build a segment iterator on the given file.
 * @param file	File to visit segments in.
 */


/**
 * @fn File::SegIter::SegIter(const SegIter& iter)
 * Copy constructor.
 * @param iter	Iterator to copy.
 */


/**
 * @class SymIter
 * Iterator on symbols of a file.
 */


/**
 * @fn File::SymIter::SymIter(const File *file)
 * Build a symbol iterator on the given file.
 * @param file	File to visit symbols in.
 */


/**
 * @fn File::SymIter::SymIter(const SymIter& iter)
 * Copy constructor.
 * @param iter	Iterator to copy.
 */


/**
 * Inst *File::findByAddress(address_t address);
 * Find an instruction by its address.
 * @param address	Instruction address.
 * @return			Found instruction or null.
 * @deprecated	Use @ref findInstAt() instead.
 */


/**
 * Find an instruction by its address.
 * @param address	Instruction address.
 * @return			Found instruction or null.
 */
Inst *File::findInstAt(address_t address) {
	for(SegIter seg(this); seg(); seg++) {
		if(seg->address() <= address && address < seg->topAddress()) {
			Inst *inst = seg->findInstAt(address);
			if(inst)
				return inst;
		}
	}
	return nullptr;
}


/**
 * Find the segment at the given address.
 * @param addr		Looked address.
 * @return			Found segment or null.
 */
Segment *File::findSegmentAt(Address addr) {
	for(SegIter seg(this); seg(); seg++)
		if(seg->contains(addr))
			return *seg;
	return 0;
}


/**
 * Find a program item by its address.
 * @param address	Program item address.
 * @return			Found program item or null.
 */
ProgItem *File::findItemAt(address_t address) {
	for(SegIter seg(this); seg(); seg++) {
		ProgItem *item = seg->findItemAt(address);
		if(item)
			return item;
	}
	return 0;
}


/**
 */
File::~File(void) {

	// Free segments
	for(SegIter seg(this); seg(); seg++)
		delete *seg;

	// Free symbols
	for(SymIter sym(this); sym(); sym++)
		delete *sym;
}


} // otawa
