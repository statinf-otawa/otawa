/*
 * otawa-config utility
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
#include <elm/string/StringBuffer.h>
#include <elm/data/HashMap.h>
#include <elm/data/quicksort.h>
#include <elm/sys/Plugger.h>
#include <elm/data/Vector.h>
#include <otawa/ilp/ILPPlugin.h>
#include <elm/sys/Directory.h>
#include <elm/sys/System.h>
#include <elm/ini.h>
#include <elm/util/UniquePtr.h>
#include <otawa/prog/Manager.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/prog/Loader.h>
#include <otawa/proc/Registry.h>
#include <elm/data/HashSet.h>

using namespace elm;
using namespace elm::option;
using namespace otawa;

class RegistrationComparator {
public:
	static int compare(const AbstractRegistration *r1, const AbstractRegistration *r2)
		{ return r1->name().compare(r2->name()); }
	int doCompare(const AbstractRegistration *r1, const AbstractRegistration *r2) const
		{ return r1->name().compare(r2->name()); }
};


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
 * @li --bindir - output the OTAWA binary path
 * @li --cflags -- output compilation C++ flags
 * @li --datadir -- output the OTAWA data path
 * @li --docdir -- output the OTAWA document path
 * @li --help, -h -- display this help message
 * @li --ilp, --list-ilps -- list ILP solver plugins available
 * @li --install, -i -- Output path to install the component
 * @li --libdir -- output the OTAWA library path
 * @li --libs -- output linkage C++ flags
 * @li --list-loaders, --loader -- list loader plugins available
 * @li --list-path, --path -- display path with plugins
 * @li --list-plugins, --plug -- list available plugins
 * @li --list-scripts, --script -- list available scripts
 * @li --make-app, -a -- information to make a third-party application
 * @li --make-plug, -p PLUGIN_NAME -- information to make a plugin
 * @li --make-tool, -t -- information to make a tool integrated in OTAWA
 * @li --otawa-version -- output the OTAWA version
 * @li --plugdir -- output the OTAWA plugin path
 * @li --prefix -- output the prefix directory of OTAWA
 * @li --rpath, -r -- output options to control RPATH
 * @li --scriptdir -- output the OTAWA script path
 * @li --version, -v -- display this application version
 */

#if defined(WIN32) || defined(WIN64)
#	define	LIB_DIR		"bin"
#else
#	define	LIB_DIR		"lib"
#endif

class Config: public option::Manager {
public:
	Config(void):
		Manager(Manager::Make("otawa-config", Version(2, 3, 0))
			.author("H. Cassé <casse@irit.fr>")
			.copyright("LGPL v2")
			.description("Get building information about the OTAWA framework")
			.free_argument("MODULES...").help().version()),

		list_ilps		(SwitchOption::Make(*this).cmd("--list-ilps").cmd("--ilp")			.description("list ILP solver plugins available")),
		list_loaders	(SwitchOption::Make(*this).cmd("--list-loaders").cmd("--loader")	.description("list loader plugins available")),
		list_plugins	(SwitchOption::Make(*this).cmd("--list-plugins").cmd("--plug")		.description("list available plugins")),
		list_scripts	(SwitchOption::Make(*this).cmd("--list-scripts").cmd("--script")	.description("list available scripts")),
		list_path		(SwitchOption::Make(*this).cmd("--list-path").cmd("--path")			.description("display path with plugins")),

		prefix			(SwitchOption::Make(*this).cmd("--prefix")							.description("output the prefix directory of OTAWA")),
		docdir			(SwitchOption::Make(*this).cmd("--docdir")							.description("output the OTAWA document path")),
		plugdir			(SwitchOption::Make(*this).cmd("--plugdir")							.description("output the OTAWA plugin path")),
		datadir			(SwitchOption::Make(*this).cmd("--datadir")							.description("output the OTAWA data path")),
		scriptdir		(SwitchOption::Make(*this).cmd("--scriptdir")						.description("output the OTAWA script path")),
		bindir			(SwitchOption::Make(*this).cmd("--bindir")							.description("output the OTAWA binary path")),
		libdir			(SwitchOption::Make(*this).cmd("--libdir")							.description("output the OTAWA library path")),

