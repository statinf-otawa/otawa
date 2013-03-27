/*
 *	$Id$
 *	Platform class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-10, IRIT UPS.
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

#include <otawa/hard/Platform.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/hard/Processor.h>
#include <elm/serial2/XOMUnserializer.h>
#include <elm/genstruct/Table.h>
#include <otawa/prog/Manager.h>
#include <otawa/hard/Memory.h>
#include <elm/xom.h>

namespace otawa { namespace hard {

/**
 * @defgroup hard Platform Description
 * 
 * This module provides information about the host platform where the loaded program
 * will run on. It includes descriptions for:
 * @li processor registers,
 * @li processor pipeline,
 * @li cache hierarchy,
 * @li memory spaces.
 *
 * The ISA (Instruction Set architecture) is provided by the executable loader.
 * It mainly includes register description and some other description (length of instructions
 * and so on).
 *
 * Other hardware items like processor, caches and memory are loaded by their own processor
 * and configuration is passed as usual in the configuration property list passed to run
 * a code processor. These configuration properties may be:
 * @li @ref otawa::CACHE_CONFIG -- cache configuration
 * @li @ref otawa::CACHE_CONFIG_ELEMENT -- cache configuration as an XML element
 * @li @ref otawa::CACHE_CONFIG_PATH -- cache configuration contained in a file
 * @li @ref otawa::MEMORY_OBJECT -- memory configuration
 * @li @ref otawa::MEMORY_ELEMENT -- memory configuration as an XML element
 * @li @ref otawa::MEMORY_PATH -- memory configuration contained in a file
 * @li @ref otawa::PROCESSOR -- processor configuration
 * @li @ref otawa::PROCESSOR_ELEMENT -- processor configuration as an XML element
 * @li @ ref otawa::PROCESSOR_PATH -- processor configuration contained in a file
 *
 * A code processor may get these hardware items from the workspace if it requires
 * the corresponding feature:
 * @li @ref otawa::hard::PROCESSOR_FEATURE
 * @li @ref otawa::hard::CACHE_CONFIGURATION_FEATURE
 * @li @ref otawa::hard::MEMORY_FEATURE
 *
 * @par Processor Format
 *
 * We describe here the format of processor configuration expressed in XML.
 * This file is unserialized according the @ref elm::serial2 module and therefore
 * supports all its features. The XML format notation is detailed in @ref hard_format .
 *
 * @code
 * <!-- PROCESSOR ::= -->
 *	<?xml version="1.0" encoding="UTF-8"?>
 *	<processor class="otawa::hard::Processor">
 *		<arch><!-- ISA name --></arch>
 *		<model><!-- processor model --></model>
 *		<builder><!-- processor builder --></builder>
 *
 *		<stages> <!-- STAGE* --> </stages>
 *
 *		<queues> <!-- QUEUE* --> </queues>
 *	<processor>
 * @endcode
 *
 * A processor is made of several stages linked by queues. A stage has a type that may
 * be:
 * @li FETCH -- this stage gets instruction from the memory (it must the first in the pipeline and only one is accepted)
 * @li LAZY -- nothing interesting the analysis, just takes time
 * @li EXEC (execution) -- the instruction are executed at this point (this stage may contains functional unit and dispatcher)
 * @li COMMIT -- this is the last stage where instruction exits the pipeline
 *
 * @code
 * <!-- STAGE ::= -->
 *	<stage id="STAGE IDENTIFIER">
 *		<name><!-- name for human user --></name>
 *		<type><!-- one of FETCH, LAZY, EXEC or COMMIT (default LAZY) --></type>
 *		<width><!-- number of processed instruction per cycle (default 1) --></width>
 *
 *		<!-- only for EXEC type of stage -->
 *		<fus><!-- FU* --></fus>
 *		<dispatch><!-- INST * --></dispatch>
 *	</stage>
 * @endcode
 *
 * The "fus" contains the list of available functional units. The dispatching of
 * instructions between the functional units is given in the "dispatch" element.
 *
 * @code
 * <!-- FU ::= -->
 * 	<fu id="FU identifier">
 * 		<name><!-- name for human user --></name>
 * 		<width><!-- number of instruction processed by cycle (default 1) --></width>?
 * 		<latency><!-- number of cycle passed in the FU --></latency>?
 * 		<pipelined><!-- TRUE if the FU is pipelined, FALSE else (default) --></pipelined>?
 * 	</fu>
 * @endcode
 *
 * The "inst" allows to dispatch instruction in the FU.
 *
 * @par Cache Configuration Format
 *
 * @par Memory Description Format
 *
 */


