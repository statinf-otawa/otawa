/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss/File.cpp -- gliss::File class implementation.
 */

#include <otawa/gliss.h>
#include <elm/debug.h>
#include <gel.h>
#include <gel_elf.h>

extern "C" gel_file_t *loader_file(memory_t* memory);

namespace otawa { namespace gliss {

/**
 * @class File
 * File implementation for GLISS PowerPC.
 */

	
/**
 * Build a GLISS PowerPC file by loading the given path.
 * @param _path	Path of the file to load.
 */
File::File(String _path, int argc, char **argv, char **envp)
: path(_path), labels_init(false) {
	
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
    loader_list[3] = (void *)"";
    loader_list[4] = NULL;
    
    // Memory configuration
    void *mem_list[3];
    int mem_size = 0;
    int mem_bits = 0;
    mem_list[0]=&mem_size;
    mem_list[1]=&mem_bits;
    mem_list[2]=NULL;

    // Initialize emulator
    _state = iss_init(mem_list, loader_list, system_list, NULL, NULL);
    if(!_state)
    	return;
    
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
    		segs.add(seg);
    	}
    }
}


/**
 * Destructor.
 */
File::~File(void) {
	clearProps();
	if(_state) {
		for(Iterator<otawa::Symbol *> sym(syms.items()); sym; sym++)
			delete *sym;
		for(Iterator<Segment *> seg(segs); seg; seg++)
			delete (CodeSegment *)*seg;
    	iss_halt(_state);
	}
}


/**
 * Get the name of the file, that is, its path.
 * @return Name of the file.
 */
CString File::name(void) {
	return path.toCString();
}


/**
 * Get the segments in the file.
 * @return Segments of the file.
 */
elm::Collection<Segment *>& File::segments(void) {
	return segs;
}


/**
 * Find the address matching the given label.
 * @param label	Name of the label to find.
 * @retrun		Address of the label or null.
 */
address_t File::findLabel(const String& label) {
	otawa::Symbol *sym = findSymbol(label);
	return sym ? sym->address() : 0;
}


/**
 * Find an instruction by its address.
 * @param addr	Address of the instruction to find.
 * @return		Return the found instruction or null.
 */
otawa::Inst *File::findByAddress(address_t addr) {
	for(Iterator<otawa::Segment *> seg(segments()); seg; seg++)
		if(seg->flags() & Segment::EXECUTABLE) {
			CodeSegment *cseg = (CodeSegment *)*seg;
			otawa::Inst *result = cseg->findByAddress(addr);
			if(result)
				return result;
		}
	return 0;
}


// elm::File overload
otawa::Symbol *File::findSymbol(String name) {
	if(!labels_init)
		initSyms();
	Option<otawa::Symbol *> sym = syms.get(name);
	if(sym)
		return *sym;
	else
		return 0;
}


// elm::File overload
elm::Collection<otawa::Symbol *>& File::symbols(void) {
	if(!labels_init)
		initSyms();
	return syms.items();
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
			this->syms.put(label, sym);
		}
	}
	gel_enum_free(iter);

	// Mark as built
	labels_init = true;
}

} } // otawa::gliss