		otawa_version	(SwitchOption::Make(*this).cmd("--otawa-version")					.description("output the OTAWA version")),

		make_plug		(Value<string>::Make(*this).cmd("-p").cmd("--make-plug")			.description("information to make a plugin").argDescription("PLUGIN_NAME")),
		make_app		(SwitchOption::Make(*this).cmd("-a").cmd("--make-app")				.description("information to make a third-party application")),
		make_tool		(SwitchOption::Make(*this).cmd("-t").cmd("--make-tool")				.description("information to make a tool integrated in OTAWA")),

		cflags			(SwitchOption::Make(*this).cmd("--cflags")							.description("output compilation C++ flags")),
		libs			(SwitchOption::Make(*this).cmd("--libs")							.description("output linkage C++ flags")),
		installdir		(SwitchOption::Make(*this).cmd("-i").cmd("--install")				.description("Output path to install the component")),
		rpath			(SwitchOption::Make(*this).cmd("--rpath").cmd("-r")					.description("output options to control RPATH")),
		analyzes		(SwitchOption::Make(*this).cmd("--analyzes")						.description("display all available analyzes")),
		feature_deps	(SwitchOption::Make(*this).cmd("--feature-deps")					.description("output a feature depency graph (in GraphViz format)")),
		verbose			(SwitchOption::Make(*this).cmd("--verbose").cmd("-V")				.description("provide details on the configuration process")),
		cmake			(SwitchOption::Make(*this).cmd("--cmake")							.description("get the path of CMake module for OTAWA")),
		locals			(ListOption<string>::Make(*this).cmd("-L").cmd("--local")			.description("path for local dependent plugin").argDescription("PATH"))
	{
	}

protected:

	virtual void process(string arg) {
		ProcessorPlugin *plugin = ProcessorPlugin::get(arg);
		if(plugin) {
			if(!plugs.contains(plugin))
				plugs.add(plugin);
		}
		else
			throw otawa::Exception(_ << " plugin " << arg << " cannot be found!");
	}

