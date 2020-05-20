/*
 *	Platform class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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

#include <otawa/prog/WorkSpace.h>
#include <otawa/hard/Memory.h>
#include <elm/serial2/XOMUnserializer.h>

using namespace elm;

namespace otawa { namespace hard {

/**
 * @class ModeTransition
 * Representation of a memory bank transition.
 * @author H. Cassé <casse@irit.fr>
 * @ingroup hard
 */


/**
 * @fn ModeTransition::ModeTransition(void);
 * Build a default transition mode with :
 * @li latency to 1,
 * @li power to 0,
 * @li no destination mode.
 */


/**
 * @fn int ModeTransition::latency(void) const;
 * Get the latency of the transition.
 * @return Transition latency (in cyles).
 */


/**
 * @fn int ModeTransition::power(void) const;
 * Get the power consumed by the transition.
 * @return	Consumed power.
 */


/**
 * @fn const Mode *ModeTransition::dest(void) const;
 * Get the destination mode of the transition.
 * @return	Destination mode.
 */


/**
 * @class Mode
 * A working mode for the hardware memory.
 * @author H. Cassé <casse@irit.fr>
 * @ingroup hard
 */


/**
 * @fn Mode::Mode(void);
 * Constructor with default initialization:
 * @li no name,
 * @li latency to 1 cycle,
 * @li static power to 0,
 * @li dynamic power to 0,
 * @li no transition available.
 */


/**
 * @fn const string& Mode::name(void) const;
 * Get the mode name.
 * @return Mode name.
 */


/**
 * @fn int Mode::latency(void) const;
 * Get the latency to access the memory in this mode.
 * @return	Access latency (in cycles).
 */


/**
 * @fn int Mode::staticPower(void) const;
 * Get the static power consumed by this memory.
 * @return	Static power.
 */


/**
 * @fn int Mode::dynamicPower(void) const;
 * Get the dynamic power consumed by this memory.
 * @return	Dynamic power.
 */


/**
 * @fn const Table<ModeTransition>& Mode::transitions(void) const;
 * Get the mode transition table (may be empty).
 * @return	Mode transition table.
 */


/**
 * @class Bus
 * A bus that tie together several memory banks.
 * @author H. Cassé <casse@irit.fr>
 * @ingroup hard
 */


/**
 * @typedef Bus::type_t
 * Type of used bus. Either @ref otawa::hard::Bus::LOCAL for an in-chip bus
 * or @ref otawa::hard::Bus::SHARED for a bus by different active parts.
 */


/**
 * @fn Bus::Bus(void);
 * Default bus building:
 * @li no name,
 * @li the type is @ref Bus::LOCAL.
 */


/**
 * @class Bank
 * A bank in the memory. Usually, objects of this class map real hardware chip but some
 * memory segments may grouped or separated according their characteristics.
 * @author H. Cassé <casse@irit.fr>
 * @ingroup hard
 */

/**
 */
Bank::Bank(void):
	_name("no name"),
	_size(0),
	_type(NONE),
	_latency(10),
	_power(0),
	_write_latency(0),
	_block_bits(0),
	_cached(true),
	_on_chip(true),
	_writable(true),
	_port_num(1),
	_bus(0)
{ }


/**
 */
Bank::Bank(cstring name, Address address, size_t size):
	_name(name),
	_address(address),
	_size(size),
	_type(NONE),
	_latency(10),
	_power(0),
	_write_latency(10),
	_block_bits(0),
	_cached(true),
	_on_chip(true),
	_writable(true),
	_port_num(1),
	_bus(0)
{ }


/**
 */
Bank::~Bank(void) {
}


/**
 * The full covering all page one memory.
 */
Bank Bank::full("DRAM", 0, 0);


/**
 * @typedef Bank::type_t
 * This type represents the type of banks:
 * @li Bank::NONE -- null type,
 * @li Bank::DRAM -- dynamic RAM,
 * @li Bank::SPM -- ScratchPad memory,
 * @li Bank::ROM -- Read-Only memory,
 * @li Bank::IO -- input/output register area.
 */


/**
 * @fn Bank::Bank(void)
 *default constructor for a bank:
 * @li no name,
 * @li size to 0 bytes,
 * @li no type,
 * @li latency to one cycle,
 * @li no power consumed,
 * @li 0 block bits,
 * @li cached,
 * @li on chip,
 * @li writable,
 * @li 1 port number,
 * @li no tied bus.
 */


