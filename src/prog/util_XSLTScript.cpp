/*
 *	otawa::XSLTScript class implementation
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

#include <otawa/util/XSLTScript.h>
#include <elm/xom/XSLTransform.h>
#include <elm/xom/XIncluder.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <elm/xom/Serializer.h>

namespace otawa {

static const cstring XSL_URI = "http://www.w3.org/1999/XSL/Transform";

// ScriptErrorHandler class
class ScriptErrorHandler: public ErrorHandler {
public:
	ScriptErrorHandler(io::Output& _log): log(_log) { }
	virtual void onError(error_level_t level, const string& message) {
		log << getLevelString(level) << ": " << message << io::endl;
	}

private:
	io::Output& log;
};


/**
 * @class XSLTScript
 * This class provides a simple way to implement OTAWA script.
 * Its behaviour may be easily customized to use the whole or
 * a part of the OTAWA script format.
 *
 * @param path			Path of the script.
 * @param monitor		Monitor for IO.
 * @param verbosity		Verbose messages if true.
 */
XSLTScript::XSLTScript(const elm::system::Path& path, Monitor& monitor, bool verbosity) throw(otawa::Exception)
: io::Monitor(monitor), doc(0), xsl(0), temp(0), state(ERROR), configured(false), verbose(verbosity) {

	// open the file
	xom::Builder builder;
	doc = builder.build(&path);
	if(!doc)
		throw otawa::Exception(_ << "script " << path << " is not valid XML !");

	// perform inclusion operation
	xom::XIncluder::resolveInPlace(doc);

	// prepare the template
	xom::Element *root = new xom::Element("xsl:stylesheet", XSL_URI);
	root->addAttribute(new xom::Attribute("version", XSL_URI, "1.0"));
	temp = new xom::Element("xsl:template", XSL_URI);
	temp->addAttribute(new xom::Attribute("match", XSL_URI, "/"));
	root->appendChild(temp);
	xsl = new xom::Document(root);

	// let's go
	state = OPENED;
}


/**
 */
XSLTScript::~XSLTScript(void) {
	if(doc)
		delete doc;
	if(xsl)
		delete xsl;
}


/**
 * @fn xom::Document *document(void) const;
 * Get the current document.
 * @return	Current document.
 */


/**
 * Prepare the configuration of the script (in term of XSLT parameters).
 * Only allowed when the script document is opened but not transformed.
 * Fails silently if no configuration is found.
 * @param name	Name of the top-level tag containing the configuration.
 */
void XSLTScript::setConfiguration(cstring name) {
	ASSERTP(state == OPENED, "invalid operation at this state");
	ASSERTP(!configured, "already configured");
	xom::Element *conf = doc->getRootElement()->getFirstChildElement(name);
	if(conf)
		setConfiguration(conf);
}


/**
 * Prepare the configuration of the script (in term of XSLT parameters).
 * @param element	Element to look in for items.
 */
void XSLTScript::setConfiguration(xom::Element *element) {
	ASSERTP(state == OPENED, "invalid operation at this state");
	ASSERTP(!configured, "already configured");
	ASSERT(element);

	xom::Elements *items = element->getChildElements("item");
	for(int i = 0; i < items->size(); i++) {
		xom::Element *item = items->get(i);
		Option<xom::String> name = item->getAttributeValue("name");
		if(!name)
			onWarning(item, "\"name\" required !");
		else {
			if(verbose)
				log << "\tscript parameter \"" << *name << "\" found.\n";
			xom::Element *param = new xom::Element("xsl:param", XSL_URI);
			xsl->getRootElement()->appendChild(param);
			param->addAttribute(new xom::Attribute("xsl:name", XSL_URI, name));
			Option<xom::String> def = item->getAttributeValue("default");
			if(def) {
				string value = (_ << '\'' << *def << '\'');
				if(verbose)
					log << "\t\tdefault value is " << value << io::endl;
				param->addAttribute(new xom::Attribute("xsl:select", XSL_URI, &value));
			}
			else if(verbose)
				log << "\t\tno default value\n";
		}
	}
	delete items;

	configured = true;
}


/**
 * Handle an error.
 * @param node	Node causing the error.
 * @param msg	Message to display.
 */
void XSLTScript::onWarning(xom::Node *node, const string& msg) {
	err << "WARNING: " << node->getDocument()->getBaseURI() << ":" << node->line() << ": " << msg << io::endl;
}


/**
 * Add a parameter to the script.
 * @param name		Parameter name.
 * @param value		Paramater value.
 */
void XSLTScript::addParam(string name, String value) {
	ASSERTP(state == OPENED, "invalid operation at this state");
	params.add(pair(name, value));
}


