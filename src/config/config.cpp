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
#include <otawa/otawa.h>
#include <elm/genstruct/HashTable.h>
#include <elm/system/Plugger.h>
#include <otawa/ilp/ILPPlugin.h>
#include <elm/sys/Directory.h>
#include <elm/sys/System.h>
#include <elm/ini.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <elm/util/AutoDestructor.h>

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
 * @li--doc -- output the OTAWA documentation path
 * @li --has-so -- exit with 0 return code if dynamic libraries are available, non-0 else
 * @li -h, --help -- display the help message
 * @li --list-ilps, --ilp -- list ILP solver plugins available
 * @li --libs -- output linkage C++ flags
 * @li --list-loaders, --loader list loader plugins available
 * @li --modules, --list-modules list available modules
 * @li --prefix	output the prefix directory of OTAWA
 * @li --procs, --list-procs -- list available processor collections
 * @li --version -- output the current version
 * @li --scripts -- output the scripts path
 * @li --list-scripts -- output the list of available scripts
 * @li -r, --rpath -- output options to control RPATH on OS supporting it.
 * @li -p, --plugin ELD_FILE -- ouput linkage options for a plugin for the given ELD file.
 * @li -i, --install -- output the directory where installing a plugin.
 */

#if defined(WIN32) || defined(WIN64)
#	define	LIB_DIR		"bin"
#else
#	define	LIB_DIR		"lib"
#endif

// Configuration class
class Configuration {
public:
	Configuration(void) {
		prefix = MANAGER.prefixPath();
		cflags << "-I" << prefix.append("include");
		libs << "-L" << prefix.append(LIB_DIR) << " -lotawa -lelm -lgel";
		addRPath(prefix.append(LIB_DIR));
	}

	/**
	 * Add an RPATH.
	 * @param path	Added path.
	 */
	void addRPath(const elm::system::Path& path) {
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

	StringBuffer cflags, libs;
	elm::system::Path prefix;
private:
	genstruct::Vector<sys::Path> rpath;
};

// Module classes
class Module {
public:
	template <class T>
	class Make {
	public:
		typedef typename T::Make M;
		inline M& doc(const string& doc) { mod->_doc = doc; return *static_cast<M *>(this); }
		inline M& require(Module *req) { mod->reqs.add(req); return *static_cast<M *>(this); }
		inline operator Module *(void) const { return mod; }
	protected:
		inline Make(T *_mod): mod(_mod) { }
		T *mod;
	};

	inline const string& name(void) const { return _name; }
	inline const string& doc(void) const { return _doc; }
	inline const genstruct::Vector<Module *>& requirements(void) const { return reqs; }
	virtual ~Module(void) { }
	virtual void adjust(::Configuration& config) = 0;
protected:
	Module(const string& name): _name(name) {  }
private:
	string _name;
	string _doc;
	genstruct::Vector<Module *> reqs;
};

class Library: public Module {
public:
	class Make: public Module::Make<Library> {
	public:
		inline Make(const string& name): Module::Make<Library>(new Library(name))  { }
		inline Make& lib(const string& name) { static_cast<Library *>(mod)->libname = name; return *this; }
	};

	virtual void adjust(::Configuration& config) {
		sys::Path path = config.prefix.append("lib");
		config.libs << " -L" << path << " -l" << libname;
		config.addRPath(path);
	}

protected:
	Library(const string& name): Module(name) { libname = name; }
private:
	string libname;
};

class Solver: public Module {
public:
	class Make: public Module::Make<Solver> {
	public:
		inline Make(const string& name): Module::Make<Solver>(new Solver(name))  { }
	};

	virtual void adjust(::Configuration& config) {
		sys::Path path = config.prefix.append("lib/otawa/ilp");
		config.libs << " -u" << name() << "_plugin " <<  path.append(sys::System::getPluginFileName(name()));
		config.addRPath(path);
	}
protected:
	Solver(const string& name): Module(name) { }
};

class Loader: public Module {
public:
	class Make: public Module::Make<Loader> {
	public:
		inline Make(const string& name): Module::Make<Loader>(new Loader(name))  { }
	};

	Loader(const string& name): Module(name) { }
	virtual void adjust(::Configuration& config) {
		sys::Path path = config.prefix.append("lib/otawa/loader");
		config.libs << " -u" << name() << "_plugin " << path.append(sys::System::getPluginFileName(name()));
		config.addRPath(path);
	}
};

class Proc: public Module {
public:
	class Make: public Module::Make<Proc> {
	public:
		inline Make(const string& name): Module::Make<Proc>(new Proc(name))  { }
		inline Make& path(const string& path) { mod->_path = path; return *this; }
	};

