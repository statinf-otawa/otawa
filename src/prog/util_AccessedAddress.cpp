/*
 *	AccessAddress and AccessesAddresses classes implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2015, IRIT UPS.
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
#include <otawa/prog/Inst.h>
#include <otawa/stack/AccessedAddress.h>

namespace otawa {

/**
 * @defgroup addr	Memory Accesses
 * This module includes all classes providing information on accessed memories.
 */

/**
 * @class AccessedAddress
 * Parent class of classes representing accesses to memory.
 * @ingroup addr
 */

/**
 * @typedef AccessedAddress::kind_t.
 * Describe the different implementation of an accessed address.
 */

/**
 * @var AccessedAddress::ANY
 * Represents the most imprecise memory access: it may potentially
 * concern the full memory address space.
 */

/**
 * @var AccessedAddress::SP
 * Represents an stack pointer relative address with a constant offset.
 * Instances of this kind can be casted into @ref SPAddress.
 */

/**
 * @var AccessedAddress::ABS
 * Represents an address defined by its absolute and constant value.
 * Instances of this kind can be casted into @ref AbsAddress.
 */

inline char sign(t::int32 v) {
	if(v < 0)
		return '-';
	else
		return '+';
}

/**
 * Print the given address.
 * @param out	Output stream.
 */
void AccessedAddress::print(io::Output& out) const {
	out << "\t" << inst->address() << "\t";
	out << "\t";
	if(store)
		out << "store ";
	else
		out << "load  ";
	switch(knd) {
	case ANY:
		out << "T";
		break;
	case ABS:
		out << "0x" << static_cast<const AbsAddress *>(this)->address();
		break;
	case SP: {
			const SPAddress *a = static_cast<const SPAddress *>(this);
			out << "SP " << sign(a->offset()) << ' ' << io::hex(elm::abs(a->offset()));
		}
		break;
	}
	out << io::endl;
}

/**
 * @fn AccessedAddress::kind_t AccessedAddress::kind(void) const;
 * Get the kind of the address. According to this kind, this instance
 * may be cast in the corresponding subclass of @ref AccessedAddress.
 * @return	Kind of address.
 */

/**
 * @fn bool AccessedAddress::isStore(void) const;
 * Test if a store is performed on the accessed address.
 * @return	True if it is a store.
 */

/**
 * @fn bool AccessedAddress::isLoad(void) const;
 * Test if a store is performed on the accessed address.
 * @return	True if it is a store.
 */

/**
 * @fn Inst *AccessedAddress::instruction(void) const;
 * Get the instruction where this memory access applies.
 * @return	Instruction accessing the memory.
 */


/**
 * This feature ensures that accessed memory information (class @ref AccessedAddress)
 * is provided for each basic block. This feature is useful for any static analysis
 * concerning the memory hierarchy (caches, flash memories with prefetch, etc).
 *
 * @p Properties
 * @li @ref ADDRESSES
 *
 * @p Statistics
 * @li @ref ADDRESS_STATS
 *
 * @p Processors
 * @li No default.
 * @li @ref AccessedAddressFromStack
 */
p::feature ADDRESS_ANALYSIS_FEATURE("otawa::ADDRESS_ANALYSIS_FEATURE", new Maker<NoProcessor>());


/**
 * Provide the set of accesses to the memory for the current basic block.
 *
 * @p Hooks
 * @li @ref BasicBlock
 *
 * @p Features
 * @li @ref ADDRESS_ANALYSIS_FEATURE
 * @li @ref ADDRESS_FROM_STACK_FEATURE
 */
Identifier<AccessedAddresses *> ADDRESSES("otawa::ADDRESSES", 0);


/**
 * This property provides statistics about @ref ADDRESS_ANALYSIS_FEATURE.
 */
Identifier<address_stat_t *> ADDRESS_STATS("otawa::ADDRESS_STATS", 0);


/**
 * @class SPAddress
 * Subclass of @ref AccessedAddress, this class represents address with
 * a constant offset relative to the stack pointer. Any accessed address
 * of kind @ref AccessedAddress::SP can be cast in this class.
 * @ingroup addr
 */


/**
 * @class AbsAddress
 * Subclass of @ref AccessedAddress, this class represents an absolute address
 * access, to a global variable or to an input/output register for example.
 * Any accessed address of kind @ref AccessedAddress::ABS can be cast in this class.
 * @ingroup addr
 */



/**
 * @class AccessedAddresses
 * For each basic block, provide the list of performed memory accesses and their addresses.
 * This class is designed to be build in an incremental way. Each analysis with the ability
 * to produce a memory address can add its own contribution to this class.
 *
 * @ingroup addr
 */


/**
 */
io::Output& operator<<(io::Output& out, address_stat_t *stats) {
	out << "ADDRESS STATICTICS\n";
	cout << "unknown:\t" << stats->all << " (" << (stats->total ? (stats->all * 100. / stats->total) : 0) << "%)\n";
	cout << "SP-relative:\t" << stats->sprel << " (" << (stats->total ? (stats->sprel * 100. / stats->total) : 0) << "%)\n";
	cout << "absolute:\t" << stats->abs << " (" << (stats->total ? (stats->abs * 100. / stats->total) : 0) << "%)\n";
	cout << "total:\t" << stats->total << " (100%)\n";
	return out;
}

}	// otawa
