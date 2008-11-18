/*
 *	$Id$
 *	Manager class interface
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
#ifndef OTAWA_MANAGER_H
#define OTAWA_MANAGER_H

#include <elm/system/Path.h>
#include <elm/genstruct/Vector.h>
#include <elm/util/MessageException.h>
#include <elm/system/Plugger.h>
#include <otawa/properties.h>
#include <otawa/base.h>
#include <otawa/program.h>
//#include <otawa/cfg.h>
#include <otawa/prog/WorkSpace.h>
//#include <otawa/prog/Loader.h>

namespace otawa {

using namespace elm;

// Classes
class File;
class Loader;
class Manager;
namespace ilp {
	class System;
}
namespace sim {
	class Simulator;
}
namespace hard {
	class Memory;
}

// LoadException class
class LoadException: public MessageException {
public:
	LoadException(const elm::String& message);
};

// Manager class
class Manager {
	friend class WorkSpace;
	genstruct::Vector<hard::Platform *> platforms;
	//datastruct::Vector<WorkSpace *> frameworks;
	elm::system::Plugger ilp_plugger;
	elm::system::Plugger loader_plugger;
	elm::system::Plugger sim_plugger;
	WorkSpace *loadBin(const elm::system::Path& path, const PropList& props);
	WorkSpace *loadXML(const elm::system::Path& path, const PropList& props);
public:
	static const CString OTAWA_NS;
	static const CString OTAWA_NAME;
	static const CString PROCESSOR_NAME;
	static const CString CACHE_CONFIG_NAME;
	static const CString MEMORY_NAME;

	Manager(void);
	~Manager(void);
	Loader *findLoader(elm::CString name);
	sim::Simulator *findSimulator(elm::CString name);
	WorkSpace *load(const elm::system::Path& path,
		const PropList& props = PropList::EMPTY);
	WorkSpace *load(const PropList& props = PropList::EMPTY);
	WorkSpace *load(xom::Element *elem,
		const PropList& props = PropList::EMPTY); 
	ilp::System *newILPSystem(String plugin = "");
	
};

// Configuration Properties
extern Identifier<CString> TASK_ENTRY;
extern Identifier<hard::Platform *> PLATFORM;
extern Identifier<Loader *> LOADER;
extern Identifier<elm::CString> PLATFORM_NAME;
extern Identifier<elm::CString>  LOADER_NAME;
extern Identifier<int> ARGC;
extern Identifier<char **> ARGV;
extern Identifier<char **> ENVP;
extern Identifier<sim::Simulator *> SIMULATOR;
extern Identifier<elm::CString> SIMULATOR_NAME;
extern Identifier<int> PIPELINE_DEPTH;
extern Identifier<bool> NO_SYSTEM;
extern Identifier<bool> NO_STACK;
extern Identifier<string> NO_RETURN_FUNCTION;

extern Identifier<elm::system::Path> CONFIG_PATH;
extern Identifier<elm::xom::Element *> CONFIG_ELEMENT;

extern Identifier<elm::system::Path> CACHE_CONFIG_PATH;
extern Identifier<elm::xom::Element *> CACHE_CONFIG_ELEMENT;
extern Identifier<hard::CacheConfiguration *> CACHE_CONFIG;

extern Identifier<elm::system::Path> MEMORY_PATH;
extern Identifier<elm::xom::Element *> MEMORY_ELEMENT;
extern Identifier<hard::Memory *> MEMORY_OBJECT;

extern Identifier<elm::system::Path> PROCESSOR_PATH;
extern Identifier<elm::xom::Element *> PROCESSOR_ELEMENT;
extern Identifier<hard::Processor *> PROCESSOR;

} // otawa

#endif	// OTAWA_MANAGER_H
