/*
 *	display::Displayer class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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

#include <otawa/display/Displayer.h>
//#include <otawa/display/GenDisplayer.h>
#include <otawa/proc/ProcessorPlugin.h>

namespace otawa { namespace display {

// extension corresponding to output mode
static cstring exts[] = {
		"dot"
		"ps",
		"pdf",
		"png",
		"gif",
		"jpg",
		"svg",
		"dot",
		"dot",
		"dot"
};


/**
 * @class Decorator
 * A decorator is used to produce the look of the different items composed
 * a graph. Its method will be called in turn to this job with the concerned
 * item as argument.
 *
 * As a default, its methods do nothing. To customize the graph output, one
 * has to create a class whose base class is @ref Decorator and to overload
 * methods for the concerned graph item.
 *
 * @ingroup display
 */

/**
 */
Decorator::~Decorator(void) {
}

/**
 * Call once for the graph to generate the graph caption.
 * @param graph		Displayed graph.
 * @param caption	Text to generate graph caption in.
 * @param style		Style to tune (only text member is used).
 */
void Decorator::decorate(graph::DiGraph *graph, Text& caption, GraphStyle& style) const {
}

/**
 * Called for each vertex to generate its look.
 * @param graph		Displayed graph.
 * @param vertex	Displayed vertex.
 * @param content	Text to generate vertex content in.
 * @param style		Style to tune (text, shape and fill members are used).
 */
void Decorator::decorate(graph::DiGraph *graph, graph::Vertex *vertex, Text& content, VertexStyle& style) const {
}

/**
 * Called for each edge to generate its look.
 * @param graph		Displayed graph.
 * @param edge		Displayed edge.
 * @param edge		Text to generate edge label in.
 * @param style		Style to tune (text and line members are used).
 */
void Decorator::decorate(graph::DiGraph *graph, graph::Edge *e, Text& label, EdgeStyle& style) const {
}

/**
 * @class Displayer
 * This class provides facility to output a graph of type @ref AbstractGraph.
 * Basically, this is only an interface for which different implementations exist
 * (usually placed in a plugin) and built from a @ref DisplayerProvider.
 *
 * It allows tuning the way the display is performed and to launch the
 * display process.
 *
 * @ingroup display
 */

/**
 */
Displayer::Displayer(graph::DiGraph *graph, const Decorator& decorator, output_mode_t out)
	: g(graph), d(decorator), _output(out), _layout(MAPPED)
{ }

/**
 */
Displayer::~Displayer(void) { }

/**
 * Obtain a displayer using a plug-in supporting the selected output type.
 * If the output type is any, return a displayer with the default plug-in.
 * @param g		Graph to display.
 * @param d		Decorator to use.
 * @param out	Output type (optional).
 */
Displayer *Displayer::make(graph::DiGraph *g, const Decorator& d, output_mode_t out) {
	return Provider::display(g, d, out);
}

/**
 * @fn void Displayer::process(void);
 * Perform the display of the configured graph with the given decorator.
 */

/**
 * @fn void Displayer::setOutput(output_t out);
 * Select the output mode.
 * @param out	Output mode to set.
 */

/**
 * Set the path of generated file. If the given file has no extension
 * add the extension corresponding to the mode.
 * @param p	Generated file path.
 */
void Displayer::setPath(const sys::Path& p) {
	_path = p;
	if(_path.extension() == "")
		_path = _path.setExtension(exts[_output]);
}

/**
 * @fn Style& Displayer::defaultVertex(void);
 * Expose the default style of vertices for tuning.
 * @return	Default vertex style.
 */

/**
 * @fn Style& Displayer::defaultEdge(void);
 * Expose the default style of edge for tuning.
 * @return	Default edge style.
 */


/**
 * @class DisplayProvider
 * An instance of this class must exist for each plugin providing
 * a particular displayer. DisplayProvider objects are stored in
 * list that may be accessed by user. The @ref DisplayProvider
 * must then implements the @ref make() method to let the user
 * create @ref Displayer instances.
 *
 * @ingroup display
 */

/**
 * Build a display provider.
 * @param name	Its name (it is advised to use the C++ fully-qualified name
 * 				to let the OTAWA plugin system automatically load it).
 */
Provider::Provider(cstring name): AbstractIdentifier(name) {
	provs.add(this);
}

/**
 */
Provider::~Provider(void) {
	provs.remove(this);
}

/**
 * @fn bool DisplayProvider::accepts(output_t out);
 * Test if the provider supports the given output.
 * @param out	Output to support.
 * @return		True if output is supported, false else.
 */

/**
 * @fn Displayer *DisplayProvider::make(const AbstractGraph& g, const Decorator& d, output_t out);
 * Called to generate a displayer for the given graph, decorator and, optionally the output mode.
 * @param g		Graph to display.
 * @param d		Decorator to generate vertices and edges content.
 * @param out	Required output.
 * @return		Graph displayer.
 */

/**
 * Get a provider for the given name or the default provide
 * if no name is given.
 * @param name	Name of the provider (default provider if no name).
 * @return		Found provider or null.
 */
Provider *Provider::get(cstring name) {

	// name given
	if(name) {

		// look in the list
		for(List<Provider *>::Iter p(provs); p(); p++)
			if(p->name() == name)
				return *p;

		// try to open it
		AbstractIdentifier *id = ProcessorPlugin::getIdentifier(name);
		return static_cast<Provider *>(id);
	}

	// look for default displayer
	else {

		// some in the list
		if(provs)
			return provs.first();

		// try to load the default
		else {
			AbstractIdentifier *id = ProcessorPlugin::getIdentifier("otawa::graphviz::Provider");
			return static_cast<Provider *>(id);
		}
	}

}

/**
 * Find a provider supporting the given output in the list
 * of existing providers.
 * @param out	Supported output.
 * @return		Found provider or null.
 */
Provider *Provider::get(output_mode_t out) {

	// any provider available?
	if(provs) {
		for(List<Provider *>::Iter p(provs); p(); p++)
			if(p->accepts(out))
				return *p;
		return 0;
	}

	// try to load the default provider
	else {
		AbstractIdentifier *id = ProcessorPlugin::getIdentifier("otawa::graphviz::Provider");
		if(id) {
			Provider *p = static_cast<Provider *>(id);
			if(p->accepts(out))
				return p;
		}
		return 0;
	}

}


/**
 * Build a displayer from the default provider.
 * Called to generate a displayer for the given graph, decorator and, optionally the output mode.
 * @param g		Graph to display.
 * @param d		Decorator to generate vertices and edges content.
 * @param out	Required output.
 * @return		Graph displayer.
 */
Displayer *Provider::display(graph::DiGraph *g, const Decorator& d, output_mode_t out) {
	if(!def) {
		def = Provider::get(out);
		if(!def)
			throw otawa::Exception(_ << "no display can be found!");
	}
	return def->make(g, d, out);
}


/**
 * List of display providers.
 */
List<Provider *> Provider::provs;


/**
 * Default provider.
 */
Provider *Provider::def = 0;


/**
 * Obtain a displayer using a plug-in supporting the selected output type.
 * If the output type is any, return a displayer with the default plug-in.
 * @param g		Graph to display.
 * @param d		Decorator to use.
 * @param out	Output type (optional).
 *
 * @ingroup display
 */
Displayer *make(graph::DiGraph *g, const Decorator& d, output_mode_t out) {
	return Provider::display(g, d, out);
}

} }	// otawa::display
