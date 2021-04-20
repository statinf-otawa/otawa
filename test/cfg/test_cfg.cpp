/*
 *	Test file for CFG features
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2018, IRIT UPS.
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

#include <elm/data/HashSet.h>
#include <elm/test.h>
#include <otawa/cfg.h>
#include <otawa/app/Application.h>
#include <otawa/cfg/Loop.h>

using namespace elm;
using namespace otawa;

class CFGTest: public Application {
public:
	CFGTest(void): Application(Make("CFGTest")) {
	}

	virtual void work(const string& entry, PropList &props) {
		require(COLLECTED_CFG_FEATURE);
		require(otawa::LOOP_INFO_FEATURE);

		for(auto g: **otawa::INVOLVED_CFGS(workspace())) {
			cout << "CFG " << g << io::endl;
			for(auto v: *g) {

				cout << "\t" << v << io::endl;
				cout << "\t\tSUCCS: ";
				for(auto w: SUCCS(v))
					cout << w << " ";
				cout << "\n";

				cout << "\t\tPREDS: ";
				for(auto w: PREDS(v))
					cout << w << " ";
				cout << "\n";

				if(LOOP_HEADER(v)) {

					cout << "\t\tBACK_EDGES: ";
					for(auto e: BACK_EDGES(v))
						cout << e << " ";
					cout << io::endl;

					cout << "\t\tENTRY_EDGES: ";
					for(auto e: ENTRY_EDGES(v))
						cout << e << " ";
					cout << io::endl;

					cout << "\t\tEXIT_EDGES: ";
					for(auto e: EXIT_EDGES(v))
						cout << e << " ";
					cout << io::endl;

				}
			}
		}
	}
	
	void compilation_test(Block *v) {
		auto l = Loop::of(v);
		for(auto e: l->entries())
			cout << e << io::endl;
		for(auto e: l->backs())
			cout << e << io::endl;
	}

};

OTAWA_RUN(CFGTest)
