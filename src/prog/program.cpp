/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	program.cc -- program classes implementation.
 */

#include <otawa/program.h>

namespace otawa {

/**
 * @class ProgObject
 * This is the base class of all objects representing program parts.
 * It provides facilities for storing and retrieving bound special properties.
 */

/**
 * Virtual destructor for clean deletion of children classes.
 */
ProgObject::~ProgObject(void) {
}


/**
 * @class ProgItem
 * Base class of the components of a program file segment.
 */

/**
 * @fn ProgItem::~ProgItem(void);
 * Protected destructor for avoiding implementation unexpected deletion.
 */
 
/**
 * @fn CString ProgItem::getName(void);
 * Get the name of the program item if some is defined. It may be the name
 * of a function for a piece of code or the name of a data.
 * @return Name ofthis item or an empty string else.
 */

/**
 * @fn address_t ProgItem::getAddress(void);
 * Get the address of the item if some has been assigned.
 * @return Address of the item or address 0 if none is assigned.
 * @note In workstation systems, it is commonly accepted that the address
 * 0 is ever invalid because it is the usual value of NULL in C. It should also
 * work the same for embedded systems.
 */
 
 /**
  * @fn size_t ProgItem::getSize(void);
  * Get the size of the item in bytes.
  * @return	Size of the item.
  */


/**
 * @class Code
 * This program item represents a piece of code. It is usually a function but
 * it may be something else according the processed language or the optimization
 * methods.
 */

/**
 * @fn Inst *Code::head(void) const;
 * Get the first instruction in the code.
 * @return First instruction.
 */

/**
 * @fn Inst *Code::tail(void) const;
 * Get the last instruction in the code.
 * @return Last instruction.
 */


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
 * @fn CString Segment::getName(void);
 * Get tne name of the segment.
 * @return Name of the segment.
 */

/**
 * @fn address_t Segment::getAddress(void);
 * Get the base address of the segment.
 * @return Base address of the segment or 0 if no address has been assigned
 * to the segment.
 */
 
/**
 * @fn size_t Segment::getSize(void);
 * Get the size of the segment.
 * @return Size of the segment.
 */

/**
 * @fn Sequence<ProgItem *> Segment::getItems(void);
 * Get the items contained in the segment.
 * @return Collection of items in the segment.
 */


/**
 * @class File
 * This class represents a file involved in the building of a process. A file
 * usually matches a program file on the system file system.
 */

/**
 * @fn CString File::getName(void);
 * Get the name of the file. It is usually its absolute path.
 * @return Name of the file.
 */

/**
 * @fn Collection<Segment *> File::getSegments(void);
 * Get the segments composing the files.
 * @return Collection of the segments in the file.
 */


}; // namespace otawa