/**
 * Look for parameter in the given property list. The parameters
 * are retrieved from the property list using the identifier id
 * that must be of type Identifier<string>. The parameter is represented
 * in the form ID=VALUE.
 * @param props		Property list to look in.
 * @param id		Property identifier.
 */
void XSLTScript::addParams(const PropList& props, Identifier<string>& id) {
	ASSERTP(state == OPENED, "invalid operation at this state");
	for(Identifier<string>::Getter param(props, id); param; param++) {
		string text = param;
		int p = text.indexOf('=');
		if(p < 0)
			throw otawa::Exception(_ << "bad parameter value: " << param);
		params.add(pair(text.substring(0, p), text.substring(p + 1)));
	}
}


/**
 * Look for parameter in the given property list. The parameters
 * are retrieved from the property list using the identifier id
 * that must be of type Identifier<Pair<string, string> >. First member
 * of the pair is the identifier, the second the value.
 * @param props		Property list to look in.
 * @param id		Property identifier.
 */
void XSLTScript::addParams(const PropList& props, Identifier<Pair<string, string> >& id) {
	ASSERTP(state == OPENED, "invalid operation at this state");
	for(Identifier<Pair<string, string> >::Getter param(props, id); param; param++)
		params.add(param);
}


/**
 * Perform the transformation.
 */
void XSLTScript::transform(void) throw(otawa::Exception) {
	ASSERTP(state == OPENED, "invalid operation at this state");

	try {

		// complete the XSL document
		xom::Element *oroot = doc->getRootElement();
		doc->removeChild(oroot);
		temp->appendChild(oroot);

		// perform the transformation
		xom::XSLTransform xslt(xsl);
		ScriptErrorHandler handler(log);
		xslt.setErrorHandler(&handler);
		for(int i = 0; i < params.count(); i++) {
			xslt.setParameter(params[i].fst, _ << '\'' << params[i].snd << '\'');
			if(verbose)
				log << "\tadding argument \"" << params[i].fst << "\" to \"" << params[i].snd << "\"\n";
		}
		xom::Element *empty_root = new xom::Element("empty");
		xom::Document *empty = new xom::Document(empty_root);

		xom::Document *res = xslt.transformDocument(empty);

		// DEBUG
		/*xom::Serializer serial(log.stream());
		serial.write(res);*/


		// cleanup at end
		delete empty;
		delete doc;
		doc = res;
		state = TRANSFORMED;
	}
	catch(xom::XSLException& e) {
		throw otawa::Exception(e.message());
	}
}


/**
 * Handle an error.
 * @param node	Node causing the error.
 * @param msg	Message to display.
 */
void XSLTScript::onError(xom::Node *node, const string& msg) {
	throw otawa::Exception(_ << node->getDocument()->getBaseURI() << ":" << node->line() << ": " << msg);
}


/**
 * Fill the given property list with the property definitions found
 * in the given element.
 * @param props		Property list to fill.
 * @param element	Element to look property definition nin.
 */
void XSLTScript::fillProps(PropList& props, xom::Element *element) {
	ASSERTP(state == TRANSFORMED, "illegal operation: only once the document is transformed !");
	ASSERT(element);

	xom::Elements *elems = element->getChildElements("config");
	for(int i = 0; i < elems->size(); i++) {
		xom::Element *config = elems->get(i);

		// get attributes
		Option<xom::String> name = config->getAttributeValue("name");
		if(!name) {
			onWarning(element, "\"name\" attribute required");
			continue;
		}
		Option<xom::String> value = config->getAttributeValue("value");
		if(!value) {
			onWarning(element, "\"value\" attribute required");
			continue;
		}

		// set the property
		if(verbose)
			log << "\tsetting property " << *name << " with " << value.value() << io::endl;
		AbstractIdentifier *id = ProcessorPlugin::getIdentifier(*name);
		if(!id)
			throw otawa::Exception(_ << "can not find identifier " << *name);
		try {
			id->fromString(props, *value);
		}
		catch(elm::Exception& e) {
			onError(config, _ << "bad formatted value: " << e.message());
		}
	}
	delete elems;
}


/**
 * Fill the given property list with the property definitions found
 * in the named element (at the top level). If the element does not exist,
 * do nothing.
 * @param props		Property list to fill.
 * @param name		Element name to look property definition nin.
 */
void XSLTScript::fillProps(PropList& props, cstring name) {
	ASSERTP(state == TRANSFORMED, "illegal operation: only once the document is transformed !");
	xom::Element *element = doc->getRootElement()->getFirstChildElement(name);
	if(element)
		fillProps(props, element);
	else if(verbose)
		info(_ << "no property definition element called " << name);
}

}	// otawa
