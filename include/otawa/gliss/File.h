/*
 *	$Id$
 *	Copyright (c) 2004, IRIT UPS.
 *
 *	src/gliss/File.h -- GLISS File class interface.
 */
#ifndef OTAWA_GLISS_FILE_H
#define OTAWA_GLISS_FILE_H

#include <elm/datastruct/HashTable.h>
#include <otawa/manager.h>
#include <otawa/gliss/Symbol.h>
#define ISS_DISASM
#include <iss_include.h>

namespace otawa { namespace gliss {

class Process;

// File class
class File: public ::otawa::File {
	friend class CodeSegment;
	friend class Process;
	String path;
	elm::datastruct::Vector<Segment *> segs;
	state_t *_state;
	datastruct::HashTable<String, otawa::Symbol *> syms;
	void initSyms(void);
	bool labels_init;
public:
	File(String _path, int argc, char **argv, char **envp);
	~File(void);
	inline bool isOK(void) { return !segs.isEmpty(); };
	otawa::Inst *findByAddress(address_t addr);
	inline state_t *state(void) const;

	// ::otawa::File overload
	virtual CString name(void);
	virtual elm::Collection<Segment *>& segments(void);
	virtual address_t findLabel(const String& label);
	virtual otawa::Symbol *findSymbol(String name);
	virtual elm::Collection<otawa::Symbol *>& symbols(void);
};

// File Inlines
inline state_t *File::state(void) const {
	return _state;
}

} } // otawa::gliss

#endif	// OTAWA_GLISS_FILE_H
