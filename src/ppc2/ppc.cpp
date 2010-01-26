/*
 *	$Id$
 *	PowerPC OTAWA plugin
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
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
#include <otawa/prog/Manager.h>
#include <otawa/prog/Loader.h>
#include <otawa/platform.h>
#include <otawa/hard/Register.h>
#include <gel/gel.h>
#include <gel/gel_elf.h>
#include <gel/image.h>
#include <gel/dwarf_line.h>
#include <otawa/proc/Processor.h>
#include <otawa/util/FlowFactLoader.h>
#include <elm/genstruct/SortedSLList.h>
#include <otawa/sim/features.h>
#include <otawa/prop/Identifier.h>


extern "C"
{
	// gliss2 C include files
	#include <ppc/api.h>
	#include <ppc/id.h>
	#include <ppc/macros.h>

	// generated code
	#include "otawa_kind.h"
	#include "otawa_target.h"
	#include "otawa_used_regs.h"
}

#include "gel_loader/gel_loader.h"

using namespace otawa::hard;



#define TRACE(m) //cerr << m << io::endl
#define LTRACE	 //cerr << "POINT " << __FILE__ << ":" << __LINE__ << io::endl
#define RTRACE(m)	//m
//#define SCAN_ARGS

// Trace for switch parsing
#define STRACE(m)	//cerr << m << io::endl


namespace otawa { namespace gliss {

// internal Identifier giving access to the GLISS state of the loaded program.
Identifier<ppc_state_t *> GLISS_STATE("otawa::gliss::gliss_state", 0);

} } // otawa::gliss



namespace otawa { namespace ppc2 {

// Platform class
class Platform: public hard::Platform {
public:
	static const Identification ID;
	Platform(const PropList& props = PropList::EMPTY);
	Platform(const Platform& platform, const PropList& props = PropList::EMPTY);

	// Registers
	static hard::Register CTR_reg;
	static hard::Register LR_reg;
	static hard::Register XER_reg;
	static const hard::PlainBank GPR_bank;
	static const hard::PlainBank FPR_bank;
	static const hard::PlainBank CR_bank;
	static const hard::MeltedBank MISC_bank;

	// otawa::Platform overload
	virtual bool accept(const Identification& id);
};


/*class Segment;
class Inst;
class BranchInst;
class Process;
*/



// SimState class
class SimState: public otawa::SimState
{
public:
	SimState(Process *process, ppc_state_t *state, ppc_decoder_t *decoder, bool _free = false)
		: otawa::SimState(process), _ppcState(state), _ppcDecoder(decoder)
	{
		ASSERT(process);
		ASSERT(state);
	}
	virtual ~SimState(void) { ppc_delete_state(_ppcState); }

	virtual void setSP(const Address& addr) { _ppcState->GPR[1] = addr.offset(); }
	inline ppc_state_t *ppcState(void) const { return _ppcState; }
	virtual Inst *execute(Inst *oinst)
	{
		ASSERTP(oinst, "null instruction pointer");

		Address addr = oinst->address();
		ppc_inst_t *inst;
		_ppcState->NIA = addr.address();
		inst = ppc_decode(_ppcDecoder, _ppcState->NIA);
		ppc_execute(_ppcState, inst);
		ppc_free_inst(inst);
		if (_ppcState->NIA == oinst->topAddress())
		{
			Inst *next = oinst->nextInst();
			while (next && next->isPseudo())
				next = next->nextInst();
			if (next && next->address() == Address(_ppcState->NIA))
				return next;
		}
		Inst *next = process()->findInstAt(_ppcState->NIA);
		ASSERTP(next, "cannot find instruction at " << (void *)_ppcState->NIA << " from " << oinst->address());
		return next;
	}

private:
	ppc_state_t *_ppcState;
	ppc_decoder_t *_ppcDecoder;
};


// Process class
class Process: public otawa::Process
{
public:
	Process(Manager *manager, hard::Platform *pf, const PropList& props = PropList::EMPTY);

	~Process()
	{
		ppc_delete_decoder(_ppcDecoder);
		ppc_unlock_platform(_ppcPlatform);
	}
	virtual int instSize(void) const { return 4; }
	void decodeRegs( Inst *inst, elm::genstruct::AllocatedTable<hard::Register *> *in, elm::genstruct::AllocatedTable<hard::Register *> *out);
	virtual otawa::SimState *newState(void)
	{
		ppc_state_t *s = ppc_new_state(_ppcPlatform);
		ASSERTP(s, "otawa::ppc2::Process::newState(), cannot create a new ppc_state");
		return new SimState(this, s, _ppcDecoder, true);
	}
	inline ppc_decoder_t *ppcDecoder() { return _ppcDecoder;}
	inline void *ppcPlatform(void) const { return _ppcPlatform; }
	void setup(void);

