/*
 *	view module feature
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_DISPLAY_VIEW_H_
#define OTAWA_DISPLAY_VIEW_H_

#include <elm/string.h>
#include <elm/data/List.h>
#include <elm/data/Vector.h>
#include <otawa/base.h>
#include <otawa/proc/Feature.h>

namespace otawa {

class Block;
class Edge;
class WorkSpace;

namespace display {
	class Text;
};

namespace view {

using namespace elm;

class View;


class View: public PreIterator<View, Address> {
public:
	View(cstring name, cstring label = "", cstring description = "");
	inline cstring name() const { return _name; }
	inline cstring label() const { return _label; }
	inline cstring description() const { return _desc; }
	
	virtual void start(BasicBlock *b) = 0;
	virtual Inst *item() const = 0;
	virtual void next() = 0;
	virtual bool ended() const = 0;

	virtual void print(io::Output& out) = 0;
	virtual void print(display::Text& out);

protected:
	virtual ~View();
private:
	cstring _name, _label, _desc;
};


// view base
class ViewBase {
public:
	virtual ~ViewBase();
	virtual void add(View *view) = 0;
	virtual void remove(View *view) = 0;
	virtual View *find(cstring name) = 0;
	virtual const List<View *>& views() = 0;
};

// features
extern p::interfaced_feature<ViewBase> BASE_FEATURE;
extern p::id<Path> DUMP_PATH;
extern p::feature DUMP_FEATURE;

} }	// otawa::view

#endif /* OTAWA_DISPLAY_VIEW_H_ */
