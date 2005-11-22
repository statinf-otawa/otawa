/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	program.h -- objects describing the program.
 */
#ifndef OTAWA_PROGRAM_H
#define OTAWA_PROGRAM_H

#include <elm/inhstruct/DLList.h>
#include <elm/Collection.h>
#include <elm/datastruct/Map.h>
#include <otawa/prog/CodeItem.h>
#include <otawa/instruction.h>
#include <otawa/prog/Symbol.h>

namespace otawa {


// Defined classes
class ProgObject;
class ProgItem;
class CodeItem;
class Data;


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
	const static int EXECUTABLE = 0x01;	/**< Segment is executable. */
	const static int WRITABLE = 0x02;	/**< Segment is writable. */

	virtual CString name(void) = 0;
	virtual int flags(void) = 0;
	virtual address_t address(void) = 0;
	virtual size_t size(void) = 0;
	virtual elm::Collection<ProgItem *>& items(void) = 0;
};


// File class

class File: public ProgObject {
public:
	static Identifier ID_Label;
	static Identifier ID_FunctionLabel;
	virtual CString name(void) = 0;
	virtual elm::Collection<Segment *>& segments(void) = 0;
	virtual address_t findLabel(const String& label) = 0;
	virtual Symbol *findSymbol(String name) = 0;
	virtual elm::Collection<Symbol *>& symbols(void) = 0;
};


};	// namespace otawa

#endif		// OTAWA_PROGRAM_H