	virtual void run(void) {

		// list options
		if(list_ilps) {
			listILPs();
			return;
		}
		if(list_loaders) {
			listLoaders();
			return;
		}
		if(list_plugins) {
			listPlugins();
			return;
		}
		if(list_scripts) {
			listScripts();
			return;
		}

		// directory options
		if(prefix) {
			cout << MANAGER.prefixPath() << io::endl;
			return;
		}
		if(docdir) {
			cout << MANAGER.prefixPath() / "share" / "Otawa" / "doc" << io::endl;
			return;
		}
		if(plugdir) {
			cout << getPluginDir() << io::endl;
			return;
		}
		if(datadir) {
			cout << MANAGER.prefixPath() / "share" / "Otawa" << io::endl;
			return;
		}
		if(scriptdir) {
			cout << getScriptDir() << io::endl;
			return;
		}
		if(bindir) {
			cout << getBinDir() << io::endl;
			return;
		}
		if(libdir) {
			cout << getLibDir() << io::endl;
			return;
		}
		if(cmake) {
			cout << MANAGER.prefixPath() / "share" / "Otawa" / "cmake" / "Otawa.cmake" << io::endl;
			return;
		}

		// other information
		if(otawa_version) {
			cout << "Otawa " << MANAGER.VERSION << " (" << MANAGER.COMPILATION_DATE << ")";
			return;
		}
		if(analyzes) {
			Vector<const AbstractRegistration *> regs;
			for(auto i = Registry::Iter(); i(); i++)
				if(*i != &AbstractRegistration::null)
					regs.add(*i);
			quicksort(regs, RegistrationComparator());
			for(auto r: regs)
				cout << r->name() << " (" << r->version() << ")\n";
			return;
		}
		if(feature_deps) {
			outputFeatureDeps();
			return;
		}

		// for a plugin, look for the ELD
		sys::Path ipath;
		if(make_tool)
			ipath = getBinDir();
		else if(make_plug) {
			try {

				// get the list of dependencies
				Path eld = *make_plug;
				if(!eld.extension())
					eld = eld.setExtension("eld");
				UniquePtr<ini::File> file(ini::File::load(eld));
				ini::Section *sect = file->get("elm-plugin");
				if(!sect)
					throw option::OptionException(_ << "no eld-plugin section in " << eld);
				Vector<string> deps;
				sect->getList("deps", deps);

				// get the required plugins
				for(int i = 0; i < deps.length(); i++) {
					ProcessorPlugin *plugin = ProcessorPlugin::get(deps[i]);
					if(plugin) {
						if(!plugs.contains(plugin))
							plugs.add(plugin);
					}
					else if(!lookupLocal(deps[i]))
						throw otawa::Exception(_ << "cannot find the plugin " << deps[i]);
				}

				// get the name and directory
				string name = sect->get("name");
				if(!name)
					throw option::OptionException(_ << "no name provided for the plugin");
				ipath = (getPluginDir() / name).parent();
			}
			catch(ini::Exception& e) {
				throw option::OptionException(e.message());
			}
		}

		// close the dependencies
		for(int i = 0; i < plugs.length(); i++)
			for(sys::Plugin::DepIter dep(plugs[i]->dependencies()); dep(); dep++)
				if(!plugs.contains(*dep))
					plugs.add(*dep);

		// display path if required
		if(installdir) {
			cout << ipath << io::endl;
			return;
		}

		// display C flags
		if(cflags) {
			cout << quote(_ << "-I" << MANAGER.prefixPath() / "include") << io::endl;
			return;
		}

		// output libs
		if(libs) {
			cout << "-L" << quote(getLibDir()) << " -lotawa -lelm -lgel++ -lgel";

			// output dependencies
			for(elm::Vector<sys::Plugin *>::Iter p = plugs; p(); p++)
				cout << ' ' << quote(p->path());

			// output RPath
			if(rpath) {
				elm::Vector<string> rpaths;
				Path lpath = rpathFor(getLibDir(), ipath);
				rpaths.add(lpath);
				cout << " -Wl,-rpath -Wl," << lpath;
				for(elm::Vector<sys::Plugin *>::Iter p = plugs; p(); p++) {
					Path rpath = rpathFor(*p, ipath);
					if(!rpaths.contains(rpath)) {
						rpaths.add(rpath);
						if(make_app)
							cout << " -Wl,-rpath " << quote(_ << "-Wl," << rpath);
						else
							cout << " -Wl,-rpath " << quote(_ << "-Wl," << rpath);
					}
				}
			}

			cout << io::endl;
		}
	}

private:

	string shortenName(string name) {
		int s = 0, e = name.length();
		if(name.startsWith("otawa::"))
			s += 7;
		if(name.endsWith("_FEATURE"))
			e -= 8;
		return name.substring(s, e - s);
	}

	void outputFeatureDeps() {
		HashSet<Pair<const AbstractFeature *, const AbstractFeature *> > edges;
		cout << "digraph features {\n";
		for(auto i = Registry::Iter(); i(); i++) {
			for(auto u: i->features())
				if(u.kind() == FeatureUsage::provide) {
					for(auto r = *i; !r->isNull(); r = &r->base())
						for(auto uu: r->features())
							if(uu.kind() == FeatureUsage::require
							&& !edges.contains(pair(&uu.feature(), &u.feature()))) {
								cout << '"' << shortenName(uu.feature().name())
									<< "\" -> \""
									<< shortenName(u.feature().name()) << "\";\n";
								edges.add(pair(&uu.feature(), &u.feature()));
							}
				}
		}
		cout << "}\n";
	}

	/**
	 * Put quotes around text containing spaces or tabulations.
	 * @param s		String to quote.
	 * @return		Quoted string if needed.
	 */
	string quote(string s) {
		for(auto c: s)
			if(c == ' ' || c == '\r')
				return _ << '\'' << s << '\'';
		return s;
	}

	/**
	 * Lookup for a local plugin.
	 * @param ppath	Plugin path.
	 */
	bool lookupLocal(Path ppath) {
		string name = ppath.namePart() + ".eld";
		for(int i = 0; i < locals.count(); i++) {
			Path path = Path(locals[i]) / name;
			if(path.exists())
				return true;
		}
		return false;
	}

