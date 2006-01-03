/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/hardware/Platform.h -- Platform class interface.
 */
#ifndef OTAWA_HARDWARE_PLATFORM_H
#define OTAWA_HARDWARE_PLATFORM_H

#include <otawa/properties.h>

namespace otawa {

// External classes
class Manager;
class CacheConfiguration;

// Platform class
class Platform {
	friend class Manager;

public:

	// Platform identification
	class Identification {
		elm::String name;
		elm::String arch;
		elm::String _abi;
		elm::String mach;
		void split(void);
	public:
		Identification(const elm::String _name);
		Identification(elm::CString arch, elm::CString abi, elm::CString machine);
		inline const elm::String& architecture(void) const;
		inline const elm::String& abi(void) const;
		inline const elm::String& machine(void) const;
	};

private:
	Identification id;
	const CacheConfiguration *_cache;
	int depth;
	void configure(const PropList& props);

protected:
	virtual ~Platform(void) { };

public:
	Platform(const Identification& id, const PropList& props = PropList::EMPTY);
	Platform(const Platform& platform, const PropList& props = PropList::EMPTY);

	// Static
	static Identifier ID_Cache;
	static Identifier ID_PipelineDepth;

	// Cache information
	inline const Identification& identification(void) const;
	inline const CacheConfiguration& cache(void) const;
	inline const int pipelineDepth(void) const;

	// Compatibility test
	virtual bool accept(const Identification& id);
	inline bool accept(elm::CString name);
	inline bool accept(const elm::String& name);
};

// Inlines
inline const Platform::Identification& Platform::identification(void) const {
	return id;
}

inline const CacheConfiguration& Platform::cache(void) const {
	return *_cache;
}

inline const int Platform::pipelineDepth(void) const {
	return depth;
}

inline bool Platform::accept(elm::CString name) {
	return accept(Identification(name));
}

inline bool Platform::accept(const elm::String& name) {
	return accept(Identification(name));
}

// Platform::Identification inlines
inline const elm::String& Platform::Identification::architecture(void) const {
	return arch;
}

inline const elm::String& Platform::Identification::abi(void) const {
	return _abi;
}

inline const elm::String& Platform::Identification::machine(void) const {
	return mach;
}

} // otawa

#endif // OTAWA_HARDWARE_PLATFORM_H
