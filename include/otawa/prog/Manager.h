/*
 *	$Id$
 *	Manager class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-09, IRIT UPS.
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
#ifndef OTAWA_MANAGER_H
#define OTAWA_MANAGER_H

#include <elm/sys/Path.h>
#include <elm/data/Vector.h>
#include <elm/util/MessageException.h>
#include <elm/xom.h>
#include <elm/sys/Plugger.h>
#include <otawa/base.h>
#include "../prop.h"

namespace otawa {

using namespace elm;

// Classes
class File;
class Loader;
class Manager;
class WorkSpace;
namespace hard {
	class CacheConfiguration;
	class Memory;
	class Platform;
	class Processor;
}
namespace ilp { class System; }
namespace sim { class Simulator; }

// LoadException class
class LoadException: public otawa::Exception {
public:
	LoadException(const elm::String& message);
};

// Manager class
class Manager {
	friend class WorkSpace;
public:
	static rtti::Type& __type;
	static const cstring
		OTAWA_NS,
		OTAWA_NAME,
		PROCESSOR_NAME,
		CACHE_CONFIG_NAME,
		MEMORY_NAME,
		COMPILATION_DATE,
		VERSION;
	static CString copyright;

	static elm::sys::Path prefixPath(void);
	static String buildPaths(cstring kind, string paths = "");
	static inline Manager *def(void) { return &_def; }

	Manager(void);
	~Manager(void);
	Loader *findLoader(string name);
	WorkSpace *load(const elm::sys::Path& path, const PropList& props = PropList::EMPTY);
	ilp::System *newILPSystem(String plugin = "", bool max = true);

	// deprecated
	sim::Simulator *findSimulator(string name);
	WorkSpace *load(const PropList& props = PropList::EMPTY);
	WorkSpace *load(xom::Element *elem, const PropList& props = PropList::EMPTY);
	elm::sys::Path retrieveConfig(const elm::sys::Path& path);
	Loader *findFileLoader(const elm::sys::Path& path);

private:
	static Manager _def;

	WorkSpace *loadBin(const elm::sys::Path& path, const PropList& props);
	WorkSpace *loadXML(const elm::sys::Path& path, const PropList& props);

	Vector<hard::Platform *> platforms;
	elm::sys::Plugger ilp_plugger;
	elm::sys::Plugger loader_plugger;
	elm::sys::Plugger sim_plugger;
	bool isVerbose(void);
	void setVerbosity(const PropList& props);
	void resetVerbosity(void);
	int verbose;
};

// Configuration Properties
extern p::id<string> TASK_ENTRY;
extern p::id<Address> TASK_ADDRESS;
extern p::id<hard::Platform *> PLATFORM;
extern p::id<Loader *> LOADER;
extern p::id<string> PLATFORM_NAME;
extern p::id<string>  LOADER_NAME;
extern p::id<int> ARGC;
extern p::id<char **> ARGV;
extern p::id<char **> ENVP;
extern p::id<sim::Simulator *> SIMULATOR;
extern p::id<elm::string> SIMULATOR_NAME;
extern p::id<int> PIPELINE_DEPTH;
extern p::id<bool> NO_SYSTEM;
extern p::id<bool> NO_STACK;
extern p::id<string> NO_RETURN_FUNCTION;

extern p::id<elm::sys::Path> CONFIG_PATH;
extern p::id<elm::xom::Element *> CONFIG_ELEMENT;

extern p::id<elm::sys::Path> CACHE_CONFIG_PATH;
extern p::id<elm::xom::Element *> CACHE_CONFIG_ELEMENT;
extern p::id<hard::CacheConfiguration *> CACHE_CONFIG;

extern p::id<elm::sys::Path> MEMORY_PATH;
extern p::id<elm::xom::Element *> MEMORY_ELEMENT;
extern p::id<hard::Memory *> MEMORY_OBJECT;

extern p::id<elm::sys::Path> PROCESSOR_PATH;
extern p::id<elm::xom::Element *> PROCESSOR_ELEMENT;
extern p::id<hard::Processor *> PROCESSOR;

extern p::id<string> LOAD_PARAM;

extern Manager& MANAGER;

} // otawa

#endif	// OTAWA_MANAGER_H
