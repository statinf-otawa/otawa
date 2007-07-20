/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	manager.h -- manager classes interface.
 */
#ifndef OTAWA_MANAGER_H
#define OTAWA_MANAGER_H

#include <elm/system/Path.h>
#include <elm/datastruct/Vector.h>
#include <elm/util/MessageException.h>
#include <elm/system/Plugger.h>
#include <otawa/properties.h>
#include <otawa/base.h>
#include <otawa/platform.h>
#include <otawa/program.h>
#include <otawa/cfg.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/prog/Loader.h>

namespace otawa {

using namespace elm;

// Classes
class File;
class Manager;
namespace ilp {
	class System;
}
namespace sim {
	class Simulator;
}

// LoadException class
class LoadException: public MessageException {
public:
	LoadException(const elm::String& message);
};

// Manager class
class Manager {
	friend class WorkSpace;
	datastruct::Vector<hard::Platform *> platforms;
	datastruct::Vector<WorkSpace *> frameworks;
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
//extern GenericIdentifier<hard::Platform::Identification *> PLATFORM_IDENTIFIER;
extern Identifier<int> ARGC;
extern Identifier<char **> ARGV;
extern Identifier<char **> ENVP;
extern Identifier<sim::Simulator *> SIMULATOR;
extern Identifier<elm::CString> SIMULATOR_NAME;
extern Identifier<int> PIPELINE_DEPTH;
extern Identifier<bool> NO_SYSTEM;
extern Identifier<bool> NO_STACK;
extern Identifier<elm::system::Path> CONFIG_PATH;
extern Identifier<elm::xom::Element *> CONFIG_ELEMENT;
extern Identifier<elm::system::Path> CACHE_CONFIG_PATH;
extern Identifier<elm::xom::Element *> CACHE_CONFIG_ELEMENT;
extern Identifier<hard::CacheConfiguration *> CACHE_CONFIG;
extern Identifier<elm::system::Path> PROCESSOR_PATH;
extern Identifier<elm::xom::Element *> PROCESSOR_ELEMENT;
extern Identifier<hard::Processor *> PROCESSOR;

} // otawa

#endif	// OTAWA_MANAGER_H
