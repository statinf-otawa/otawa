/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	program.h -- objects describing the program.
 */
#ifndef OTAWA_PROGRAM_H
#define OTAWA_PROGRAM_H

#include <assert.h>
#include <elm/string.h>
#include <elm/inhstruct/DLList.h>
#include <elm/datastruct/Collection.h>
#include <otawa/base.h>
#include <otawa/properties.h>
#include <otawa/instruction.h>

namespace otawa {


// Defined classes
class ProgObject;
class ProgItem;
class Code;
class Data;


// ProgObject class
class ProgObject: public PropList {
public:
	virtual ~ProgObject(void);
};


// ProgItem class
class ProgItem: public ProgObject {
protected:
	virtual ~ProgItem(void)  { };
public:
	virtual CString name(void) = 0;
	virtual address_t address(void) = 0;
	virtual size_t size(void) = 0;
	virtual Code *toCode(void) { return 0; };
	virtual Data *toData(void) { return 0; };
};


// Code class
class Inst;
class Code: public ProgItem, public inhstruct::DLList {
public:
	inline Inst *first(void) const { return (Inst *)inhstruct::DLList::first(); };
	inline Inst *last(void) const { return (Inst *)inhstruct::DLList::last(); };
	virtual Code *toCode(void) { return this; };
};


// Data class
class Type;
class Data: public ProgItem {
public:
	virtual Type *getType(void) = 0;
	virtual Data *toData(void) { return this; };
};


// Segment class
class Segment: public ProgObject {
protected:
	virtual ~Segment(void) { };
public:
	const static int EXECUTABLE = 0x01;
	const static int WRITABLE = 0x02;

	virtual CString name(void) = 0;
	virtual int flags(void) = 0;
	virtual address_t address(void) = 0;
	virtual size_t size(void) = 0;
	virtual datastruct::Collection<ProgItem *>& items(void) = 0;
};


// File class

class File: public ProgObject {
public:
	static id_t ID_Label;
	virtual CString name(void) = 0;
	virtual const datastruct::Collection<Segment *>& segments(void) const = 0;
	virtual address_t findLabel(const String& label) = 0;
};


};	// namespace otawa

#endif		// OTAWA_PROGRAM_H
