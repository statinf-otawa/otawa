/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/hardware_Platform.cpp -- Platform class implementation.
 */

#include <otawa/hardware/Platform.h>
#include <otawa/hardware/CacheConfiguration.h>

namespace otawa {

/**
 * @class Platform Platform.h "otawa/hardware/Platform.h"
 * This class records information about the architecture where the processed
 * program will run.
 */


/**
 * Empty register bank table.
 */
const elm::genstruct::Table<hard::RegBank *> Platform::null_banks(0, 0);


/**
 */
void Platform::configure(const PropList& props) {
	for(PropList::PropIter prop(props); prop; prop++) {
		if(prop == ID_Cache)
			_cache = prop.get<const CacheConfiguration *>();
		else if(prop == ID_PipelineDepth)
			depth = prop.get<int>();
	}
}


/**
 * Build a platform from the given description.
 * @param _id		Platform identification.
 * @param props		Properties describing the platform.
 */
Platform::Platform(const Platform::Identification& _id, const PropList& props)
: id(_id), _cache(&CacheConfiguration::NO_CACHE), depth(5), _banks(&null_banks) {
	configure(props);
}


/**
 * Build a platform by cloning and reconfiguring the new platform.
 * @param platform	Platform to clone.
 * @param props		Description properties.
 */
Platform::Platform(const Platform& platform, const PropList& props)
: id(platform.identification()), _cache(&platform.cache()), depth(5),
_banks(&null_banks) {
	configure(props);
}


/**
 * Tag for setting the cache of the platform. Takes a "const
 * CacheConfiguration *" agument.
 */
Identifier Platform::ID_Cache("Platform::Cache");


/**
 * Tag for setting the cache of the platform. Takes a "const
 * CacheConfiguration *" agument.
 */
Identifier Platform::ID_PipelineDepth("Platform::PipelineDepth");


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

} // otawa
