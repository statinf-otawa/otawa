/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	manager.h -- manager classes interface.
 */
#ifndef OTAWA_MANAGER_H
#define OTAWA_MANAGER_H

#include <elm/datastruct/Vector.h>
#include <otawa/properties.h>
#include <otawa/base.h>
#include <otawa/platform.h>
#include <otawa/program.h>
#include <otawa/cfg.h>
#include <otawa/prog/Process.h>
#include <otawa/prog/FrameWork.h>
using namespace elm;

namespace otawa {

// Classes
class File;
class Platform;
class Manager;

// LoadException class
class LoadException: public Exception {
public:
	LoadException(const String& message);
	LoadException(const char *format, va_list args);
	LoadException(const char *format, ...);
};


// UnsupportedPlatformException class
class UnsupportedPlatformException: public Exception {
public:
	UnsupportedPlatformException(const String& message);
	UnsupportedPlatformException(const char *format, va_list args);
	UnsupportedPlatformException(const char *format, ...);
};


// Loader class
class Loader {
	friend class Manager;
protected:
	virtual ~Loader(void) { };
public:

	// Method
	virtual CString getName(void) const = 0;
	virtual Process *load(Manager *man, CString path, PropList& props) = 0;
	virtual Process *create(Manager *man, PropList& props) = 0;
	
	// Usual properties
	static id_t ID_Platform;
	static id_t ID_Loader;
	static id_t ID_PlatformName;
	static id_t ID_LoaderName;
	static id_t ID_PlatformId;
	static id_t ID_Argc;
	static id_t ID_Argv;
	static id_t ID_Envp;

	// Default platform and loader
	static Loader& LOADER_Gliss_PowerPC;
	static CString LOADER_NAME_Gliss_PowerPC;
	static CString LOADER_NAME_Heptane;
	static CString PLATFORM_NAME_PowerPC_Gliss;
};


// Manager class
class Manager {
	friend class FrameWork;
	datastruct::Vector<Loader *> loaders;
	datastruct::Vector<Platform *> platforms;
	datastruct::Vector<FrameWork *> frameworks;
public:
	Manager(void);
	~Manager(void);
	Loader *findLoader(CString name);
	Platform *findPlatform(CString name);
	Platform *findPlatform(const PlatformId& id);
	FrameWork *load(CString path, PropList& props);
};


};

#endif	// OTAWA_MANAGER_H
