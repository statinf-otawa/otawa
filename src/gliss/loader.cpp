/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	loader.cpp -- GLISS loader classes interface.
 */

#include <elm/io/io.h>
#define ISS_DISASM
#include "gliss.h"

namespace otawa { namespace gliss {


/**
 * @class File
 * File implementation for GLISS PowerPC.
 */

/**
 * Build a GLISS PowerPC file by loading the given path.
 * @param _path	Path of the file to load.
 */
File::File(String _path): path(_path) {
	
	// System configuration
    void *system_list[5+1];
    int argc = 1;
    char *argv[] = { "", 0 };
    char *envp[] = { 0 };
    int page_size =4096;
    system_list[0]=&argc;
    system_list[1]=argv;
    system_list[2]=envp;
    system_list[3]=&page_size; 
    system_list[4]=NULL; 
    system_list[5]=NULL;

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
	if(isOK())
		clear();
	if(state)
    	iss_halt(state);
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
	Option<address_t> addr = labels.get(label);
	if(addr)
		return *addr;
	else
		return 0;
}

/**
 * @class Process
 * Process implementation for GLISS PowerPC.
 */

/**
 * Process constructor.
 * @param _man	Caller manager.
 */
Process::Process(Manager *_man): man(_man) {
}

/**
 * Clear the loaded file.
 */
void Process::clear(void) {
	for(int i = 0; i < _files.count(); i++)
		delete _files[i];
	_files.clear();
}

/**
 * Get the list of loaded files.
 * @return	Loader files.
 */
const elm::datastruct::Collection<otawa::File *> *Process::files(void) const {
	return &_files;
}


/**
 * Not implemented. Ever fails.
 * @return Created file or null.
 */
::otawa::File *Process::createFile(void) {
	return 0;
}


/**
 * Load the given file.
 * @return Loaded file.
 * @note GLISS loader can only load one file. Load a new file delete the old one.
 */
::otawa::File *Process::loadFile(CString path) {
	clear();
	File *file = new otawa::gliss::File(path);
	if(!file->isOK()) {
		delete file;
		return 0;
	}
	else {
		_files.add(file);
		return file;
	}
}


/**
 * Get the GLISS platform.
 * @return GLISS platform.
 */
::otawa::Platform *Process::platform(void) const {
	return 0; // !!TODO!! &Platform::platform;
};


/**
 * Get the current manager.
 * @return Manager.
 */
Manager *Process::manager(void) const {
	return man;
}


/**
 * @class Loader
 * Implementation of the Otawa Loader class for GLISS PowerPC.
 */

/**
 * Get the name of the loader.
 * @return Loader name.
 */
CString Loader::getName(void) const {
	return LOADER_Heptane;
}

/**
 * Load a file with the current loader.
 * @param man		Caller manager.
 * @param path		Path to the file.
 * @param props	Properties.
 * @return	Created process or null if there is an error.
 */
Process *Loader::load(Manager *man, CString path, PropList& props) {
	Process *proc = create(man, props);
	if(!proc->loadFile(path)) {
		delete proc;
		return 0;
	}
	else
		return proc;
}


/**
 * Create an empty process.
 * @param man		Caller manager.
 * @param props	Properties.
 * @return	Created process.
 */
Process *Loader::create(Manager *man, PropList& props) {
	return new Process(man);
}


// PPC GLISS Loader entry point
static Loader static_loader;
otawa::Loader& loader = static_loader;


/**
 * @class CodeSegment
 * A segment of code with the PPC Gliss loader.
 */

/**
 * Constructor.
 * @param _file		Container file.
 * @param name	Name of the segment.
 * @param memory	Gliss memory.
 * @param address	Address of the segment in the simulator memory.
 * @param size			Size of the segment.
 */
CodeSegment::CodeSegment(File& _file, CString name, memory_t *memory, address_t address, size_t size)
: file(_file), _name(name), code(memory, address, size), built(false) {
}

// Overloaded
CString CodeSegment::name(void) {
	return _name.toCString();
}

// Overloaded
address_t CodeSegment::address(void) {
	return code.addr;
}

// Overloaded
size_t CodeSegment::size(void) {
	return code._size;
}

// Overloaded
Collection<ProgItem *>& CodeSegment::items(void) {
	if(!built)
		build();
	return _items;
}

// Overloaded
int CodeSegment::flags(void) {
	return EXECUTABLE;
}

/**
 * Build the code and the instructions in the current segment.
 */
void CodeSegment::build(void) {
	code_t buffer[20];
	instruction_t *inst;
	
	// Link the code
	built = true;
	_items.add(&code);
	
	// Build the instructions
	for(offset_t off = 0; off < code._size; off += 4) {
		address_t addr = code.addr + off;
		
		// Get the instruction
		iss_fetch((::address_t)(unsigned long)addr, buffer);
		inst = iss_decode((::address_t)(unsigned long)addr, buffer);
		assert(inst);
	
		// Look for its kind
		switch(inst->ident) {
		case ID_B_:
		case ID_BA_:
		case ID_BL_:
		case ID_BLA_:
		case ID_BC_:
		case ID_BCA_:
		case ID_BCL_:
		case ID_BCLA_:
		case ID_BCCTR_:
		case ID_BCCTRL_:
		case ID_BCLR_:
		case ID_BCLRL_:
		case ID_SC:
			code.addLast(new ControlInst(*this, addr));
			break;
		default:
			code.addLast(new Inst(*this, addr));
			break;
		}
		
		// Cleanup
		iss_free(inst);
	}
	
	// Read the symbols
	Elf32_Sym *syms = Tables.sym_tbl;
	char *names = Tables.symstr_tbl;
	int sym_cnt = Tables.sec_header_tbl[Tables.symtbl_ndx].sh_size
		/ Tables.sec_header_tbl[Tables.symtbl_ndx].sh_entsize;
	for(int i = 0; i < sym_cnt; i++) {
		address_t addr = 0;
		
		// Function symbol
		if(ELF32_ST_TYPE(syms[i].st_info)== STT_FUNC
		&& syms[i].st_shndx != SHN_UNDEF)
			addr = (address_t)syms[i].st_value;
		
		// Simple label symbol
		else if(ELF32_ST_TYPE(syms[i].st_info)== STT_NOTYPE
		&& syms[i].st_shndx == Text.txt_index)
			addr = (address_t)syms[i].st_value;

		// Build the label if required
		if(addr) {
			String label(&names[syms[i].st_name]);
			file.labels.put(label, addr);
			Inst *inst = (Inst *)findByAddress(addr);
			if(inst)
				inst->set<String>(File::ID_Label, label);
		}
	}
}


/**
 * @class CodeSegment::Code
 * As, in this loader, there is no function identification, the entire segment is viewed as a monolithic piece of code
 * and only one code representation is required and embeded in the segment object.
 */

// Overloaded
CString CodeSegment::Code::name(void) {
	return "";
}

// Overloaded
address_t CodeSegment::Code::address(void) {
	return addr;
}

// Overloaded
size_t CodeSegment::Code::size(void) {
	return _size;
}


/**
 * Find an instructions thanks to its address.
 * @param addr	Address of the instruction to find.
 * @return			Found instruction or null if not found.
 */
otawa::Inst *CodeSegment::findByAddress(address_t addr) {
	
	// In the segement ?
	if(addr < code.address() || addr >= code.address() + code.size())
		return 0;
	
	// Look in the instruction
	/* !!TODO!! May be improved using an indirect table.
	 * Issue: manage the indirect table with modifications of the code.
	 */
	for(otawa::Inst *inst = code.first(); !inst->atEnd(); inst = inst->next())
		if(!inst->isPseudo() && inst->address() == addr)
			return inst;
	return 0;
}


} }	// otawa::gliss
