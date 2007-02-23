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
#define ISS_DISASM
#include <iss_include.h>

namespace otawa { namespace gliss {

class Process;

// File class
class File: public ::otawa::File {
	friend class CodeSegment;
	friend class Process;
	String path;
	state_t *_state;
	void initSyms(void);
	bool labels_init;
public:
	File(String _path, int argc, char **argv, char **envp, bool no_sys = false);
	~File(void);
	inline bool isOK(void) { return !_state; };
	inline state_t *state(void) const;
	otawa::Inst *findByAddress(address_t addr);
};

// File Inlines
inline state_t *File::state(void) const {
	return _state;
}

} } // otawa::gliss

#endif	// OTAWA_GLISS_FILE_H
