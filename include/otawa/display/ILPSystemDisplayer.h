/*
 *	$Id$
 *	display::ILPSystemDisplayer class interface
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
#ifndef DISPLAY_ILPSYSTEMDISPLAYER_H_
#define DISPLAY_ILPSYSTEMDISPLAYER_H_

#include <otawa/proc/Processor.h>
#include <otawa/prop/Identifier.h>
#include <elm/system/Path.h>
#include <elm/io.h>
#include <elm/genstruct/HashTable.h>

// Extern classes
namespace otawa { namespace ilp {
	class Constraint;
	class System;
	class Var;
} }

namespace otawa { namespace display {

using namespace elm;

// ILPSystemAddon class
class ILPSystemDisplayer;
class ILPSystemAddon {
public:
	virtual cstring title(void) const = 0;
	virtual void display(io::Output& out, WorkSpace *ws, ILPSystemDisplayer *displayer) = 0;
};

// ILPSystemDisplayer class
class ILPSystemDisplayer: public Processor {
public:
	static Registration<ILPSystemDisplayer> reg;
	ILPSystemDisplayer(AbstractRegistration& r = reg);
	virtual void configure(const PropList& props);

	// Configuration
	static Identifier<Path> PATH;

	// Add-ons
	static Identifier<ILPSystemAddon *> ADDON;

	// Only for add-ons
	void displayCons(ilp::Constraint *cons, bool with_label = false);
	void displayVar(ilp::Var *var);
	string nameOf(ilp::Var *var);

protected:
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void setup (WorkSpace *fw);

private:
	elm::genstruct::HashTable<ilp::Var *, string> names;
	int cnt;
	ilp::System *system;
	io::Output cout;
	Path path;
};

} } // display


#endif /* DISPLAY_ILPSYSTEMDISPLAYER_H_ */