	Path rpathFor(const Path& p, const Path& ipath) {
		if(!ipath)
			return p;
		else
			return Path("$ORIGIN") / p.relativeTo(ipath);
	}

	/**
	 * Compute RPath for the given plugin.
	 */
	Path rpathFor(sys::Plugin *p, sys::Path ipath) {
		return rpathFor(p->path().dirPart(), ipath);
	}

	/**
	 * Show a list of plugins.
	 * @param kind		Type of the plugin.
	 * @param rec		Perform recursive research.
	 */
	void list(cstring kind, cstring name, cstring version) {
		// this implementation (a) does not support the new plugin system
		// and (b) hook any plugin without control and display all plugins as is.
		elm::sys::Plugger plugger(name, version, otawa::Manager::buildPaths(kind));
		if(!verbose)
			plugger.setQuiet(true);

		// get the initial directories
		elm::Vector<LockPtr<sys::Directory> > paths;
		for(sys::Plugger::PathIterator path(plugger); path(); path++) {
			LockPtr<sys::FileItem> item = sys::FileItem::get(*path);
			if(item && item->toDirectory())
				paths.add(item->toDirectory());
		}
		int builtin = paths.length();

		// recursively build other directories
		while(paths) {
			bool is_builtin = builtin == paths.length();
			builtin--;
			LockPtr<sys::Directory> dir = paths.pop();
			if(!is_builtin)
				plugger.addPath(dir->path());
			for(sys::Directory::Iter child(dir); child; child++)
				if(child->toDirectory())
					paths.add(child->toDirectory());
		}

		// look plugins
		elm::Vector<sys::Plugin *> found;
		for(elm::sys::Plugger::Iter plugin(plugger); plugin(); plugin++) {
			if(verbose)
				cerr << "INFO: plugging " << plugin.path() << io::endl;
			sys::Plugin *handle = plugin.plug();
			if(handle && !found.contains(handle)) {
				cout << handle->name();
				if(list_path)
					cout << " (" << handle->path() << ")";
				cout << io::endl;
				found.add(handle);
			}
		}
	}

	void listILPs(void) {
		list("ilp", OTAWA_ILP_NAME, OTAWA_ILP_VERSION);
	}

	void listLoaders(void) {
		list("loader", OTAWA_LOADER_NAME, OTAWA_LOADER_VERSION);
	}

	void listPlugins(void) {
		list("proc", OTAWA_PROC_NAME, OTAWA_PROC_VERSION);
	}

	Path getScriptDir(void) const {
		return MANAGER.prefixPath() / "share" / "Otawa" / "scripts";
	}

	Path getLibDir(void) const {
		return MANAGER.prefixPath() / LIB_DIR;
	}

	Path getPluginDir(void) const {
		return MANAGER.prefixPath() / LIB_DIR / "otawa";
	}

	Path getBinDir(void) const {
		return MANAGER.prefixPath() / "bin";
	}

	void listScripts(void) {
		LockPtr<sys::FileItem> item = sys::FileItem::get(getScriptDir());
		LockPtr<sys::Directory> dir = item->toDirectory();
		if(!dir)
			cerr << "ERROR: script directory \"" << getScriptDir() << "\" is not a directory !\n";
		else
			for(sys::Directory::Iter file(dir); file; file++)
				if(file->path().extension() == "osx")
					cout << file->path().basePart().namePart() << io::endl;
	}

	SwitchOption
		list_ilps,
		list_loaders,
		list_plugins,
		list_scripts,
		list_path;

	SwitchOption
		prefix,
		docdir,
		plugdir,
		datadir,
		scriptdir,
		bindir,
		libdir;

	SwitchOption
		otawa_version;

	option::Value<string> make_plug;

	SwitchOption
		make_app,
		make_tool,
		cflags,
		libs,
		installdir,
		rpath,
		analyzes,
		feature_deps,
		verbose,
		cmake;
	option::ListOption<string> locals;

	elm::Vector<sys::Plugin *> plugs;
};

int main(int argc, char *argv[]) {
	return Config().manage(argc, argv);
}