/**
 * @class Platform Platform.h
 * @ingroup hard
 * This class records information about the architecture where the processed
 * program will run.
 */


/**
 * Empty register bank table.
 */
const elm::genstruct::Table<const hard::RegBank *> Platform::null_banks(0, 0);


/**
 */
void Platform::configure(const PropList& props) {
	
	// Configure cache
	CacheConfiguration *cache = CACHE_CONFIG(props);
	if(cache) {
		_cache = cache;
		flags &= ~HAS_CACHE;
	}
	else {
		xom::Element *element = CACHE_CONFIG_ELEMENT(props);
		if(element) 
			loadCacheConfig(element);
		else {
			elm::system::Path path = CACHE_CONFIG_PATH(props);
			if(path)
				loadCacheConfig(path);
			else {
				element = CONFIG_ELEMENT(props);
				if(element) {
					xom::Element *cache_elem = element->getFirstChildElement(
						Manager::CACHE_CONFIG_NAME, Manager::OTAWA_NS);
					if(cache_elem)
						loadCacheConfig(cache_elem);
				}
			}
		}
	}
	
	// Configure pipeline depth
	int new_depth = PIPELINE_DEPTH(props);
	if(new_depth > 0)
		depth = new_depth;
	
	// Configure processor 
	Processor *new_processor = otawa::PROCESSOR(props);
	if(new_processor) {
		_processor = new_processor;
		flags &= ~HAS_PROCESSOR;
	}
	else {
		xom::Element *element = PROCESSOR_ELEMENT(props);
		if(element) 
			loadProcessor(element);
		else {
			elm::system::Path path = PROCESSOR_PATH(props);
			if(path)
				loadProcessor(path);
			else {
				element = CONFIG_ELEMENT(props);
				if(element) {
					xom::Element *proc_elem = element->getFirstChildElement(
						Manager::PROCESSOR_NAME, Manager::OTAWA_NS);
					if(proc_elem)
						loadProcessor(proc_elem);
				}
			}
		}
	}
	
	// configure the memory
	Memory *memory = MEMORY_OBJECT(props);
	if(memory) {
		_memory = memory;
		flags &= ~HAS_MEMORY;
	}
	else {
		xom::Element *element = MEMORY_ELEMENT(props);
		if(element) 
			loadMemory(element);
		else {
			elm::system::Path path = MEMORY_PATH(props);
			if(path)
				loadMemory(path);
			else {
				element = CONFIG_ELEMENT(props);
				if(element) {
					xom::Element *memory_elem = element->getFirstChildElement(
						Manager::MEMORY_NAME, Manager::OTAWA_NS);
					if(memory_elem)
						loadMemory(memory_elem);
				}
			}
		}
	}
}


/**
 * Build a platform from the given description.
 * @param _id		Platform identification.
 * @param props		Properties describing the platform.
 */
Platform::Platform(const Platform::Identification& _id, const PropList& props)
:	flags(0),
	id(_id),
	_cache(&CacheConfiguration::NO_CACHE),
	_processor(0),
	_memory(&Memory::full),
	depth(5),
	rcnt(0),
	_banks(&null_banks)
{
	configure(props);
}


/**
 * Build a platform by cloning and reconfiguring the new platform.
 * @param platform	Platform to clone.
 * @param props		Description properties.
 */
