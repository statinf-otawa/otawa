/*
 *	$Id$
 *	Script processor interface
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
#ifndef OTAWA_SCRIPT_SCRIPT_H_
#define OTAWA_SCRIPT_SCRIPT_H_

#include <elm/system/Path.h>
#include <otawa/proc/Processor.h>
#include <elm/util/ErrorHandler.h>

namespace elm { namespace xom { class Node; } }

namespace otawa { namespace script {

// Script class
class Script: public Processor, public ErrorHandler {
public:

	Script(void);
	virtual void configure (const PropList &props);
	static Registration<Script> reg;

	virtual void onError(error_level_t level, const string &message);

protected:
	virtual void processWorkSpace(WorkSpace *fw);

private:
	void work(WorkSpace *ws);
	void onError(xom::Node *node, const string& msg);
	void onWarning(xom::Node *node, const string& msg);
	void makeConfig(xom::Element *elem, PropList& props);
	elm::system::Path path;
	PropList props;
};

// script path
extern Identifier<elm::system::Path> PATH;
extern Identifier<Pair<string, string> > PARAM;

} } // otawa::script

#endif /* OTAWA_SCRIPT_SCRIPT_H_ */
