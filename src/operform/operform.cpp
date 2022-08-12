/*
 *	operform command
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
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

#include <elm/util/Cleaner.h>
#include <elm/sys/StopWatch.h>
#include <otawa/app/CFGApplication.h>
#include <otawa/cfg/features.h>
#include <otawa/cfgio/Output.h>
#include <otawa/display/CFGOutput.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/prog/WorkSpace.h>

using namespace elm;
using namespace otawa;

class OPerform: public CFGApplication {
public:
	OPerform(void):
		CFGApplication(Make("operform", Version(1, 0, 0))
			.description("perform a set of analysis (feature or code processor) and dump the resulting CFG collection.")
			.free_argument("EXECUTABLE ENTRY? (require:FEATURE|process:PROCESSOR)*")),
		ids(option::ListOption<cstring>::Make(*this).cmd("-p").cmd("--prop").description("select which property to output").argDescription("ID")),
		out(option::ValueOption<string>::Make(*this).cmd("-o").cmd("--out").description("select output file").argDescription("PATH")),
		no_insts(option::Switch::Make(*this).cmd("-I").cmd("--no-insts").description("do not include instructions in output")),
		dot(option::Switch::Make(*this).cmd("-D").cmd("--dot").description("select .dot output")),
		xml(option::Switch::Make(*this).cmd("-X").cmd("--xml").description("dump the CFG result in XML")),
		timed(option::Switch::Make(*this).cmd("-T").cmd("--timed").description("display the run time of each require: or process:")),
		cfg(option::Switch::Make(*this).cmd("-G").cmd("--cfg").description("output the CFG after the application of requirements and processings.")),
		view(option::ValueOption<string>::Make(*this).cmd("-V").cmd("--view").description("display the given view").argDescription("view"))
	{
	}

protected:

	virtual void prepare(PropList &props) {
		for(int i = 0; i < ids.count(); i++)
			cfgio::INCLUDE(props).add(ids[i].toCString());
		if(no_insts)
			cfgio::NO_INSTS(props) = true;
	}

	virtual void work(PropList& props) {
		const Vector<string> &args = arguments();

		for(int i = 0; i < args.count(); i++) {
			string a = args[i];

			if(a.startsWith("require:")) {
				setTask(props, "main");
				string n = a.substring(8);
				AbstractFeature *f = ProcessorPlugin::getFeature(n);
				if(!f)
					throw otawa::Exception(_ << "cannot find feature " << n);
				else {
					sys::StopWatch sw;
					sw.start();
					workspace()->require(*f, props);
					sw.stop();
					if(timed)
						cout << f->name() << ": " << sw.delay().millis() <<  io::endl;
				}
			}
			else if(a.startsWith("process:")) {
				setTask(props, "main");
				string n = a.substring(8);
				Processor *p = ProcessorPlugin::getProcessor(n);
				if(!p)
					throw otawa::Exception(_ << "cannot find processor " << n);
				else {
					sys::StopWatch sw;
					sw.start();
					workspace()->run(p, props);
					sw.stop();
					if(timed)
						cout << p->name() << ": " << sw.delay() <<  io::endl;
				}
			}
			else if(!setTask(props, a)){
				cerr << "WARNING: don't know what to do with: " << a << ". Ignoring it.\n";
				cerr << "Maybe you have forgot to specify \'require\' or \'process\' for one of the feature/processor?\n";
			}
		}

	}

	void complete(PropList& props) {
		if(!workspace()->isProvided(COLLECTED_CFG_FEATURE))
			workspace()->require(COLLECTED_CFG_FEATURE, props);

		// XML output
		if(xml) {
			if(out)
				cfgio::OUTPUT(props) = *out;
			workspace()->run<cfgio::Output>(props);
		}

		// DOT output
		if(dot) {
			if(out)
				display::CFGOutput::PATH(props) = *out;
			if(view != "") {
				workspace()->require(view::BASE_FEATURE, props);
				auto v = view::BASE_FEATURE.get(workspace())->find((*view).asNullTerminated());
				if(v == nullptr) {
					cerr << "ERROR: cannot find view " << *view << io::endl;
					return;
				}
				display::CFGOutput::VIEW(props) = v;
			}
			display::CFGOutput::KIND(props) = display::OUTPUT_DOT;
			workspace()->run<display::CFGOutput>(props);			
		}
		completeTask();
	}

private:
	bool setTask(PropList& props, string entry) {
		if(task)
			return false;
		else {
			task = entry;
			startTask(entry);
			prepareCFG(entry, props);
			return true;
		}
	}

	string task;
	option::ListOption<string> ids;
	option::ValueOption<string> out;
	option::Switch no_insts;
	option::Switch dot, xml, timed, cfg;
	option::ValueOption<string> view;
	CleanList clean;
};

OTAWA_RUN(OPerform);
