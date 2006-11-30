/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/hard/Platform.h -- Platform class interface.
 */
#ifndef OTAWA_HARD_PLATFORM_H
#define OTAWA_HARD_PLATFORM_H

#include <elm/system/Path.h>
#include <otawa/properties.h>
#include <otawa/hard/Register.h>
#include <elm/datastruct/Collection.h>

namespace otawa {

class Manager;
	
namespace hard {
	
// Extern Classes
class CacheConfiguration;
class Processor;

// Platform class
class Platform {
public:

	static const elm::String ANY;

	// Architecture
	static const elm::String POWERPC;
	
	// ABI
	static const elm::String ELF;
	static const elm::String EABI;
	static const elm::String LINUX;
	static const elm::String LINUX_2_4;
	static const elm::String LINUX_2_6;
	
	// Platform
	static const elm::String MAC;
	static const elm::String SIM;

	// Platform identification
	class Identification {
		elm::String _name;
		elm::String _arch;
		elm::String _abi;
		elm::String _mach;
		void split(void);
	public:
		Identification(const elm::String& name);
		Identification(const elm::String& arch, const elm::String& abi,
			const elm::String& mach = ANY);
		inline const elm::String& name(void) const;
		inline const elm::String& architecture(void) const;
		inline const elm::String& abi(void) const;
		inline const elm::String& machine(void) const;
		bool matches(const Identification& id);
		Identification& operator=(const Identification& id);
	};

	// Platform
	static const Identification ANY_PLATFORM;
	
private:
	static const unsigned long HAS_PROCESSOR = 0x00000001;
	static const unsigned long HAS_CACHE = 0x00000002;
	unsigned long flags;
	Identification id;
	const CacheConfiguration *_cache;
	Processor *_processor;
	int depth;

	void configure(const PropList& props);

protected:
	friend class otawa::Manager;
	static const elm::genstruct::Table<const hard::RegBank *> null_banks;
	const elm::genstruct::Table<const hard::RegBank *> *_banks;
	virtual ~Platform(void);

public:
	Platform(const Identification& id, const PropList& props = PropList::EMPTY);
	Platform(const Platform& platform, const PropList& props = PropList::EMPTY);

	// Cache information
	inline const Identification& identification(void) const;
	inline const CacheConfiguration& cache(void) const;
	inline const int pipelineDepth(void) const;
	void loadCacheConfig(const elm::system::Path& path);
	void loadCacheConfig(elm::xom::Element *element);

	// Compatibility test
	virtual bool accept(const Identification& id);
	inline bool accept(elm::CString name);
	inline bool accept(const elm::String& name);
	
	// Register bank access
	inline const elm::genstruct::Table<const hard::RegBank *>& banks(void) const;
	
	// Configuration Loader
	void loadProcessor(const elm::system::Path& path);
	void loadProcessor(elm::xom::Element *element);
	inline const Processor *processor(void) const { return _processor; };
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

inline const elm::genstruct::Table<const hard::RegBank *>& Platform::banks(void) const {
	return *_banks;
}


// Platform::Identification inlines
inline const elm::String& Platform::Identification::architecture(void) const {
	return _arch;
}

inline const elm::String& Platform::Identification::abi(void) const {
	return _abi;
}

inline const elm::String& Platform::Identification::machine(void) const {
	return _mach;
}

inline const elm::String& Platform::Identification::name(void) const {
	return _name;
}

} } // otawa::hard

#endif // OTAWA_HARD_PLATFORM_H
