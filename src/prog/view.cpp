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

#include <elm/data/ListMap.h>
#include <otawa/cfg/features.h>
#include <otawa/display/display.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/prog/features.h>
#include <otawa/view/features.h>

namespace otawa { namespace view {

/**
 * @defgroup view	View Module
 *
 * @code
 * #include <otawa/view/features.h>
 * @endcode
 *
 * * a class, @ref Manager, to collect all available views.
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
 * 
 * @ingroup view
 */

///
View::View(cstring name, cstring label, cstring description )
	: _name(name), _label(label), _desc(description)
		{  }

/**
 * cstring View::name() const;
 * Get the name of the view.
 * @return View name.
 */

/**
 * @fn cstring View::label() const;
 * Get the human-readable name of the view;
 * @return	Human readable label;
 */

/**
 * @fn cstring View::description() const;
 * Get the description of the view.
 * @return	View description.
 */

/**
 * @fn void View::start(BasicBlock *b);
 * Start the traversal of the block b for the view.
 * @param b		Block to explore.
 */

/**
 * @fn Address View::item() const;
 * Get the instruction supporting the current viewed item;
 * @return	Current viewed item instruction;
 */

/**
 * @fn void View::next();
 * Pass to the next viewed item.
 */

/**
 * @fn bool View::ended() const;
 * Test if all items of the current block has been traversed.
 * @return	True if all items are traversed, false else.
 */

/**
 * @fn void View::print(io::Output& out);
 * Print the current viewed item.
 * @param out	Stream to output to (non formatted).
 */

/**
 * Print the current viewed item in a formatted way.
 * @param out 	Formatted output to write to;
 */
void View::print(display::Text& out) {
	print(out.out());	
}

///
View::~View() { }


/**
 * @class ViewBase
 * ViewBase is an interface allowing to store all availaible views and
 * to retrieve them;
 * 
 * Provied by @ref otawa::view::BASE_FEATURE.
 * 
 * @ingroup view
 */

///
ViewBase::~ViewBase() {}

/**
 * @fn void add(View *view);
 * Add a view to the base.
 * @param view		Added view;
 */

/**
 * @fn void remove(View *view);
 * Remove a view from the base.
 * @param view 		Removed view.
 */

/**
 * @fn View *find(cstring name);
 * Find a view by its name.
 * @param name	Looked name.
 * @return		Found view or a null pointer.
 */

/**
 * @fn const List<View *>& views();
 * Get the list of views.
 * @return	Iterator on the views.
 */


/**
 * @ingroup view
 */
class ViewBaseImpl: public Processor, public ViewBase {
public:
	static p::declare reg;
	ViewBaseImpl(p::declare& r = reg): Processor(r) {}
	
	void *interfaceFor(const otawa::AbstractFeature& feature) override {
		if(&feature == &BASE_FEATURE)
			return static_cast<ViewBase *>(this);
		else
			return nullptr;
	}
	
	void add(View *view) override {
		_views.add(view);
		_map.put(view->name(), view);
	}
	
	void remove(View *view) override {
		_views.remove(view);
		_map.remove(view->name());
	}
	
	View *find(cstring name) override {
		return _map.get(name, nullptr);
	}
	
	const List<View *>& views() override {
		return _views;
	}

private:
	List<View *> _views;
	ListMap<cstring, View *> _map;
	
};

///
p::declare ViewBaseImpl::reg
	= p::init("otawa::view::ViewBaseImpl", Version(1, 0, 0))
	.provide(BASE_FEATURE)
	.make<ViewBaseImpl>();


/**
 * This feature ensures that the base for viewers is set up.
 * 
 * Default implementation: @ref ViewBaseImpl.
 * 
 * @ingroup view
 */
p::interfaced_feature<ViewBase> BASE_FEATURE(
	"otawa::view::BASE_FEATURE",
	p::make<ViewBaseImpl>());


/**
 * @ingroup view
 */
class Dumper: public Processor {
public:
	static p::declare reg;
	Dumper(p::declare& r = reg): Processor(r) {}

	void configure(const PropList& props) override {
		Processor::configure(props);
		path = DUMP_PATH(props);
	}

protected:

	void setup(WorkSpace * ws) override {
		if(path.isEmpty())
			path = TASK_INFO_FEATURE.get(ws)->workDirectory();
	}
	
	void processWorkSpace(WorkSpace *ws) override {
		
	// cleanup old views
	auto path = TASK_INFO_FEATURE.get(ws)->workDirectory();
	Vector<string> to_remove;
	for(auto f: path.readDir())
		if(f.endsWith("-view.csv"))
			to_remove.add(f);
	for(auto f: to_remove)
		(path / f).remove();
		
		
		// dump the views
		for(auto view: BASE_FEATURE.get(ws)->views())
			dumpView(*view);
	}
	
private:
	
	void dumpView(View& view) {
		
		// open the file
		auto p = path / (view.name() + "-view.csv");
		auto s = p.write();
		io::Output out(*s);
		
		// dump definition
		out << "#Name: " << view.name() << io::endl;
		if(view.label())
			out << "#Label: " << view.label() << io::endl;
		if(view.description())
			out << "#Description: " << string(view.description()).replace("\n", " ") << io::endl;
		
		// perform the dump
		for(auto g: *COLLECTED_CFG_FEATURE.get(workspace())) {
			for(auto v: *g)
				if(v->isBasic()) {
					auto bb = v->toBasic();
					view.start(bb);
					while(!view.ended()) {
						out << g->index() << '\t'
							<< v->index() << '\t'
							<< view.item()->address() << '\t';
						view.print(out);
						out << io::endl;
						view.next();
					}
				}
		}
		
		// close the file
		delete s;
	}
	
	Path path;
};

///
p::declare Dumper::reg
	= p::init("otawa::view::Dumper", Version(1, 0, 0))
	.provide(DUMP_FEATURE)
	.require(COLLECTED_CFG_FEATURE)
	.require(BASE_FEATURE)
	.make<Dumper>();


/**
 * Configuration fo @ref DUMP_FEATURE.
 * Select the directory to dump files to.
 * @ingroup view
 */
p::id<Path> DUMP_PATH("otawa::view::DUMP_PATH");

/**
 * This features forces the dump of views to disk, by default to the task
 * workinf directory, or to the one passed to @ref DUMP_PATH.
 * 
 * @par implementations
 * * @ref Dumper (default)
 * 
 * @par Configuration
 * * @ref DUMP_PATH
 * 
 * @ingroup view
 */
p::feature DUMP_FEATURE("otawa::view::DUMP_FEATURE", p::make<Dumper>());


} }	// otawa::view
