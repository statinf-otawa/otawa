/*
 *	otawa::XSLTScript class interface
 *	Script processor implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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
#ifndef OTAWA_XSLTSCRIPT_H_
#define OTAWA_XSLTSCRIPT_H_

#include <elm/xom.h>
#include <elm/data/Vector.h>
#include <elm/sys/Path.h>
#include <elm/io/Monitor.h>

#include "../prop.h"

namespace otawa {

using namespace elm;

class XSLTScript: private io::Monitor {
public:
	XSLTScript(const elm::sys::Path& path, Monitor& monitor = elm::monitor, bool verbosity = false);
	virtual ~XSLTScript(void);
	xom::Document *document(void) const { return doc; }
	void setConfiguration(cstring name = "configuration");
	void setConfiguration(xom::Element *element);
	void addParam(string name, String value);
	void addParams(const PropList& props, Identifier<string>& id);
	void addParams(const PropList& props, Identifier<Pair<string, string> >& id);
	void transform(void);
	void fillProps(PropList& props, xom::Element *element);
	void fillProps(PropList& props, cstring name);

private:
	void onWarning(xom::Node *node, const string& msg);
	void onError(xom::Node *node, const string& msg);

		xom::Document *doc, *xsl;
	xom::Element *temp;
	enum {
		ERROR,
		OPENED,
		TRANSFORMED
	} state;
	Vector<Pair<string, string> > params;
	bool configured;
	bool verbose;
};

} // otawa


#endif /* OTAWA_XMLSCRIPT_H_ */
