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

}; // namespace otawa
