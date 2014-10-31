
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
#include <otawa/hard/Platform.h>
#include <otawa/proc/DynFeature.h>
#include <otawa/util/FlowFactLoader.h>

using namespace elm;

#define DEBUG(x)	//x

namespace otawa { namespace script {

/**
 * @defgroup script Scripts
 *
 * Writing a chain of analyzes for a specific microprocessor target is a hard path
 * made of the setup of a WCET computation method, analyzes of the target hardware
 * and analyzes of non-standard facilities of the hardware.
 *
 * This work may be relieved using scripts: a script is an XML files that includes:
 * * an XHTML documentation of the supported hardware,
 * * parameters definition that may be set externally to control the behavior of the script,
 * * description of involved hardware (pipeline, caches, memory, etc),
 * * analyzes involved on the computation of WCET possibly tuned by the effective parameters.
 *
 * All parts of the script are fully dynamic using the [XSLT](http://www.w3.org/standards/xml/transformation)
 * language to generate the hardware description and the performed analyzes. More information
 * may be found on this format in the developer manual of OTAWA. Below is a simple example of script:
 *
 * ~~~{.xml}
 * <?xml version="1.0" encoding="iso-8859-1" standalone="yes" ?>
 * <otawa-script xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 * <name>Trivial WCET computation</name>
 *
 * <info>
 * This script may be used to compute trivial WCET (not realistic)
 * with scalar pipelined processors. In the extreme case, it may give
 * a rough overestimation of the real WCET.
 * </info>
 *
 * <configuration>
 *	<item name="virtual" type="bool" default="false" label="function call virtualization">
 *		<help>Cause the function call to be replaced by inlining the function body.
 *		Improve the precision of the WCET but may augment the computation time.</help>
 *	</item>
 *	<item name="stages" type="int" default="5" label="number of stages">
 *		<help>Number of stages in the pipeline.</help>
 *	</item>
 * </configuration>
 *
 * <script>
 *	<xsl:if test="$virtual!=0">
 *		<step processor="otawa::Virtualizer"/>
 *	</xsl:if>
 *	<step processor="otawa::ipet::WCETComputation">
 *		<!--config name="otawa::Processor::VERBOSE" value="true"/-->
 *		<config name="otawa::ipet::PIPELINE_DEPTH" value="{$stages}"/>
 *	</step>
 * </script>
 * </otawa-script>
 * ~~~
 */

class ParseError: public otawa::Exception {
public:
	inline ParseError(const string& msg): otawa::Exception(msg) { }
};

static const cstring XSL_URI = "http://www.w3.org/1999/XSL/Transform";


/**
 * Label matching the scripts (as passed in XML item elements).
 */
cstring ScriptItem::type_labels[] = {
	"bool",
	"int",
	"string",
	"range",
	"enum",
	""
};

/**
 * @class ScriptItem
 * These objects represents the item of the configuration
 * of a script.
 * @ingroup script
 */


/**
 * Called to generate the string of parameter value
 * before passing it to the XSLT processor. The result
 * must be an XPath valid expression.
 * @param value		Value of the parameter.
 * @return			XPath expression representing the parameter value.
 */
string ScriptItem::makeParam(const string& value) {
	return value;
}


/**
 * Protected constructor.
 */
ScriptItem::ScriptItem(type_t t, xom::Element& elt): type(t), multi(false) {

	// parse items
	Option<xom::String> v = elt.getAttributeValue("name");
	if(v)
		name = v;
	v = elt.getAttributeValue("default");
	if(v)
		deflt = v;
	v = elt.getAttributeValue("label");
	if(v)
		label = v;
	xom::Element *help_elem = elt.getFirstChildElement("help");
	if(help_elem)
		help = help_elem->getValue();
	v = elt.getAttributeValue("multi");
	if(v && (v == "yes" || v == "true"))
		multi = true;
}


/**
 */
ScriptItem::~ScriptItem(void) {
}


// StringItem class
class StringItem: public ScriptItem {
public:

	StringItem(xom::Element& elt): ScriptItem(T_STRING, elt) { }

	virtual string makeParam(const string& value) {
		StringBuffer buf;
		buf << '"';
		int p = value.indexOf('"');
		int last_p = -1;
		while(p >= 0) {
			if (p > 0)
				buf << value.substring(last_p + 1, p - last_p - 1);
			buf << "&quot;";
			last_p = p;
			p = value.indexOf('"', p + 1);
		}
		buf << value.substring(last_p + 1);
		buf << '"';
		return buf.toString();
	}

};


// BoolItem class
class BoolItem: public ScriptItem {
public:
	BoolItem(xom::Element& elt): ScriptItem(T_BOOL, elt) { }

