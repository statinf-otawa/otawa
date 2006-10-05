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
#include <otawa/prog/FrameWork.h>
#include <otawa/prog/Loader.h>

using namespace elm;	// !!TODO!! Remove it !!!!

namespace otawa {

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
	LoadException(elm::CString format, VarArg& args);
	LoadException(elm::CString format, ...);
};

// Manager class
class Manager {
	friend class FrameWork;
	datastruct::Vector<hard::Platform *> platforms;
	datastruct::Vector<FrameWork *> frameworks;
	elm::system::Plugger ilp_plugger;
	elm::system::Plugger loader_plugger;
	elm::system::Plugger sim_plugger;
public:
	static const CString OTAWA_NS;
	static const CString OTAWA_NAME;
	static const CString PROCESSOR_NAME;

	Manager(void);
	~Manager(void);
	Loader *findLoader(elm::CString name);
	sim::Simulator *findSimulator(elm::CString name);
	FrameWork *load(const elm::system::Path& path, PropList& props);
	ilp::System *newILPSystem(String plugin = "");
	
};

// Configuration Properties
extern GenericIdentifier<CString> TASK_ENTRY;
extern GenericIdentifier<hard::Platform *> PLATFORM;
extern GenericIdentifier<Loader *> LOADER;
extern GenericIdentifier<elm::CString> PLATFORM_NAME;
extern  GenericIdentifier<elm::CString>  LOADER_NAME;
//extern GenericIdentifier<hard::Platform::Identification *> PLATFORM_IDENTIFIER;
extern GenericIdentifier<int> ARGC;
extern GenericIdentifier<char **> ARGV;
extern GenericIdentifier<char **> ENVP;
extern GenericIdentifier<sim::Simulator *> SIMULATOR;
extern GenericIdentifier<elm::CString> SIMULATOR_NAME;
extern GenericIdentifier<hard::CacheConfiguration *> CACHE_CONFIG;
extern GenericIdentifier<int> PIPELINE_DEPTH;
extern GenericIdentifier<bool> NO_SYSTEM;
extern GenericIdentifier<elm::system::Path> CONFIG_PATH;
extern GenericIdentifier<elm::xom::Element *> CONFIG_ELEMENT;
extern GenericIdentifier<elm::system::Path> PROCESSOR_PATH;
extern GenericIdentifier<elm::xom::Element *> PROCESSOR_ELEMENT;
extern GenericIdentifier<hard::Processor *> PROCESSOR;

} // otawa

#endif	// OTAWA_MANAGER_H
