/*
 *	$Id$
 *	loader::old_gliss::Process class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-7, IRIT UPS.
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

#include <elm/assert.h>
#include <otawa/loader/old_gliss/Process.h>
#include <otawa/prog/Manager.h>
#include "old_gliss.h"
#include "elf.h"
#include "elfread.h"
#include <otawa/loader/gliss.h>
#include "config.h"
extern "C" {
#include <gel/dwarf_line.h>
}

#define TRACE(m) //cout << m << io::endl

extern "C" Elf32_Ehdr Ehdr;

namespace otawa { namespace gliss { extern Identifier<state_t *> GLISS_STATE; } }

namespace otawa { namespace loader { namespace old_gliss {


// Segment class
class Process;
class Segment: public otawa::Segment {

public:
	Segment(
		Process& process,
		CString name,
		address_t address,
		size_t size)
	:	otawa::Segment(name, address, size, EXECUTABLE),
		proc(process)
	{
	}

protected:
	virtual Inst *decode(address_t address);

private:
	Process& proc;
};


// File class
class File: public otawa::File {
public:
	File(const string& name): otawa::File(name), init(false), file(0), map(0) { }

	Option<Pair<cstring, int> > getSourceLine(Address addr)
	throw (UnsupportedFeatureException) {
		setup();
		if(!map)
			return none;
		const char *file;
		int line;
		if(!map
		|| dwarf_line_from_address(map, addr.offset(), &file, &line) < 0)
			return none;
		return some(pair(cstring(file), line));
	}

	void getAddresses(cstring file, int line,
	Vector<Pair<Address, Address> >& addresses)
	throw (UnsupportedFeatureException) {
		//cerr << "looking for " << file << ":" << line << io::endl;
		setup();
		addresses.clear();
		if(!map)
			return;
		dwarf_line_iter_t iter;
		dwarf_location_t loc, ploc = { 0, 0, 0, 0 };
		for(loc = dwarf_first_line(&iter, map); loc.file; loc = dwarf_next_line(&iter)) {
			cstring lfile = loc.file;
			//cerr << loc.file << ":" << loc.line << ", " << loc.low_addr << "-" << loc.high_addr << io::endl;
			if(file == loc.file || lfile.endsWith(file)) {
				if(line == loc.line) {
					//cerr << "added (1) " << loc.file << ":" << loc.line << " -> " << loc.low_addr << io::endl;
					addresses.add(pair(Address(loc.low_addr), Address(loc.high_addr)));
				}
				else if(loc.file == ploc.file && line > ploc.line && line < loc.line) {
					//cerr << "added (2) " << ploc.file << ":" << ploc.line << " -> " << ploc.low_addr << io::endl;
					addresses.add(pair(Address(ploc.low_addr), Address(ploc.high_addr)));
				}
			}
			ploc = loc;
		}
	}

protected:
	virtual ~File(void) {
		if(map)
			 dwarf_delete_line_map(map);
		if(file)
			gel_close(file);
	}

private:
	void setup(void) {
		if(init)
			return;
		init = true;

		// Open the file
		if(!file) {
			file = gel_open(&name(), 0, GEL_OPEN_NOPLUGINS);
			if(!file) {
				cerr << "WARNING: file \"" << name()
					 << "\" seems to have disappeared !\n";
				return;
			}
		}

		// Open the map
		map = dwarf_new_line_map(file, 0);
	}

	bool init;
	gel_file_t *file;
	 dwarf_line_map_t *map;
};


/**
 * @class Process
 * This class provides support to build a loader plug-in based on the GLISS
 * tool with the old ELF loader system. Currently, this includes the "arm"
 * and the "m68h" processors.
 *
 * This class allows to load a binary file, extract the instructions and the
 * symbols (labels and function). You have to provide a consistent
 * platform description for the processor.
 *
 * The details of the interface with GLISS are managed by this class and you
 * have only to write the platform description and the recognition of the
 * instruction.
 */


 /**
  * Build a process for the old GLISS system.
  * @param manager	Current manager.
  * @param loader	Current loader.
  * @param platform	Current platform.
  * @param props	Building properties.
  */
 Process::Process(
 	Manager *manager,
 	Loader *loader,
 	hard::Platform *platform,
	const PropList& props)
:	otawa::Process(manager, props),
	_start(0),
	_platform(platform),
	_state(0),
	_loader(loader)
{
	ASSERTP(manager, "manager required");
	ASSERTP(platform, "platform required");

	static char empty[] = "";
	static char *default_argv[] = { empty, 0 };
	static char *default_envp[] = { 0 };
	argc = ARGC(props);
	if(argc < 0)
		argc = 1;
	argv = ARGV(props);
	if(!argv)
		argv = default_argv;
	envp = ENVP(props);
	if(!envp)
		envp = default_envp;
	provide(MEMORY_ACCESS_FEATURE);
	provide(SOURCE_LINE_FEATURE);
}


/**
 * Inst *Process::decode(address_t addr);
 * This function is called each time an instruction need to be decoded.
 * It is usually only one time per instruction. This function must be
 * defined by the user of this class.
 * @param addr	Address of the instruction to decode.
 * @return		The decoded instruction or null if it is not an instruction.
 */


