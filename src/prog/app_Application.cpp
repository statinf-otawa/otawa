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
#include <otawa/util/FlowFactLoader.h>

namespace otawa {

/**
 * @defgroup commands Commands
 *
 * This page provide documentation about the commands provided by OTAWA:
 * @li @ref dumpcfg
 * @li @ref mkff
 * @li @ref odec
 * @li @ref odfa
 * @li @ref odisasm
 * @li @ref oipet (deprecated)
 * @li @ref opcg
 * @li @ref ostat
 * @li @ref otawa-config
 * @li @ref owcet
 */

/**
 * @defgroup application Application Writing
 *
 * OTAWA may be used from an graphical user interface like Eclipse
 * or from the command line. In the latter case, you can either use
 * the existing commands (@ref dumpcfg, @ref owcet, etc) but you can also
 * write your own commands adapted to a specific process not already provided
 * by OTAWA.
 *
 * Although the usual way to write a command with a @c main function works,
 * you can save a tedious task of handling parameters, opening the executable, processing
 * error by using the @ref otawa::app::Application class.
 *
 * Just define your own class inheriting and specializing app::Application like below
 * and you do neither need to declare the @c main function yourself thanks to the @c OTAWA_RUN macro.
 *
 * @code
 * #include <otawa/app/Application.h>
 * using namespace otawa;
 *
 * class MyCommand: public Application {
 * public:
 * 	MyCommand(void): Application("my-command", Version(1, 0, 2)) { }
 * protected:
 *	void work(PropList &props) throw(elm::Exception) {
 *		// do_something useful on the opened workspace
 *	}
 * };
 * OTAWA_RUN(MyApp)
 * @endcode
 *
 * Basically, the produced command takes as first argument the executable and as other free arguments
 * the function names to process. If no other free argument is provided, the command is applied on
 * the @c main function of the executable. For each function name, a call to @c work() method
 * is performed, the passed property list is initialized accordingly and a workspace ready to use
 * is created. To get it, just call the @ref Application::workspace() method. You can now perform
 * any required analysis.
 *
 * In addition, the @ref app::Application class provides standards options like:
 * @li --add-prop ID=VALUE -- set a configuration property (passed to manager and other processors),
 * @li -f|--flowfacts PATH -- select a flow fact file to load
 * @li -h|--help -- option help display,
 * @li --load-param ID=VALUE -- add a load parameter (passed to the manager load command)
 * @li --log one of proc, deps, cfg, bb or inst -- select level of log
 * @li -v|--verbose -- verbose mode activation.
 *
 * In addition, you can also defines your own options using the @ref elm::option classes:
 * @code
 * using namespace elm;
 * class MyCommand: public app::Application {
 * public:
 * 	MyCommand(void): app::Application("my-command", Version(1, 0, 2)),
 * 		option1(*this),
 * 		option2(*this), ... { }
 * private:
 *	option::SwitchOption option1;
 *	option::ValueOption<int> option2;
 *	...
 * };
 * @endcode
 *
 * Finally, just compile your OTAWA application and it is ready to run and to process an executable.
 */

/**
 * @class LogOption
 * Defines a command line option supporting log level
 * thanks to strings 'proc', 'cfg' or 'bb'.
 * @ingroup application
 */


/**
 * Declare a standard log option identified by "--log".
 * If the option is not passed, revert to value @ref Processor::LOG_NONE.
 * @param man	Used option manager.
 */
LogOption::LogOption(option::Manager& man)
: option::AbstractValueOption(man,
	option::cmd, "--log",
	option::description, "select level of log",
	option::arg_desc, "one of proc, deps, cfg, bb or inst",
	option::require,
	option::end),
log_level(Processor::LOG_NONE)
{ }


/**
 * Build a log option with the given configuration.
 * If the option is not passed, revert to value @ref Processor::LOG_NONE.
 * @param man	Used option manager.
 * @param tag	First tag.
 * @param ...	Must be a list of tags and their argument ended by option::end.
 */
LogOption::LogOption(option::Manager& man, int tag, ...): AbstractValueOption(man), log_level(Processor::LOG_NONE) {
	VARARG_BEGIN(args, tag)
		init(man, tag, args);
	VARARG_END
}


/**
 * @fn LogOption::operator Processor::log_level_t(void) const;
 * Get the set log level.
 * @return	Set log level.
 */


/**
 * Processor::log_level_t LogOption::operator*(void) const;
 * Get the set log level.
 * @return	Set log level.
 */


/**
 * Process the passed log level.
 * @param arg	Passed argument.
 * @throw option::OptionException	If the argument does not match.
 */
void LogOption::process(String arg) {
	static Pair<cstring, Processor::log_level_t> ids[] = {
		pair(cstring("proc"), Processor::LOG_PROC),
		pair(cstring("file"), Processor::LOG_FILE),
		pair(cstring("deps"), Processor::LOG_FILE),
		pair(cstring("fun"), Processor::LOG_FUN),
		pair(cstring("cfg"), Processor::LOG_FUN),
		pair(cstring("block"), Processor::LOG_BLOCK),
		pair(cstring("bb"), Processor::LOG_BLOCK),
		pair(cstring("inst"), Processor::LOG_INST),
		pair(cstring(""), Processor::LOG_NONE)
	};

	// look for the log level
	for(int i = 0; ids[i].snd; i++)
		if(ids[i].fst == arg) {
			log_level = ids[i].snd;
			return;
		}

	// if not found, throw an option exception
	throw option::OptionException(_ << "unknown log level: " << arg);
}


/**
 * @class Application
 * A class making easier the use of applications built on OTAWA. It automatically support for
 * @li --add-prop ID=VALUE -- set a configuration property (passed to manager and other processors),
 * @li -f|--flowfacts PATH -- select a flow fact file to load
 * @li -h|--help -- option help display,
 * @li --load-param ID=VALUE -- add a load parameter (passed to the manager load command)
 * @li -v|--verbose -- verbose mode activation,
 * @li first argument as binary program,
 * @li following arguments as task entries.
 * 
 * @p
 * Usually, it specialized according your application purpose:
 * @code
 * #include <otawa/app/Application.h>
 * class MyApp: public Application {
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
 * void work(PropList &props) throw(elm::Exception) {
 * 			// do_something useful on the opened workspace
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
 * @ingroup application
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
	Manager(
		option::program, &_program,
		option::version, new Version(_version),
		option::description, &_description,
		option::author, &_author,
		option::copyright, &_copyright,
		option::free_arg, "PROGRAM [FUNCTION ...]",
		option::end),
	help(*this, 'h', "help", "display this help", false),
	verbose(*this, 'v', "verbose", "verbose display of the process (same as --log bb)", false),
	sets(*this, option::cmd, "--add-prop", option::description, "set a configuration property", option::arg_desc, "ID=VALUE", option::end),
	params(*this, option::cmd, "--load-param", option::description, "add a load parameter", option::arg_desc, "ID=VALUE", option::end),
	ff(*this, option::cmd, "--flowfacts", option::cmd, "-f", option::description, "select the flowfacts to load", option::arg_desc, "PATH", option::end),
	log_level(*this),
	props2(0),
	result(0),
	ws(0)
{ }
	

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
		if(help) {
			displayHelp();
			return 1;
		}
		if(!path)
			throw option::OptionException("no PROGRAM given");
		if(!entries)
			entries.add("main");
		if(verbose)
			Processor::VERBOSE(props) = true;
		if(*log_level)
			Processor::LOG_LEVEL(props) = log_level;

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
		
		// load the program
		ws = MANAGER.load(path, props);

		// if required, load the flowfacts
		if(ff) {
			for(int i = 0; i < ff.count(); i++)
				otawa::FLOW_FACTS_PATH(props).add(Path(ff[i]));
			// Delay loading of flowfacts in order to
			// let scripts hook flowfacts nodes.
			//ws->require(FLOW_FACTS_FEATURE, props);
		}

		// do the work
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
		ASSERT(props2);
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