/**
 * Full bank, covering all address range of page 0.
 */
static Bank full("RAM", 0, 0);


/**
 * @fn const string& Bank::name() const;
 * Get the name of the bank.
 * @return Bank name.
 */


/**
 * @fn const Address& Bank::address() const;
 * Get the base address of the bank.
 * @return	Base address.
 */


/**
 * @fn const int Bank::size() const;
 * Get the size of the bank.
 * @return	Bank size (in bytes).
 */


/**
 * @fn type_t Bank::type() const;
 * Get the type of the bank.
 * @return	Bank type.
 */


/**
 * @fn int Bank::latency() const;
 * Get the default latency for accessing this bank.
 * @return	Default latency (in cycle).
 */


/**
 * @fn int Bank::power() const;
 * Get the default power for accessing this bank.
 * @return	Default power.
 */


/**
 * @fn int Bank::blockBits() const;
 * Get the number of bits in the block size of the bank.
 * @return	Default bits per block (0 if there is no blocking).
 */


/**
 * @fn int Bank::blockSize() const;
 * Get the block size of this bank.
 * @return	Block size (in bytes, 0 if the bank is not blocked).
 */


/**
 * @fn const Table<const Mode *>& Bank::modes();
 * Get the mode transition table of the bank. The first mode is the default mode.
 * @return	Mode transition table.
 */


/**
 * @fn bool Bank::isCached() const;
 * Test if the bank is cached.
 * @return	True if the bank is cached, false else.
 */


/**
 * @fn bool Bank::isOnChip() const;
 * Test if the bank is on the same chip than the processor.
 * @return	True if it is on the same chip, false else.
 */


/**
 * @fn bool Bank::isWritable() const;
 * Test if the bank is writable.
 * @return	True if the bank is writable, false else.
 */


/**
 * @fn int Bank::portNum() const;
 * Get the number of port on the bank bus.
 * @return	Number of ports.
 */

/**
 * @fn const Bus *Bank::bus() const;
 * Get the bus of the bank.
 * @return	Bank bus.
 */


/**
 * @fn Address Bank::topAddress() const;
 * Get the top successor address of the bank, that is, bank address + bank size.
 * @return	Bank top address.
 */


/**
 * @fn bool Bank::contains(Address addr) const;
 * Test if the address is contained in the bank.
 * @return	True if the address is in the bank, false else.
 */


/**
 * @fn MemArea Bank::area() const;
 * Get the area covered by this memory bank.
 * @return	Bank memory area.
 */

/**
 * @class Memory
 * Class to represent the whole memory of the platform.
 * @author H. Cassé <casse@irit.fr>
 * @ingroup hard
 */


/**
 * @fn const Table<const Bank *>& Memory::banks(void) const;
 * Get the list of banks in memory.
 * @return	List of memory banks.
 */


/**
 * @fn const Table<const Bus *>& Memory::buses(void) const;
 * Get the list of buses connected to memory banks.
 * @return	List of memory buses.
 */


/**
 * Default null memory.
 */
const Memory Memory::null;


/**
 * Full memory: all memory is occupied by big RAM bank.
 */
const Memory Memory::full(true);


/**
 * Get the bank matching the given address.
 * @param address	Address to find bank for.
 * @return			Found bank or null.
 */
const Bank *Memory::get(Address address) const {
	for(int i = 0; i < _banks.count(); i++)
		if(_banks[i]->contains(address))
			return _banks[i];
	return 0;
}


/**
 * Load a memory configuration from the given element.
 * @param element	Element to load from.
 * @return			Built cache configuration.
 */
Memory *Memory::load(elm::xom::Element *element) {
	elm::serial2::XOMUnserializer unserializer(element);
	Memory *conf = new Memory();
	try {
		unserializer >> *conf;
		return conf;
	}
	catch(elm::Exception& exn) {
		delete conf;
		throw LoadException(_ << "cannot load the memory configuration:" << exn.message());
	}
	return 0;
}


/**
 * Load a memory configuration from an XML file.
 * @param path	Path to the file.
 * @return		Built cache configuration.
 */
Memory *Memory::load(const elm::sys::Path& path) {
	elm::serial2::XOMUnserializer unserializer(path);
	Memory *conf = new Memory();
	try {
		unserializer >> *conf;
		return conf;
	}
	catch(elm::Exception& exn) {
		delete conf;
		throw LoadException(_ << "cannot load the memory configuration from \"" << path << "\": " << exn.message());
	}
	return 0;
}