	virtual string makeParam(const string& value) {
		if(value == "true")
			return "1";
		else if(value == "false")
			return "0";
		else
			return value;
	}
};


class EnumItem: public ScriptItem {
public:
	EnumItem(xom::Element& elt): ScriptItem(T_ENUM, elt) {

		// look for default value
		xom::Elements *elems = elt.getChildElements("value");
		for(int i = 0; i < elems->size(); i++) {
			xom::Element *elem = elems->get(i);
			xom::Attribute *attr = elem->getAttribute("default");
			if(attr && attr->getValue() == "true") {
				attr = elem->getAttribute("value");
				if(attr)
					deflt = attr->getValue();
				break;
			}
		}

		// if no value found, this is the first
		if(!deflt && elems->size() > 0) {
			xom::Element *elem = elems->get(0);
			xom::Attribute *attr = elem->getAttribute("value");
			if(attr)
				deflt = attr->getValue();
		}
		delete elems;
	}
};


/**
 * Parse an XML element to build the matching script item.
 * @param elt	Element to parse.
 * @return		Built element.
 * @throw		ParseError	If there is an error.
 */
ScriptItem *ScriptItem::parse(xom::Element& elt) {
	Option<xom::String> v = elt.getAttributeValue("type");
	if(!v)
		throw ParseError("no type");
	if(*v == "string")
		return new StringItem(elt);
	else if(*v == "bool")
		return new BoolItem(elt);
	else if(*v == "enum")
		return new EnumItem(elt);
	else {
		for(int i = 0; i < T_MAX; i++)
			if(*v == type_labels[i])
				return new ScriptItem(type_t(i), elt);
		throw ParseError(_ << "unknown type " << *v);
	}
}


// Registration
Registration<Script> Script::reg(
	"otawa::script::Script",
	Version(1, 0, 0),
	p::end
);


// ScriptErrorHandler class
class ScriptErrorHandler: public ErrorHandler {
public:
	ScriptErrorHandler(Output& _log): log(_log) { }
	virtual void onError(error_level_t level, const string& message) {
		log << getLevelString(level) << ": " << message << io::endl;
	}

private:
	Output& log;
};


/**
 * @class Script
 * A script processor allows to interpret a file that performs a WCET computation.
 * This file is expressed in XML and allows to use dynamic processors and features.
 * Its documentation may be found in module documentation.
 *
 * @par Configuration
 * @li @ref PATH			path to the script file to load
 * @li @ref PARAM			parameter for the script interpretation
 * @li @ref ONLY_CONFIG		cause the processor to stop its work after the configuration item building (no XSLT processing)
 * @li @ref TIME_STAT		cause the script to generate computation for each executed step
 *
 * @par Properties
 * This processor initialize the following properties before passing them
 * to the script processor configuration:
 * @li @ref SCRIPT		reference on the script XML
 * @li @ref CONFIG		reference on the configuration part of the XML
 *
 * @ingroup script
 */

/**
 */
Script::Script(void): Processor(reg) {
}


/**
 */
void Script::configure(const PropList &props) {
	Processor::configure(props);
	path = PATH(props);
	this->props = props;
	only_config = ONLY_CONFIG(props);
	timed = TIME_STAT(props);
}


/**
 */
void Script::processWorkSpace(WorkSpace *ws) {
	ErrorHandler *old = ProcessorPlugin::getErrorHandler();
	try {
		if(logFor(LOG_DEPS))
			ProcessorPlugin::setErrorHandler(this);
		work(ws);
		if(logFor(LOG_DEPS))
			ProcessorPlugin::setErrorHandler(old);
	}
	catch(elm::Exception& e) {
		if(logFor(LOG_DEPS))
			ProcessorPlugin::setErrorHandler(old);
		throw;
	}
}


/**
 * Do the real work.
 */
void Script::work(WorkSpace *ws) {

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
	elm::system::Path initial_base = doc->getBaseURI();
	initial_base = initial_base.parent().absolute();

	// build the script items
	items.clear();
	xom::Element *oroot = doc->getRootElement();
	xom::Element *conf = oroot->getFirstChildElement("configuration");
	if(conf) {
		xom::Elements *items = conf->getChildElements("item");
		for(int i = 0; i < items->size(); i++) {
			xom::Element *item = items->get(i);
			ScriptItem *sitem = ScriptItem::parse(*item);
			this->items.add(sitem);
			if(logFor(LOG_DEPS))
				log << "\tscript parameter \"" << sitem->name << "\" found.\n";
		}
		delete items;
	}
	if(only_config) {
		delete doc;
		return;
	}

	// build XSL
	xom::Element *root = new xom::Element("xsl:stylesheet", XSL_URI);
	root->addAttribute(new xom::Attribute("version", XSL_URI, "1.0"));
	xom::Document *xsl = new xom::Document(root);
	xom::Element *temp = new xom::Element("xsl:template", XSL_URI);
	root->appendChild(temp);
	temp->addAttribute(new xom::Attribute("match", XSL_URI, "/"));
	doc->removeChild(oroot);
	temp->appendChild(oroot);

	// prepare the work document
	xom::Element *empty_root = new xom::Element("empty");
	xom::Document *empty = new xom::Document(empty_root);

	// build the parameter declaration
	for(ItemIter item(*this); item; item++) {

		// single parameter
		if(!item->multi) {
			xom::Element *param = new xom::Element("xsl:param", XSL_URI);
			root->appendChild(param);
			param->addAttribute(new xom::Attribute("xsl:name", XSL_URI, item->name.toCString()));
			if(item->deflt)
				param->addAttribute(new xom::Attribute("xsl:select", XSL_URI, item->makeParam(item->deflt).toCString()));
		}

		// multi-parameter
		else
			for(Identifier<Pair<string, string> >::Getter param(props, PARAM); param; param++)
				if((*param).fst == item->name) {
					xom::Element *param_elt = new xom::Element(item->name.toCString());
					empty_root->appendChild(param_elt);
					param_elt->addAttribute(new xom::Attribute("value", item->makeParam((*param).snd).toCString()));
				}
	}

	// !!DEBUG!!
	/*DEBUG(OutStream *out = elm::system::System::createFile("out.xml");
	xom::Serializer serial(*out);
	serial.write(xsl);
	delete out);*/

	// perform the transformation
	xom::XSLTransform xslt(xsl);
	ScriptErrorHandler handler(log);
	xslt.setErrorHandler(&handler);
	for(Identifier<Pair<string, string> >::Getter param(props, PARAM); param; param++) {
		bool found = false;
		for(ItemIter item(*this); item; item++) {
			if(!item->multi && item->name == (*param).fst) {
				found = true;
				xslt.setParameter((*param).fst, item->makeParam((*param).snd));
				if(logFor(LOG_DEPS))
					log << "\tadding argument \"" << (*param).fst << "\" to \"" << (*param).snd << "\"\n";
			}
		}
		if(!found)
			warn(_ << "unknown configuration parameter: " << (*param).fst);
	}
	xom::Document *res = xslt.transformDocument(empty);
	delete empty;
	delete xsl;
	delete doc;
	res->setBaseURI(path.toString().toCString());

	// !!DEBUG!!
	/*DEBUG(OutStream *out2 = elm::system::System::createFile("out2.xml");
	xom::Serializer serial2(*out2);
	serial2.write(res);
	delete out2;)*/

	// set the script parameter
	xom::Element *script = res->getRootElement();
	SCRIPT(props) = script;

	// process the path
	xom::Elements *elems = script->getChildElements("path");
	for(int i = 0; i < elems->size(); i++) {
		xom::Element *path_elem = elems->get(i);
		Option<xom::String> value = path_elem->getAttributeValue("to");
		if(value) {
			Path path = *value;
			if(path.isRelative()) {
				Path base = path_elem->getBaseURI();
				if(!base)
					base = initial_base;
				path = base / path;
			}
			ProcessorPlugin::addPath(path);
			if(logFor(LOG_DEPS))
				log << "\tadding path \"" << path << "\"\n";
		}
	}
	delete elems;

	// scan the platform
	xom::Element *pf = script->getFirstChildElement("platform");
	script::PLATFORM(props) = pf;
	if(pf) {
		if(logFor(LOG_DEPS))
			log << "\tfound platform description.\n";
		xom::Element *proc = pf->getFirstChildElement("processor");
		if(proc) {
			PROCESSOR_ELEMENT(props) = proc;
			if(logFor(LOG_DEPS))
				log << "\tprocessor configuration found.\n";
		}
		xom::Element *cache = pf->getFirstChildElement("cache-config");
		if(cache) {
			CACHE_CONFIG_ELEMENT(props) = cache;
			if(logFor(LOG_DEPS))
				log << "\tcache configuration found.\n";
		}
		xom::Element *mem = pf->getFirstChildElement("memory");
		if(mem) {
			MEMORY_ELEMENT(props) = mem;
			if(logFor(LOG_DEPS))
				log << "\tmemory configuration found.\n";
		}
	}
	else if(logFor(LOG_DEPS))
		log << "\tno platform description.\n";

	// scan flowfacts
	xom::Elements *ffs = script->getChildElements("flowfacts");
	for (int i = 0; i < ffs->size(); ++i)
		FLOW_FACTS_NODES.add(props, ffs->get(i));

	// scan configuration in the script
	if(script->getLocalName() != "otawa-script")
		onError(script, "not an OTAWA script");
	xom::Element *steps = script->getFirstChildElement("script");
	if(!steps)
		onError(script, "no script list part");
	makeConfig(steps, props);

	// execute the script
	sys::StopWatch sw;
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
						if(logFor(LOG_DEPS))
							log << "INFO: preparing to run " << *name << io::endl;
						PropList list = props;
						makeConfig(step, list);
						DynProcessor proc(name);
						if(timed)
							sw.start();
						proc.process(ws, list);
						if(timed) {
							sw.stop();
							cerr << "INFO: time(" << *name << ") = "<< (float(sw.delay()) / 1000) << "ms\n";
						}
						break;
					}
					Option<xom::String> require = step->getAttributeValue("require");
					if(require) {
						if(logFor(LOG_DEPS))
							log << "INFO: requiring " << *require << io::endl;
						PropList list = props;
						makeConfig(step, list);
						DynFeature feature(*require);
						ws->require(feature, list);
						break;
					}
					onWarning(step, "nothing do to here !");
					break;
				}
				else if(step->getLocalName() == "config")
					break;
			}
			onWarning(node, "garbage here");
			break;
		default:
			onWarning(node, "garbage here");
			break;
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
		bool add = false;
		Option<xom::String> add_value = config->getAttributeValue("add");
		if(add_value) {
			if(add_value == "yes" || add_value == "true")
				add = true;
			else if(add_value != "no" && add_value != "false")
				onWarning(elem, "add attribute accepts only values from yes/true, no/false! Considered false!");
		}

		// set the property
		if(logFor(LOG_DEPS))
			log << "\tsetting property " << *name << " with " << *value << io::endl;
		AbstractIdentifier *id = ProcessorPlugin::getIdentifier(*name);
		if(!id)
			throw Exception(_ << "can not find identifier " << *name);
		try {
			if(!add)
				id->fromString(props, *value);
			else {
				PropList p;
				id->fromString(p, *value);
				props.addProp(p.extractProp(id));
			}
		}
		catch(elm::Exception& e) {
			onError(config, _ << "bad formatted value: " << e.message());
		}
	}
	delete elems;
}


