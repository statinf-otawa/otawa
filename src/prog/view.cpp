/*
 *	view module implementation
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

#include <otawa/cfg/features.h>
#include <otawa/display/display.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/view/features.h>

namespace otawa { namespace view {

/**
 * @degroup view	View Module
 *
 * @code
 * #include <otawa/view/features.h>
 * @endcode
 *
 * The view modules aims to provide different view of the program
 * according to the different program representations and properties
 * calculated by the applied analysis. The objective is to support
 * automatic display of analyzes results from OTAWA tools like
 * @ref operform.
 *
 * When a program domain (instructions, cache accesses, etc)
 * is interfaced with the @ref view module, it can be automatically surveyed
 * or explored by human user. This may help to debug the corresponding analysis
 * or to get a better understanding of the work of the program relatively
 * to the concerned address.
 *
 * @ref main module is made of:
 * * a set of classes to declare views and viewed properties (@ref View, @ref PropertyType),
 * * a set of classes to visit the view and properties (@ref Viewer, @ref PropertyViewer),
 * * a class, @ref Manager, to collect all available views.
 */


/**
 * @class Named
 * This class represents an identified object with a unique name
 * and a name displayed to the user, called a label.
 * @ingroup view
 */

/**
 */
Named::Named(cstring name, string label): AbstractIdentifier(name), _label(label) {
}

/**
 * @fn string Named::label(void) const;
 * Get the label of an object.
 * @return	Label.
 */


/**
 * @class Viewer
 * This class provides a way to explore a view of the program, that is, an aspect
 * of the work of the program. This may the instructions composing the program,
 * the accesses to the caches, the semantic instructions, etc.
 *
 * Basically, this class is a re-startable iterator: the @ref start() is called
 * to explore a basic block or an edge and then each instruction be explored
 * in turn. In parallel, the instantiated values can be also displayed.
 *
 * A good to implement this interface is to use existing iterators
 * on the corresponding program representation.
 *
 * @ingroup view
 */

/**
 */
Viewer::Viewer(WorkSpace *ws, const Vector<PropertyType *>& types): _ws(ws), _props(types.count()) {
	for(auto t: types)
		_props.add(t->visit());
}

/**
 */
Viewer::~Viewer(void) {
	for(auto t: _props)
		delete t;
}

/**
 * @fn void Viewer::start(Block *b);
 * Start exploring a block.
 * @param b		Block to explore.
 */

/**
 * @fn void Viewer::start(Edge *e);
 * Start exploring an edge.
 * @param e		Edge to explore.
 */

/**
 * @fn Address Viewer::item(void) const;
 * Get the address of the current instruction.
 * @return	Current instruction address.
 */

/**
 * @fn void Viewer::next(void);
 * Move to the next instruction.
 */

/**
 * @fn bool Viewer::ended(void) const;
 * Test if the current traversal is ended.
 * @return	True if the traversal is ended, false else.
 */

/**
 * @fn void Viewer::print(io::Output& out) const;
 * Display the current instruction.
 * @param out	Stream to output to.
 */

/**
 * Print the viewed element in a formatted style.
 * Default implement call print(io::Output&) version.
 * @param out	Formatetd output to display to.
 */
void Viewer::print(display::Text& out) {
	print(out.out());
}

/**
 * Print the required property.
 * @param type	Type of property to display.
 * @param out	Stream to output to.
 */
void Viewer::print(PropertyType *type, io::Output& out) const {
	for(auto pv = *_props; pv(); pv++)
		if(pv->type() == type) {
			pv->print(out);
			break;
		}
}

/**
 * Print the required property on a formatted stream.
 * @param type	Type of property to display.
 * @param out	Formatted output to display to.
 */
void Viewer::print(PropertyType *type, display::Text& out) const {
	for(auto pv = *_props; pv(); pv++)
		if(pv->type() == type) {
			pv->print(out);
			break;
		}
}

/**
 * Get the property viewer corresponding to a property type.
 * @param type	Looked property type.
 * @return		Found property viewer or null.
 */
const PropertyViewer *Viewer::property(PropertyType *type) const {
	for(auto pv = *_props; pv(); pv++)
		if(pv->type() == type)
			return *pv;
	return nullptr;
}


/**
 * @class PropertyViewer
 * This class is in charge of displaying some property of the program.
 * It is controlled by a @ref Viewer that makes it evolving along
 * the evolution of the viewer.
 *
 * @ingroup view
 */

/**
 * @fn PropertyType *PropertyViewer::type(void) const;
 * Get the type of the property viewer.
 * @return	Property viewer.
 */

/**
 */
PropertyViewer::~PropertyViewer(void) {
}

/**
 * @fn void PropertViewer::print(io::Output& out);
 * Print the current property value.
 * @param out	Stream to output to.
 */
void PropertyViewer::print(display::Text& out) {

}

/**
 */
PropertyViewer::PropertyViewer(PropertyType *type): _type(type) {
}


/**
 * @fn void PropertyViewer::start(Block *b);
 * Start the property exploration for the given block.
 * @param b		Block to explore.
 */

