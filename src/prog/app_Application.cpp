/*
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

#include <elm/io/ansi.h>
#include <elm/sys/System.h>
#include <otawa/app/Application.h>
#include <otawa/cfgio/Output.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/stats/features.h>
#include <otawa/util/SymAddress.h>
#include <otawa/prog/Manager.h>
#include <otawa/view/features.h>
#include <otawa/prog/File.h>

#include "../../include/otawa/flowfact/FlowFactLoader.h"

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
 * @li @ref opcg
 * @li @ref ostat
 * @li @ref otawa-config
 * @li @ref otawa-install
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
 * @par Using Application class
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
 * 	MyCommand(void): Application(Make("my-command", Version(1, 0, 2))) { }
 * protected:
 *	void work(PropList &props) throw(elm::Exception) {
 *		// do_something useful on the opened workspace
 *	}
 * };
 * OTAWA_RUN(MyApp)
 * @endcode
 *
 * Basically, the produced command takes as first argument the executable and as other free arguments
 * the function to process. The functions to process are described either by their name,
 * or by their address (integer address or 0x-prefixed hexadecimal address).
 * If no other free argument is provided, the command is applied on
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
 * 	MyCommand(void): app::Application(Make("my-command", Version(1, 0, 2))),
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
 *
 * @par Facilities of the application class
 *
 * The application provides several helper functions:
 *	* error() -- display an error message,
 *	* warn() -- display a warning,
 *	* info() -- display an information,
 *	* fail() -- display an error and stop,
 *	* require() -- require a feature
 *	* workspace() -- current workspace,
 *	* run() -- execute a code processor,
 *	* exit() -- stop the application.
 *
 * In addition, Application provides display and logging services as a code processor:
 *	* out -- current output
 *	* log -- current log system
 *	* logFor() -- test the logging level.
 */

/*class StatOutput: public StatCollector::Collector {
public:
	StatOutput(Output& out): _out(out) { }
	virtual ~StatOutput() { }
	virtual void collect(const Address &address, t::uint32 size, int value, const ContextualPath& ctx) {
		_out << value << "\t" << address << "\t" << size << "\t" <<  ctx << io::endl;
	}

private:
	Output& _out;
};*/


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
: option::AbstractValueOption(Make(man)
	.cmd("--log")
	.description("select level of log (one of proc, deps, cfg, bb or inst)")
	.argDescription("LEVEL")),
log_level(Processor::LOG_NONE)
{ }


/**
 * Declare a standard log option identified by "--log".
 * If the option is not passed, revert to value @ref Processor::LOG_NONE.
 * @param man	Used option manager.
 */
LogOption::LogOption(option::Manager *man)
: option::AbstractValueOption(Make(man)
	.cmd("--log")
	.description("select level of log (one of proc, deps, cfg, bb or inst)")
	.argDescription("LEVEL")),
log_level(Processor::LOG_NONE)
{ }


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
 * Build an application using the new Manager make-based approach
 * (look ELM documentation for more detail). To build the application,
 * you have just to do something like:
 * @code
 * class MyApllication: public Application {
 * public:
 * 		MyApplication(void): Application(Make("my_program", Version(1, 0, 3))
 * 			.description("this is my application")
 * 			.author("me <me@here.com")) { }
 * };
 * @endcode
 *
 * @param make	Maker instance to use.
 */
