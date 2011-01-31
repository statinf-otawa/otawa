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

using namespace otawa;

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
	flowfacts(*this, option::cmd, "-f", option::cmd, "--flowfacts", option::help, "use the given to get flowfacts information", option::arg_desc, "PATH", option::end)
	{ }

protected:
	virtual void work (const string &entry, PropList &props) throw(elm::Exception) {

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

		// process flow facts
		if(flowfacts)
			FLOW_FACTS_PATH(props) = Path(flowfacts);

		// launch the script
		//Processor::VERBOSE(props) = true;
		TASK_ENTRY(props) = entry;
		script::PATH(props) = *script;
		script::Script scr;
		scr.process(workspace(), props);

		// display the result
		otawa::time_t wcet = ipet::WCET(workspace());
		if(wcet == -1)
			cout << "ERROR: no WCET computed (see errors above).\n";
		else
			cout << "WCET[" << entry << "] = " << ipet::WCET(workspace()) << " cycles\n";
	}

private:
	option::StringList params;
	option::StringOption script;
	option::ValueOption<string> flowfacts;
	string bin, task;
};

int main(int argc, char **argv) {
	OWCET owcet;
	owcet.run(argc, argv);
}
