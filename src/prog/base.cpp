/*
 *	$Id$
 *	Base declaration implementation.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-07, IRIT UPS.
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

#include <otawa/base.h>
using namespace elm;

namespace otawa {

/**
 * @class Exception
 * Base class of Otawa exceptions.
 */
 
/**
 * Build an empty exception.
 */
Exception::Exception(void): MessageException("") {
}

/**
 * Build a simple exception with the given message.
 * @param message	Message of the exception.
 */
Exception::Exception(string message): MessageException(message) {
}


/**
 * Build OTAWA exception from another exception.
 * @param exn	Exception to build from.
 */
Exception::Exception(elm::Exception& exn): MessageException(exn.message()) {
}


/**
 * Build OTAWA exception from a message and another exception.
 * Message and exception message are joined separated by a ":".
 * @param message	Exception message.
 * @param exn		Exception to join with.
 */
Exception::Exception(string message, elm::Exception& exn)
: MessageException(_ << message << ": " << exn.message()) {
}


/**
 * @fn const String& Exception::getMessage(void) const
 * Get the exception message.
 * @return	Exception message.
 */


/**
 * @class Address
 * The representation of an address in OTAWA. To match most architectures,
 * the address is split in two component:
 * @li a page component,
 * @li an offset component from the page.
 * 
 * The page component may be used to represent different, non overlapping
 * address spaces. For example, a Harvard architecture for example: page 0 may
 * be used to store code and page 1 to store data.
 * 
 * It is bad idea to use the page to
 * represent segemented memory as in the paged mode of the x86 architecture.
 * The rule is that each item must have a unique address in order to perform
 * valid address comparison and computation. For example, the M68HCxx
 * banking memory that assign different code banks in memory from 0x8000 to
 * 0xC000 is better handled by assigning to each an absolute address above
 * the maximal address computed by: 0x10000 + bank_number * 0x4000.
 * 
 * Finally, notice that the page 0xffffffff is reserved to represent the
 * null address.
 */


/**
 * Null address. Please that the null address is not the adress 0 but an
 * invalid address that can not be mapped to the platform memory :
 * <center>Address::null != Address(0x00000000)</center>
 */
Address Address::null;


/**
 * @fn Address::Address(void);
 * Build a null address.
 */


/**
 * @fn Address::Address(offset_t offset);
 * Build a simple absolute address.
 * @param offset	Absolute value of the address.
 */


/**
 * @fn Address::Address(page_t page, offset_t offset)
 * Build a full address.
 * @param page		Address page.
 * @param offset	Offset in the page.
 */


/**
 * @fn Address::Address(const Address& address)
 * Build an address by cloning.
 * @param address	Cloned address.
 */


/**
 * @fn page_t Address::page(void) const;
 * Get the page number.
 * @return	Page number.
 */


/**
 * @fn offset_t Address::offset(void) const;
 * Get the offset value.
 * @return	Offset value.
 */


/**
 * @fn offset_t Address::operator*(void) const;
 * Short cut to offset().
 */


/**
 * @fn bool Address::isNull(void) const;
 * Test if the address is null.
 * @return	True if it is the null address, false else.
 */


/**
 * @fn Address::operator offset_t(void) const;
 * shortcut to offset().
 */


/**
 * @fn bool Address::equals(const Address& address) const;
 * Test if two address are equals.
 * @param address	Address to compare with.
 * @return			True if both addresses are equals, false else.
 */


/**
 * @fn int Address::compare(const Address& address);
 * Compare two addresses and returns:
 * @li <0 if the current address is less than the given one,
 * @li =0 if both addresses are equal,
 * @li >0 if the current address is greater than the given one.
 * @warning	It is an error to compare address of different pages.
 * @param address	Address to compare to.
 * @return			See above.
 */


/**
 */
elm::io::Output& operator<<(elm::io::Output& out, Address addr) {
	if(addr.isNull())
		out<< "<null>";
	else {
		if(addr.page())
			out << addr.page() << ':';
		out << ot::address(addr.offset());
	}
	return out;
}


/**
 * @typedef signed long long time_t;
 * This type represents timing in OTAWA, in processor cycles.
 */


namespace ot {

/**
 * Build a format to display addresses.
 * @param addr	Address to display.
 * @return		Format to display the address.
 */
elm::io::IntFormat address(Address addr) {
	return elm::io::right(elm::io::width(8, elm::io::pad('0',
		elm::io::hex(addr.offset()))));
}

}	// ot


/**
 * @class MemArea
 * Utility class representing an area in the memory defined by a base address
 * and a size.
 */


/**
 * Test if the current mem area includes (not strictly) the given one.
 * @param a		Memory area tested for inclusion.
 * @return		True if the current memory area contains the a memory area.
 */
bool MemArea::includes(const MemArea& a) const {
	return address() <= a.address() && a.topAddress() <= topAddress();
}


/**
 * Test if the current memory area meets at least one byte
 * of the given one.
 * @param a		Memory area to test.
 * @return		True if there is, at least, one byte in common between both memory areas.
 */
bool MemArea::meets(const MemArea& a) const {
	return	(address() <= a.address() && a.address() < topAddress())
		||	(a.address() <= address() && address() < a.topAddress());
}


/**
 * Build the intersection of both memory areas.
 * @param a		Memory area to meet with.
 * @return		Result of meet.
 */
MemArea MemArea::meet(const MemArea& a) const {
	Address low = max(address(), a.address());
	Address high = min(topAddress(), a.topAddress());
	if(low < high)
		return MemArea(low, high);
	else
		return null;
}


/**
 * Build a memory area that is the inclusive join of both memory areas.
 * @param a		Memory area to join with.
 * @return		Result of inclusive join.
 */
MemArea MemArea::join(const MemArea& a) const {
	return MemArea(min(address(), a.address()), max(topAddress(), a.topAddress()));
}


/**
 * Null memory area.
 */
MemArea MemArea::null;


/**
 */
io::Output& operator<<(io::Output& out, const MemArea& a) {
	if(a.isNull())
		out << "<null area>";
	else
		out << a.address() << ":" << a.topAddress();
	return out;
}


}	// otawa