Application::Application(const Make& make):
	Manager(Make(make).help().version()),
	//help(option::SwitchOption::Make(*this).cmd("-h").cmd("--help").description("display this help")),
	verbose(option::SwitchOption::Make(*this).cmd("-v").cmd("--verbose").description("verbose display of the process (same as --log bb)")),
	dump(option::SwitchOption::Make(*this).cmd("--dump").description("dump results of all analyzes")),
	sets(option::ListOption<string>::Make(*this).cmd("--add-prop").description("set a configuration property").argDescription("ID=VALUE")),
	params(option::ListOption<string>::Make(*this).cmd("--load-param").description("add a load parameter").argDescription("ID=VALUE")),
	ff(option::ListOption<string>::Make(*this).cmd("--flowfacts").cmd("-f").description("select the flowfacts to load").argDescription("PATH")),
	ff_ignore_incomplete(option::SwitchOption::Make(*this).cmd("--flowfacts-ignore-incomplete").description("ignore incomplete flowfacts (marked with '?')")),
	work_dir(option::Value<string>::Make(*this).cmd("--work-dir").description("change the working directory").arg("PATH")),
	dump_to(option::Value<string>::Make(*this).cmd("--dump-to").description("dump the results of analyzes to PATH").arg("PATH")),
	record_stats(option::SwitchOption::Make(this).cmd("--stats").help("outputs available statistics in work directory")),
	log_for(option::ListOption<string>::Make(this).cmd("--log-for").help("only apply logging to the given processor")),
	dump_for(option::ListOption<string>::Make(this).cmd("--dump-for").help("dump results of the named analyzes").arg("ANALYSIS NAME")),
	view(option::SwitchOption::Make(*this).cmd("-W").cmd("--view").description("Dump views of the executable.")),
	all_cfgs(option::SwitchOption::Make(*this).cmd("--all_cfgs").description("Apply to all functions/CFGs.")),
	log_level(*this),
	props2(0),
	ws(0)
{ }


/**
 * Build the application.
 * @param _program		Program name.
 * @param _version		Current version (optional).
 * @param _description	Description of the application (optional).
 * @param _author		Author identification.
 * @param _copyright	Applied copyright.
 * @deprecated
 */
Application::Application(
	cstring _program,
	Version _version,
	cstring _description,
	cstring _author,
	cstring _copyright
): Application(Make(_program, _version)
	.description(_description)
	.author(_author)
	.copyright(_copyright)
	.help()
	.version())
{ }


/**
 */
Application::~Application(void) {
}


/**
 * Run the application:
 * @li open the workspace,
 * @li call the work() method.
 */
void Application::run() {

		// process arguments
		if(!path)
			throw option::OptionException("no PROGRAM given");
		if(!_args)
			_args.add("main");

		// process logging
		if(verbose)
			Processor::VERBOSE(props) = true;
		if(*log_level)
			Processor::LOG_LEVEL(props) = log_level;
		for(auto name: log_for)
			Processor::LOG_FOR(props).add(name);

		// process dumping
		if(dump)
			DUMP(props) = true;
		if(dump_to)
			DUMP_TO(props) = dump_to;
		for(auto name: dump_for)
			DUMP_FOR(props).add(name);

		// process the sets
		bool failed = false;
		for(int i = 0; i < sets.count(); i++) {

			// scan the argument
			string arg = sets[i];
			int p = arg.indexOf('=');
			if(p < 0) {
				error(_ << "bad --set argument: \"" << arg << "\"");
				failed = true;
				continue;
			}
			string name = arg.substring(0, p);
			string val = arg.substring(p + 1);

			// find the identifier
			AbstractIdentifier *id = AbstractIdentifier::find(name);
			if(!id) {
				id = ProcessorPlugin::getIdentifier(name.toCString());
				if(!id) {
					error(_ << " unknown identifier \"" << name << "\"");
					failed = true;
					continue;
				}
			}

			// scan the value
			id->fromString(props, val);
		}
		if(failed)
			return;

		// prepare the load params
		for(int i = 0; i < params.count(); i++)
			LOAD_PARAM(props).add(params[i]);

		// prepare the work
		prepare(props);

		// load the program
		ws = MANAGER.load(path, props);
		if(work_dir)
			ws->workDir(*work_dir);

		if(all_cfgs) {
			_args.clear();
			for(auto symb : ws->process()->program()->symbols())  {
				if(symb->kind() == Symbol::FUNCTION) {
					_args.add(symb->name());
				}
			}
		}

		// if required, load the flowfacts
		if(ff)
			for(int i = 0; i < ff.count(); i++)
				otawa::FLOW_FACTS_PATH(props).add(Path(ff[i]));
		if(ff_ignore_incomplete)
			otawa::FLOW_FACTS_IGNORE_UNKNOWN(props) = true;

		// do the work
		Monitor::configure(props);
		Monitor::setWorkspace(ws);
		work(props);
		complete(props);


	// cleanup
	if(ws)
		delete ws;
}


/**
 * This method may be overriden by the child class in order
 * to customize the building of configuration property list.
 * @param props		Property list to customize.
 */
void Application::prepare(PropList& props) {
}



/**
 * This method is called at end of the processing of the
 * free arguments of command line.
 */
