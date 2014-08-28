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

#include <errno.h>
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
#include <otawa/loader/powerpc.h>
#include <otawa/prog/sem.h>
#include <otawa/proc/ProcessorPlugin.h>

extern "C"
{
	// gliss2 C include files
#	include <ppc/api.h>
#	include <ppc/id.h>
#	include <ppc/macros.h>
#	include <ppc/config.h>

	// generated code
	#include "otawa_kind.h"
	#include "otawa_target.h"
	#include "otawa_used_regs.h"
	#include "otawa_pred.h"
}

using namespace otawa::hard;



#define TRACE(m) //cerr << m << io::endl
#define LTRACE	 //cerr << "POINT " << __FILE__ << ":" << __LINE__ << io::endl
#define RTRACE(m)	//m
//#define SCAN_ARGS

// Trace for switch parsing
#define STRACE(m)	//cerr << m << io::endl


namespace otawa {

namespace ppc {


/**
 * Put in configuration MANAGER::load() property list to activate
 * VLE decoding (at microprocesor start time).
 */
Identifier<bool> VLE_ENABLED("otawa::ppc::VLE_ENABLED", false);

/**
 * Get static prediction information interface for the PowerPC loader.
 *
 * @par Owner Feature
 * @li @ref otawa::ppc::INFO_FEATURE
 *
 * @par Hooks
 * @li @ref otawa::Inst
 */
Identifier<prediction_t> STATIC_PREDICTION("otawa::ppc::STATIC_PREDICTION", PRED_NONE);


/**
 * Get GLISS decoder used in the loader.
 *
 * @par Owner Feature
 * @li @ref otawa::ppc::INFO_FEATURE
 *
 * @par Hooks
 * @li @ref otawa::Process
 */
Identifier<void *> GLISS2_DECODER("otawa::ppc::GLISS2_DECODER", 0);


/**
 * This feature is put by the OTAWA PowerPC loader to get non-standard information
 * currently including:
 * @li static prediction of conditional branches
 *
 * @par Default Processor
 * None.
 *
 * @par  Properties
 * @li @ref otawa::ppc::STATIC_PREDICTION
 * @li @ref otawa::ppc::GLISS2_DECODER
 */
Feature<NoProcessor> INFO_FEATURE("otawa::ppc::INFO_FEATURE");

} // otawa::ppc

namespace ppc2 {

// registers
static const PlainBank GPR_bank("GPR", hard::Register::INT,  32, "r%d", 32);
static const PlainBank FPR_bank("FPR", hard::Register::FLOAT, 64, "fr%d", 32);
static const PlainBank CR_bank("CR", hard::Register::BITS, 4, "cr%d", 8);
static hard::Register CTR_reg("ctr", hard::Register::BITS, 32);
static hard::Register LR_reg("lr", hard::Register::ADDR, 32);
static hard::Register XER_reg("xer", hard::Register::INT, 32);
static const hard::MeltedBank MISC_bank("MISC", &CTR_reg, &LR_reg, &XER_reg, 0);

// Register banks
static const RegBank *banks[] = {
	&GPR_bank,
	&FPR_bank,
	&CR_bank,
	&MISC_bank
};
static const elm::genstruct::Table<const RegBank *> banks_table(banks, 4);


// semantics functions
#define _GPR(n)			GPR_bank[n]->platformNumber()
#define _FPR(n)			FPR_bank[n]->platformNumber()
#define _CR(n)			CR_bank[n]->platformNumber()
#define _XER			XER_reg.platformNumber()
#define _LR				LR_reg.platformNumber()
#define _CTR			CTR_reg.platformNumber()

#define _EQ				sem::EQ
#define _NE				sem::NE
#define _LT				sem::LT
#define _LE				sem::LE
#define _GT				sem::GT
#define _GE				sem::GE
#define _ANY_COND		sem::ANY_COND

#define _add(d, a, b)	block.add(otawa::sem::add(d, a, b))
#define _and(d, a, b)	block.add(otawa::sem::_and(d, a, b))
#define _asr(d, a, b)	block.add(otawa::sem::asr(d, a, b))
#define _branch(d)		block.add(otawa::sem::branch(d))
#define _cmp(d, a, b)	block.add(otawa::sem::cmp(d, a, b))
#define _cmpu(d, a, b)	block.add(otawa::sem::cmp(d, a, b))
#define _div(d, a, b)	block.add(otawa::sem::div(d, a, b))
#define _divu(d, a, b)	block.add(otawa::sem::divu(d, a, b))
#define _if(d, a, b)	block.add(otawa::sem::_if(d, a, b))
#define _load(d, a, b)	block.add(otawa::sem::load(d, a, b))
#define _mul(d, a, b)	block.add(otawa::sem::mul(d, a, b))
#define _mulu(d, a, b)	block.add(otawa::sem::mulu(d, a, b))
#define _not(d, a)		block.add(otawa::sem::_not(d, a))
#define _or(d, a, b)	block.add(otawa::sem::_or(d, a, b))
#define _set(d, a)		block.add(otawa::sem::set(d, a))
#define _seti(d, i)		block.add(otawa::sem::seti(d, i))
#define _shl(d, a, b)	block.add(otawa::sem::shl(d, a, b))
#define _shr(d, a, b)	block.add(otawa::sem::shr(d, a, b))
#define _store(d, a, b)	block.add(otawa::sem::store(d, a, b))
#define _scratch(a)		block.add(otawa::sem::scratch(a));
#define _sub(d, a, b)	block.add(otawa::sem::sub(d, a, b))
#define _xor(d, a, b)		block.add(otawa::sem::_xor(d, a, b))

#include "otawa_sem.h"



// Platform class
class Platform: public hard::Platform {
public:
	static const Identification ID;
	Platform(const PropList& props = PropList::EMPTY);
	Platform(const Platform& platform, const PropList& props = PropList::EMPTY);

