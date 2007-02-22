/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	ProgItem class interface
 */
#ifndef OTAWA_PROG_PROGITEM_H
#define OTAWA_PROG_PROGITEM_H

#include <assert.h>
#include <elm/string.h>
#include <elm/inhstruct/DLList.h>
#include <otawa/base.h>
#include <otawa/properties.h>

namespace otawa {

// Extern classes
class CodeItem;
class Inst;
class DataItem;
class Segment;

// ProgItem class
class ProgItem: public PropList, public inhstruct::DLNode {
public:
	typedef enum {
		blank = 0,
		code,
		data
	} kind_t;

	ProgItem(address_t address, size_t size);
	inline kind_t kind(void) const { return _kind; }
	inline address_t address(void) { return _address; }
	inline address_t topAddress(void) const { return _address + _size; }
	inline size_t size(void) { return _size; }
	inline bool isBlank(void) { return _kind == blank; }
	inline bool isCode(void) { return _kind == code; }
	inline bool isData(void) { return _kind == data; }
	inline CodeItem *toCode(void)
		{ if(_kind == code) return (CodeItem *)this; else return 0; }
	inline DataItem *toData(void)
		{ if(_kind == data) return (DataItem *)this; else return 0; }

protected:
	ProgItem(kind_t kind, address_t address, size_t size);
	virtual ~ProgItem(void);

private:
	friend class Segment;
	kind_t _kind;
	address_t _address;
	size_t _size;
};

} // otawa

#endif // OTAWA_PROG_PROGITEM_H
