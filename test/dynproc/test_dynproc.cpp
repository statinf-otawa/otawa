/*
 *	$Id$
 *	ProcessorPlugin test
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS <casse@irit.fr>
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

#include <elm/io.h>
#include <elm/options.h>
#include <otawa/app/Application.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/proc/Registry.h>
#include <otawa/proc/DynProcessor.h>

using namespace otawa;
using namespace elm;

class DynProc: public Application {
public:
	DynProc(void):
		Application("dynproc", Version(1, 0, 0)),
		do_plugin(*this, 'n', "plugin", "get the plugin", false),
		do_proc(*this, 'p', "processor", "get the processor", false),
		do_feature(*this, 'f', "feature", "get the feature", false),
		do_identifier(*this, 'i', "identifier", "get identifier", false),
		do_list(*this, 'l', "list", "list available processors", false),
		do_bad_feature(*this, 'b', "bad", "test for a bad feature", false)
		{ }

	virtual void work(const string &entry, PropList &props) throw (elm::Exception) {

		// test plugin
		if(do_plugin) {
			ProcessorPlugin *plug = ProcessorPlugin::get("dyn");
			if(plug)
				cout << "SUCCESS: got plugin";
			else
				cout << "FAILURE: cannot find plugin !\n";
		}

		// test processor
		if(do_proc) {
			try {
				DynProcessor proc("dyn::DynProcessor");
				cerr << "SUCCESS: got processor\n";
				proc.process(workspace(), props);
			}
			catch(ProcessorNotFound& e) {
				cerr << "FAILURE: cannot find processor !\n";
			}
		}

		// test identifier
		if(do_identifier) {
			AbstractIdentifier *id = ProcessorPlugin::getIdentifier("dyn::ID");
			if(!id)
				cerr << "FAILURE: cannot find the identifier\n";
			else
				cerr << "SUCCESS: identifier found !\n";
		}

		// test feature
		if(do_feature) {
			try {
				DynFeature feature("dyn::FEATURE");
				cerr << "SUCCESS: feature found !\n";
				require(feature);
			}
			catch(FeatureNotFound& e) {
				cerr << "FAILURE: cannot find the feature\n";
			}
		}

		// test bad feature
		if(do_bad_feature) {
			AbstractFeature *id = ProcessorPlugin::getFeature("dyn::ID");
			if(!id)
				cerr << "SUCCESS: bad feature not found\n";
			else
				cerr << "FAILURE: bad feature found !\n";
		}

		// list processors
		if(do_list) {
			bool one = false;
			cerr << "PROCESSORS\n";
			for(Registry::Iter reg; reg; reg++) {
				cerr << "PROCESSOR " << reg->name() << io::endl;
				one = true;
			}
			if(!one)
				cerr << "NO PROCESSOR\n";
		}
	}

private:
	option::BoolOption do_plugin, do_proc, do_feature, do_identifier, do_list, do_bad_feature;
};

int main(int argc, char **argv) {
	DynProc app;
	return app.run(argc, argv);
}