	Proc(const string& name): Module(name) { }
	virtual void adjust(::Configuration& config) {
		sys::Path path = config.prefix.append("lib/otawa/proc").append(_path);
		config.libs << " " << path.append(sys::System::getPluginFileName(name()));
		config.addRPath(path);
	}
private:
	string _path;
};

// Main class

class Config: public option::Manager {
public:
	Config(void):
		Manager(
			option::program, "otawa-config",
			option::version, new Version(2, 1, 0),
			option::author, "H. Cassé <casse@irit.fr>",
			option::copyright, "LGPL v2",
			option::description, "Get building information about the OTAWA framework",
			option::free_arg, "MODULES...",
			option::end),
		cflags(*this, cmd, "--cflags", option::description, "output compilation C++ flags", end),
		data(*this, cmd, "--data", option::description, "output the OTAWA data path", end),
		doc(*this, cmd, "--doc", option::description, "output the OTAWa document path", end),
		has_so(*this, cmd, "--has-so", option::description, "exit with 0 if dynamic libraries are available, non-0 else", end),
		help(*this, cmd, "-h", cmd, "--help", option::description, "display the help message", end),
		ilp(*this, cmd, "--list-ilps", cmd, "--ilp", option::description, "list ILP solver plugins available", end),
		libs(*this, cmd, "--libs", option::description, "output linkage C++ flags", end),
		loader(*this, cmd, "--list-loaders", cmd, "--loader", option::description, "list loader plugins available", end),
		modules(*this, cmd, "--list-modules", cmd, "--modules", option::description, "list available modules", end),
		prefix(*this, cmd, "--prefix", option::description, "output the prefix directory of OTAWA", end),
		procs(*this, cmd, "--list-procs", cmd, "--procs", option::description, "list available processor collections", end),
		show_version(*this, cmd, "--version", option::description, "output the current version", end),
		scripts(*this, cmd, "--scripts", option::description, "output the scripts path", end),
		list_scripts(*this, cmd, "--list-scripts", option::description, "output the list of available scripts", end),
		rpath(*this, cmd, "--rpath", cmd, "-r", option::description, "output options to control RPATH", end),
		install(SwitchOption::Make(*this).cmd("-i").cmd("--install").description("Output path of plugin directory")),
		eld(ValueOption<string>::Make(*this).cmd("-p").cmd("--plugin").argDescription("ELD_FILE").description("ELD file to generate linkage options"))
	{

		// initialize the modules
		add(Library::Make("display").doc("graph displayer library").lib("odisplay"));
		add(Library::Make("gensim").doc("generic temporal simulator"));
		add(Solver::Make("lp_solve4").doc("lp_solve 4.x ILP solver"));
		add(Solver::Make("lp_solve5").doc("config_lp_solve5"));
		add(::Loader::Make("ppc2").doc("PowerPC architecture loader"));
		add(::Loader::Make("arm2").doc("ARM architecture loader"));
		add(Proc::Make("bpred").doc("branch prediction library").path("otawa"));
		add(Proc::Make("dcache").doc("simple L1 data cache analysis").path("otawa"));
		add(Proc::Make("cfgio").doc("CFG input/output").path("otawa"));
		Module *ast = Proc::Make("ast").path("otawa").doc("Abstract Syntactic Tree library");
		add(ast);
		add(Proc::Make("ets").path("otawa").require(ast).doc("Extended Timing Schema library"));
	}

	void run(int argc, char **argv) {

		// perform the parse
		this->parse(argc, argv);
		if(help) {
			displayHelp();
			return;
		}

		// close the list of modules
		genstruct::Vector<Module *> cmods;
		for(int i = 0; i < mods.length(); i++)
			if(!cmods.contains(mods[i])) {
				Module *mod = mods[i];
				cmods.add(mod);
				const genstruct::Vector<Module *>& reqs = mod->requirements();
				for(int i = 0; i < reqs.length(); i++)
					if(!cmods.contains(reqs[i]))
						cmods.add(reqs[i]);
			}

		// perform adjustment according to the modules
		for(int i = 0; i < cmods.length(); i++)
			cmods[i]->adjust(config);

		// perform adjustment according to the plugins
		for(int i = 0; i < plugs.length(); i++) {
			Path path = plugs[i]->path();
			config.libs << ' ' << path;
			config.addRPath(path.parent());
		}

		// load the ELD
		if(eld)
			adjustELD(config);

		// do the display
		if(prefix)
			cout << config.prefix << io::endl;
		if(cflags)
			cout << config.cflags.toString() << io::endl;
		if(libs)
			cout << config.libs.toString() << io::endl;
		if(data)
			cout << config.prefix.append("share/Otawa") << io::endl;
		if(doc)
			cout << config.prefix.append("share/Otawa/autodoc/index.html") << io::endl;
		if(ilp)
			show("ilp");
		if(loader)
			show("loader");
		if(procs)
			show("proc", true);
		if(modules)
			for(HashTable<string, Module *>::Iterator mod(modmap); mod; mod++)
				cout << '[' << mod->name() << "]\n" << mod->doc() << io::endl << io::endl;
		if(scripts)
			cout  << getScriptDir() << io::endl;
		if(list_scripts)
			showScripts();
		if(rpath)
			cout << "-Wl,-rpath -Wl," << config.getRPath() << io::endl;
		if(install)
			cout << (MANAGER.prefixPath() / "lib/otawa/proc") << io::endl;
	}

protected:
	virtual void process(string arg) {

		// add module information
		if(modmap.exists(arg))
			mods.add(modmap.get(arg, 0));

		// look for a matching plugin
		else {
			ProcessorPlugin *plugin = ProcessorPlugin::get(arg);
			if(plugin)
				plugs.add(plugin);
			else
				throw OptionException(_ << " plugin " << arg << " cannot be found!");
		}
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

	void add(Module *mod) {
		modmap.put(mod->name(), mod);
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

	HashTable<string, Module *> modmap;
	genstruct::Vector<Module *> mods;
	genstruct::Vector<ProcessorPlugin *> plugs;
	::Configuration config;
	option::ValueOption<string> eld;
	SwitchOption
		cflags,
		data,
		doc,
		has_so,
		help,
		ilp,
		libs,
		loader,
		modules,
		prefix,
		procs,
		show_version,
		scripts,
		list_scripts,
		rpath,
		install;
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