Platform::Platform(const Platform& platform, const PropList& props)
:	flags(0),
	id(platform.identification()),
	_cache(&platform.cache()),
	_processor(0),
	_memory(&platform.memory()),
	depth(5),
	rcnt(0),
	_banks(&null_banks)
{
	configure(props);
}


/**
 */
Platform::~Platform(void) {
	if(flags & HAS_PROCESSOR)
		delete _processor;
	if(flags & HAS_CACHE)
		delete _cache;
	if(flags & HAS_MEMORY)
		delete _memory;
}


/**
 * @fn const Identification& Platform::identification(void) const
 * Get identification of the current platform.
 * @return	Platform identification.
 */


/**
 * Check if the platform handle the given platform description.
 * @param _id	Platform identifier to check.
 * @return		True if the platform accepts this kind of architecture.
 */
bool Platform::accept(const Identification& _id) {
	return id.architecture() == _id.architecture()
		&& id.abi() == _id.abi()
		&& id.machine() == _id.machine();
}


/**
 * @fn bool Platform::accept(CString name);
 * Check if the platform accepts the given kind of architecture.
 * @param name of the acrhitecture.
 * @result True if the platform accepts the given architecture.
 */


/**
 * @fn bool Platform::accept(const String& name);
 * Check if the platform accepts the given kind of architecture.
 * @param name of the acrhitecture.
 * @result True if the platform accepts the given architecture.
 */


/**
 * @fn const CacheConfiguration& Platform::cache(void) const
 * Get the cache configuration.
 * @return Cache configuration.
 */


/**
 * @fn const int Platform::pipelineDepth(void) const;
 * Get the depth of the pipeline of the platform processor.
 * @return Pipeline depth.
 */


/**
 * Represents any architecture, platform, machine.
 */
const elm::String Platform::ANY("*");


/**
 * Represents the PowerPC architecture.
 */
const elm::String Platform::POWERPC("powerpc");


/**
 * Represents the ELF ABI.
 */	
const elm::String Platform::ELF("elf");


/**
 * Represents the EABI ABI.
 */
const elm::String Platform::EABI("eabi");


/**
 * Represents the Linux ABI.
 */
const elm::String Platform::LINUX("linux");


/**
 * Represents the Linux 2.4 ABI.
 */
const elm::String Platform::LINUX_2_4("linux/2.4");


/**
 * Represents the Linux 2.6 ABI.
 */
const elm::String Platform::LINUX_2_6("linux/2.5");


/**
 * Represents a Macintosh machine (seems to be too wide, must be refined).
 */
const elm::String Platform::MAC("mac");


/**
 * Represents a simulator machine.
 */
const elm::String Platform::SIM("sim");


/**
 * Represents any platform.
 */
const Platform::Identification Platform::ANY_PLATFORM(ANY);


/**
 * @class Platform::Identification
 * @ingroup hard
 * <p>This class represents a platform identification that is composed by:</p>
 * @li architecture: processor family like 'x86', 'powerpc', 'arm', ...
 * possibly followed by the exact model : i586, arm7, ...
 * @li ABI or Application Binary Interface that defines the encoding of binary
 * and system call protocol like 'elf' or 'coff' followed by the version of the
 * running system "linux2.2", "linux2.4", ...
 * @li machine that gives information about the hardware configuration of the
 * system, for example, 'pcat' or 'sim' for simulator.
 * <p>Each component may be datailed by one or many sub-components
 * separated by '/' slashes. For example, a full description of a classic
 * executable environment on Linux may be:
 * 'x86/i586-elf/linux/libc2.4-pcat'.</p>
 */


/**
 * Split the platform name into its components.
 */