	// Process Overloads
	virtual hard::Platform *platform(void);
	virtual otawa::Inst *start(void);
	virtual File *loadFile(elm::CString path);
	virtual void get(Address at, signed char& val);
	virtual void get(Address at, unsigned char& val);
	virtual void get(Address at, signed short& val);
	virtual void get(Address at, unsigned short& val);
	virtual void get(Address at, signed long& val);
	virtual void get(Address at, unsigned long& val);
	virtual void get(Address at, signed long long& val);
	virtual void get(Address at, unsigned long long& val);
	virtual void get(Address at, Address& val);
	virtual void get(Address at, string& str);
	virtual void get(Address at, char *buf, int size);
	virtual Option<Pair<cstring, int> > getSourceLine(Address addr)
		throw (UnsupportedFeatureException);
	virtual void getAddresses(cstring file, int line, Vector<Pair<Address, Address> >& addresses)
		throw (UnsupportedFeatureException);

protected:
	friend class Segment;
	virtual otawa::Inst *decode(Address addr);
	virtual gel_file_t *gelFile(void) { return loader_file(_ppcMemory); }
	virtual ppc_memory_t *ppcMemory(void) { return _ppcMemory; }
private:
	otawa::Inst *_start;
	hard::Platform *_platform;
	ppc_platform_t *_ppcPlatform;
	ppc_memory_t *_ppcMemory;
	ppc_decoder_t *_ppcDecoder;
	int argc;
	char **argv, **envp;
	bool no_stack;
	bool init;
	struct dwarf_line_map_t *map;
	struct gel_file_info_t *file;
	gel_file_t *_gelFile;
};

// Process display
elm::io::Output& operator<<(elm::io::Output& out, Process *proc);


// Inst class
class Inst: public otawa::Inst
{
public:

	inline Inst(Process& process, kind_t kind, Address addr)
		: proc(process), _kind(kind), _addr(addr), isRegsDone(false)
	{
		if (!_ppcDecoder)
			_ppcDecoder = process.ppcDecoder();
		ASSERTP(_ppcDecoder, "otawa::ppc::Inst::Inst(), cannot retrieve the ppc_decoder");
	}

	inline ~Inst() { }

	/**
	 */
	void dump(io::Output& out)
	{
		char out_buffer[200];
		ppc_inst_t *inst = ppc_decode(_ppcDecoder, _addr);
		ppc_disasm(out_buffer, inst);
		ppc_free_inst(inst);
		out << out_buffer;
	}

	virtual kind_t kind() { return _kind; }
	virtual address_t address() const { return _addr; }
	virtual size_t size() const { return 4; }
	virtual Process &process() { return proc; }
	virtual const elm::genstruct::Table<hard::Register *>& readRegs()
	{
		if ( ! isRegsDone)
		{
			decodeRegs();
			isRegsDone = true;
		}
		return in_regs;
	}
	virtual const elm::genstruct::Table<hard::Register *>& writtenRegs()
	{
		if ( ! isRegsDone)
		{
			decodeRegs();
			isRegsDone = true;
		}
		return out_regs;
	}

protected:
	kind_t _kind;
	elm::genstruct::AllocatedTable<hard::Register *> in_regs;
	elm::genstruct::AllocatedTable<hard::Register *> out_regs;
	virtual void decodeRegs(void)
	{
		((Process&)process()).decodeRegs(this, &in_regs, &out_regs);
	}
private:
	ppc_address_t _addr;
	Process &proc;
	bool isRegsDone;
	static ppc_decoder_t *_ppcDecoder;
};

ppc_decoder_t *Inst::_ppcDecoder = 0;


// BranchInst class
class BranchInst: public Inst
{
public:

	inline BranchInst(Process& process, kind_t kind, Address addr)
		: Inst(process, kind, addr), _target(0), isTargetDone(false)
	{
		if (!_ppcDecoder)
			_ppcDecoder = process.ppcDecoder();
		ASSERTP(_ppcDecoder, "otawa::ppc2::BranchInst::BranchInst(), cannot retrieve the decoder");
	}