	// otawa::Platform overload
	virtual bool accept(const Identification& id);
	virtual const Register *getSP(void) const { return GPR_bank[1]; }
};


// SimState class
class SimState: public otawa::SimState
{
public:
	SimState(Process *process, ppc_state_t *state, ppc_decoder_t *decoder, bool _free = false)
	: otawa::SimState(process), _ppcState(state), _ppcDecoder(decoder) {
		ASSERT(process);
		ASSERT(state);
	}

	virtual ~SimState(void) {
		ppc_delete_state(_ppcState);
	}

	virtual void setSP(const Address& addr) { _ppcState->GPR[1] = addr.offset(); }

	inline ppc_state_t *ppcState(void) const { return _ppcState; }

	virtual Inst *execute(Inst *oinst) {
		ASSERTP(oinst, "null instruction pointer");

		Address addr = oinst->address();
		ppc_inst_t *inst;
		_ppcState->NIA = addr.offset();
		inst = ppc_decode(_ppcDecoder, _ppcState->NIA);
		ppc_execute(_ppcState, inst);
		ppc_free_inst(inst);
		if (_ppcState->NIA == oinst->topAddress()) {
			Inst *next = oinst->nextInst();
			while (next /*&& next->isPseudo()*/)
				next = next->nextInst();
			if(next && next->address() == Address(_ppcState->NIA))
				return next;
		}
		Inst *next = process()->findInstAt(_ppcState->NIA);
		ASSERTP(next, "cannot find instruction at " << Address(_ppcState->NIA) << " from " << oinst->address());
		return next;
	}

private:
	ppc_state_t *_ppcState;
	ppc_decoder_t *_ppcDecoder;
};

// Pre-declaration
class Inst;
class Segment;

/**
 * This class provides support to build a loader plug-in based on the GLISS V2
 * with ELF file loading based on the GEL library. Currently, this only includes
 * the PPC ISA.
 *
 * This class allows to load a binary file, extract the instructions and the
 * symbols (labels and function). You have to provide a consistent
 * platform description for the processor.
 *
 * The details of the interface with V2 GLISS are managed by this class and you
 * have only to write :
 *   - the platform description,
 *   - the recognition of the instruction,
 *	 - the assignment of the memory pointer.
 */
class Process: public otawa::Process {
public:
	Process(Manager *manager, hard::Platform *pf, const PropList& props = PropList::EMPTY);

	~Process();

	virtual otawa::SimState *newState(void) {
		ppc_state_t *s = ppc_new_state(_ppcPlatform);
		ASSERTP(s, "otawa::ppc2::Process::newState(), cannot create a new ppc_state");
		return new SimState(this, s, _ppcDecoder, true);
	}

