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


// Process class
class Process {
public:
	virtual const Collection<File *> *files(void) const = 0;
	virtual File *createFile(void) = 0;
	virtual File *loadFile(CString path) = 0;
	virtual Platform *platform(void) const = 0;
	virtual Manager *manager(void) const = 0;
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

	// Default platform and loader
	static CString LOADER_Heptane;	// = "heptane";
	static CString PLATFORM_PowerPC_GLiss;	// = "powerpc-powerpc640-elf-gliss";
};


// FrameWork class
class FrameWork: public Process, public PropList {
	Process *proc;
public:
	FrameWork(Process *_proc);
	~FrameWork(void);
	inline Process *getProcess(void) const { return proc; };
	
	// File management
	virtual const Collection<File *> *files(void) const
		{ return proc->files(); };
	virtual File *createFile(void)
		{ return proc->createFile(); };
	virtual File *loadFile(CString path)
		{ return proc->loadFile(path); };
	virtual Platform *platform(void) const 
		{ return proc->platform(); };
	virtual Manager *manager(void) const
		{ return proc->manager(); };
	
	// CFG Management
	void buildCFG(void);
	CFGInfo *getCFG(void);
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
