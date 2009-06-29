/*
 *	$Id$
 *	Script processor implementation
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

#include <elm/xom/Builder.h>
#include <elm/xom/XIncluder.h>
#include <elm/xom/Element.h>
#include <elm/xom/Attribute.h>
#include <elm/xom/Serializer.h>
#include <elm/system/System.h>
#include <otawa/script/Script.h>
#include <otawa/prog/File.h>
#include <otawa/prog/WorkSpace.h>
#include <elm/xom/XSLTransform.h>
#include <otawa/proc/DynProcessor.h>
#include <elm/xom/Elements.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/prog/Manager.h>

using namespace elm;

namespace otawa { namespace script {

static const cstring XSL_URI = "http://www.w3.org/1999/XSL/Transform";

/**
 * @class Script
 * A script processor allows to interpret a file that performs a WCET computation.
 * This file is expressed in XML and allows to use dynamic processors and features.
 * Its documentation may be found in module documentation.
 */

/**
 */
Script::Script(void): Processor("otawa::script::Script", Version(1, 0, 0)) {
}


/**
 */
void Script::configure(const PropList &props) {
	Processor::configure(props);
	path = PATH(props);
	this->props = props;
}


/**
 */
void Script::processWorkSpace(WorkSpace *ws) {

	// find the script
	if(!path) {
		path = ws->process()->program()->name();
		path.setExtension("osx");
	}
	if(!path.exists())
		throw otawa::Exception(_ << "script " << path << " does not exist !");

	// load the script
	xom::Builder builder;
	xom::Document *doc = builder.build(&path);
	if(!doc)
		throw otawa::Exception(_ << "script " << path << " is not valid XML !");
	xom::XIncluder::resolveInPlace(doc);

	// build XSL
	xom::Element *root = new xom::Element("xsl:stylesheet", XSL_URI);
	root->addAttribute(new xom::Attribute("version", XSL_URI, "1.0"));
	xom::Document *xsl = new xom::Document(root);
	xom::Element *temp = new xom::Element("xsl:template", XSL_URI);
	root->appendChild(temp);
	temp->addAttribute(new xom::Attribute("match", XSL_URI, "/"));
	xom::Element *oroot = doc->getRootElement();
	doc->removeChild(oroot);
	temp->appendChild(oroot);

	// build the parameter declaration
	xom::Element *conf = oroot->getFirstChildElement("configuration");
	if(conf) {
		xom::Elements *items = conf->getChildElements("item");
		for(int i = 0; i < items->size(); i++) {
			xom::Element *item = items->get(i);
			Option<xom::String> name = item->getAttributeValue("name");
			if(!name)
				onWarning(item, "\"name\" required !");
			else {
				if(isVerbose())
					log << "\toparameter " << name << " found.\n";
				xom::Element *param = new xom::Element("xsl:param", XSL_URI);
				root->appendChild(param);
				param->addAttribute(new xom::Attribute("xsl:name", XSL_URI, name));
				Option<xom::String> def = item->getAttributeValue("default");
				if(def)
					param->addAttribute(new xom::Attribute("xsl:select", XSL_URI, def));
			}
		}
		delete items;
	}

	// !!DEBUG!!
	OutStream *out = system::System::createFile("out.xml");
	xom::Serializer serial(*out);
	serial.write(xsl);
	delete out;

	// perform the transformation
	xom::XSLTransform xslt(xsl);
	for(Identifier<Pair<string, string> >::Getter param(props, PARAM); param; param++) {
		xslt.setParameter((*param).fst, (*param).snd);
		if(isVerbose())
			log << "\tadding argument " << (*param).fst << " to " << (*param).snd << io::endl;
	}
	xom::Element *empty_root = new xom::Element("empty");
	xom::Document *empty = new xom::Document(empty_root);
	xom::Document *res = xslt.transformDocument(empty);
	delete empty;
	delete xsl;
	delete doc;

	// !!DEBUG!!
	OutStream *out2 = system::System::createFile("out2.xml");
	xom::Serializer serial2(*out2);
	serial2.write(res);
	delete out2;
	xom::Element *script = res->getRootElement();

	// process the path
	xom::Elements *elems = script->getChildElements("path");
	for(int i = 0; i < elems->size(); i++) {
		xom::Element *path_elem = elems->get(i);
		Option<xom::String> value = path_elem->getAttributeValue("to");
		if(value) {
			Path path = *value;
			if(path.isRelative()) {
				Path base = path_elem->getBaseURI();
				path = base / path;
			}
			ProcessorPlugin::addPath(path);
		}
	}
	delete elems;

	// scant the platform
	xom::Element *pf = script->getFirstChildElement("platform");
	if(pf) {
		xom::Element *proc = script->getFirstChildElement("processor");
		if(proc)
			PROCESSOR_ELEMENT(props) = proc;
		xom::Element *cache = script->getFirstChildElement("cache-config");
		if(cache)
			CACHE_CONFIG_ELEMENT(props) = cache;
		xom::Element *mem = script->getFirstChildElement("memory");
		if(mem)
			MEMORY_ELEMENT(props) = mem;
	}

	// execute the script
	if(script->getLocalName() != "otawa-script")
		onError(script, "not an OTAWA script");
	xom::Element *steps = script->getFirstChildElement("script");
	if(!steps)
		onError(script, "no script list part");
	for(int i = 0; i < steps->getChildCount(); i++) {
		xom::Node *node = steps->getChild(i);
		switch(node->kind()) {
		case xom::Node::COMMENT:
		case xom::Node::TEXT:
			break;
		case xom::Node::ELEMENT: {
				xom::Element *step = (xom::Element *)node;
				if(step->getLocalName() == "step") {
					Option<xom::String> name = step->getAttributeValue("processor");
					if(name) {
						if(isVerbose())
							log << "INFO: preparing to run " << *name << io::endl;
						PropList list = props;
						makeConfig(step, list);
						DynProcessor proc(name);
						proc.process(ws, list);
						break;
					}
					onWarning(step, "nothing to to here !");
					break;
				}
			}
		default:
			onWarning(node, "garbage here");
		}
	}

	// cleanup
	delete res;
};