void Platform::Identification::split(void) {
	
	// Find the architecture
	int start = 0, pos = _name.indexOf('-');
	_arch = _name.substring(0, pos >= 0 ? pos : _name.length());
	
	// Find the ABI
	if(pos >= 0) {
		start = pos + 1;
		pos = _name.indexOf('-', start);
		_abi = _name.substring(start, pos >= 0 ? pos - start : _name.length() - start);
	}
	else
		_abi = ANY;
	
	// Find the machine
	if(pos >= 0) {
		start = pos + 1;
		pos = _name.indexOf('-', start);
		_mach = _name.substring(start, pos >= 0 ? pos - start : _name.length() - start);
	}
	else
		_mach = ANY;
}


/**
 * Build a platform identifier from a string.
 * @param name	Name of the platform.
 */
Platform::Identification::Identification(const String& name): _name(name) {
	split();
}


/**
 * Build an explicit platform identifier.
 * @param arch		Archietcture.
 * @param abi		ABI.
 * @param mach		Machine.
 */
Platform::Identification::Identification(const elm::String& arch,
const elm::String& abi, const elm::String& mach)
: _name(arch + "-" + abi + "-" + mach), _arch(arch), _abi(abi), _mach(mach) {
}

/**
 * @fn const elm::String& Platform::Identification::architecture(void) const;
 * Get the architecture component of the platform identifier.
 * @return Architecture component.
 */

/**
 * @fn const elm::String& Platform::Identification::abi(void) const;
 * Get the ABI component of the platform identifier.
 * @return ABI component.
 */

/**
 * @fn const elm::String& Platform::Identification::machine(void) const;
 * Get the machine component of the platform identifier.
 * @return Machine component.
 */

/**
 * @fn const elm::String& Platform::Identification::name(void) const;
 * Get the full name of the identification.
 * @return Identification name.
 */

/**
 * Copy the given identification in the current one.
 * @param id	Copied identification.
 * @return		Current identification.
 */
Platform::Identification& Platform::Identification::operator=(
const Platform::Identification& id) {
	_name = id._name;
	_arch = id._arch;
	_abi = id._abi;
	_mach = id._mach;
	return *this;
}


/**
 * Check if the current identification matches the given one.
 * Note that the "*" "any" identity is used only by the current identification.
 * Matching is also performed by prefix. For example, an identification "xxx"
 * in the current identification matches a "xxx/yyy" in the given identification.
 * @return	True if both identification matches, false else.
 */
bool Platform::Identification::matches(const Identification& id) {
	
	// Check architecture
	if(_arch != ANY) {
		if(!id._arch.startsWith(_arch)
		|| (id._arch.length() != _arch.length() && id._arch[_arch.length()] != '/'))
			return false;
	}
	
	// Check ABI
	if(_abi != ANY) {
		if(!id._abi.startsWith(_abi)
		|| (id._abi.length() != _abi.length() && id._abi[_abi.length()] != '/'))
			return false;
	}
	
	// Check machine
	if(_mach != ANY) {
		if(!id._mach.startsWith(_arch)
		|| (id._mach.length() != _mach.length() && id._mach[_mach.length()] != '/'))
			return false;
	}
	
	// All is fine
	return true;
}


/**
 * Load the processor configuration.
 * @param path	Path to the file.
 * @throws	elm::io::IOException	If a configuration file cannot be loaded.
 */
void Platform::loadProcessor(const elm::system::Path& path) {

	// free an existing one
	if(flags & HAS_PROCESSOR) {
		delete _processor;
		_processor = 0;
		flags &= ~HAS_PROCESSOR;
	}

	// load the new one
	_processor = Processor::load(path);
	flags |= HAS_PROCESSOR;
}


/**
 * Load the processor configuration from the given element.
 * @param element			Element to use.
 * @throws	LoadException	If the XML element is mal-formed.
 */
void Platform::loadProcessor(elm::xom::Element *element) {
	ASSERT(element);

	// free it already exists
	if(flags & HAS_PROCESSOR) {
		delete _processor;
		_processor = 0;
		flags &= ~HAS_PROCESSOR;
	}

	// load the new one
	_processor = Processor::load(element);
	flags |= HAS_PROCESSOR;
}


