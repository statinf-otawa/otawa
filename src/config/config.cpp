/*
 *	$Id$
 *	"Half" abstract interpretation class interface.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#include <elm/options.h>
#include <elm/genstruct/Vector.h>
#include <elm/string/StringBuffer.h>
#include <elm/genstruct/HashTable.h>
#include <elm/system/Plugger.h>
#include <otawa/ilp/ILPPlugin.h>
#include <elm/sys/Directory.h>
#include <elm/sys/System.h>
#include <elm/ini.h>
#include <elm/util/AutoDestructor.h>
#include <otawa/prog/Manager.h>
#include <otawa/proc/ProcessorPlugin.h>

using namespace elm;
using namespace elm::option;
using namespace otawa;

/**
 * @addtogroup commands
 * @section otawa-config otawa-config Command
 *
 * This command provides useful information to compile an OTAWA application or a plugin.
 *
 * @par Syntax
 * @code
 * > otawa-config [OPTIONS] MODULES...
 * @endcode
 *
 * The following options are available:
 * @li --cflags -- output compilation C++ flags
 * @li --data -- output the OTAWA data path
 * @li --doc -- output the OTAWA documentation path
 * @li --has-so -- exit with 0 return code if dynamic libraries are available, non-0 else
 * @li -h, --help -- display the help message
 * @li --list-ilps, --ilp -- list ILP solver plugins available
 * @li --libs -- output linkage C++ flags
 * @li --list-loaders, --loader list loader plugins available
 * @li --prefix	output the prefix directory of OTAWA
 * @li --procs, --list-procs -- list available processor collections
 * @li --version -- output the current version
 * @li --scripts -- output the scripts path
 * @li --list-scripts -- output the list of available scripts
 * @li -r, --rpath -- output options to control RPATH on OS supporting it.
 * @li -p, --plugin ELD_FILE -- ouput linkage options for a plugin for the given ELD file.
 * @li -i, --install -- output the directory where installing a plugin.
 * @li --oversion -- output version of OTAWA.
 * @li -R, --origin-rpath -- output options to control RPATH for installation in OTAWA directory on OS supporting it.
 */

#if defined(WIN32) || defined(WIN64)
#	define	LIB_DIR		"bin"
#else
#	define	LIB_DIR		"lib"
#endif

// Configuration class
class Configuration {
public:
	Configuration(void): _origin(false) {
		prefix = MANAGER.prefixPath();
		cflags << "-I" << prefix.append("include");
		libs << "-L" << prefix.append(LIB_DIR) << " -lotawa -lelm -lgel";
		addRPath(prefix.append(LIB_DIR));
	}

	/**
	 * Add an RPATH.
	 * @param path	Added path.
	 */
	void addRPath(elm::system::Path path) {
		sys::Path prefix = MANAGER.prefixPath();
		if(_origin && path.subPathOf(prefix))
			path = sys::Path(_ << "\\$ORIGIN") / path.toString().substring(prefix.toString().length() + 1);
		if(!rpath.contains(path))
			rpath.add(path);
	}

	/**
	 * Get the RPATH value.
	 * @return	RPATH value.
	 */
	string getRPath(void) {
		StringBuffer buf;
		for(int i = 0; i < rpath.length(); i++) {
			if(i != 0)
				buf << ':';
			buf << rpath[i];
		}
		return buf.toString();
	}

	void setOrigin(bool origin) {
		_origin = origin;
		rpath.clear();
		addRPath(prefix.append(LIB_DIR));
	}

	StringBuffer cflags, libs;
	elm::system::Path prefix;
private:
	genstruct::Vector<sys::Path> rpath;
	bool _origin;
};