/**
 * @fn void *Process::state(void) const
 * Return the state as returned by GLISS. It may be casted to the state_t * type
 * found in GLISS. There was no way to avoid such a wild conversion.
 * @return	State as returned by GLISS.
 */


/**
 */
hard::Platform *Process::platform(void) {
	return _platform;
}


/**
 */
Inst *Process::start(void) {
	return _start;
}


/**
 */
otawa::File *Process::loadFile(elm::CString path) {

	// Check if there is not an already opened file !
	if(program())
		throw Exception("loader cannot open multiple files !");

	// System configuration
    void *system_list[5+1];
    int page_size = 4096;
    system_list[0] = &argc;
    system_list[1] = argv;
    system_list[2] = envp;
    system_list[3] = &page_size;
    system_list[4] = NULL;
    system_list[5] = NULL;

    // Loader configuration
    void *loader_list[1+1];
    loader_list[0]=(void *)path.chars();
    loader_list[1]=NULL;

    // Memory configuration
    void *mem_list[2+1];
    int mem_size = 0;
    int mem_bits = 0;
    mem_list[0]=&mem_size;
    mem_list[1]=&mem_bits;
    mem_list[2]=NULL;

    // Initialize emulator
    state_t *state = iss_init(mem_list, loader_list, system_list, NULL, NULL);
    if(!state)
    	throw LoadException(_ << "cannot load \"" << path << "\".");
    _state = state;
    File *file = new File(path);

    // Initialize the text segments
    for(struct text_secs *text = Text.secs; text; text = text->next) {
    	Segment *seg = new Segment(*this, text->name, (address_t)text->address,
    		text->size);
    	file->addSegment(seg);
    }

	// Add the symbols
	Elf32_Sym *syms = Tables.sym_tbl;
	char *names = Tables.symstr_tbl;
	int sym_cnt = Tables.sec_header_tbl[Tables.symtbl_ndx].sh_size
		/ Tables.sec_header_tbl[Tables.symtbl_ndx].sh_entsize;
	for(int i = 0; i < sym_cnt; i++) {
		address_t addr = 0;
		Symbol::kind_t kind;

		// Function symbol
		if(ELF32_ST_TYPE(syms[i].st_info)== STT_FUNC
		&& syms[i].st_shndx != SHN_UNDEF) {
			kind = Symbol::FUNCTION;
			addr = (address_t)syms[i].st_value;
		}

		// Simple label symbol
		else if(ELF32_ST_TYPE(syms[i].st_info)== STT_NOTYPE
		&& syms[i].st_shndx == Text.txt_index) {
			kind = Symbol::LABEL;
			addr = (address_t)syms[i].st_value;
		}

		// Build the label if required
		if(addr) {
			String label(&names[syms[i].st_name]);
			TRACE("symbol " << label << "(" << addr << ")");
			Symbol *sym = new Symbol(*file, label, kind, addr);
			file->addSymbol(sym);
		}
	}

	// Last initializations
	addFile(file);
	_start = findInstAt((address_t)Ehdr.e_entry);
	otawa::gliss::GLISS_STATE(this) = _state;
	_memory = memory();
	ASSERTP(_memory, "memory information mandatory");
	return file;
}


/**
 */
Loader *Process::loader(void) const {
	return _loader;
}


// Memory read
#if IS_BIG == 1
#	define GET(t) iss_mem_read##t##_big((memory_t *)_memory, at.address())
#else
#	define READ_MEM(t) iss_mem_read##t##_little(_memory, at.address())
#endif
#define GET(t, s) \
	void Process::get(Address at, t& val) { val = READ_MEM(s); }
GET(signed char, 8);
GET(unsigned char, 8);
GET(signed short, 16);
GET(unsigned short, 16);
GET(signed long, 32);
GET(unsigned long, 32);
GET(signed long long, 64);
GET(unsigned long long, 64);
GET(Address, 32);


/**
 */
void Process::get(Address at, string& str) {
	Address base = at;
	while(!READ_MEM(8))
		at = at + 1;
	int len = at - base;
	char buf[len];
	get(base, buf, len);
	str = String(buf, len);
}


/**
 */
void Process::get(Address at, char *buf, int size)
	{ iss_mem_read_buf(_memory, at.address(), buf, size); }


/**
 */
Inst *Segment::decode(address_t address) {
	Inst *result = proc.decode(address);
	TRACE("otawa::loader::old_gliss::Segment::decode(" << address << ") = "
		<< (void *)result);
	return result;
}


/**
 */
Option<Pair<cstring, int> > Process::getSourceLine(Address addr)
throw (UnsupportedFeatureException) {
	File *file = (File *)program();
	return file->getSourceLine(addr);
}


/**
 */
void Process::getAddresses(
	cstring file,
	int line,
	Vector<Pair<Address, Address> >& addresses)
throw (UnsupportedFeatureException) {
	File *ofile = (File *)program();
	return ofile->getAddresses(file, line, addresses);
}

} } } // otawa::loader::old_gliss