/**
 * @fn Processor *Platform::processor(void);
 * Get the current processor (possibly derivated from the current configuration).
 * @return	Current processor.
 */


/**
 * Load a cache configuration from the given path.
 * @param path			Path to the cache configuration.
 * @throws LoadException	Throws if there is an error.
 */
void Platform::loadCacheConfig(const elm::system::Path& path) {
	try {
		CacheConfiguration *new_cache = CacheConfiguration::load(path);
		if(flags & HAS_CACHE)
			delete _cache;
		flags |= HAS_CACHE;
		_cache = new_cache;
	}
	catch(elm::Exception& e) {
		throw LoadException(&e.message());
	}
}


/**
 * Load a cache configuration from an XML element.
 * @param element	Element to read from.
 * @throws LoadException	Thrown if there is an error.
 */
void Platform::loadCacheConfig(elm::xom::Element *element) {
	try {
		CacheConfiguration *new_cache = CacheConfiguration::load(element);
		if(flags & HAS_CACHE)
			delete _cache;
		flags |= HAS_CACHE;
		_cache = new_cache;
	}
	catch(elm::Exception& e) {
		throw LoadException(&e.message());
	}
}


/**
 * Load a memory configuration from the given path.
 * @param path				Path to the memory configuration.
 * @throws LoadException	Throws if there is an error.
 */
void Platform::loadMemory(const elm::system::Path& path) throw(LoadException) {
	Memory *new_memory = Memory::load(path);
	if(flags & HAS_MEMORY)
		delete _memory;
	flags |= HAS_MEMORY;
	_memory = new_memory;
}


/**
 * Load a memory configuration from an XML element.
 * @param element	Element to read from.
 * @throws LoadException	Thrown if there is an error.
 */
void Platform::loadMemory(elm::xom::Element *element) throw(LoadException) {
	Memory *new_memory = Memory::load(element);
	if(flags & HAS_MEMORY)
		delete _memory;
	flags |= HAS_MEMORY;
	_memory = new_memory;
}


/**
 * Set the list of banks for an actual platform.
 * @param banks	Table of register banks.
 * @notice	This method is only accessible from derivated class implementing
 *			an actual platform.
 */
void Platform::setBanks(const banks_t& banks) {
	_banks = &banks;
	rcnt = 0;
	for(int i = 0; i < banks.count(); i++)
		for(int j = 0; j < banks[i]->count(); j++)
			banks[i]->get(j)->pfnum = rcnt++;
}


/**
 * @fn int Platform::regCount(void) const;
 * Get the count of registers in the current platform.
 * @return	Count of registers.
 */


/**
 * Find the register matching the given unique identifier.
 * @param uniq		Unique identifier of the register.
 * @return			Found register or null.
 */
Register *Platform::findReg(int uniq) const {
	for(int i = 0; i < _banks->count(); i++)
		for(int j = 0; j < _banks->get(i)->count(); j++)
			if(_banks->get(i)->get(j)->pfnum == uniq)
				return _banks->get(i)->get(j);
	return 0;
}

/**
 * Display the identification.
 * @param out	Used output.
 */
void Platform::Identification::print(io::Output& out) const {
	out << name();
}

/**
 * Find a register by its name.
 * @param name		Name of the register to find.
 * @return			Found register or null.
 */
const Register *Platform::findReg(const string& name) const {
	for(int i = 0; i < _banks->count(); i++)
		for(int j = 0; j < _banks->get(i)->count(); j++)
			if(_banks->get(i)->get(j)->name() == name)
				return _banks->get(i)->get(j);
	return 0;
}


/**
 * Get the register, usually (depending on the ABI), devoted to contain
 * the stack pointer.
 * @return	SP register or null if none is defined.
 */
const Register *Platform::getSP(void) const {
	return 0;
}


/**
 * Get the register containing the Program Counter.
 * @return	PC register or null.
 */
const Register *Platform::getPC(void) const {
	return 0;
}

} } // otawa::hard
