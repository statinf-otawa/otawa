/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	loader::new_gliss::Process class interface
 */

#include <elm/assert.h>
#include <otawa/loader/new_gliss/Process.h>
#include <otawa/prog/Manager.h>
#include <gel/gel.h>
#include <gel/gel_elf.h>
#include "old_gliss.h"
#include <otawa/loader/gliss.h>

#define TRACE(m) //cerr << m << io::endl

namespace otawa { namespace loader { namespace new_gliss {


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


/**
 * @class Process
 * This class provides support to build a loader plug-in based on the GLISS
 * with ELF file loading based on the GEL library. Currently, this only includes
 * the PPC ISA.
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
  * Build a process for the new GLISS system.
  * @param manager	Current manager.
  * @param platform	Current platform.
  * @param props	Building properties.
  */ 
 Process::Process(
 	Manager *manager,
 	hard::Platform *platform,
	const PropList& props)
:	otawa::Process(manager, props),
	_start(0),
	_platform(platform), 
	_state(0)
{
	ASSERTP(manager, "manager required");
	ASSERTP(platform, "platform required");

	static char *default_argv[] = { "", 0 };
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
File *Process::loadFile(elm::CString path) {

	// Check if there is not an already opened file !
	if(program())
		throw Exception("loader cannot open multiple files !", 0);

	
	// System configuration
    void *system_list[3];
    int page_size = 4096;
    system_list[0] = &page_size;
    system_list[1] = NULL; 
    system_list[2] = NULL;

    // Loader configuration
	static char *ld_library_path[] = { 0 };
    void *loader_list[5];
    argv[0] = (char *)&path;
    loader_list[0] = argv;
    loader_list[1] = envp;
    loader_list[2] = NULL;
    loader_list[3] = (void *)ld_library_path;
    loader_list[4] = NULL;
    //cout << (void *)envp << " = " << loader_list[1] << io::endl;
    
    // Memory configuration
    void *mem_list[3];
    int mem_size = 0;
    int mem_bits = 0;
    mem_list[0]=&mem_size;
    mem_list[1]=&mem_bits;
    mem_list[2]=NULL;

    // Initialize emulator
    state_t *state = iss_init(mem_list, loader_list, system_list, NULL, NULL);
    if(!state)
    	throw LoadException("cannot load \"%s\".", &path);
    _state = state;
    File *file = new otawa::File(path);
	addFile(file);
    
    // Build segments
    gel_file_t *gel_file = (gel_file_t *)gelFile();
    assert(file); 
    gel_file_info_t infos;
	gel_file_infos(gel_file, &infos);
    for(int i = 0; i < infos.sectnum; i++) {
    	gel_sect_info_t infos;
    	gel_sect_t *sect = gel_getsectbyidx(gel_file, i);
    	assert(sect);
    	gel_sect_infos(sect, &infos);
    	if(infos.flags & SHF_EXECINSTR) {
    		Segment *seg = new Segment(*this, infos.name, infos.vaddr, infos.size);
    		file->addSegment(seg);
    	}
    }
    
    // Initialize symbols
	gel_enum_t *iter = gel_enum_file_symbol(gel_file);
	gel_enum_initpos(iter);
	for(char *name = (char *)gel_enum_next(iter); name;
	name = (char *)gel_enum_next(iter)) {
		assert(name);
		address_t addr = 0;
		Symbol::kind_t kind;
		gel_sym_t *sym = gel_find_file_symbol(gel_file, name);
		assert(sym);
		gel_sym_info_t infos;
		gel_sym_infos(sym, &infos);
		switch(ELF32_ST_TYPE(infos.info)) {
		case STT_FUNC:
			kind = Symbol::FUNCTION;
			addr = (address_t)infos.vaddr;
			break;
		case STT_NOTYPE:
			kind = Symbol::LABEL;
			addr = (address_t)infos.vaddr;
			break;
		}
		
		// Build the label if required
		if(addr) {
			String label(infos.name);
			Symbol *sym = new Symbol(*file, label, kind, addr);
			file->addSymbol(sym);
			TRACE("function " << label << " at " << addr);
		}
	}
	gel_enum_free(iter);

	// Last initializations
	_start = findInstAt((address_t)infos.entry);
	otawa::gliss::GLISS_STATE(this) = _state;
	return file;
}


/**
 */
Inst *Segment::decode(address_t address) {
	Inst *result = proc.decode(address);
	TRACE("otawa::loader::old_gliss::Segment::decode(" << address << ") = "
		<< (void *)result);
	return result;
}

} } } // otawa::loader::new_gliss
