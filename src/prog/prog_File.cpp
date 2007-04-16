/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	File class implementation
 */

#include <otawa/prog/File.h>
#include <otawa/prog/Symbol.h>

namespace otawa {
	
/**
 * @class File
 * This class represents a file involved in the building of a process. A file
 * usually matches a program file on the system file system.
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
 * Property with this identifier is put on instructions or basic blocks which a symbol is known for.
 */
Identifier<String> LABEL("label", "", otawa::NS);


/**
 * This property is put on instruction. An instruction may accept many
 * properties of this type.
 */
Identifier<String> FUNCTION_LABEL("function_label", "", otawa::NS);


/**
 * Find the address of the given label.
 * @param label Label to find.
 * @return	Address of the label or null if label is not found.
 */
address_t File::findLabel(const String& label) {
	Symbol *sym = syms.get(label, 0);
	if(!sym)
		return 0;
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
 * Find an instruction by its address.
 * @param address	Instruction address.
 * @return			Found instruction or null.
 */
Inst *File::findByAddress(address_t address) {
	for(SegIter seg(this); seg; seg++)
		if(seg->address() <= address && address < seg->topAddress()) {
			Inst *inst = seg->findInstAt(address);
			if(inst)
				return inst;
		}
	return 0;
}

} // otawa
