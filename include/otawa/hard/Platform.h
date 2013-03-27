/*
 *	Platform class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-13, IRIT UPS.
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
#ifndef OTAWA_HARD_PLATFORM_H
#define OTAWA_HARD_PLATFORM_H

#include <elm/system/Path.h>
#include <otawa/properties.h>
#include <otawa/hard/Register.h>
#include <otawa/prog/Manager.h>

// External classes
namespace elm {
	namespace xom {
		class Element;
	}
}

namespace otawa {

class Manager;
	
namespace hard {
	
// Extern Classes
class CacheConfiguration;
class Memory;
class Processor;

// Platform class
class Platform {
public:
	typedef elm::genstruct::Table<const hard::RegBank *> banks_t;

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
		void print(io::Output& out) const;
	};

	// Constructors
	static const Identification ANY_PLATFORM;
	Platform(const Identification& id, const PropList& props = PropList::EMPTY);
	Platform(const Platform& platform, const PropList& props = PropList::EMPTY);

	// Identification
	inline const Identification& identification(void) const;
	virtual bool accept(const Identification& id);
	inline bool accept(elm::CString name);
	inline bool accept(const elm::String& name);
	
	// Register bank access
	inline const banks_t& banks(void) const;
	inline int regCount(void) const { return rcnt; }
	Register *findReg(int uniq) const;
	const Register *findReg(const string& name) const;
	virtual const Register *getSP(void) const;
	virtual const Register *getPC(void) const;
	
	// deprecated
	inline const CacheConfiguration& cache(void) const;
	void loadCacheConfig(const elm::system::Path& path);
	void loadCacheConfig(elm::xom::Element *element);
	inline const Memory& memory(void) const { return *_memory; }
	void loadMemory(const elm::system::Path& path) throw(otawa::LoadException);
	void loadMemory(elm::xom::Element *element) throw(otawa::LoadException); 
	void loadProcessor(const elm::system::Path& path);
	void loadProcessor(elm::xom::Element *element);
	inline const Processor *processor(void) const { return _processor; };
	inline const int pipelineDepth(void) const;

protected:
	friend class otawa::Manager;
	static const banks_t null_banks;
	virtual ~Platform(void);
	void setBanks(const banks_t& banks);

private:
	static const unsigned long HAS_PROCESSOR	= 0x00000001;
	static const unsigned long HAS_CACHE		= 0x00000002;
	static const unsigned long HAS_MEMORY		= 0x00000004;
	unsigned long flags;
	Identification id;
	const CacheConfiguration *_cache;
	Processor *_processor;
	const Memory *_memory;
	int depth;
	int rcnt;
	const banks_t *_banks;

	void configure(const PropList& props);
};
inline io::Output& operator<<(io::Output& out, const Platform::Identification& id) { id.print(out); return out; }

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
