/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	manager.h -- manager classes interface.
 */
#ifndef OTAWA_MANAGER_H
#define OTAWA_MANAGER_H

#include <elm/datastruct/Vector.h>
#include <elm/system/Plugger.h>
#include <otawa/properties.h>
#include <otawa/base.h>
#include <otawa/platform.h>
#include <otawa/program.h>
#include <otawa/cfg.h>
#include <otawa/prog/FrameWork.h>
#include <otawa/prog/Loader.h>
using namespace elm;

namespace otawa {

// Classes
class File;
class Manager;
namespace ilp {
	class System;
}

// LoadException class
class LoadException: public Exception {
public:
	LoadException(const String& message);
	LoadException(const char *format, VarArg& args);
	LoadException(const char *format, ...);
};


// UnsupportedPlatformException class
class UnsupportedPlatformException: public Exception {
public:
	UnsupportedPlatformException(const String& message);
	UnsupportedPlatformException(const char *format, VarArg& args);
	UnsupportedPlatformException(const char *format, ...);
};


// Manager class
class Manager {
	friend class FrameWork;
	datastruct::Vector<hard::Platform *> platforms;
	datastruct::Vector<FrameWork *> frameworks;
	elm::system::Plugger ilp_plugger;
	elm::system::Plugger loader_plugger;
public:
	Manager(void);
	~Manager(void);
	Loader *findLoader(CString name);
	hard::Platform *findPlatform(CString name);
	hard::Platform *findPlatform(const hard::Platform::Identification& id);
	FrameWork *load(CString path, PropList& props);
	ilp::System *newILPSystem(String plugin = "");
};


};

#endif	// OTAWA_MANAGER_H
