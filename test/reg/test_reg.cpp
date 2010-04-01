/*
 *	$Id$
 *	Registration test program
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008-10, IRIT UPS.
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
#include <otawa/proc/Registry.h>
#include <otawa/ipet.h>

using namespace elm;
using namespace otawa;

Identifier<bool> ID_BOOL("");
Identifier<char> ID_CHAR("");
Identifier<signed char> ID_SCHAR("");
Identifier<unsigned char> ID_UCHAR("");
Identifier<cstring> ID_CSTRING("");
Identifier<string> ID_STRING("");

int main(void) {
	ipet::VarAssignment var;
	
	// Display registrations
	for(Registry::Iter reg; reg; reg++) {
		cout << "REGISTRATION " << reg->name() << " " << reg->version() << io::endl;
		
		// base
		AbstractRegistration *base = &reg->base();
		if(base)
			cout << "\tbase = " << base->name() << " V" << base->version() << io::endl;

		// Configuration
		cout << "\tconfiguration\n";
		for(ConfigIter config(**reg); config; config++)
			cout << "\t\t" << config->name() << ": "
				 << config->type() << io::endl;
		
		// Features
		cout << "\tfeatures\n";
		for(FeatureIter feature(**reg); feature; feature++) {
			cout << "\t\t";
			switch((*feature)->kind()) {
			case FeatureUsage::none: ASSERT(false);
			case FeatureUsage::require: cout << "require "; break;
			case FeatureUsage::provide: cout << "provide "; break;
			case FeatureUsage::invalidate: cout << "invalidate "; break;
			case FeatureUsage::use: cout << "use "; break;
			}
			cout << (*feature)->feature().name() << io::endl;
		}
		
		// Existence test
		const AbstractRegistration *found = Registry::find(&reg->name());
		cout << "\treg = " << (void *)*reg << ", found = " << (void *)found << "\n\n";
	}
	
	
	// Bad find
	const AbstractRegistration *found = Registry::find("oksdfsdfsdf");
	cout << "bad found = " << (void *)found << io::endl;
	
	// Display identifiers types
	cout << "ID_BOOL: " << ID_BOOL.type() << io::endl;
	cout << "ID_CHAR: " << ID_CHAR.type() << io::endl;
	cout << "ID_UCHAR: " << ID_UCHAR.type() << io::endl;
	cout << "ID_SCHAR: " << ID_SCHAR.type() << io::endl;
	cout << "ID_CSTRING: " << ID_CSTRING.type() << io::endl;
	cout << "ID_STRING: " << ID_STRING.type() << io::endl;
}
