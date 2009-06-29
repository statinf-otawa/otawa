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
	params(*this, 'p', "param", "parameter passed to the script", "IDENTIFIER=VALUE")
	{ }

protected:
	virtual void process(string arg) {

	}

private:
	option::StringList params;
	string script, bin, task;
};

int main(int argc, char **argv) {
	OWCET owcet;
	owcet.run(argc, argv);
}