	inline ~BranchInst() { }

	virtual size_t size() const { return 4; }
	virtual otawa::Inst *target()
	{
		if ( ! isTargetDone )
		{
			ppc_address_t a = decodeTargetAddress();
			if (a)
				_target = process().findInstAt(a);
			isTargetDone = true;
		}
		return _target;
	}

protected:
	virtual ppc_address_t decodeTargetAddress(void);

private:
	static ppc_decoder_t *_ppcDecoder;
	otawa::Inst *_target;
	bool isTargetDone;
};

ppc_decoder_t *BranchInst::_ppcDecoder = 0;


/**
 * Register banks.
 */
static const RegBank *banks[] = {
	&Platform::GPR_bank,
	&Platform::FPR_bank,
	&Platform::CR_bank,
	&Platform::MISC_bank
};

static const elm::genstruct::Table<const RegBank *> banks_table(banks, 4);


/**
 * GPR register bank.
 */
const PlainBank Platform::GPR_bank("GPR", hard::Register::INT,  32, "r%d", 32);


/**
 * FPR register bank.
 */
const PlainBank Platform::FPR_bank("FPR", hard::Register::FLOAT, 64, "fr%d", 32);


/**
 * CR register bank
 */
const PlainBank Platform::CR_bank("CR", hard::Register::BITS, 4, "cr%d", 8);


/**
 * CTR register
 */
hard::Register Platform::CTR_reg("ctr", hard::Register::BITS, 32);


/**
 * LR register
 */
hard::Register Platform::LR_reg("lr", hard::Register::ADDR, 32);


/**
 * XER register
 */
hard::Register Platform::XER_reg("xer", hard::Register::INT, 32);


/**
 * MISC register bank
 */
const hard::MeltedBank Platform::MISC_bank("MISC", &Platform::CTR_reg,
	&Platform::LR_reg, &Platform::XER_reg, 0);


/**
 * Identification of the default platform.
 */
const Platform::Identification Platform::ID("powerpc-elf-");


/**
 * Build a new gliss platform with the given configuration.
 * @param props		Configuration properties.
 */
Platform::Platform(const PropList& props): hard::Platform(ID, props) {
	setBanks(banks_table);
}


/**
 * Build a new platform by cloning.
 * @param platform	Platform to clone.
 * @param props		Configuration properties.
 */
Platform::Platform(const Platform& platform, const PropList& props)
: hard::Platform(platform, props) {
	setBanks(banks_table);
}


/**
 */
bool Platform::accept(const Identification& id) {
	return id.abi() == "elf" && id.architecture() == "powerpc";
}


// Segment class

class Segment: public otawa::Segment {

public:
	Segment(Process& process,
		CString name,
		address_t address,
		size_t size) :
	otawa::Segment(name, address, size, EXECUTABLE), proc(process)
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
 * have only to write :
 *   - the platform description,
 *   - the recognition of the instruction,
 *	 - the assignment of the memory pointer.
 */


