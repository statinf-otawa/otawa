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
	Loader *findLoader(elm::CString name);
	WorkSpace *load(const elm::sys::Path& path, const PropList& props = PropList::EMPTY);
	ilp::System *newILPSystem(String plugin = "", bool max = true);

	// deprecated
	sim::Simulator *findSimulator(elm::CString name);
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
extern Identifier<string> TASK_ENTRY;
extern Identifier<Address> TASK_ADDRESS;
extern Identifier<hard::Platform *> PLATFORM;
extern Identifier<Loader *> LOADER;
extern Identifier<elm::CString> PLATFORM_NAME;
extern Identifier<string>  LOADER_NAME;
extern Identifier<int> ARGC;
extern Identifier<char **> ARGV;
extern Identifier<char **> ENVP;
extern Identifier<sim::Simulator *> SIMULATOR;
extern Identifier<elm::CString> SIMULATOR_NAME;
extern Identifier<int> PIPELINE_DEPTH;
extern Identifier<bool> NO_SYSTEM;
extern Identifier<bool> NO_STACK;
extern Identifier<string> NO_RETURN_FUNCTION;

extern Identifier<elm::sys::Path> CONFIG_PATH;
extern Identifier<elm::xom::Element *> CONFIG_ELEMENT;

extern Identifier<elm::sys::Path> CACHE_CONFIG_PATH;
extern Identifier<elm::xom::Element *> CACHE_CONFIG_ELEMENT;
extern Identifier<hard::CacheConfiguration *> CACHE_CONFIG;

extern Identifier<elm::sys::Path> MEMORY_PATH;
extern Identifier<elm::xom::Element *> MEMORY_ELEMENT;
extern Identifier<hard::Memory *> MEMORY_OBJECT;

extern Identifier<elm::sys::Path> PROCESSOR_PATH;
extern Identifier<elm::xom::Element *> PROCESSOR_ELEMENT;
extern Identifier<hard::Processor *> PROCESSOR;

extern Identifier<string> LOAD_PARAM;

extern Manager& MANAGER;

} // otawa

#endif	// OTAWA_MANAGER_H
