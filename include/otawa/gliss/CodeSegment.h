/*
 *	$Id$
 *	Copyright (c) 2004, IRIT UPS.
 *
 *	src/gliss/CodeSegment.h -- GLISS CodeSegment class interface.
 */
#ifndef OTAWA_GLISS_CODESEGMENT_H
#define OTAWA_GLISS_CODESEGMENT_H

#include <otawa/program.h>

namespace otawa { namespace gliss {

// Predefined classes
class File;
	
// CodeSegment class
class CodeSegment: public ::otawa::Segment {

	// Code representation
	class Code: public otawa::Code {
	public:
		inhstruct::DLList insts;
		memory_t *mem;
		address_t addr;
		size_t _size;

		Code(memory_t *memory, address_t address, size_t size);
		~Code(void);
		CString name(void);
		address_t address(void);
		size_t size(void);
		virtual Inst *first(void) const;
		virtual Inst *last(void) const;
	};

	// attributes
	File& file;
	String _name;
	Code code;
	bool built;
	datastruct::Vector<ProgItem *> _items;
	void build(void);

public:
	CodeSegment(File& _file, CString name, memory_t *memory, address_t address, size_t size);
	otawa::Inst *findByAddress(address_t addr);

	// Segment overload
	virtual CString name(void);
	virtual int flags(void);
	virtual address_t address(void);
	virtual size_t size(void);
	virtual datastruct::Collection<ProgItem *>& items(void);
};
	
} } // otawa::gliss

#endif // OTAWA_GLISS_CODESEGMENT_H
