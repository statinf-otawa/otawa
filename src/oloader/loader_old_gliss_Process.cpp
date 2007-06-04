/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	loader::old_gliss::Process class interface
 */

#include <elm/assert.h>
#include <otawa/loader/old_gliss/Process.h>
#include <otawa/prog/Manager.h>
#include "old_gliss.h"
#include "elf.h"
#include "elfread.h"
#include <otawa/loader/gliss.h>

#define TRACE(m) cout << m << io::endl

extern "C" Elf32_Ehdr Ehdr;

/*namespace otawa { namespace gliss {
	extern otawa::Identifier<state_t *> GLISS_STATE;
} } // otawa::gliss*/

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
    File *file = new otawa::File(path);
    
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

} } } // otawa::loader::old_gliss