/**
 * @fn void PropertyViewer::start(Edge *e);
 * Start the property exploration for the given edge.
 * @param e		Edge to explore.
 */

/**
 * @fn void PropertyViewer::step(const Viewer& viewer) const;
 * Perform one step in exploration.
 * @param Current viewer.
 */


/**
 * @class PropertyType
 * Type of a property that can be explored with a viewer.
 * @ingroup view
 */

/**
 * Build a property type.
 * @param view	View where the property applies.
 * @param name	Name of the view.
 * @param label	Label identifying the view to the human user.
 */
PropertyType::PropertyType(View& view, cstring name, string label): Named(name, label), _view(view) {
	view._props.add(this);
}

/**
 */
PropertyType::~PropertyType(void) {
	_view._props.remove(this);
}


/**
 * @fn bool PropertyType::isAvailable(WorkSpace *ws);
 * Test if the property is available in the given workspace.
 * @param ws	Workspace to test in.
 * @return		True if the property is available, false else.
 */

/**
 * @fn PropertyViewer *PropertyType::visit(void);
 * This function is called to get a viewer on the values of this property.
 * @return	Viewer for the property.
 */

/**
 * @class View
 * A view provides an aspect of the program to display. The analyzes applied in OTAWA
 * often based on abstract interpretation does not consider all aspects of the execution
 * together but only survey one. For example, the cache analysis us only interested
 * on memory accesses while the block time analysis is only interested on execution
 * of instructions in the pipeline. This class provides such a view of the program.
 * On a particular view, you can have one or several analysis providing dedicated
 * properties that recorded in the view.
 */

/**
 * Build a view that records itself to the view manager (notice that the @ref FEATURE
 * must be required before creating a view).
 */
View::View(cstring name, string label): Named(name, label) {
}

/**
 */
View::~View(void) {
}

/**
 */
//rtti::Class<View, Named, rtti::no_inst> View::__type("otawa::view::View");

/**
 */
/*const rtti::Type& View::getType(void) const {
	return __type;
}*/


/**
 * @fn List<PropertyType *>::Iter View::types(void) const;
 * Get the list of available properties in the current view.
 * @return	Viewable properties.
 */

/**
 * Get a re-startable view explorator for the given set of properties.
 */
Viewer *View::explore(WorkSpace *ws, const Vector<PropertyType *>& types) {
	return nullptr;
}


/**
 * @class Manager
 * Manager of views. Provides mainly the sollection of available views.
 */

/**
 * Get the manager for the given workspace.
 * @param ws	Work space to get manager from.
 * @return		Found manager or null pointer.
 */
Manager *Manager::get(WorkSpace *ws) {
	return *MANAGER(ws);
}


/**
 */
Manager::Manager(WorkSpace *ws): _ws(ws) {
}


/**
 * Add a view to the view manager of the given workspace.
 * This only function can only be called once the otawa::view::FEATURE
 * is provided.
 *
 * @param ws	Workspace to add view to.
 * @param view	To add.
 */
void Manager::add(WorkSpace *ws, View *view) {
	Manager *man = get(ws);
	ASSERTP(man != nullptr, "Can only be called once the view::FEATURE is provided!\n");
	man->_views.add(view);
}


/**
 * Remove a view from the workspace.
 * Notice this method quietly ignore if the view is not in the workspace.
 * @param ws	Workspace to remove from.
 * @param view	Removed view.
 */
void Manager::remove(WorkSpace *ws, View *view) {
	Manager *man = get(ws);
	ASSERTP(man != nullptr, "Can only be called once the view::FEATURE is provided!\n");
	man->_views.remove(view);
}


/**
 * Find a view b its name in a workspace.
 * @param ws	Workspace to look in.
 * @param name	Looked view name.
 * @return		Found view or null pointer.
 */
View *Manager::find(WorkSpace *ws, string name) {
	for(auto v: get(ws)->_views)
		if(v->name() == name)
			return v;
	return nullptr;
}


/**
 * @fn List<View *>::Iter Manager::views(void) const;
 * Get the views available in the manager.
 * @return	List of views.
 */

class Maker: public Processor {
public:
	static p::declare reg;
	Maker(void): Processor(reg) { }

protected:

	void processWorkSpace(WorkSpace *ws) override {
		MANAGER(ws) = new Manager(ws);
	}

	void destroy(WorkSpace *ws) override {
		delete MANAGER(ws);
		MANAGER(ws).remove();
	}

};

/**
 */
p::declare Maker::reg = p::init("otawa::view::Maker", Version(1, 0, 0))
	.provide(FEATURE)
	.make<Maker>();


/**
 * Feature ensuring there is a view manager.
 *
 * @par Properties
 *	* MANAGER
 */
p::feature FEATURE("otawa::view::FEATURE", p::make<Maker>());

/**
 * Property to set the manager of views.
 *
 * @par Feature
 *	* FEATURE
 *
 * @par Hooks
 *	* WorkSpace
 */
p::id<Manager *> MANAGER("otawa::view::MANAGER", nullptr);

} }	// otawa::view

