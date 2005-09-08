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
 * @class Platform
 * This class records information about the architecture where the processed
 * program will run.
 */


/**
 */
void Platform::configure(const PropList& props) {
	for(PropList::PropIter prop(props); prop; prop++) {
		if(prop == ID_Cache)
			_cache = prop.get<const CacheConfiguration *>();
	}
}


/**
 * Build a platform from the given description.
 * @param _id		Platform identification.
 * @param props		Properties describing the platform.
 */
Platform::Platform(const Platform::Identification& _id, const PropList& props)
: id(_id), _cache(&CacheConfiguration::NO_CACHE) {
	configure(props);
}


/**
 * Build a platform by cloning and reconfiguring the new platform.
 * @param platform	Platform to clone.
 * @param props		Description properties.
 */
Platform::Platform(const Platform& platform, const PropList& props)
: id(platform.identification()), _cache(&platform.cache()) {
	configure(props);
}


/**
 * Tag for setting the cache of the platform. Takes a "const
 * CacheConfiguration *" agument.
 */
Identifier Platform::ID_Cache("Platform::Cache");


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
 * @class Platform::Identification
 * <p>This class represents a platform identification that is composed by:</p>
 * @li architecture: processor family like 'x86', 'powerpc', 'arm', ...
 * @li ABI or Application Binary Interface that defines the encoding of binary
 * and system call protocol like 'elf' or 'coff'.
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
	int start = 0, pos = name.indexOf('-');
	arch = name.substring(0, pos >= 0 ? pos : name.length());
	
	// Find the ABI
	if(pos >= 0) {
		start = pos + 1;
		pos = name.indexOf('-', start);
		_abi = name.substring(start, pos >= 0 ? pos - start : name.length() - start);
	}
	
	// Find the machine
	if(pos >= 0) {
		start = pos + 1;
		pos = name.indexOf('-', start);
		mach = name.substring(start, pos >= 0 ? pos - start : name.length() - start);
	}
}


/**
 * Build a platform identifier from a string.
 * @param _name	Name of the platform.
 */
Platform::Identification::Identification(const String _name): name(_name) {
	split();
}

/**
 * Build an explicit platform identifier.
 * @param _arch		Archietcture.
 * @param _abi		ABI.
 * @param _mach	Machine.
 */
Platform::Identification::Identification(CString _arch, CString _abi, CString _mach)
: name(_arch + "-" + _abi + "-" + _mach) {
	split();
}

/**
 * @fn const String& PlatformId::architecture(void) const;
 * Get the architecture component of the platform identifier.
 * @return Architecture component.
 */

/**
 * @fn const String& PlatformId::abi(void) const;
 * Get the ABI component of the platform identifier.
 * @return ABI component.
 */

/**
 * @fn const String& PlatformId::machine(void) const;
 * Get the machine component of the platform identifier.
 * @return Machine component.
 */

} // otawa
