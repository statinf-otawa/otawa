/*
 *	$Id$
 *	Script processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009-12, IRIT UPS.
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

#include <elm/data/Vector.h>
#include <elm/sys/Path.h>
#include <otawa/proc/Processor.h>
#include <elm/util/ErrorHandler.h>

namespace elm { namespace xom {
	class Node;
	class Element;
	class XSLTransform;
} }

namespace otawa { namespace script {

// ScriptItem class
class ScriptItem {
public:
	typedef enum {
		T_BOOL = 0,
		T_INT = 1,
		T_STRING = 2,
		T_RANGE = 3,
		T_ENUM = 4,
		T_MAX = 5
	} type_t;

	static cstring type_labels[];

	static ScriptItem *parse(xom::Element& elt);

	virtual ~ScriptItem(void);
	virtual string makeParam(const string& value);

	string name;
	type_t type;
	string deflt;
	string label;
	string help;
	bool multi;

protected:
	ScriptItem(type_t t, xom::Element& elt);
};


// Script class
class Script: public Processor, public ErrorHandler {
public:

	Script(void);
	virtual void configure (const PropList &props);
	static p::declare reg;

	virtual void onError(error_level_t level, const string &message);

	class ItemIter: public Vector<ScriptItem *>::Iter {
	public:
		inline ItemIter(Script& script)
			: Vector<ScriptItem *>::Iter(script.items) { }
	};

	inline int version(void) const { return _version; }

protected:
	virtual void processWorkSpace(WorkSpace *fw);

private:
	void work(WorkSpace *ws);
	void declareGlobals(xom::XSLTransform& trans);
	void onError(xom::Node *node, const string& msg);
	void onWarning(xom::Node *node, const string& msg);
	void makeConfig(xom::Element *elem, PropList& props);
	elm::sys::Path path;
	PropList props;
	Vector<ScriptItem *> items;
	bool only_config, timed;
	int _version;
};

// script path
extern Identifier<elm::sys::Path> PATH;
extern Identifier<Pair<string, string> > PARAM;
extern Identifier<xom::Element *> SCRIPT;
extern Identifier<xom::Element *> PLATFORM;
extern Identifier<bool> ONLY_CONFIG;
extern Identifier<bool> TIME_STAT;

} } // otawa::script

#endif /* OTAWA_SCRIPT_SCRIPT_H_ */
