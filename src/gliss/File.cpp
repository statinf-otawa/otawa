/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss/File.cpp -- gliss::File class implementation.
 */

#include <otawa/gliss.h>
#include <elm/debug.h>
#include <gel/gel.h>
#include <gel/gel_elf.h>

extern "C" gel_file_t *loader_file(memory_t* memory);

namespace otawa { namespace gliss {

/**
 * @class File
 * File implementation for GLISS PowerPC.
 */

	
/**
 * Build a GLISS PowerPC file by loading the given path.
 * @param _path		Path of the file to load.
 * @param argc		Program call argc value.
 * @param argv		Program call argv value.
 * @param envp		Program call envp value.
 * @param no_sys	Do not initialize the system support.
 */
File::File(String _path, int argc, char **argv, char **envp, bool no_sys)
: otawa::File(_path), labels_init(false) {
	static char *ld_library_path[] = { 0 };
	
	// System configuration
    void *system_list[3];
    int page_size = 4096;
    system_list[0] = &page_size;
    system_list[1] = NULL; 
    system_list[2] = NULL;

    // Loader configuration
    void *loader_list[5];
    argv[0] = (char *)&_path.toCString();
    loader_list[0] = argv;
    loader_list[1] = envp;
    loader_list[2] = NULL;
    loader_list[3] = (void *)ld_library_path;
    loader_list[4] = NULL;
    
    // Memory configuration
    void *mem_list[3];
    int mem_size = 0;
    int mem_bits = 0;
    mem_list[0]=&mem_size;
    mem_list[1]=&mem_bits;
    mem_list[2]=NULL;

    // Initialize emulator
    _state = iss_init(mem_list, loader_list, (no_sys ? NULL : system_list), NULL, NULL);
    if(!_state)
    	throw LoadException("cannot load \"%s\"", &_path);

	// Record symbols
    initSyms();
    
    // Initialize the text segments
    gel_file_t *file = loader_file(_state->M);
    assert(file); 
    gel_file_info_t infos;
	gel_file_infos(file, &infos);
    for(int i = 0; i < infos.sectnum; i++) {
    	gel_sect_info_t infos;
    	gel_sect_t *sect = gel_getsectbyidx(file, i);
    	assert(sect);
    	gel_sect_infos(sect, &infos);
    	if(infos.flags & SHF_EXECINSTR) {
    		CodeSegment *seg = new CodeSegment(*this, infos.name, _state->M, infos.vaddr, infos.size);
    		addSegment(seg);
    	}
    }
}


/**
 * Destructor.
 */
File::~File(void) {
	clearProps();
	if(_state) {
		for(SymIter sym(this); sym; sym++)
			delete *sym;
		for(SegIter seg(this); seg; seg++)
			delete (CodeSegment *)*seg;
    	iss_halt(_state);
	}
}


/**
 * Initialize the symbol table for this file if it is not already initialized.
 */
void File::initSyms(void) {
    gel_file_t *file = loader_file(_state->M);
    assert(file); 
	
	// Traverse ELF symbol table
	gel_enum_t *iter = gel_enum_file_symbol(file);
	gel_enum_initpos(iter);
	for(char *name = (char *)gel_enum_next(iter); name;
	name = (char *)gel_enum_next(iter)) {
		assert(name);
		address_t addr = 0;
		Symbol::kind_t kind;
		gel_sym_t *sym = gel_find_file_symbol(file, name);
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
			Symbol *sym = new Symbol(*this, label, kind, addr);
			addSymbol(sym);
		}
	}
	gel_enum_free(iter);

	// Mark as built
	labels_init = true;
}

/**
 * Find an instruction by its address.
 * @param addr	Address of the instruction to find.
 * @return		Return the found instruction or null.
 */
otawa::Inst *File::findByAddress(address_t addr) {
	for(SegIter seg(this); seg; seg++)
		if(seg->flags() & Segment::EXECUTABLE
		&& addr >= seg->address() && addr < seg->topAddress()) {
			CodeSegment *cseg = (CodeSegment *)*seg;
			otawa::Inst *result = cseg->findInstAt(addr);
			if(result)
				return result;
		}
	return 0;
}

} } // otawa::gliss