	virtual int instSize(void) const {
#		ifdef PPC_WITH_VLE
			if(vle_enabled)
				return 0;
			else
				return 4;
#		else
			return 4;
#		endif
	}
	void decodeRegs(otawa::Inst *inst, elm::genstruct::AllocatedTable<hard::Register *> *in, elm::genstruct::AllocatedTable<hard::Register *> *out);
	inline ppc_decoder_t *ppcDecoder() { return _ppcDecoder;}
	inline void *ppcPlatform(void) const { return _ppcPlatform; }
	void setup(void);

	// Process Overloads
	virtual hard::Platform *platform(void);
	virtual otawa::Inst *start(void);
	virtual File *loadFile(elm::CString path);
	virtual void get(Address at, t::int8& val);
	virtual void get(Address at, t::uint8& val);
	virtual void get(Address at, t::int16& val);
	virtual void get(Address at, t::uint16& val);
	virtual void get(Address at, t::int32& val);
	virtual void get(Address at, t::uint32& val);
	virtual void get(Address at, t::int64& val);
	virtual void get(Address at, t::uint64& val);
	virtual void get(Address at, Address& val);
	virtual void get(Address at, string& str);
	virtual void get(Address at, char *buf, int size);
	virtual Option<Pair<cstring, int> > getSourceLine(Address addr)
		throw (UnsupportedFeatureException);
	virtual void getAddresses(cstring file, int line, Vector<Pair<Address, Address> >& addresses)
		throw (UnsupportedFeatureException);

	// Info overload
	virtual int opcode(otawa::Inst *inst) const {
		ppc_inst_t *i = decode_ppc(inst->address());
		int code = i->ident;
		ppc_free_inst(i);
		return code;
	}

protected:
	friend class Segment;
	friend class Inst;
	friend class BranchInst;
	virtual otawa::Inst *decode(Address addr);
	virtual gel_file_t *gelFile(void) { return _gelFile; }
	virtual ppc_memory_t *ppcMemory(void) { return _ppcMemory; }

private:
	ppc_inst_t *decode_ppc(Address addr) const {
#		ifndef PPC_WITH_VLE
			return ppc_decode(_ppcDecoder, ppc_address_t(addr.offset()));
#		else
			if(!vle_enabled)
				return ppc_decode_PPC(_ppcDecoder, ppc_address_t(addr.offset()));
			else
				return ppc_decode_VLE(_ppcDecoder, ppc_address_t(addr.offset()));
#		endif
	}

	int get_size(const otawa::Inst *inst) {
		ppc_inst_t *i = decode_ppc(inst->address());
		int r = ppc_get_inst_size(i) >> 3;
		ppc_free_inst(i);
		return r;
	}

	otawa::Inst *_start;
	hard::Platform *_platform;
	ppc_platform_t *_ppcPlatform;
	ppc_memory_t *_ppcMemory;
	ppc_decoder_t *_ppcDecoder;
	int argc;
	char **argv, **envp;
	bool no_stack;
	bool init;
	struct gel_line_map_t *map;
	gel_file_t *_gelFile;
	bool vle_enabled;
};

// Process display
elm::io::Output& operator<<(elm::io::Output& out, Process *proc)
	{ out << "Process(" << (void *)proc << ")"; return out; }


// Inst class
class Inst: public otawa::Inst {
public:

	inline Inst(Process& process, kind_t kind, Address addr)
		: proc(process), _kind(kind), _addr(addr), isRegsDone(false) { }

	/**
	 */
	void dump(io::Output& out) {
		char out_buffer[200];
		ppc_inst_t *inst = proc.decode_ppc(_addr);
		ppc_disasm(out_buffer, inst);
		ppc_free_inst(inst);
		out << out_buffer;
	}

	virtual kind_t kind() { return _kind; }
	virtual address_t address() const { return _addr; }

	virtual t::uint32 size() const {
#		ifdef PPC_WITH_VLE
			return proc.get_size(this);
#		else
			return 4;
#		endif
	}

	virtual Process &process() { return proc; }

	virtual const elm::genstruct::Table<hard::Register *>& readRegs() {
		if ( ! isRegsDone) {
			decodeRegs();
			isRegsDone = true;
		}
		return in_regs;
	}