void Application::complete(PropList& props) {
}


/**
 * This method must be overriden to implement the action of the application.
 * As a default, it call the work(string, props) method for each free argument on a command line
 * (supposed to be a task entry).
 *
 * When this method is called, the program has been already loaded.
 * @throw	elm::Exception	For any found error.
 */
void Application::work(PropList &props) {
	for(int i = 0; i < _args.count(); i++) {
		startTask(_args[i]);
		work(_args[i], *props2);
		completeTask();
	}
}


/**
 * Start the processing of the task corresponding to the entry.
 * @param entry	Entry to process.
 */
void Application::startTask(const string& entry) {

	// determine entry address
	Address addr = parseAddress(entry);
	TASK_ADDRESS(props) = addr;
	if(record_stats)
		Processor::COLLECT_STATS(props) = true;

	// Allow to reload the entry point to a new address
	if(workspace()->isProvided(TASK_INFO_FEATURE))
		workspace()->invalidate(TASK_INFO_FEATURE);

	// prepare properties
	props2 = new PropList(props);
}


/**
 * Complete the processing of the current task.
 */
void Application::completeTask() {

	// manage stats
	if(record_stats)
		stats();
	else if(view && workspace()->provides(view::BASE_FEATURE))
		workspace()->require(view::DUMP_FEATURE, props);

	// cleanup properties
	delete props2;
	props2 = nullptr;
}


/**
 * Generate statistics for the current workspace.
 */
void Application::stats() {
	workspace()->require(CFG_DUMP_FEATURE, props);
	workspace()->require(STATS_DUMP_FEATURE, props);
	if(workspace()->provides(view::BASE_FEATURE))
		workspace()->require(view::DUMP_FEATURE, props);
}



/**
 * Parse a symbolic address and return it.
 * @param s					Address string to parse.
 * @return					Matching address.
 * @throw otawa::Exception	If the address cannot be resolved or parsed.
 */
Address Application::parseAddress(const string& s) {
	SymAddress *saddr;
	try {
		saddr = SymAddress::parse(s);
	}
	catch(otawa::Exception& e) {
		throw Exception(_ << "cannot parse entry address '" << s << "'");
	}
	Address addr = saddr->toAddress(ws);
	if(addr.isNull())
		throw Exception(_ << "address " << saddr << " cannot be resolved.");
	delete saddr;
	return addr;
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
void Application::work(const string& entry, PropList &props) {
}


/**
 * @fn const Vector<string> Application::arguments(void) const;
 * Get the free argument of the application except the first one identified as the binary path.
 * If there is no free argument, "main" is automatically added.
 * @return	Free argument array.
 */


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
 * Run the given processor in the current workspace using the current property
 * list as configuration.
 * @param p		Processor to run.
 */
void Application::run(Processor *p) {
	ASSERTP(props2, "run() is only callable from work(task_name, props) function");
	ws->run(p, *props2);
}


/**
 * @fn T *Application::run();
 * Create a code processor of type T and run it in the current workspace using the
 * current property list as configuration.
 *
 * Notice that the workspace is in charge of releasing the processor instance
 * at the right time.
 *
 * @param T		Type of processor to run.
 * @return		Built processor.
 */


/**
 * Display an error message and stop the application with the given error code.
 * @param code	Return code of the application.
 * @param msg	Message to display.
 */
void Application::fail(int code, string msg) {
	error(msg);
	exit(code);
}


/**
 * Display an error message with the given message.
 * @param msg	Message to display.
 */
void Application::error(string msg) {
	cerr << io::BRIGHT_RED << "ERROR: " << io::PLAIN << msg << io::endl;
}


/**
 * Display a warning message with the given message.
 * @param msg	Message to display.
 */
void Application::warn(string msg) {
	cerr << io::BRIGHT_GREEN << "WARNING: " << io::PLAIN << msg << io::endl;
}


/**
 * Display an information to the user with the given message.
 * @param msg	Message to display.
 */
void Application::info(string msg) {
	cerr << io::CYAN << "INFO: " << io::PLAIN << msg << io::endl;
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
		_args.add(arg);
}


/**
 * Stop the current application and return the given code to the OS.
 * @param code	Code to return to OS (0 for success, != 0 for error).
 */
void Application::exit(int code) {
	sys::System::exit(code);
}

}	// otawa

