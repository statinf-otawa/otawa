/*
 * operform.cpp
 *
 *  Created on: 27 nov. 2015
 *      Author: casse
 */

#include <otawa/app/Application.h>
#include <otawa/cfg/features.h>
#include <otawa/cfgio/Output.h>
#include <otawa/display/CFGOutput.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/prog/WorkSpace.h>

using namespace elm;
using namespace otawa;

class OPerform: public Application {
public:
	OPerform(void):
		Application("operform", Version(0, 1, 0)),
		ids(option::ListOption<cstring>::Make(*this).cmd("-p").cmd("--prop").description("select which property to output").argDescription("ID")),
		out(option::ValueOption<string>::Make(*this).cmd("-o").cmd("--out").description("select output file").argDescription("PATH")),
		no_insts(option::Switch::Make(*this).cmd("-I").cmd("--no-insts").description("do not include instructions in output")),
		dot(option::Switch::Make(*this).cmd("-D").cmd("--dot").description("select .dot output"))
	{
		info.description("perform a set of analysis (feature or code processor) and dump the resulting CFG collection.");
		info.free_argument("EXECUTABLE ENTRY? (require:FEATURE|process:PROCESSOR)*");
	}

protected:

	virtual void prepare(PropList &props) {
		for(int i = 0; i < ids.count(); i++)
			cfgio::INCLUDE(props).add(&ids[i]);
		if(no_insts)
			cfgio::NO_INSTS(props) = true;
	}

	virtual void work(PropList& props) throw(elm::Exception) {
		const genstruct::Vector<string> &args = arguments();

		for(int i = 0; i < args.count(); i++) {
			string a = args[i];

			if(a.startsWith("require:")) {
				setTask(props, "main");
				string n = a.substring(8);
				AbstractFeature *f = ProcessorPlugin::getFeature(&n);
				if(!f)
					cerr << "WARNING: cannot find feature " << n << ". Action forgiven.\n";
				else
					workspace()->require(*f, props);
			}
			else if(a.startsWith("process:")) {
				setTask(props, "main");
				string n = a.substring(8);
				Processor *p = ProcessorPlugin::getProcessor(&n);
				if(!p)
					cerr << "WARNING: cannot find feature " << n << ". Action forgiven.\n";
				else {
					p->process(workspace(), props);
					delete p;
				}
			}
			else if(!setTask(props, a))
				cerr << "WARNING: don't know what to do with: " << a << ". Ignoring it.\n";

		}

	}

	void complete(PropList& props) throw(elm::Exception) {
		if(!workspace()->isProvided(COLLECTED_CFG_FEATURE))
			workspace()->require(COLLECTED_CFG_FEATURE, props);

		// XML output
		if(!dot) {
			if(out)
				cfgio::OUTPUT(props) = *out;
			cfgio::Output output;
			output.process(workspace(), props);
		}

		// DOT output
		else {
			if(out)
				display::CFGOutput::PATH(props) = *out;
			display::CFGOutput::KIND(props) = display::OUTPUT_DOT;
			display::CFGOutput output;
			output.process(workspace(), props);
		}
	}

private:
	bool setTask(PropList& props, string entry) {
		if(task)
			return false;
		else {
			task = entry;
			TASK_ENTRY(props) = entry;
			return true;
		}
	}

	string task;
	option::ListOption<string> ids;
	option::ValueOption<string> out;
	option::Switch no_insts;
	option::Switch dot;
};

OTAWA_RUN(OPerform);