	virtual const elm::genstruct::Table<hard::Register *>& writtenRegs() {
		if ( ! isRegsDone) {
			decodeRegs();
			isRegsDone = true;
		}
		return out_regs;
	}

	virtual void semInsts(sem::Block &block) {
		ppc_inst_t *inst = proc.decode_ppc(_addr);
		ppc_sem(inst, block);
		ppc_free_inst(inst);
	}

protected:
	virtual void decodeRegs(void) {
		proc.decodeRegs(this, &in_regs, &out_regs);
	}

	kind_t _kind;
	elm::genstruct::AllocatedTable<hard::Register *> in_regs;
	elm::genstruct::AllocatedTable<hard::Register *> out_regs;
	Process &proc;

private:
	ppc_address_t _addr;
	bool isRegsDone;
};


// BranchInst class
class BranchInst: public Inst {
public:

	inline BranchInst(Process& process, kind_t kind, Address addr)
	: Inst(process, kind, addr), _target(0), isTargetDone(false) {

		// only prediction for conditional and control
		if(!isConditional())
			return;

		// get description
		ppc_inst_t *inst = proc.decode_ppc(address());

		// manage the prediction:
		ppc::prediction_t pred = ppc::prediction_t(ppc_pred(inst));
		if(pred != ppc::PRED_NONE)
			ppc::STATIC_PREDICTION(this) = pred;

		// free descriptor
		ppc_free_inst(inst);
	}

	virtual t::uint32 size() const {
#		ifdef PPC_WITH_VLE
			return proc.get_size(this);
#		else
			return 4;
#		endif
	}

	virtual otawa::Inst *target() {
		if (!isTargetDone) {
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
	otawa::Inst *_target;
	bool isTargetDone;
};


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
		ot::size size,
		t::uint flags)
	: otawa::Segment(name, address, size, flags), proc(process) { }

protected:
	virtual otawa::Inst *decode(address_t address)
		{ return proc.decode(address); }

private:
	Process& proc;
};