/**
 * Memory constructor.
 */
Memory::Memory(bool full):
	_waccess(0),
	_wread(0),
	_wwrite(0),
	_baccess(0),
	_bread(0),
	_bwrite(0)
{
	if(full) {
		_banks = AllocArray<const Bank *>(1);
		_banks[0] = &Bank::full;
	}
}


/**
 */
Memory::~Memory(void) {
}

/**
 * Compute the worst access latency.
 * @return Worst access latency.
 */
ot::time Memory::worstAccessTime(void) const {
	if(_waccess == 0)
		_waccess = max(worstReadAccess(), worstWriteAccess());
	return _waccess;
}


/**
 * Compute the worst read access latency.
 * @return Worst read access latency.
 */
ot::time Memory::worstReadTime(void) const {
	if(_wread == 0)
		for(int i = 0; i < _banks.count(); i++)
			_wread = max(_wread, _banks[i]->latency());
	return _wread;
}


/**
 * Compute the worst read access latency.
 * @return Worst read access latency.
 */
ot::time Memory::worstWriteTime(void) const {
	if(_wwrite == 0)
		for(int i = 0; i < _banks.count(); i++)
			_wwrite = max(_wwrite, _banks[i]->writeLatency());
	return _wwrite;
}

/**
 * Compute the best access latency.
 * @return Worst access latency.
 */
ot::time Memory::bestAccessTime(void) const {
	if(_baccess == 0)
		_baccess = min(bestReadTime(), bestWriteTime());
	return _baccess;
}


/**
 * Compute the best read access latency.
 * @return Worst read access latency.
 */
ot::time Memory::bestReadTime(void) const {
	if(_bread == 0) {
		_bread = worstReadTime();
		for(int i = 0; i < _banks.count(); i++)
			_wread = min(_bread, _banks[i]->latency());
	}
	return _bread;
}


/**
 * Compute the best read access latency.
 * @return Worst read access latency.
 */
ot::time Memory::bestWriteTime(void) const {
	if(_bwrite == 0) {
		_bwrite = bestWriteTime();
		for(int i = 0; i < _banks.count(); i++)
			_bwrite = min(_bwrite, _banks[i]->writeLatency());
	}
	return _bwrite;
}

/**
 * Compute the read time for the given address. If a bank is found,
 * return the read latency of the bank, else the worst read time
 * is returned.
 * @param a		Read address.
 * @return		Worst-case read access time.
 */
ot::time Memory::readTime(Address a) const {
	const Bank *b = get(a);
	if(!b)
		return worstReadTime();
	else
		return b->readLatency();
}

/**
 * Compute the write time for the given address. If a bank is found,
 * return the write latency of the bank, else the worst write time
 * is returned.
 * @param a		Written address.
 * @return		Worst-case write access time.
 */
ot::time Memory::writeTime(Address a) const {
	const Bank *b = get(a);
	if(!b)
		return worstWriteTime();
	else
		return b->writeLatency();
}

/**
 * Compute the access time for the given address. If a bank is found,
 * return the access latency of the bank, else the worst access time
 * is returned. The access time is the maximum of read and write time.
 * @param a		Read address.
 * @return		Worst-case access time.
 */
ot::time Memory::accessTime(Address a) const {
	const Bank *b = get(a);
	if(!b)
		return worstAccessTime();
	else
		return max(b->readLatency(), b->writeLatency());
}


/**
 * This processor attempts to get a memory description.
 * It looks the following item from its configuration (in the given order):
 * @li @ref MEMORY_OBJECT
 * @li @ref MEMORY_ELEMENT (and unserialize from the given XML element)
 * @li @trg MEMORY_PATH (and download from the given path).
 */
class MemoryProcessor: public otawa::Processor {
public:
	static p::declare reg;
	MemoryProcessor(p::declare &r = reg): Processor(r), mem(nullptr), xml(nullptr), to_free(false) { }

	void configure(const PropList& props) override {
		Processor::configure(props);
		mem = MEMORY_OBJECT(props);
		if(!mem) {
			xml = MEMORY_ELEMENT(props);
			if(!xml)
				path = MEMORY_PATH(props);
		}
	}

	void *interfaceFor(const AbstractFeature& feature) override {
		return const_cast<Memory *>(mem);
	}

	void destroy(WorkSpace *ws) {
		if(to_free)
			delete mem;
	}

protected:

	void processWorkSpace(WorkSpace *ws) override {

		// find the memory configuration
		if(mem != nullptr) {
			if(logFor(LOG_DEPS))
				log << "\tcustom memory configuration\n";
		}
		else if(xml != nullptr) {
			mem = Memory::load(xml);
			to_free = true;
			if(logFor(LOG_DEPS))
				log << "\tmemory configuration from XML element\n";
		}
		else if(path) {
			if(logFor(LOG_DEPS))
				log << "\tmemory configuration from \"" << path << "\"\n";
			mem = Memory::load(path);
			to_free = true;
		}
		else {
			if(logFor(LOG_DEPS))
				log << "\tno memory configuration\n";
			mem = &Memory::full;
			to_free = false;
		}

		// verbose display
		if(isVerbose() && mem != nullptr)
			for(auto b: mem->banks())
			//for(int i = 0; i < mem->banks().count(); i++)
				log << "\t\t" << b->name() << "\t"
					<< b->address() << "-"
					<< (b->topAddress() - 1)
					<< io::endl;
	}

private:
	const Memory *mem;
	xom::Element *xml;
	Path path;
	bool to_free;
};


/**
 */
Bus::~Bus(void) {
}


/**
 */
Mode::~Mode(void) {
}


/**
 */
ModeTransition::~ModeTransition(void) {
}


/**
 */
p::declare MemoryProcessor::reg = p::init("otawa::MemoryProcessor", Version(1, 0, 0))
	.provide(MEMORY_FEATURE)
	.maker<MemoryProcessor>();


/**
 * This feature ensures we have obtained the memory configuration
 * of the system. This feature provides an interface that can be obtained with:
 * <code c++>
 * const hard::Memory *mem = hard::MEMOR_FEATURE.get(workspace);
 * </code>
 */
p::interfaced_feature<const Memory> MEMORY_FEATURE("otawa::hard::MEMORY_FEATURE", p::make<MemoryProcessor>());

} // hard

io::Output &operator <<(io::Output &o, const hard::Bank::type_t &t) {
	static cstring names[] = {
		"NONE",
		"DRAM",
		"SPM",
		"ROM",
		"IO"
	};
	o << names[t];
	return o;
}

} // otawa

SERIALIZE(otawa::hard::ModeTransition);
SERIALIZE(otawa::hard::Mode);
SERIALIZE(otawa::hard::Bus);
SERIALIZE(otawa::hard::Bank);
SERIALIZE(otawa::hard::Memory);

ENUM_BEGIN(otawa::hard::Bus::type_t)
	VALUE(otawa::hard::Bus::LOCAL),
	VALUE(otawa::hard::Bus::SHARED)
ENUM_END

ENUM_BEGIN(otawa::hard::Bank::type_t)
		VALUE(otawa::hard::Bank::NONE),
		VALUE(otawa::hard::Bank::DRAM),
		VALUE(otawa::hard::Bank::SPM),
		VALUE(otawa::hard::Bank::ROM),
		VALUE(otawa::hard::Bank::IO)
ENUM_END

namespace elm { namespace serial2 {
	class AddressType: public rtti::Type, public rtti::Serializable {
	public:
		AddressType(void): rtti::Type("otawa::Address") { }
		virtual const Type& type(void) const { return *this; }
		virtual void *instantiate(void) const { return new otawa::Address(); }
		virtual void serialize(serial2::Serializer& ser, const void *data) const;
		virtual void unserialize(serial2::Unserializer& uns, void *data) const;
	};
	static AddressType _class;

	void __serialize(elm::serial2::Serializer &serializer, const otawa::Address& address) {
		serializer.beginObject(_class, &address);
		otawa::Address::page_t page = address.page();
		otawa::Address::offset_t offset = address.offset();
		serializer & field("page", page) & field("offset", offset);
		serializer.endObject(_class, &address);
	}

	void __unserialize(serial2::Unserializer &serializer, otawa::Address& address) {
		serializer.beginObject(_class, &address);
		otawa::Address::page_t page = 0;
		otawa::Address::offset_t offset = 0;
		serializer & field("page", page) & field("offset", offset);
		serializer.endObject(_class, &address);
		address = otawa::Address(page, offset);
	}

	void AddressType::serialize(serial2::Serializer& ser, const void *data) const {
		__serialize(ser, data);
	}

	void AddressType::unserialize(serial2::Unserializer& uns, void *data) const {
		__unserialize(uns, data);
	}

} }
