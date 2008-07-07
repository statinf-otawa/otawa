/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/prop_NameSpace.cpp -- NameSpace class implementation.
 */

#include <assert.h>
#include <otawa/prop/NameSpace.h>

/**
 * The parent of all namespaces.
 */
otawa::NameSpace NS("");


namespace otawa {


/**
 * OTAWA namespace.
 */
NameSpace NS("otawa");


/**
 * @class NameSpace
 * Represents a name space in the OTAWA property naming system. The namespaces
 * are used to group identifiers in a consistant way and to reduce risks
 * of name collision. 
 */


/**
 * Retrieve an identifier matching the given name.
 * @param _name	Identifier name.
 * @return		Null if not found else the identifier.
 */
AbstractIdentifier *NameSpace::get(const elm::String& _name) {
	NameSpace *ns = this;
	string name = _name;
	int dot = name.indexOf('.');
	while(dot >= 0) {
		AbstractIdentifier *id = ns->names.get(name.substring(0, dot), 0);
		if(!id)
			return 0;
		ns = id->toNameSpace();
		if(!ns)
			return 0;
		name = name.substring(dot);
		dot = name.indexOf('.');
	}
	return ns->names.get(name, 0);
}


/**
 * Add the given identifier to the namespace.
 * @param id	Identifier to add.
 */
void NameSpace::add(AbstractIdentifier *id) {
	names.add(id->name(), id);
}


/**
 * Build a namespace.
 * @param name		Namespace name or prefix from XML viewpoint.
 * @param parent	Parent namespace.
 */
NameSpace::NameSpace(elm::CString name, NameSpace& parent)
:	AbstractIdentifier(name),
	_parent(parent)	
{
}


/**
 * @fn elm::CString NameSpace::prefix(void) const;
 * Get the namespace prefix.
 * @return Namespace prefix.
 */


/**
 * Get the namespace URI.
 * @return	Namespace URI.
 */
elm::String NameSpace::uri(void) {
	if(!_uri) {
		if(this == &::NS)
			_uri = "http://www.irit.fr/recherches/ARCHI/MARCH";
		else
			_uri = _parent.uri() + "/" + name();
	}
	return _uri;
}


/**
 * @fn NameSpace const &Namespace::parent(void) const;
 * Get the namespace parent.
 * @return	Namespace parent.
 */


/**
 */
NameSpace *NameSpace::toNameSpace(void) {
	return this;
}	


/**
 */
void NameSpace::print(elm::io::Output& out) {
	if(this == &_parent)
		return;
	else
		out << name() << '.';
}

} // otawa