 /**
  * Build a process for the new GLISS V2 system.
  * @param manager	Current manager.
  * @param platform	Current platform.
  * @param props	Building properties.
  */
Process::Process(Manager *manager, hard::Platform *platform, const PropList& props)
:	otawa::Process(manager, props),
 	_start(0),
 	_platform(platform),
	_ppcMemory(0),
	init(false),
	map(0),
	no_stack(true),
	vle_enabled(otawa::ppc::VLE_ENABLED(props)),
	_gelFile(0)
{
	ASSERTP(manager, "manager required");
	ASSERTP(platform, "platform required");

	// gliss2 ppc structs
	_ppcPlatform = ppc_new_platform();
	ASSERTP(_ppcPlatform, "cannot create a ppc_platform");
	_ppcDecoder = ppc_new_decoder(_ppcPlatform);
	ASSERTP(_ppcDecoder, "cannot create a ppc_decoder");
	_ppcMemory = ppc_get_memory(_ppcPlatform, PPC_MAIN_MEMORY);
	ASSERTP(_ppcMemory, "cannot get main ppc_memory");
	ppc_lock_platform(_ppcPlatform);
#	ifdef PPC_WITH_VLE
		ppc_set_cond_state(_ppcDecoder, 0);
#	endif

	// build arguments
	static char no_name[1] = { 0 };
	static char *default_argv[] = { no_name, 0 };
	static char *default_envp[] = { 0 };
	argc = ARGC(props);
	if (argc < 0)
		argc = 1;
	argv = ARGV(props);
	if (!argv)
		argv = default_argv;
	else
		no_stack = false;
	envp = ENVP(props);
	if (!envp)
		envp = default_envp;
	else
		no_stack = false;

	// handle features
	provide(MEMORY_ACCESS_FEATURE);
	provide(SOURCE_LINE_FEATURE);
	provide(CONTROL_DECODING_FEATURE);
	provide(REGISTER_USAGE_FEATURE);
	provide(MEMORY_ACCESSES);
	provide(otawa::ppc::INFO_FEATURE);
	ppc::GLISS2_DECODER(this) = _ppcDecoder;
}


/**
 */
Process::~Process() {
	ppc_delete_decoder(_ppcDecoder);
	ppc_unlock_platform(_ppcPlatform);
	if(_gelFile)
		gel_close(_gelFile);
}



/**
 */
Option<Pair<cstring, int> > Process::getSourceLine(Address addr) throw (UnsupportedFeatureException) {
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
void Process::getAddresses(cstring file, int line, Vector<Pair<Address, Address> >& addresses) throw (UnsupportedFeatureException) {
	setup();
	addresses.clear();
	if (!map)
		return;
	dwarf_line_iter_t iter;
	dwarf_location_t loc, ploc = { 0, 0, 0, 0 };
	for (loc = dwarf_first_line(&iter, map); loc.file; loc = dwarf_next_line(&iter))
	{
		cstring lfile = loc.file;
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
void Process::setup(void) {
	ASSERT(_gelFile);
	if(init)
		return;
	init = true;
	map = gel_new_line_map(_gelFile);
}


/**
 */
hard::Platform *Process::platform(void) {
	return _platform;
}


/**
 */
otawa::Inst *Process::start(void) {
	return _start;
}

inline IntFormat bytef(t::uint8 i) {
	return io::right(io::hex(io::pad('0', io::width(2, i))));
}

/**
 */
File *Process::loadFile(elm::CString path) {
	LTRACE;

	// Check if there is not an already opened file !
	if(program())
		throw LoadException("loader cannot open multiple files !");

	// make the file
	LTRACE;
	File *file = new otawa::File(path);
	addFile(file);

	// build the environment
	gel_env_t genv = *gel_default_env();
	genv.argv = argv;
	genv.envp = envp;
	if(no_stack)
		genv.flags = GEL_ENV_NO_STACK;

	// build the GEL image
	LTRACE;
	_gelFile = gel_open(&path, NULL, 0);
	if(!_gelFile)
		throw LoadException(_ << "cannot load \"" << path << "\": " << gel_strerror());
	gel_image_t *gimage = gel_image_load(_gelFile, &genv, 0);
	if(!gimage) {
		gel_close(_gelFile);
		throw LoadException(_ << "cannot build image of \"" << path << "\": " << gel_strerror());
	}

	// build the GLISS image
	gel_image_info_t iinfo;
	gel_image_infos(gimage, &iinfo);
	for(int i = 0; i < iinfo.membersnum; i++) {
		gel_cursor_t cursor;
		gel_block2cursor(iinfo.members[i], &cursor);
		ppc_mem_write(_ppcMemory,
			gel_cursor_vaddr(cursor),
			gel_cursor_addr(&cursor),
			gel_cursor_avail(cursor));
	}

	// cleanup image
	gel_image_close(gimage);

	// build segments
	LTRACE;
	gel_file_info_t infos;
	gel_file_infos(_gelFile, &infos);
	for (int i = 0; i < infos.sectnum; i++) {
		gel_sect_info_t infos;
		gel_sect_t *sect = gel_getsectbyidx(_gelFile, i);
		ASSERT(sect);
		gel_sect_infos(sect, &infos);
		if(infos.vaddr != 0 && infos.size != 0) {
			t::uint flags = 0;
			if(infos.flags & SHF_WRITE)
				flags |= Segment::WRITABLE;
			if(infos.flags & SHF_EXECINSTR)
				flags |= Segment::EXECUTABLE;
			if(infos.type == SHT_PROGBITS)
				flags |= Segment::INITIALIZED;
			Segment *seg = new Segment(*this, infos.name, infos.vaddr, infos.size, flags);
			file->addSegment(seg);
		}
	}

	// Initialize symbols
	LTRACE;
	gel_sym_iter_t iter;
	gel_sym_t *sym;
	for(sym = gel_sym_first(&iter, _gelFile); sym; sym = gel_sym_next(&iter)) {

		/* get information */
		gel_sym_info_t infos;
		gel_sym_infos(sym, &infos);
		if(cstring(infos.name) == "")
			continue;

		// compute the kind
		Symbol::kind_t kind = Symbol::NONE;
		switch(ELF32_ST_TYPE(infos.info)) {
		case STT_FUNC:
			kind = Symbol::FUNCTION;
			TRACE("SYMBOL: function " << infos.name << " at " << addr);
			break;
		case STT_NOTYPE:
			kind = Symbol::LABEL;
			TRACE("SYMBOL: notype " << infos.name << " at " << addr);
			break;
		case STT_OBJECT:
			kind = Symbol::DATA;
			TRACE("SYMBOL: object " << infos.name << " at " << addr);
			break;
		default:
			continue;
		}

		// Build the label if required
		String label(infos.name);
		Symbol *symbol = new Symbol(*file, label, kind, infos.vaddr, infos.size);
		file->addSymbol(symbol);
		TRACE("function " << label << " at " << addr);
	}

	// Last initializations
	LTRACE;
	_ppcMemory = ppcMemory();
	ASSERTP(_ppcMemory, "memory information mandatory");
	_start = findInstAt((address_t)infos.entry);
	return file;
}


// Memory read
#define GET(t, s) \
	void Process::get(Address at, t& val) { \
			val = ppc_mem_read##s(_ppcMemory, at.offset()); \
			/*cerr << "val = " << (void *)(int)val << " at " << at << io::endl;*/ \
	}
GET(t::int8, 8);
GET(t::uint8, 8);
GET(t::int16, 16);
GET(t::uint16, 16);
GET(t::int32, 32);
GET(t::uint32, 32);
GET(t::int64, 64);
GET(t::uint64, 64);
GET(Address, 32);


/**
 */
void Process::get(Address at, string& str) {
	Address base = at;
	while(!ppc_mem_read8(_ppcMemory, at.offset()))
		at = at + 1;
	int len = at - base;
	char buf[len];
	get(base, buf, len);
	str = String(buf, len);
}


/**
 */
void Process::get(Address at, char *buf, int size)
	{ ppc_mem_read(_ppcMemory, at.offset(), buf, size); }


/**
 */
otawa::Inst *Process::decode(Address addr) {

	// Decode the instruction
	ppc_inst_t *inst;
	TRACE("ADDR " << addr);
	inst = decode_ppc(addr);

	// Build the instruction
	Inst::kind_t kind = 0;
	otawa::Inst *result = 0;

	// get the kind from the nmp otawa_kind attribute
	if(inst->ident == PPC_UNKNOWN)
		kind = Inst::IS_UNKNOWN;
	else
		kind = ppc_kind(inst);
	bool is_branch = kind & Inst::IS_CONTROL;

	// build the object
	if (is_branch)
		result = new BranchInst(*this, kind, addr);
	else
		result = new Inst(*this, kind, addr);

	// cleanup
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
	inst = proc.decode_ppc(address());

	// retrieve the target addr from the nmp otawa_target attribute
	Address target_addr = ppc_target(inst);

	// cleanup
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
	ASSERT(reg_count > 0);

	const hard::RegBank *reg_bank = 0;
	hard::Register *reg_no_bank = 0;
	switch (gliss_bank)
	{
		case 255:
			return;
		case BANK_GPR:
			reg_bank = &GPR_bank;
			break;
		case BANK_FPR:
			reg_bank = &FPR_bank;
			break;
		case BANK_CR:
			reg_bank = &CR_bank;
			break;
		case BANK_XER:
			reg_no_bank = &XER_reg;
			break;
		case BANK_LR:
			reg_no_bank = &LR_reg;
			break;
		case BANK_CTR:
			reg_no_bank = &CTR_reg;
			break;
		default:
			ASSERTP(false, "unknown bank " << gliss_bank);
			break;
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
	inst = decode_ppc(oinst->address());
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
otawa::Process *Loader::create(Manager *man, const PropList& props) {
	return new Process(man, new Platform(props), props);
}


/**
 * PowerPC is also a processor plugin.
 */
class Plugin: public ProcessorPlugin {
public:
	Plugin(): ProcessorPlugin("otawa::ppc", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
	virtual elm::genstruct::Table<AbstractRegistration *>& processors(void) const {
		static elm::genstruct::Table<AbstractRegistration *> none;
		return none;
	}
};

} }	// namespace otawa::ppc2


// PowerPC GLISS Loader entry point
otawa::ppc2::Loader OTAWA_LOADER_HOOK;
otawa::ppc2::Loader& ppc2_plugin = OTAWA_LOADER_HOOK;
otawa::ppc2::Plugin OTAWA_PROC_HOOK;
