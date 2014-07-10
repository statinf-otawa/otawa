/*
 *	$Id$
 *	owcet command implementation
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

#include <otawa/app/Application.h>
#include <elm/option/StringList.h>
#include <otawa/script/Script.h>
#include <otawa/ipet/IPET.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/ilp/System.h>
#include <otawa/stats/StatInfo.h>
#include <otawa/cfg/features.h>

using namespace otawa;

/**
 * @addtogroup commands
 * @section owcet owcet Command
 *
 * owcet command allows to compute WCET of an application using a script
 * provided for a particular architecture implementation. It provide full power
 * of OTAWA framework including selection of architecture-dependent plugin
 * and fine-tuned computation scripts.
 *
 * @par Syntax
 *
 * @code
 * SYNTAX: owcet [options] EXECUTABLE_PATH [TASK_ENTRY] -s SCRIPT_PATH
 * @endcode
 *
 * The @i EXECUTABLE_PATH must be the path of the executable file to compute WCET for.
 * This executable file must be in a machine language understood by OTAWA (currently OTAWA
 * supports PowerPC, ARM, Sparc, HCS12 and partly TriCore).
 *
 * The optional @i TASK_ENTRY designs the function to compute the WCET for and usually
 * matches a task in the real-time applications. If this parameter is not given,
 * the @tt main function is used.
 *
 * The @i SCRIPT_PATH is the path of the file containing the script. These file are usually
 * found in $OTAWA_HOME/share/Otawa/scripts. When an architecture plugin is installed,
 * it provides usually its own script.
 *
 * Other options includes:
 * @li -p, --param ID=VAL: several parameters with this form may be passed; these definition are used
 * to pass parameters to the script and the supported @i ID depends on the launched script (see its documentation
 * for more details),
 * @li -f, --flowfacts PATH: OTAWA can not automatically found loops so this options is used
 * to design the file containing loop bounds; supported formats includes .ff or .ffx (@ref ff). Flowfacts allows also
 * to pass specific configuration for the flow execution of a program.
 *
 * @par Hints
 *
 * Real-time systems do not usually perform formatted output as @tt printf family of functions but such a function
 * are often used in case of error. Usually formatting performed by @tt printf is big and complex piece of code
 * that, in the case we take it into account in the WCET computation, would dominate the execution time
 * and drive to a very overestimated WCET. To avoid this, you can command to OTAWA to ignore them
 * without recompiling your application. Just create a file named YOU_EXECUTABLE.ff and write inside:
 * @code
 * nocall "printf";
 * @endcode
 * You can do the like with any function disturbing your computation. You can find in @ref f4 more details
 * about the commande @tt .ff files. You may also use XML format called @tt .ffx (see @ref ffx).
 */

class Summer: public StatCollector::Collector {
public:
	Summer(void): s(0) { }
	inline int sum(void) const { return s; }
	virtual void enter(const ContextualStep &step) { }
	virtual void leave(void) { }
	virtual void collect (const Address &address, t::uint32 size, int value) {
		s += value;
	}
private:
	int s;
};


class OWCET: public Application {
public:
	OWCET(void): Application(
		"owcet",
		Version(1, 0, 0),
		"Compute the WCET of task using a processor script (.osx)."
		"H. Cassé <casse@irit.fr>",
		"Copyright (c) IRIT - UPS <casse@irit.fr>"
	),
	params(*this, 'p', "param", "parameter passed to the script", "IDENTIFIER=VALUE"),
	script(*this, 's', "script", "script used to compute WCET", "PATH", ""),
	ilp_dump(*this, option::cmd, "-i", option::cmd, "--dump-ilp", option::help, "dump ILP system to stdandard output", option::end),
	list(*this, option::cmd, "--list", option::help, "list configuration items", option::end),
	timed(*this, option::cmd, "--timed", option::cmd, "-t", option::help, "display computation", option::end),
	display_stats(*this, option::cmd, "-S", option::cmd, "--stats", option::help, "display statistics", option::end)
	{ }

protected:
	virtual void work (const string &entry, PropList &props) throw(elm::Exception) {

		// set statistics option
		if(display_stats)
			Processor::COLLECT_STATS(props) = true;

		// any script
		if(!script)
			throw option::OptionException("a script must be given !");

		// fullfill the parameters
		for(int i = 0; i < params.count(); i++) {
			string param = params[i];
			int idx = param.indexOf('=');
			if(idx < 0)
				cerr << "WARNING: argument " << param << " is malformed: VARIABLE=VALUE\n";
			else
				script::PARAM(props).add(pair(param.substring(0, idx), param.substring(idx + 1)));
		}

		// look for the script
		Path path = *script;
		if(!path.exists() && !path.isAbsolute()) {
			Path file = *script;
			if(file.extension() != "osx")
				file = file.setExtension("osx");
			bool found = false;
			string paths = MANAGER.buildPaths("../../share/Otawa/scripts", "");
			for(Path::PathIter p(paths); p; p++) {
				path = Path(*p) / file;
				if(isVerbose())
					cerr << "INFO: looking script in directory " << *p << io::endl;
				if(path.exists()) {
					found = true;
					break;
				}
			}
			if(!found)
				throw elm::option::OptionException(_ << "cannot find script " << *script);
		}
		if(isVerbose())
			cerr << "INFO: using script from " << path << io::endl;

		// launch the script
		if(list)
			script::ONLY_CONFIG(props) = true;
		if(timed)
			script::TIME_STAT(props) = true;
		TASK_ENTRY(props) = entry;
		script::PATH(props) = path;
		script::Script scr;
		scr.process(workspace(), props);

		// process the list option
		if(list) {
			cerr << "CONFIGURATION OF " << *script << io::endl;
			for(script::Script::ItemIter item(scr); item; item++) {
				cerr << "* item " << item->name << ": " << script::ScriptItem::type_labels[item->type];
				if(item->deflt)
					cerr << " (" << item->deflt << ")";
				cerr << io::endl;
				if(item->help)
					cerr << item->help;
				cerr << io::endl;
			}
		}

		// display the result
		ot::time wcet = ipet::WCET(workspace());
		if(wcet == -1)
			throw otawa::Exception("no WCET computed (see errors above).");
		else
			cout << "WCET[" << entry << "] = " << ipet::WCET(workspace()) << " cycles\n";

		// ILP dump
		if(ilp_dump) {
			ilp::System *sys = ipet::SYSTEM(workspace());
			sys->dump(out);
		}

		// display statistics
		if(display_stats) {
			bool found = false;

			// traverse all stats
			for(StatInfo::Iter stat(workspace()); stat; stat++) {
				found = true;
				Summer s;
				stat->collect(s);
				int t = stat->total();
				cerr << stat->name() << ": " << s.sum()
					 << " / " << t << ' ' << stat->unit()
					 << " (" << (float(s.sum()) / t * 100) << "%)\n";
			}

			// no state message
			if(!found)
				cerr << "No statistics to display.\n";

			//StatInfo *info = StatInfo::ID(workspace());
			//if(!info)
			//else {
			//const CFGCollection *coll = otawa::INVOLVED_CFGS(workspace());
		}
	}

private:
	option::StringList params;
	option::StringOption script;
	option::SwitchOption ilp_dump;
	option::SwitchOption list;
	option::SwitchOption timed;
	string bin, task;
	option::SwitchOption display_stats;
};

int main(int argc, char **argv) {
	OWCET owcet;
	return owcet.run(argc, argv);
}