 /**
  * Build a process for the new GLISS system.
  * @param manager	Current manager.
  * @param platform	Current platform.
  * @param props	Building properties.
  */
Process::Process(Manager *manager, hard::Platform *platform, const PropList& props)
	: otawa::Process(manager, props), _start(0), _platform(platform),
	_ppcMemory(0), init(false), map(0), file(0)
{
	ASSERTP(manager, "manager required");
	ASSERTP(platform, "platform required");

	// gliss2 ppc structs
	_ppcPlatform = ppc_new_platform();
	ASSERTP(_ppcPlatform, "otawa::ppc2::Process::Process(..), cannot create a ppc_platform");
	_ppcDecoder = ppc_new_decoder(_ppcPlatform);
	ASSERTP(_ppcDecoder, "otawa::ppc2::Process::Process(..), cannot create a ppc_decoder");
	_ppcMemory = ppc_get_memory(_ppcPlatform, PPC_MAIN_MEMORY);
	ASSERTP(_ppcMemory, "otawa::ppc2::Process::Process(..), cannot get main ppc_memory");
	ppc_lock_platform(_ppcPlatform);

	// build arguments
	char no_name[1] = { 0 };
	static char *default_argv[] = { no_name, 0 };
	static char *default_envp[] = { 0 };
	argc = ARGC(props);
	if (argc < 0)
		argc = 1;
	argv = ARGV(props);
	if (!argv)
		argv = default_argv;
	envp = ENVP(props);
	if (!envp)
		envp = default_envp;

	// handle features
	provide(MEMORY_ACCESS_FEATURE);
	provide(SOURCE_LINE_FEATURE);
	provide(CONTROL_DECODING_FEATURE);
	provide(REGISTER_USAGE_FEATURE);
	provide(MEMORY_ACCESSES);

}



/**
 */
Option<Pair<cstring, int> > Process::getSourceLine(Address addr)
	throw (UnsupportedFeatureException)
{
	setup();
	if (!map)
		return none;
	const char *file;
	int line;
	if (!map || dwarf_line_from_address(map, addr.offset(), &file, &line) < 0)
		return none;
	return some(pair(cstring(file), line));
}


/**
 */
void Process::getAddresses(cstring file, int line, Vector<Pair<Address, Address> >& addresses)
	throw (UnsupportedFeatureException)
{
	setup();
	addresses.clear();
	if (!map)
		return;
	dwarf_line_iter_t iter;
	dwarf_location_t loc, ploc = { 0, 0, 0, 0 };
	for (loc = dwarf_first_line(&iter, map); loc.file; loc = dwarf_next_line(&iter))
	{
		cstring lfile = loc.file;
		//cerr << loc.file << ":" << loc.line << ", " << loc.low_addr << "-" << loc.high_addr << io::endl;
		if (file == loc.file || lfile.endsWith(file))
		{
			if (line == loc.line)
			{
				//cerr << "added (1) " << loc.file << ":" << loc.line << " -> " << loc.low_addr << io::endl;
				addresses.add(pair(Address(loc.low_addr), Address(loc.high_addr)));
			}
			else if(loc.file == ploc.file && line > ploc.line && line < loc.line)
			{
				//cerr << "added (2) " << ploc.file << ":" << ploc.line << " -> " << ploc.low_addr << io::endl;
				addresses.add(pair(Address(ploc.low_addr), Address(ploc.high_addr)));
			}
		}
		ploc = loc;
	}
}


/**
 * Setup the source line map.
 */
void Process::setup(void)
{
	if(init)
		return;
	init = true;

	// Open the file
	if(!file) {
		file = gel_open(&program()->name(), 0, GEL_OPEN_NOPLUGINS);
		if (!file) {
			cerr << "WARNING: file \"" << program()->name()
				 << "\" seems to have disappeared !\n";
			return;
		}
	}

	// Open the map
	map = dwarf_new_line_map(file, 0);
}


/**
 * @fun Inst *Process::decode(address_t addr);
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
hard::Platform *Process::platform(void)
{
	return _platform;
}


/**
 */
otawa::Inst *Process::start(void)
{
	return _start;
}


/**
 */
File *Process::loadFile(elm::CString path)
{
	LTRACE;

	// Check if there is not an already opened file !
	if (program())
		throw LoadException("loader cannot open multiple files !");

	File *file = new otawa::File(path);
	addFile(file);

	// initializing platform and state
	if (!_ppcPlatform)
		throw LoadException("invalid ppc_platform !");
	if (ppc_load_platform(_ppcPlatform, (char *)&path) == -1)
		throw LoadException(_ << "cannot load \"" << path << "\".");
	SimState *state = dynamic_cast<SimState *>(newState());
	ppc_state_t *ppcState = state->ppcState();
	if (!ppcState)
		throw LoadException("invalid ppc_state !");

	// initializing target environment
	/*ppc_env_t sim_env = { argc, argv, 0, envp, 0, 0, 0, 0 };
	ppc_loader_t *ppcLoader = ppc_loader_open((char *)&path);
	if (ppcLoader == NULL)
		throw LoadException("invalid ppc_loader !");
	ppc_stack_fill_env(ppcLoader, ppc_get_memory(_ppcPlatform, PPC_MAIN_MEMORY), &sim_env);
	ppc_registers_fill_env(&sim_env, ppcState);
	ppc_loader_close(ppcLoader);*/

	// we must call loader_init to initialise gel structs
	// Loader configuration
	LTRACE;
	static char *ld_library_path[] = { 0 };
	void *loader_list[6];
	argv[0] = (char *)&path;
	loader_list[0] = argv;
	loader_list[1] = envp;
	loader_list[2] = NULL;
	loader_list[3] = (void *)ld_library_path;
	loader_list[4] = NULL;
	loader_list[5] = NULL;
	loader_init(ppcState, ppc_get_memory(_ppcPlatform, PPC_MAIN_MEMORY), loader_list);


	// Build segments
	gel_file_t *gel_file = (gel_file_t *)gelFile();
	assert(gel_file);
	gel_file_info_t infos;
	gel_file_infos(gel_file, &infos);
	for (int i = 0; i < infos.sectnum; i++)
	{
		gel_sect_info_t infos;
		gel_sect_t *sect = gel_getsectbyidx(gel_file, i);
		assert(sect);
		gel_sect_infos(sect, &infos);
		if (infos.flags & SHF_EXECINSTR)
		{
			Segment *seg = new Segment(*this, infos.name, infos.vaddr, infos.size);
			file->addSegment(seg);
		}
	}

	// Initialize symbols
	LTRACE;
	gel_enum_t *iter = gel_enum_file_symbol(gel_file);
	gel_enum_initpos(iter);
	for (char *name = (char *)gel_enum_next(iter); name; name = (char *)gel_enum_next(iter))
	{
		assert(name);
		address_t addr = 0;
		Symbol::kind_t kind;
		gel_sym_t *sym = gel_find_file_symbol(gel_file, name);
		assert(sym);
		gel_sym_info_t infos;
		gel_sym_infos(sym, &infos);
		switch(ELF32_ST_TYPE(infos.info))
		{
		case STT_FUNC:
			kind = Symbol::FUNCTION;
			addr = (address_t)infos.vaddr;
			TRACE("SYMBOL: function " << infos.name << " at " << addr);
			break;
		case STT_NOTYPE:
			kind = Symbol::LABEL;
			addr = (address_t)infos.vaddr;
			TRACE("SYMBOL: notype " << infos.name << " at " << addr);
			break;
		default:
			continue;
		}

		// Build the label if required
		if(addr)
		{
			String label(infos.name);
			Symbol *sym = new Symbol(*file, label, kind, addr);
			file->addSymbol(sym);
			TRACE("function " << label << " at " << addr);
		}
	}
	gel_enum_free(iter);

	// Last initializations
	LTRACE;
	_ppcMemory = ppcMemory();
	ASSERTP(_ppcMemory, "memory information mandatory");
	_start = findInstAt((address_t)infos.entry);
	otawa::gliss::GLISS_STATE(this) = ppcState;
	/*gel_image_t *image = loader_image(_ppcMemory);
	gel_env_t *env = gel_image_env(image);
	if (env)
	{
		if (env->argc_return)
			ARGC(this) = env->argc_return;
		if (env->argv_return)
			ARGV_ADDRESS(this) = env->argv_return;
		if (env->envp_return)
			ENVP_ADDRESS(this) = env->envp_return;
		if (env->auxv_return)
			AUXV_ADDRESS(this) = env->auxv_return;
		if (env->sp_return)
			SP_ADDRESS(this) = env->sp_return;
	}*/
	return file;
}


static inline unsigned char read8(ppc_memory_t *memory, const Address& at)
{
	return ppc_mem_read8(memory, at.offset());
}

static inline unsigned short read16(ppc_memory_t *memory, const Address& at)
{
	return ppc_mem_read16(memory, at.offset());
}

static inline unsigned long read32(ppc_memory_t *memory, const Address& at)
{
	return ppc_mem_read32(memory, at.offset());
}

static inline unsigned long long read64(ppc_memory_t *memory, const Address& at)
{
	return ppc_mem_read64(memory, at.offset());
}


// Memory read
#define GET(t, s) \
	void Process::get(Address at, t& val) { \
			val = read##s(_ppcMemory, at.address()); \
			/*cerr << "val = " << (void *)(int)val << " at " << at << io::endl;*/ \
	}
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
void Process::get(Address at, string& str)
{
	Address base = at;
	while(!ppc_mem_read8(_ppcMemory, at.address()))
		at = at + 1;
	int len = at - base;
	char buf[len];
	get(base, buf, len);
	str = String(buf, len);
}


/**
 */
void Process::get(Address at, char *buf, int size)
	{ ppc_mem_read(_ppcMemory, at.address(), buf, size); }


/**
 */
Inst *Segment::decode(address_t address)
{
	Inst *result = dynamic_cast<Inst *>(proc.decode(address));
	TRACE("otawa::ppc2::Segment::decode(" << address << ") = "
		<< (void *)result);
	return result;
}
///////////////////////////////////////////////////////////////////////otawa_newloader  end


/**
 */
otawa::Inst *Process::decode(Address addr)
{

	// Decode the instruction
	ppc_inst_t *inst;
	TRACE("ADDR " << addr);
	inst = ppc_decode(_ppcDecoder, (ppc_address_t)addr.address());

	Inst::kind_t kind = 0;

	// Build the instruction
	otawa::Inst *result = 0;
	if(inst->ident == PPC_UNKNOWN)
	{
		TRACE("UNKNOWN !!!\n" << result);
	}
	else
	{
		// get the kind from the nmp otawa_kind attribute
		kind = ppc_kind(inst);
	}

	// detect the false branch instructions
	bool is_branch = true;
	switch (inst->ident)
	{
		case PPC_BL_D:
			if (PPC_BL_D_x_x_BRANCH_ADDR_n == 1)
			{
				kind = Inst::IS_ALU | Inst::IS_INT;
				is_branch = false;
			}
			break;
		case PPC_BCL_D_D_D:
			if (PPC_BCL_D_D_D_x_x_x_BD_n == 1)
			{
				kind = Inst::IS_ALU | Inst::IS_INT;
				cerr << "INFO: no control at " << io::endl;
				is_branch = false;
			}
		case PPC_B_D:
		case PPC_BLA_D:
		case PPC_BCLA_D_D_D:
		case PPC_BCCTRL_D_D:
		case PPC_BC_D_D_D:
		case PPC_BCA_D_D_D:
		case PPC_BCCTR_D_D:
		case PPC_BCLR_D_D:
		case PPC_BCLRL_D_D:
			break;
		default:
			is_branch = false;
	}

	if (is_branch)
		result = new BranchInst(*this, kind, addr);
	else
		result = new Inst(*this, kind, addr);
	// Cleanup
	ASSERT(result);
	ppc_free_inst(inst);
	return result;
}


/**
 */
ppc_address_t BranchInst::decodeTargetAddress(void) {

	// Decode the instruction
	ppc_inst_t *inst;
	TRACE("ADDR " << addr);
	inst = ppc_decode(_ppcDecoder, (ppc_address_t)address());

	// retrieve the target addr from the nmp otawa_target attribute
	Address target_addr = 0;
	switch (inst->ident)
	{
		case PPC_BL_D:
		case PPC_B_D:
		case PPC_BLA_D:
		case PPC_BA_D:
		case PPC_BCL_D_D_D:
		case PPC_BC_D_D_D:
		case PPC_BCLA_D_D_D:
		case PPC_BCA_D_D_D:
		case PPC_BCCTR_D_D:
		case PPC_BCCTRL_D_D:
			target_addr = ppc_target(inst);
			break;
		default:
			break;
	}

	// Return result
	ppc_free_inst(inst);
	return target_addr;
}


// Platform definition
static hard::Platform::Identification PFID("ppc-*-*");



// read and written registers infos

// values taken from nmp files

// access types
#define READ_REG     1
#define WRITE_REG    2
#define REG_RANGE    0x10
#define READ_RANGE   READ_REG | REG_RANGE
#define WRITE_RANGE  WRITE_REG | REG_RANGE
#define END_REG      0

// reg banks macros
#define BANK_GPR  4
#define BANK_CR   5
#define BANK_XER  6
#define BANK_LR   8
#define BANK_CTR  9
#define BANK_FPR  13

// convert a gliss reg info into one or several otawa Registers,
// in and out are supposed initialized by the caller
static void translate_gliss_reg_info(otawa_ppc_reg_t reg_info, elm::genstruct::Vector<hard::Register *> &in, elm::genstruct::Vector<hard::Register *> &out)
{
	if (reg_info == END_REG)
		return;

	uint8_t access_type	= ((reg_info & 0xFF000000) >> 24);
	uint8_t gliss_bank 	= ((reg_info & 0x00FF0000) >> 16);
	bool is_range 		= access_type & REG_RANGE;
	// READ_REG and WRITE_REG can be both specified at the same time
	bool is_read 		= access_type & READ_REG;
	bool is_write 		= access_type & WRITE_REG;
	uint16_t reg_num_lo 	= (is_range) ? ((reg_info & 0x0000FF00) >> 8) : (reg_info & 0x0000FFFF);
	uint16_t reg_num_up 	= (is_range) ? (reg_info & 0x000000FF) : reg_num_lo;
	int reg_count 		= reg_num_up - reg_num_lo + 1;
	assert(reg_count > 0);

	const hard::RegBank *reg_bank = 0;
	hard::Register *reg_no_bank = 0;
	switch (gliss_bank)
	{
		case 255:
			return;
		case BANK_GPR:
			reg_bank = &Platform::GPR_bank;
			break;
		case BANK_FPR:
			reg_bank = &Platform::FPR_bank;
			break;
		case BANK_CR:
			reg_bank = &Platform::CR_bank;
			break;
		case BANK_XER:
			reg_no_bank = &Platform::XER_reg;
			break;
		case BANK_LR:
			reg_no_bank = &Platform::LR_reg;
			break;
		case BANK_CTR:
			reg_no_bank = &Platform::CTR_reg;
			break;
		default:
			ASSERTP(false, "unknown bank " << gliss_bank);
	}

	//otawa_reg.allocate(reg_count);
	for (int i = reg_num_lo ; i <= reg_num_up ; i++)
	{
		if (reg_bank)
		{
			if (is_read)
				in.add(reg_bank->get(i));
			if (is_write)
				out.add(reg_bank->get(i));
		}
		else
		{
			if (is_read)
				in.add(reg_no_bank);
			if (is_write)
				out.add(reg_no_bank);
		}
	}
}

/**
 */
void Process::decodeRegs(otawa::Inst *oinst, elm::genstruct::AllocatedTable<hard::Register *> *in,
	elm::genstruct::AllocatedTable<hard::Register *> *out)
{

	// Decode instruction
	ppc_inst_t *inst;
	inst = ppc_decode(_ppcDecoder, oinst->address().address());
	if(inst->ident == PPC_UNKNOWN)
	{
		ppc_free_inst(inst);
		return;
	}

	// get register infos
	elm::genstruct::Vector<hard::Register *> reg_in;
	elm::genstruct::Vector<hard::Register *> reg_out;
	otawa_ppc_reg_t *addr_reg_info = ppc_used_regs(inst);
	if(addr_reg_info)
		for (int i = 0; addr_reg_info[i] != END_REG; i++ )
			translate_gliss_reg_info(addr_reg_info[i], reg_in, reg_out);

	// store results
	int cpt_in = reg_in.length();
	in->allocate(cpt_in);
	for (int i = 0 ; i < cpt_in ; i++)
		in->set(i, reg_in.get(i));
	int cpt_out = reg_out.length();
	out->allocate(cpt_out);
	for (int i = 0 ; i < cpt_out ; i++)
		out->set(i, reg_out.get(i));

	// Free instruction
	ppc_free_inst(inst);
}

// otawa::loader::ppc::Loader class
class Loader: public otawa::Loader {
public:
	Loader(void);