/**
 * Handle an error.
 * @param node	Node causing the error.
 * @param msg	Message to display.
 */
void Script::onError(xom::Node *node, const string& msg) {
	throw Exception(_ << node->getDocument()->getBaseURI() << ":" << node->line() << ": " << msg);
}


/**
 * Handle an error.
 * @param node	Node causing the error.
 * @param msg	Message to display.
 */
void Script::onWarning(xom::Node *node, const string& msg) {
	log << "WARNING: " << node->getDocument()->getBaseURI() << ":" << node->line() << ": " << msg << io::endl;
}


/**
 * Scan the configuration properties in the given element
 * and fill the given property list.
 * @param elem	XOM element to get configuration from.
 * @param props	Property list to fill.
 */
void Script::makeConfig(xom::Element *elem, PropList& props) {
	cerr << "makeConfig()\n";
	xom::Elements *elems = elem->getChildElements("config");
	for(int i = 0; i < elems->size(); i++) {
		xom::Element *config = elems->get(i);

		// get attributes
		Option<xom::String> name = config->getAttributeValue("name");
		if(!name) {
			onWarning(elem, "\"name\" attribute required");
			continue;
		}
		Option<xom::String> value = config->getAttributeValue("value");
		if(!value) {
			onWarning(elem, "\"value\" attribute required");
			continue;
		}

		// set the property
		cerr << "config done for " << *name << " with " << *value << io::endl;
		AbstractIdentifier *id = ProcessorPlugin::getIdentifier(*name);
		if(!id)
			throw Exception(_ << "can not find identifier " << *name);
		id->fromString(props, *value);
	}
	delete elems;
}


/**
 * This identifier configures the @ref Script to use the given path.
 */
Identifier<elm::system::Path> PATH("otawa::script::PATH", "");


/**
 * This identifier configures the @ref Script to use the argument (identifier, value)
 * as a parameter. There is usually several parameter that may be accumulated with
 * PARAM(props).add(pair(identifier, value)) .
 */
Identifier<Pair<string, string> > PARAM("otawa::script::PARAM", pair(string(""), string("")));

} } // otawa::script

