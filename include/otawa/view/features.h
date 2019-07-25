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
class PropertyType;
class PropertyViewer;
class Manager;


class Named: public AbstractIdentifier {
public:
	Named(cstring name, string label = "");
	inline string label(void) const { return _label; }
private:
	string _label;
};

class Viewer: public PreIterator<Viewer, Address> {
	friend class View;
public:
	virtual ~Viewer(void);

	virtual void start(Block *b) = 0;
	virtual void start(Edge *e) = 0;
	virtual Address item(void) const = 0;
	virtual void next(void) = 0;
	virtual bool ended(void) const = 0;

	virtual void print(io::Output& out) = 0;
	virtual void print(display::Text& out);
	void print(PropertyType *type, io::Output& out) const;
	void print(PropertyType *type, display::Text& out) const;
	const PropertyViewer *property(PropertyType *type) const;

protected:
	Viewer(WorkSpace *ws, const Vector<PropertyType *>& props);
	inline WorkSpace *workspace(void) const { return _ws; }

private:
	WorkSpace *_ws;
	Vector<PropertyViewer *> _props;
};

class PropertyViewer {
	friend class Viewer;
public:
	inline PropertyType *type(void) const { return _type; }
	virtual ~PropertyViewer(void);

	virtual void print(io::Output& out) = 0;
	virtual void print(display::Text& out);

protected:
	PropertyViewer(PropertyType *type);
private:
	virtual void start(Block *b) = 0;
	virtual void start(Edge *e) = 0;
	virtual void step(const Viewer& it) = 0;
	PropertyType *_type;
};

class PropertyType: public Named {
	friend class View;
	friend class Viewer;
public:
	//static rtti::Class<View, Named> __type;
	//const rtti::Type& getType(void) const override;

	PropertyType(View& view, cstring name, string label = "");
	virtual ~PropertyType(void);
	virtual bool isAvailable(WorkSpace *ws) = 0;
private:
	virtual PropertyViewer *visit(void) = 0;
	View& _view;
};

class View: public Named {
	friend class Manager;
	friend class PropertyType;
public:
	//static rtti::Class<View, Named, rtti::no_inst> __type;
	//const rtti::Type& getType(void) const override;

	View(cstring name, string label = "");
	inline const List<PropertyType *>& types(void) const { return _props; }

	virtual Viewer *explore(WorkSpace *ws, const Vector<PropertyType *>& types);
protected:
	virtual ~View(void);
private:
	List<PropertyType *> _props;
};

class Manager {
	friend class View;
public:
	Manager(WorkSpace *ws);
	static Manager *get(WorkSpace *ws);
	static void add(WorkSpace *ws, View *view);
	static void remove(WorkSpace *ws, View *view);
	static View *find(WorkSpace *ws, string name);
	List<View *>::Iter views(void) const { return *_views; }
	inline WorkSpace *workspace(void) const { return _ws; }
private:
	List<View *> _views;
	WorkSpace *_ws;
};

inline List<View *>::Iter views(WorkSpace *ws) { return Manager::get(ws)->views(); }

// features
extern p::feature FEATURE;
extern p::id<Manager *> MANAGER;

} }	// otawa::view

#endif /* OTAWA_DISPLAY_VIEW_H_ */
