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

using namespace elm;
using namespace elm::option;
using namespace otawa;

// Configuration class
class Configuration {
public:
	Configuration(void) {
		prefix = MANAGER.prefixPath();
		cflags << "-I" << prefix.append("include");
		libs << "-L" << prefix.append("lib") << " -lotawa -lelm -lgel";
	}

	StringBuffer cflags, libs;
	elm::system::Path prefix;
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
		config.libs << " -L"
			<< config.prefix.append("lib/otawa/ilp") << " -l" << libname;
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
		config.libs << " -u" << name() << "_plugin -L"
			<< config.prefix.append("lib/otawa/ilp") << " -l" << name();
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
		config.libs << " -u" << name() << "_plugin -L"
			<< config.prefix.append("lib/otawa/loader") << " -l" << name();
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
		config.libs << " -L"
			<< config.prefix.append("lib/otawa/proc").append(_path) << " -l" << name();
	}
private:
	string _path;
};

// Main class
class Config: public option::Manager {
public:
	Config(void):
		cflags(*this, cmd, "--cflags", option::description, "output compilation C++ flags", end),
		data(*this, cmd, "--data", option::description, "output the OTAWA data path", end),
		doc(*this, cmd, "--doc", option::description, "output the OTAWa document path", end),
		has_so(*this, cmd, "--has-so", option::description, "exit with 0 if dynamic libraries are available, non-0 else", end),
		help(*this, cmd, "-h", cmd, "--help", option::description, "display the help message", end),
		ilp(*this, cmd, "--ilp", option::description, "list ILP solver plugins available", end),
		libs(*this, cmd, "--libs", option::description, "output linkage C++ flags", end),
		loader(*this, cmd, "--loader", option::description, "list loader plugins available", end),
		modules(*this, cmd, "--modules", option::description, "list available modules", end),
		prefix(*this, cmd, "--prefix", option::description, "output the prefix directory of OTAWA", end),
		procs(*this, cmd, "--procs", option::description, "list available processor collections", end),
		show_version(*this, cmd, "--version", option::description, "output the current version", end)
	{
		// initialize the options
		program = "otawa-config";
		version = Version(2, 0, 0);
		author = "H. Cassé <casse@irit.fr>";
		copyright = "LGPL v2";
		description = "Get building information about the OTAWA framework";
		free_argument_description = "MODULES...";

		// initialize the modules
		add(Library::Make("display").doc("graph displayer library").lib("odisplay"));
		add(Library::Make("gensim").doc("generic temporal simulator"));
		add(Solver::Make("lp_solve4").doc("lp_solve 4.x ILP solver"));
		add(Solver::Make("lp_solve5").doc("config_lp_solve5"));
		add(::Loader::Make("ppc2").doc("PowerPC architecture loader"));
		add(::Loader::Make("arm2").doc("ARM architecture loader"));
		add(Proc::Make("bpred").doc("branch prediction library"));
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
			show("proc");
		if(modules)
			for(HashTable<string, Module *>::Iterator mod(modmap); mod; mod++)
				cout << '[' << mod->name() << "]\n" << mod->doc() << io::endl << io::endl;
	}

protected:
	virtual void process(string arg) {
		if(modmap.exists(arg))
			mods.add(modmap.get(arg, 0));
		else
			throw OptionException(_ << " module " << arg << " is unknown !");
	}

private:
	void add(Module *mod) {
		modmap.put(mod->name(), mod);
	}

	void show(cstring kind) {
		elm::system::Plugger plugger(
			OTAWA_ILP_NAME,
			OTAWA_ILP_VERSION,
			otawa::Manager::buildPaths(kind));
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

	HashTable<string, Module *> modmap;
	genstruct::Vector<Module *> mods;
	::Configuration config;
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
		show_version;
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
