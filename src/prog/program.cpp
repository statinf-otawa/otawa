/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	program.cc -- program classes implementation.
 */

#include <otawa/program.h>

namespace otawa {

/**
 * @class Data
 * This class represent a data item in the program. According information
 * available to Otawa, it may represent a block of anonymous data or 
 * may match some real variable from the underlying programming language.
 */

/**
 * @fn Type *Data::getType(void);
 * Get the type of the data block.
 * @return Type of the data block. Even when the type is not explicitly known,
 * a byte block type object is returned.
 */


/**
 * @class Segment
 * @par In usual file format like ELF, COFF and so on, the program file is
 * divided in segment according platform needs or memory propertes.
 * @par Usually, we find a ".text" segment containing program code,
 * ".data" containing initialized data, ".bss" containing uninitialized data,
 * ".rodata" containing read-only data. Yet, more segments may be available.
 * @par Making the segment an abstract class allows the platform applying
 * constraints to its address and size.
 */

/**
 * @fn Segment::~Segment(void);
 * Protected destructor for avoiding implementation unexpected deletion.
 */

/**
 * @fn CString Segment::name(void);
 * Get tne name of the segment.
 * @return Name of the segment.
 */

/**
 * @fn address_t Segment::address(void);
 * Get the base address of the segment.
 * @return Base address of the segment or 0 if no address has been assigned
 * to the segment.
 */
 
/**
 * @fn size_t Segment::size(void);
 * Get the size of the segment.
 * @return Size of the segment.
 */

/**
 * @fn elm::Collection<ProgItem *> Segment::items(void);
 * Get the items contained in the segment.
 * @return Collection of items in the segment.
 */

/**
 * @fn int Segment::flags(void);
 * Get flag information about the segment. This flags are composed by OR'ing the constants
 * EXECUTABLE and WRITABLE.
 * @return Flags value.
 */

/**
 * @class File
 * This class represents a file involved in the building of a process. A file
 * usually matches a program file on the system file system.
 */

/**
 * @fn CString File::name(void);
 * Get the name of the file. It is usually its absolute path.
 * @return Name of the file.
 */

/**
 * @fn const elm::Collection<Segment *> File::segments(void) const;
 * Get the segments composing the files.
 * @return Collection of the segments in the file.
 */


/**
 * Property with this identifier is put on instructions or basic blocks which a symbol is known for.
 * Its property is of type String.
 */
Identifier File::ID_Label("otawa.file.label");


/**
 * This property is put on instruction and gives a Symbol * pointing the
 * instruction. An instruction may accept many properties of this type.
 */
Identifier File::ID_FunctionLabel("otawa.file.function_label");


/**
 * @fn address_t File::findLabel(const String& label);
 * Find the address of the given label.
 * @param label Label to find.
 * @return	Address of the label or null if label is not found.
 */


/**
 * @fn Symbol *File::findSymbol(String name);
 * Find a symbol  by its name.
 * @param name	Symbol name.
 * @return		Found symbol or null.
 */


/**
 * @fn elm::Collection<Symbol *>& symbols(void) ;
 * Get the collection of existing symbols.
 * @return	Symbol collection.
 */

}; // namespace otawa