// Main class
class Config: public option::Manager {
public:
	Config(void):
		Manager(Manager::Make("otawa-config", Version(2, 2, 0))
			.author("H. Cassé <casse@irit.fr>")
			.copyright("LGPL v2")
			.description("Get building information about the OTAWA framework")
			.free_argument("MODULES...")),
		eld				(ValueOption<string>::Make(*this).cmd("-p").cmd("--plugin")			.description("ELD file to generate linkage options").argDescription("ELD_FILE")),
		cflags			(SwitchOption::Make(*this).cmd("--cflags")							.description("output compilation C++ flags")),
		data			(SwitchOption::Make(*this).cmd("--data")							.description("output the OTAWA data path")),
		doc				(SwitchOption::Make(*this).cmd("--doc")								.description("output the OTAWa document path")),
		has_so			(SwitchOption::Make(*this).cmd("--has-so")							.description("exit with 0 if dynamic libraries are available, non-0 else")),
		ilp				(SwitchOption::Make(*this).cmd("--list-ilps").cmd("--ilp")			.description("list ILP solver plugins available")),
		libs			(SwitchOption::Make(*this).cmd("--libs")							.description("output linkage C++ flags")),
		loader			(SwitchOption::Make(*this).cmd("--list-loaders").cmd("--loader")	.description("list loader plugins available")),
		prefix			(SwitchOption::Make(*this).cmd("--prefix")							.description("output the prefix directory of OTAWA")),
		procs			(SwitchOption::Make(*this).cmd("--list-procs").cmd("--procs")		.description("list available processor collections")),
		show_version	(SwitchOption::Make(*this).cmd("--version")							.description("output the current version")),
		scripts			(SwitchOption::Make(*this).cmd("--scripts")							.description("output the scripts path")),
		list_scripts	(SwitchOption::Make(*this).cmd("--list-scripts")					.description("output the list of available scripts")),
		rpath			(SwitchOption::Make(*this).cmd("--rpath").cmd("-r")					.description("output options to control RPATH")),
		install			(SwitchOption::Make(*this).cmd("-i").cmd("--install")				.description("Output path of plugin directory")),
		oversion		(SwitchOption::Make(*this).cmd("--oversion")						.description("output version of OTAWA.")),
		orpath			(SwitchOption::Make(*this).cmd("-R").cmd("--origin-rpath")			.description("output options to control RPATH for an installation relative to OTAWA")),
		help			(SwitchOption::Make(*this).cmd("-h").cmd("--help")			.description("display this help message."))
	{
	}

	void run(int argc, char **argv) {

		// perform the parse
		this->parse(argc, argv);
		if(help) {
			displayHelp();
			return;
		}

		// record the origin
		if(orpath)
			config.setOrigin(true);

		// perform adjustment according to the plugins
		for(int i = 0; i < plugs.length(); i++) {

			// adjust output
			Path path = plugs[i]->path();
			config.libs << ' ' << path;
			config.addRPath(path.parent());

			// handle dependencies
			for(sys::Plugin::DepIter dep = plugs[i]->dependencies(); dep; dep++)
				if(!plugs.contains(*dep))
					plugs.add(*dep);
		}

		// load the ELD
		if(eld)
			adjustELD(config);

		// do the display
		if(oversion)
			cout << "OTAWA rev. " << MANAGER.VERSION << " (" << MANAGER.COMPILATION_DATE << ")";
		if(prefix)
			cout << config.prefix;
		if(cflags)
			cout << config.cflags.toString();
		if(data)
			cout << config.prefix.append("share/Otawa");
		if(doc)
			cout << config.prefix.append("share/Otawa/autodoc/index.html");
		if(ilp)
			show("ilp");
		if(loader)
			show("loader");
		if(procs)
			show("proc", true);
		if(scripts)
			cout  << getScriptDir();
		if(list_scripts)
			showScripts();
		if(rpath || orpath)
			cout << "-Wl,-rpath -Wl," << config.getRPath() << ' ';
		if(libs)
			cout << config.libs.toString();
		if(install)
			cout << (MANAGER.prefixPath() / "lib/otawa/proc");
		cout << endl;
	}

protected:
	virtual void process(string arg) {
		ProcessorPlugin *plugin = ProcessorPlugin::get(arg);
		if(plugin)
			plugs.add(plugin);
		else
			throw OptionException(_ << " plugin " << arg << " cannot be found!");
	}

private:

	/**
	 * Adjust the configuration according to the ELD file.
	 * @param config	Configuration to adjust.
	 */
	void adjustELD(::Configuration& config) throw(option::OptionException) {
		try {

			// get the list of dependencies
			AutoDestructor<ini::File> file(ini::File::load(*eld));
			ini::Section *sect = file->get("elm-plugin");
			if(!sect)
				throw option::OptionException(_ << "no eld-plugin section in " << *eld);
			genstruct::Vector<string> plugins;
			sect->getList("deps", plugins);

			// get the required plugins
			for(int i = 0; i < plugins.length(); i++) {
				ProcessorPlugin *plugin = ProcessorPlugin::get(plugins[i]);
				if(!plugin)
					throw option::OptionException(_ << "cannot find the plugin " << plugins[i]);
				config.libs << ' ' << plugin->path();
			}
		}
		catch(ini::Exception& e) {
			throw option::OptionException(e.message());
		}
	}

	/**
	 * Get the scripts paths.
	 */
	Path getScriptDir() {
		return config.prefix.append("share/Otawa/scripts");
	}

	/**
	 * Show a list of plugins.
	 * @param kind		Type of the plugin.
	 * @param rec		Perform recursive research.
	 */
	void show(cstring kind, bool rec = false) {
		elm::system::Plugger plugger(
			OTAWA_ILP_NAME,
			OTAWA_ILP_VERSION,
			otawa::Manager::buildPaths(kind));

		// if recursive, add subdirectories
		if(rec) {

			// get the initial directories
			Vector<sys::Directory *> paths;
			for(sys::Plugger::PathIterator path(plugger); path; path++) {
				sys::FileItem *item = sys::FileItem::get(*path);
				if(item && item->toDirectory()) {
					paths.add(item->toDirectory());
					item->toDirectory()->use();
				}
			}
			int builtin = paths.length();

			// recursively build other directories
			while(paths) {
				bool is_builtin = builtin == paths.length();
				builtin--;
				sys::Directory *dir = paths.pop();
				if(!is_builtin)
					plugger.addPath(dir->path());
				for(sys::Directory::Iterator child(dir); child; child++)
					if(child->toDirectory()) {
						paths.add(child->toDirectory());
						child->toDirectory()->use();
					}
				dir->release();
			}
		}

		// look available plugins
		bool first = true;
		for(elm::system::Plugger::Iterator plugin(plugger); plugin; plugin++) {
			if(first)
				first = false;
			else
				cout << ", ";
			cout << *plugin;
		}
		cout << io::endl;
	}

	/**
	 * Display the list of available scripts.
	 */
	void showScripts() {
		sys::FileItem *item = sys::FileItem::get(getScriptDir());
		sys::Directory *dir = item->toDirectory();
		if(!dir)
			cerr << "ERROR: script directory \"" << getScriptDir() << "\" is not a directory !\n";
		else
			for(sys::Directory::Iterator file(dir); file; file++)
				if(file->path().extension() == "osx")
					cout << file->path().basePart().namePart() << io::endl;
	}

	genstruct::Vector<sys::Plugin *> plugs;
	::Configuration config;
	option::ValueOption<string> eld;
	SwitchOption
		cflags,
		data,
		doc,
		has_so,
		ilp,
		libs,
		loader,
		prefix,
		procs,
		show_version,
		scripts,
		list_scripts,
		rpath,
		install,
		oversion,
		orpath,
		help;
};

int main(int argc, char **argv) {
	try {
		Config c;
		c.run(argc, argv);
	}
	catch(OptionException& e) {
		cerr << "ERROR: " << e.message() << io::endl;
		return 1;
	}
}
