/*
 *	$Id$
 *	Application class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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

#include <otawa/app/Application.h>
#include <otawa/proc/ProcessorPlugin.h>

namespace otawa {

/**
 * @class Application
 * A class making easier the use of applications built on OTAWA. It automatically support for
 * @li --set ID=VALUE -- set a property in the configuration property list (passed to manager and other processors),
 * @li -h|--help -- option help display,
 * @li -v|--verbose -- verbose mode activation,
 * @li first argument as binary program,
 * @li following arguments as task entries.
 * 
 * @p
 * Usually, it specialized according your application purpose:
 * @code
 * #include <otawa/app/Application.h>
 * class MyApp: public Application [
 * public:
 *		MyApplication(void):
 * 			Application("my_application", ...),
 * 			option1(*this, ...),
 * 			option2(*this, ...)
 * 		{
 * 			...
 * 		}
 * 
 * protected:
 * 		virtual void work(void) {
 * 			// do_something useful on the opened wotkspace
 * 		} 
 * 
 * private:
 * 		MyOption1 option1;
 * 		MyOption2 option2;
 * 		...
 * };
 *
 * OTAWA_RUN(MyApp)
 * @endcode
 * 
 */


/**
 * Build the application.
 * @param _program		Program name.
 * @param _version		Current version (optional).
 * @param _description	Description of the application (optional).
 * @param _author		Author identification.
 * @param _copyright	Applied copyright.
 */
Application::Application(
	cstring _program,
	Version _version,
	cstring _description,
	cstring _author,
	cstring _copyright
):
	help(*this, 'h', "help", "display this help", false),
	verbose(*this, 'v', "verbose", "verbose display of the process", false),
	sets(*this, option::cmd, "--add-prop", option::description, "set a configuration property", option::arg_desc, "ID=VALUE", option::end),
	params(*this, option::cmd, "--load-param", option::description, "add a load parameter", option::arg_desc, "ID=VALUE", option::end),
	props2(0),
	result(0),
	ws(0)
{
	program = _program;
	version = _version;
	description = _description;
	author = _author;
	copyright = _copyright;
	free_argument_description = "PROGRAM [FUNCTION ...]";
}
	

/**
 */
Application::~Application(void) {
}


/**
 * Run the application:
 * @li scan the options from argc and argv,
 * @li open the workspace,
 * @li call the work() method.
 * @param argc	Argument count as passed to main().
 * @param argv	Argument list as passed to main().
 * @return		A return code adapted to the current OS.
 */
int Application::run(int argc, char **argv) {
	try {
		
		// process arguments
		parse(argc, argv);
		if(!path)
			throw option::OptionException("no PROGRAM given");
		if(!entries)
			entries.add("main");
		if(help) {
			displayHelp();
			return 1;
		}
		if(verbose)
			Processor::VERBOSE(props) = true;

		// process the sets
		bool failed = false;
		for(int i = 0; i < sets.count(); i++) {

			// scan the argument
			string arg = sets[i];
			int p = arg.indexOf('=');
			if(p < 0) {
				cerr << "ERROR: bad --set argument: \"" << arg << "\"\n";
				failed = true;
				continue;
			}
			string name = arg.substring(0, p);
			string val = arg.substring(p + 1);

			// find the identifier
			AbstractIdentifier *id = AbstractIdentifier::find(name);
			if(!id) {
				id = ProcessorPlugin::getIdentifier(&name);
				if(!id) {
					cerr << "ERROR: unknown identifier \"" << name << "\"\n";
					failed = true;
					continue;
				}
			}

			// scan the value
			id->fromString(props, val);
		}
		if(failed)
			return 2;

		// prepare the load params
		for(int i = 0; i < params.count(); i++)
			LOAD_PARAM(props).add(params[i]);

		// prepare the work
		prepare(props);
		
		// do the work
		ws = MANAGER.load(path, props);
		work(props);
	}
	catch(option::OptionException& e) {
		cerr << "ERROR: " << e.message() << io::endl;
		displayHelp();
		result = 1;
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << io::endl;
		result = 1;
	}

	// cleanup
	if(ws)
		delete ws;
	return result;
}


/**
 * This method may be overriden by the child class in order
 * to customize the building of configuration property list.
 * @param props		Property list to customize.
 */
void Application::prepare(PropList& props) {
}


/**
 * This method must be overriden to implement the action of the application.
 * As a default, it call the work(string, props) method for each free argument on a command line
 * (supposed to be a task entry).
 *
 * When this method is called, the program has been already loaded.
 * @throw	elm::Exception	For any found error.
 */
void Application::work(PropList &props) throw(elm::Exception) {
	for(int i = 0; i < entries.count(); i++) {
		props2 = new PropList(props);
		ASSERT(props2 != NULL);
		TASK_ENTRY(props2) = entries[i].toCString();
		work(entries[i], *props2);
		delete props2;
		props2 = 0;
	}
}


/**
 * This method must be overriden to give a special behaviour for each free argument
 * of the command line (supposed to be the tasks entry points). As a default, do nothing.
 *
 * When this method is called, the program has already been loaded and the workspace
 * is marked as having as entry point the one passed in argument: threfore the workspace is ready
 * for computation.
 *
 * @param entry				Task entry point name or any free argument.
 * @throw elm::Exception	For any found error.
 */
void Application::work(const string& entry, PropList &props) throw(elm::Exception) {
}


/**
 * @fn WorkSpace *Application::workspace(void);
 * Provide the current workspace.
 * @return	Current workspace.
 */


/**
 * Perform a require request on the current process with the current configuration property list
 * (as configured in prepare()).
 * @param feature	Feature to require.
 */
void Application::require(const AbstractFeature&  feature) {
	ASSERTP(props2, "require() is only callable from work(task_name, props) function");
	ws->require(feature, *props2);
}


/**
 * If overriden, this method allows to provide a customized behaviour to
 * the usual processing of free arguments.
 * @param arg	Current argument.
 */
void Application::process(string arg) {
	if(!path)
		path = arg;
	else
		entries.add(arg);
}

}	// otawa
