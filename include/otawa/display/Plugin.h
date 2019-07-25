/*
 *	$Id$
 *	Plugin class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
#ifndef OTAWA_DISPLAY_DRIVER_PLUGIN_H_
#define OTAWA_DISPLAY_DRIVER_PLUGIN_H_

#include <elm/sys/Plugin.h>

namespace otawa { namespace display {

using namespace elm;

// Definitions
#define OTAWA_DISPLAY_HOOK		display_plugin
#define OTAWA_DISPLAY_NAME		"display_plugin"
#define OTAWA_DISPLAY_VERSION	Version(1, 0, 0)

// Plugin class
class Plugin: public elm::sys::Plugin {
public:
	Plugin(
		elm::CString name,
		const elm::Version& version,
		const elm::Version& plugger_version);
};

} } // otawa::display

#endif /* OTAWA_DISPLAY_DRIVER_PLUGIN_H_ */