	// otawa::Loader overload
	virtual CString getName(void) const;
	virtual otawa::Process *load(Manager *_man, CString path, const PropList& props);
	virtual otawa::Process *create(Manager *_man, const PropList& props);
};


// Alias table
static string table[] = {
	"elf_20"
};
static elm::genstruct::Table<string> ppc_aliases(table, 1);


/**
 * Build a new loader.
 */
Loader::Loader(void)
: otawa::Loader("ppc", Version(2, 0, 0), OTAWA_LOADER_VERSION, ppc_aliases) {
}


/**
 * Get the name of the loader.
 * @return Loader name.
 */
CString Loader::getName(void) const
{
	return "ppc";
}


/**
 * Load a file with the current loader.
 * @param man		Caller manager.
 * @param path		Path to the file.
 * @param props	Properties.
 * @return	Created process or null if there is an error.
 */
otawa::Process *Loader::load(Manager *man, CString path, const PropList& props)
{
	otawa::Process *proc = create(man, props);
	if (!proc->loadProgram(path))
	{
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
 * @return		Created process.
 */
otawa::Process *Loader::create(Manager *man, const PropList& props)
{
	//cout << "INFO: using ppc2 loader.\n";
	return new Process(man, new Platform(props), props);
}



//using namespace otawa::loader;


} }	// namespace otawa::ppc


// PowerPC GLISS Loader entry point
otawa::ppc2::Loader OTAWA_LOADER_HOOK;
otawa::ppc2::Loader& ppc2_plugin = OTAWA_LOADER_HOOK;
