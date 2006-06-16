/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/prog/Loader.h -- interface for Loader class.
 */
#ifndef OTAWA_PROG_LOADER_H
#define OTAWA_PROG_LOADER_H

#include <otawa/base.h>
#include <otawa/properties.h>
#include <otawa/prog/Process.h>
#include <otawa/hard/Platform.h>

namespace otawa {

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
	static GenericIdentifier<hard::Platform *> ID_Platform;
	static GenericIdentifier<Loader *> ID_Loader;
	static GenericIdentifier<elm::CString> ID_PlatformName;
	static GenericIdentifier<elm::CString>  ID_LoaderName;
	static GenericIdentifier<hard::Platform::Identification *> ID_PlatformId;
	static GenericIdentifier<int> ID_Argc;
	static GenericIdentifier<char **> ID_Argv;
	static GenericIdentifier<char **> ID_Envp;

	// Default platform and loader
	static Loader& LOADER_Gliss_PowerPC;
	static Loader& LOADER_Heptane_PowerPC;
	static CString LOADER_NAME_Gliss_PowerPC;
	static CString LOADER_NAME_Heptane;
	static CString PLATFORM_NAME_PowerPC_Gliss;
};

} // otawa

#endif	// OTAWA_PROG_LOADER_H
