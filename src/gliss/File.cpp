/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss/File.cpp -- gliss::File class implementation.
 */

#include <otawa/gliss.h>

// Elf Header information
extern Elf32_Ehdr Ehdr;

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
    state = iss_init(mem_list, loader_list, system_list, NULL, NULL);
    if(!state)
    	return;
    
    // Initialize the text segments
    struct text_secs *text;
    for(text = Text.secs; text; text = text->next) {
    	CodeSegment *seg = new CodeSegment(*this, text->name, state->M, (address_t)text->address, text->size);
    	segs.add(seg);
    }
}


/**
 * Destructor.
 */
File::~File(void) {
	clearProps();
	if(state) {
		for(Iterator<Segment *> seg(segs); seg; seg++)
			delete (CodeSegment *)*seg;
    	iss_halt(state);
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
const elm::datastruct::Collection<Segment *>& File::segments(void) const {
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
	
	// Need to initialize labels?
	if(!labels_init) {
		for(Iterator<Segment *> seg(segs); seg; seg++)
			for(Iterator<ProgItem *> item(seg->items()); item; item++);
		labels_init = true;
	}
	
	// Look for the label
	Option<otawa::Symbol *> sym = syms.get(name);
	if(sym)
		return *sym;
	else
		return 0;
}


// elm::File overload
const elm::datastruct::Collection<otawa::Symbol *>& File::symbols(void) {
	return syms.items();
}

} } // otawa::gliss