/**
 */
void Script::onError(error_level_t level, const string &message) {
	log << getLevelString(level) << ": " << message << io::endl;
}


/**
 * This identifier configures the @ref Script to use the given path.
 * @ingroup script
 */
Identifier<elm::system::Path> PATH("otawa::script::PATH", "");


/**
 * This identifier configures the @ref Script to use the argument (identifier, value)
 * as a parameter. There is usually several parameter that may be accumulated with
 * PARAM(props).add(pair(identifier, value)) .
 * @ingroup script
 */
Identifier<Pair<string, string> > PARAM("otawa::script::PARAM", pair(string(""), string("")));


/**
 * Put by the @ref Script intrepreter in the configuration properties launching the processor.
 * XML node representing the script.
 * @ingroup script
 */
Identifier<xom::Element *> SCRIPT("otawa::script::SCRIPT", 0);


/**
 * Put by the @ref Script intrepreter in the configuration properties launching the processor.
 * XML node representing the configuration part of the script.
 * @ingroup script
 */
Identifier<xom::Element *> PLATFORM("otawa::script::PLATFORM", 0);


/**
 * This property informs the script to stop its work just after
 * parsing the configuration items.
 * @ingroup script
 */
Identifier<bool> ONLY_CONFIG("otawa::script::ONLY_CONFIG", false);


/**
 * This property asks the script to time the performed steps.
 * @ingroup script
 */
Identifier<bool> TIME_STAT("otawa::script::TIME_STAT", false);

} } // otawa::script

