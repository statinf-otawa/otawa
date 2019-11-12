/*
 *	$Id$
 *	ProcessorPlugin class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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
#ifndef OTAWA_PROC_PROCESSORPLUGIN_H_
#define OTAWA_PROC_PROCESSORPLUGIN_H_

#include <elm/data/Array.h>
#include <elm/sys/Plugin.h>
#include <elm/sys/Path.h>
#include <otawa/proc/Registration.h>
#include <elm/util/ErrorHandler.h>

namespace otawa {

// Definitions
#define OTAWA_PROC_HOOK		proc_plugin
#define OTAWA_PROC_NAME		"proc_plugin"
#define OTAWA_PROC_VERSION	"1.0.0"
#define OTAWA_PROC_ID(name, version, date)	ELM_PLUGIN_ID(OTAWA_PROC_NAME, name " V" version " (" date ") [" OTAWA_PROC_VERSION "]")

// ProcessorPlugin class
class ProcessorPlugin: public elm::sys::Plugin {
public:
	ProcessorPlugin(make& make);
	ProcessorPlugin(cstring name, const elm::Version& version, const elm::Version& plugger_version);
	~ProcessorPlugin(void);
	virtual const Array<AbstractRegistration *>& processors(void) const;

	static void addPath(const elm::sys::Path& path);
	static void removePath(const elm::sys::Path& path);
	static ProcessorPlugin *get(string name);
	static bool isPlugged(string name);
	static Processor *getProcessor(string name);
	static AbstractFeature *getFeature(string name);
	static AbstractIdentifier *getIdentifier(string name);
	static void setErrorHandler(ErrorHandler *error_handler);
	static ErrorHandler *getErrorHandler(void);

private:
	static void init(void);
};

}	// otawa

#endif /* OTAWA_PROC_PROCESSORPLUGIN_H_ */

