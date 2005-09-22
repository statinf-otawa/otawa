/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/prog/ProgItem.h -- ProgItem class interface.
 */
#ifndef OTAWA_PROG_PROGITEM_H
#define OTAWA_PROG_PROGITEM_H

#include <assert.h>
#include <elm/string.h>
#include <otawa/base.h>
#include <otawa/properties.h>

namespace otawa {

// Extern classes
class CodeItem;
class Data;

// ProgObject class
class ProgObject: public PropList {
};

// ProgItem class
class ProgItem: public ProgObject {
protected:
	virtual ~ProgItem(void);
public:
	virtual CString name(void) = 0;
	virtual address_t address(void) = 0;
	virtual size_t size(void) = 0;
	virtual CodeItem *toCode(void);
	virtual Data *toData(void);
};

} // otawa

#endif // OTAWA_PROG_PROGITEM_H
