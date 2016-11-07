/*
 *	arm2 -- OTAWA loader to support ARMv5 ISA with GLISS2
 *	PowerPC OTAWA plugin
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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

#include <gel/debug_line.h>
#include <gel/gel.h>
#include <gel/gel_elf.h>

#include <elm/stree/MarkerBuilder.h>
#include <elm/stree/SegmentBuilder.h>

#include "config.h"
#include <otawa/hard.h>
#include <otawa/loader/arm.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/prog/Loader.h>
#include <otawa/prog/sem.h>
#include <otawa/program.h>

extern "C" {
#	include <arm/grt.h>
#	include <arm/api.h>
#	include <arm/config.h>
#	include <arm/used_regs.h>
}

namespace otawa {

namespace arm {

/*
 * Configuration
 * * ARM_SIM -- integrate functional simulator in ARM plugin.
 */

#define VERSION "2.1.0"
OTAWA_LOADER_ID("arm2", VERSION, OTAWA_DATE);

/**
 * @class Info
 * Information provided by an ARM loader supporting GLISS2 engine.
 */

/**
 * @fn void *Info::decode(Inst *inst);
 * Get the decode instruction by GLISS2.
 * The obtained data must be fried with free() method.
 * @param inst	Instruction to decode.
 * @return		Decoded data castable to arm_inst_t *.
 */

/**
 * @fn void Info::free(void *decoded);
 * Free an instruction previously obtained by decode() method.
 * @paramd decoded	Result of decode.
 */


/**
 * t::uint16 Info::multiMask(Inst *inst);
 * For a multiple register memory instruction,
 * get the mask of processed register (1 bit per register).
 * @param inst	Concerned instruction.
 * @return		Mask of processed registers.
 */


/**
 * Identifier used to retrieve ARM specific information.
 * Must be accessed dynamically because ARM loader is a plugin !
 *
 * @par Hooks
 * @li Process from ARM loader.
 */
Identifier <Info *> Info::ID("otawa::arm::Info::ID", 0);


/**
 */
Info::~Info(void) {
}

}

namespace arm2 {

#include "otawa_kind.h"
#include "otawa_target.h"
#include "otawa_multi.h"


/****** Platform definition ******/

#ifdef ARM_SIM
#	ifndef ARM_MEM_SPY
#		error "To support simulation, ARMv5T must be compiled with option WITH_MEM_SPY."
#	endif
class SimState: public otawa::SimState {
public:
	SimState(Process *process, arm_platform_t *platform, arm_decoder_t *_decoder)
	:	otawa::SimState(process) {
		sim = arm_new_sim(arm_new_state(platform), process->start()->address(), 0);
		arm_mem_set_spy(arm_get_memory(platform, 0), spy, this);
	}

	virtual ~SimState(void) {
		arm_delete_sim(sim);
	}

	virtual Inst *execute(Inst *inst) {

		// execute current instruction
		dr = dw = 0; // enable spy
		arm_step(sim);
		dr = dw = -1; // disable spy

		// get next instruction
		arm_state_t *state = sim->state;
		Inst *next = inst->nextInst();
		if(next && (next->address().offset() == state->GPR[15]))
			return next;
		else
			return this->process()->findInstAt(state->GPR[15]);
	}

	// register access
	virtual void setSP(const Address& addr) {
		sim->state->GPR[13] = addr.offset();
	}

	virtual t::uint32 getReg(hard::Register *r) {
		t::uint32 temp = 0;
		if((r->bank()->name() == "misc") && (r->name() == "sr"))
			temp = sim->state->Ucpsr;
		else if (r->bank()->name() == "GPR")
			temp = sim->state->GPR[r->number()];
		return temp;
	}

	virtual void setReg(hard::Register *r, t::uint32 v) {
		if((r->bank()->name() == "misc") && (r->name() == "sr"))
			sim->state->Ucpsr = v;
		else if (r->bank()->name() == "GPR")
			sim->state->GPR[r->number()] = v;
	 }

	// memory accesses
	virtual Address lowerRead(void) {
		return lr;
	}

	virtual Address upperRead(void) {
		return ur;
	}

	virtual Address lowerWrite(void) {
		return lw;
	}

	virtual Address upperWrite(void) {
		return uw;
	}

private:
	arm_sim_t *sim;
	int dr, dw; // -1: disable, 0:ready to serve, 1:first data, 2+: later data
	arm_address_t lr, ur, lw, uw;

	static void spy(arm_memory_t *mem, arm_address_t addr, arm_size_t size, arm_access_t access, void *data) {
		SimState *ss = static_cast<SimState *>(data);
		if(access == arm_access_read) {
			if(ss->dr == -1) { } // ignore other reads due to non-ISS operations
			else if(ss->dr == 0) { // read instruction from memory
				ss->lr = ss->ur = addr;
				ss->dr = 1;
			}
			else if(ss->dr == 1) { // first data read set the lower and upper read to the same value
				ss->lr = ss->ur = addr;
				ss->dr++;
			}
			else if(addr < ss->lr)
				ss->lr = addr;
			else if(addr > ss->ur)
				ss->ur = addr;
		}
		else {
			if(ss->dw == -1) { } // ignore other writes due to non-ISS operation
			else if(ss->dw == 0) { // first write to data
				ss->lw = ss->uw = addr;
				ss->dw = 1;
			}
			else if(addr < ss->lw)
				ss->lw = addr;
			else if(addr > ss->uw)
				ss->uw = addr;
		}
	}
};
#endif

// registers
static hard::PlainBank gpr("GPR", hard::Register::INT, 32, "r%d", 16);
static hard::Register sr("sr", hard::Register::BITS, 32);
static hard::MeltedBank misc("misc", &sr, (void *) 0);
static const hard::RegBank *banks_tab[] = { &gpr, &misc };
static genstruct::Table<const hard::RegBank *> banks_table(banks_tab, 2);

// register decoding
class RegisterDecoder {
public:
	RegisterDecoder(void) {

		// clear the map
		for(int i = 0; i < ARM_REG_COUNT; i++)
			map[i] = 0;

		// initialize it
#		ifdef ARM7
			map[ARM_REG_APSR] = &sr;
#		else
			map[ARM_REG_UCPSR] = &sr;
#		endif
		for(int i = 0; i < 16; i++)
			map[ARM_REG_GPR(i)] = gpr[i];
	}

	inline hard::Register *operator[](int i) const { ASSERT(i < ARM_REG_COUNT); return map[i]; }

private:
	hard::Register *map[ARM_REG_COUNT];
};
static RegisterDecoder register_decoder;


// semantics functions
#define _GPR(i)			gpr[i]->platformNumber()
#define _CPSR()			sr.platformNumber()
#define _BRANCH(t)		sem::branch(t)
#define _TRAP()			sem::trap(0)
#define _CONT()			sem::cont()
#define _IF(c, r, o)	sem::_if(c, r, o)
#define _LOAD(d, a, b)	sem::load(d, a, b)
#define _STORE(d, a, b)	sem::store(d, a, b)
#define _SCRATCH(d)		sem::scratch(d)

#include "otawa_sem.h"


// platform
class Platform: public hard::Platform {
public:
	static const Identification ID;

	Platform(const PropList& props = PropList::EMPTY): hard::Platform(ID, props)
		{ setBanks(banks_table); }
	Platform(const Platform& platform, const PropList& props = PropList::EMPTY)
		: hard::Platform(platform, props)
		{ setBanks(banks_table); }

	// otawa::Platform overload
	virtual bool accept(const Identification& id) { return id.abi() == "eabi" && id.architecture() == "arm"; }
	virtual const hard::Register *getSP(void) const { return gpr[13]; }
};
const Platform::Identification Platform::ID("arm-eabi-");


/****** Instruction declarations ******/

class Process;

#include "otawa_condition.h"

// Inst class
class Inst: public otawa::Inst {
public:

	inline Inst(Process& process, kind_t kind, Address addr, int size)
		: proc(process), _kind(kind), _size(size), _addr(addr), isRegsDone(false) {
		}

	// Inst overload
	virtual void dump(io::Output& out);
	virtual kind_t kind() { return _kind; }
	virtual address_t address() const { return _addr; }
	virtual t::uint32 size() const { return _size; }
	virtual Process &process() { return proc; }

	virtual const elm::genstruct::Table<hard::Register *>& readRegs() {
		if (!isRegsDone) {
			decodeRegs();
			isRegsDone = true;
		}
		return in_regs;
	}

	virtual const elm::genstruct::Table<hard::Register *>& writtenRegs() {
		if(!isRegsDone) {
			decodeRegs();
			isRegsDone = true;
		}
		return out_regs;
	}

	virtual void semInsts(sem::Block &block);
	virtual sem::cond_t condition(void);

protected:
	Process &proc;
	kind_t _kind;
	int _size;

private:
	void decodeRegs(void);
	elm::genstruct::AllocatedTable<hard::Register *> in_regs;
	elm::genstruct::AllocatedTable<hard::Register *> out_regs;
	arm_address_t _addr;
	bool isRegsDone;
};


// BranchInst class
class BranchInst: public Inst {
public:

	inline BranchInst(Process& process, kind_t kind, Address addr, int size)
		: Inst(process, kind & ~IS_CONTROL, addr, size), _target(0), isTargetDone(false)
		{ }

	virtual otawa::Inst *target();
	virtual kind_t kind(void);

protected:
	arm_address_t decodeTargetAddress(void);

private:
	otawa::Inst *_target;
	bool isTargetDone;
};



/****** Segment class ******/
class Segment: public otawa::Segment {
public:
	Segment(Process& process,
		CString name,
		address_t address,
		t::uint32 size,
		int flags = EXECUTABLE)
	: otawa::Segment(name, address, size, flags), proc(process) { }

protected:
	virtual otawa::Inst *decode(address_t address);

private:
	Process& proc;
};


/****** Process class ******/

typedef enum { NONE = 0, ARM = 1, THUMB = 2, DATA = 3 } area_t;

io::Output& operator<<(io::Output& out, area_t area) {
	switch(area) {
	case NONE:	out << "none"; break;
	case ARM:	out << "arm"; break;
	case THUMB:	out << "thumb"; break;
	case DATA:	out << "data"; break;
	}
	return out;
}

class Process: public otawa::Process, public arm::Info {
	typedef elm::stree::Tree<Address::offset_t, area_t> area_tree_t;
public:
	static const t::uint32 IS_BL_0			= 0x08000000,
							 IS_BL_1 		= 0x04000000,
							 IS_BX_IP 		= 0x02000000,
							 IS_THUMB_BX	= 0x01000000;

	Process(Manager *manager, hard::Platform *pf, const PropList& props = PropList::EMPTY)
	:	otawa::Process(manager, props),
	 	_start(0),
	 	oplatform(pf),
	 	_platform(0),
		_memory(0),
		_decoder(0),
		map(0),
		_file(0),
		argc(0),
		argv(0),
		envp(0),
		no_stack(true),
		init(false)
	{
		ASSERTP(manager, "manager required");
		ASSERTP(pf, "platform required");

		// gliss2 ppc structs
		_platform = arm_new_platform();
		ASSERTP(_platform, "cannot create an arm_platform");
		_decoder = arm_new_decoder(_platform);
		ASSERTP(_decoder, "cannot create an arm_decoder");
		_memory = arm_get_memory(_platform, ARM_MAIN_MEMORY);
		ASSERTP(_memory, "cannot get main arm_memory");
		arm_lock_platform(_platform);
#		ifdef ARM_THUMB
			arm_set_cond_state(_decoder, 0);
#		endif

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
		provide(CONDITIONAL_INSTRUCTIONS_FEATURE);
		Info::ID(this) = this;
	}

	virtual ~Process() {
		arm_delete_decoder(_decoder);
		arm_unlock_platform(_platform);
		if(_file)
			gel_close(_file);
	}

	// Process overloads
	virtual int instSize(void) const {
#		ifdef ARM_THUMB
			return 0;
#		else
			return 4;
#		endif
	}
	virtual hard::Platform *platform(void) { return oplatform; }
	virtual otawa::Inst *start(void) { return _start; }

#	ifdef ARM_SIM
		virtual SimState *newState(void) {
			return new SimState(this, _platform, _decoder);
		}
#	endif

	virtual File *loadFile(elm::CString path) {

		// check if there is not an already opened file !
		if(program())
			throw LoadException("loader cannot open multiple files !");

		// make the file
		File *file = new otawa::File(path);
		addFile(file);

		// build the environment
		gel_env_t genv = *gel_default_env();
		genv.argv = argv;
		genv.envp = envp;
		if(no_stack)
			genv.flags = GEL_ENV_NO_STACK;

		// build the GEL image
		_file = gel_open(&path, NULL, GEL_OPEN_QUIET);
		if(!_file)
			throw LoadException(_ << "cannot load \"" << path << "\": " << gel_strerror());
		gel_image_t *gimage = gel_image_load(_file, &genv, 0);
		if(!gimage) {
			gel_close(_file);
			throw LoadException(_ << "cannot build image of \"" << path << "\": " << gel_strerror());
		}

		// build the GLISS image
		gel_image_info_t iinfo;
		gel_image_infos(gimage, &iinfo);
		for(t::uint32 i = 0; i < iinfo.membersnum; i++) {
			gel_cursor_t cursor;
			gel_block2cursor(iinfo.members[i], &cursor);
			if(gel_cursor_avail(cursor) > 0)
				arm_mem_write(_memory,
					gel_cursor_vaddr(cursor),
					gel_cursor_addr(&cursor),
					gel_cursor_avail(cursor));
		}

		// cleanup image
		gel_image_close(gimage);

		// build segments
		stree::SegmentBuilder<t::uint32, bool> builder(false);
		gel_file_info_t infos;
		gel_file_infos(_file, &infos);
		for (int i = 0; i < infos.sectnum; i++) {
			gel_sect_info_t infos;
			gel_sect_t *sect = gel_getsectbyidx(_file, i);
			ASSERT(sect);
			gel_sect_infos(sect, &infos);
			if(infos.vaddr != 0 && infos.size != 0) {
				int flags = 0;
				if(infos.flags & SHF_EXECINSTR) {
					flags |= Segment::EXECUTABLE;
					builder.add(infos.vaddr, infos.vaddr + infos.size, true);
				}
				if(infos.flags & SHF_WRITE)
					flags |= Segment::WRITABLE;
				if(infos.type == SHT_PROGBITS)
					flags |= Segment::INITIALIZED;
				Segment *seg = new Segment(*this, infos.name, infos.vaddr, infos.size, flags);
				file->addSegment(seg);
			}
		}
		stree::Tree<t::uint32, bool> execs;
		builder.make(execs);

		// Initialize symbols
#		ifdef ARM_THUMB
			stree::MarkerBuilder<Address::offset_t, area_t> area_builder;
			area_builder.add(0UL, (infos.entry & 0x1) ? THUMB : ARM);
			area_builder.add(0xffffffffUL, NONE);
#		endif
		gel_sym_iter_t iter;
		gel_sym_t *sym;
		for(sym = gel_sym_first(&iter, _file); sym; sym = gel_sym_next(&iter)) {

			// get the symbol description
			gel_sym_info_t infos;
			gel_sym_infos(sym, &infos);

			// handle the $a, $t or $d symbols
#			ifdef ARM_THUMB
				if(infos.name[0] == '$') {
					area_t area = NONE;
					switch(infos.name[1]) {
					case 'a': area = ARM; break;
					case 'd': area = DATA; break;
					case 't': area = THUMB; break;
					}
					if(area != NONE) {
						area_builder.add(Address::offset_t(infos.vaddr & 0xfffffffe), area);
						continue;
					}
				}
#			endif

			// compute the kind
			Symbol::kind_t kind = Symbol::NONE;
			t::uint32 mask = 0xffffffff;
			switch(ELF32_ST_TYPE(infos.info)) {
			case STT_FUNC:
				kind = Symbol::FUNCTION;
				mask = 0xfffffffe;
				break;
			case STT_NOTYPE:
				if(!execs.contains(infos.vaddr & mask))
					continue;
				kind = Symbol::LABEL;
				break;
			case STT_OBJECT:
				kind = Symbol::DATA;
				break;
			default:
				continue;
			}

			// Build the label if required
			String label(infos.name);
			Symbol *symbol = new Symbol(*file, label, kind, infos.vaddr & mask, infos.size);
			file->addSymbol(symbol);
		}
		//gel_enum_free(iter);
#		ifdef ARM_THUMB
			area_builder.make(area_tree);
#		endif

		// Last initializations
		_start = findInstAt(Address(infos.entry & 0xfffffffe));
		return file;
	}

	// internal work
	void decodeRegs(Inst *oinst,
		elm::genstruct::AllocatedTable<hard::Register *> *in,
		elm::genstruct::AllocatedTable<hard::Register *> *out)
	{
		// Decode instruction
		arm_inst_t *inst = decode_raw(oinst->address());
		if(inst->ident == ARM_UNKNOWN) {
			free_inst(inst);
			return;
		}

		// get the registers
		arm_used_regs_read_t rds;
		arm_used_regs_write_t wrs;
		arm_used_regs(inst, rds, wrs);

		// convert registers to OTAWA
		elm::genstruct::Vector<hard::Register *> reg_in;
		elm::genstruct::Vector<hard::Register *> reg_out;
		for (int i = 0; rds[i] != -1; i++ ) {
			hard::Register *r = register_decoder[rds[i]];
			if(r)
				reg_in.add(r);
		}
		for (int i = 0; wrs[i] != -1; i++ ) {
			hard::Register *r = register_decoder[wrs[i]];
			if(r)
				reg_out.add(r);
		}

		// make the in and the out
		in->allocate(reg_in.length());
		for(int i = 0 ; i < reg_in.length(); i++)
			in->set(i, reg_in.get(i));
		out->allocate(reg_out.length());
		for (int i = 0 ; i < reg_out.length(); i++)
			out->set(i, reg_out.get(i));

		// Free instruction
		free_inst(inst);
	}

	otawa::Inst *decode(Address addr) {
		arm_inst_t *inst = decode_raw(addr);
		Inst::kind_t kind = 0;
		otawa::Inst *result = 0;
		kind = arm_kind(inst);
		int size;
		if(inst->ident != 0)
			size = arm_get_inst_size(inst) >> 3;
		else if(isThumb(addr))
			size = 2;
		else
			size = 4;
		if(kind & Inst::IS_CONTROL)
			result = new BranchInst(*this, kind, addr, size);
		else
			result = new Inst(*this, kind, addr, size);
		ASSERT(result);
		t::uint16 multi = arm_multi(inst);
		if(multi)
			otawa::arm::NUM_REGS_LOAD_STORE(result) = elm::ones(multi);
		char buf[256];
		arm_disasm(buf, inst);
		t::uint8 b0, b1;
		get(addr, b0);
		get(addr + 1, b1);
		free_inst(inst);
		return result;
	}


	// GLISS2 ARM access
	inline int opcode(Inst *inst) const {
		arm_inst_t *i = decode_raw(inst->address());
		int code = i->ident;
		free_inst(i);
		return code;
	}

	inline ::arm_inst_t *decode_raw(Address addr) const {
#		ifdef ARM_THUMB
			if(isThumb(addr))
				return arm_decode_THUMB(decoder(), ::arm_address_t(addr.offset()));
			else
				return arm_decode_ARM(decoder(), ::arm_address_t(addr.offset()));
#		else
			return arm_decode(decoder(), ::arm_address_t(addr.offset()));
#		endif
	}

	inline void free_inst(arm_inst_t *inst) const { arm_free_inst(inst); }
	virtual gel_file_t *file(void) const { return _file; }
	virtual arm_memory_t *memory(void) const { return _memory; }
	inline arm_decoder_t *decoder() const { return _decoder; }
	inline void *platform(void) const { return _platform; }

	virtual Option<Pair<cstring, int> > getSourceLine(Address addr) throw (UnsupportedFeatureException) {
		setup_debug();
		if (!map)
			return none;
		const char *file;
		int line;
		if (!map || gel_line_from_address(map, addr.offset(), &file, &line) < 0)
			return none;
		return some(pair(cstring(file), line));
	}

	virtual void getAddresses(cstring file, int line, Vector<Pair<Address, Address> >& addresses) throw (UnsupportedFeatureException) {
		setup_debug();
		addresses.clear();
		if (!map)
			return;
		gel_line_iter_t iter;
		gel_location_t loc, ploc = { 0, 0, 0, 0 };
		// TODO		Not very performant but it works.
		for (loc = gel_first_line(&iter, map); loc.file; loc = gel_next_line(&iter)) {
			cstring lfile = loc.file;
			if (file == loc.file || lfile.endsWith(file)) {
				if (t::uint32(line) == loc.line)
					addresses.add(pair(Address(loc.low_addr), Address(loc.high_addr)));
				else if(loc.file == ploc.file && t::uint32(line) > ploc.line && t::uint32(line) < loc.line)
					addresses.add(pair(Address(ploc.low_addr), Address(ploc.high_addr)));
			}
			ploc = loc;
		}
	}

	virtual void get(Address at, t::int8& val)
		{ val = arm_mem_read8(_memory, at.offset()); }
	virtual void get(Address at, t::uint8& val)
		{ val = arm_mem_read8(_memory, at.offset()); }
	virtual void get(Address at, t::int16& val)
		{ val = arm_mem_read16(_memory, at.offset()); }
	virtual void get(Address at, t::uint16& val)
		{ val = arm_mem_read16(_memory, at.offset()); }
	virtual void get(Address at, t::int32& val)
		{ val = arm_mem_read32(_memory, at.offset()); }
	virtual void get(Address at, t::uint32& val)
		{ val = arm_mem_read32(_memory, at.offset()); }
	virtual void get(Address at, t::int64& val)
		{ val = arm_mem_read64(_memory, at.offset()); }
	virtual void get(Address at, t::uint64& val)
		{ val = arm_mem_read64(_memory, at.offset()); }
	virtual void get(Address at, Address& val)
		{ val = arm_mem_read32(_memory, at.offset()); }
	virtual void get(Address at, string& str) {
		Address base = at;
		while(!arm_mem_read8(_memory, at.offset()))
			at = at + 1;
		int len = at - base;
		char buf[len];
		get(base, buf, len);
		str = String(buf, len);
	}
	virtual void get(Address at, char *buf, int size)
		{ arm_mem_read(_memory, at.offset(), buf, size); }

	// otawa::arm::Info overload
	virtual void *decode(otawa::Inst *inst) { return decode_raw(inst->address()); }
	virtual void free(void *decoded) { free_inst(static_cast<arm_inst_t *>(decoded)); }

	virtual t::uint16 multiMask(otawa::Inst *inst) {
		arm_inst_t *i = decode_raw(inst->address());
		t::uint16 r = arm_multi(i);
		free_inst(i);
		return r;
	}

	virtual void handleIO(Address addr, t::uint32 size, otawa::arm::IOManager& man) {
#		ifndef ARM_MEM_IO
			ASSERTP(false, "WITH_MEM_IO not configured in arm GLISS plugin!");
#		else
			//io_man = &man;
			arm_set_range_callback(memory(), addr.offset(), addr.offset() + size, io_callback, &man);
#		endif
	}

private:

#	ifdef ARM_MEM_IO
	static void io_callback(arm_address_t addr, int size, void *data, int type_access, void *cdata) {
		otawa::arm::IOManager *man = static_cast<otawa::arm::IOManager *>(cdata);
		if(type_access == ARM_MEM_READ)
			man->read(addr, size, static_cast<t::uint8 *>(data));
		else if(type_access == ARM_MEM_WRITE)
			man->write(addr, size, static_cast<t::uint8 *>(data));
		else
			ASSERT(0);
	}
#	endif

	/**
	 * Test if the given address matches a thumb code area.
	 * @param address	Address to test.
	 * @return			True if the address in the thumb area, false else.
	 */
	bool isThumb(const Address& address) const {
#		ifdef ARM_THUMB
			area_t kind = area_tree.get(address.offset());
			return kind == THUMB;
#		else
			return false;
#		endif
	}

	void setup_debug(void) {
		ASSERT(_file);
		if(init)
			return;
		init = true;
		map = gel_new_line_map(_file);
	}

	otawa::Inst *_start;
	hard::Platform *oplatform;
	arm_platform_t *_platform;
	arm_memory_t *_memory;
	arm_decoder_t *_decoder;
	gel_line_map_t *map;
	gel_file_t *_file;
	int argc;
	char **argv, **envp;
	bool no_stack;
	bool init;
#	ifdef ARM_THUMB
		area_tree_t area_tree;
#	endif
#	ifdef ARM_MEM_IO
		otawa::arm::IOManager *io_man;
#	endif
};


/****** Instructions implementation ******/

void Inst::dump(io::Output& out) {
	char out_buffer[200];
	arm_inst_t *inst = proc.decode_raw(_addr);
	arm_disasm(out_buffer, inst);
	proc.free_inst(inst);
	out << out_buffer;
}

void Inst::decodeRegs(void) {

	// Decode instruction
	arm_inst_t *inst = proc.decode_raw(address());
	if(inst->ident == ARM_UNKNOWN)
		return;

	// get register infos
	arm_used_regs_read_t rds;
	arm_used_regs_write_t wrs;
	elm::genstruct::Vector<hard::Register *> reg_in;
	elm::genstruct::Vector<hard::Register *> reg_out;
	arm_used_regs(inst, rds, wrs);
	for (int i = 0; rds[i] != -1; i++ ) {
		hard::Register *r = register_decoder[rds[i]];
		if(r)
			reg_in.add(r);
	}
	for (int i = 0; wrs[i] != -1; i++ ) {
		hard::Register *r = register_decoder[wrs[i]];
		if(r)
			reg_out.add(r);
	}

	// store results
	in_regs.allocate(reg_in.length());
	for(int i = 0 ; i < reg_in.length(); i++)
		in_regs.set(i, reg_in.get(i));
	out_regs.allocate(reg_out.length());
	for (int i = 0 ; i < reg_out.length(); i++)
		out_regs.set(i, reg_out.get(i));

	// Free instruction
	arm_free_inst(inst);
}


arm_address_t BranchInst::decodeTargetAddress(void) {

	// get the target
	arm_inst_t *inst= proc.decode_raw(address());
	Address target_addr = arm_target(inst);

	// thumb-1 case

#		ifdef ARM_THUMB

		// blx/0; blx/1
		if(_kind & Process::IS_BL_1) {
			arm_inst_t *pinst = proc.decode_raw(address() - 2);
			Inst::kind_t pkind = arm_kind(pinst);
			if(pkind & Process::IS_BL_0) {
				target_addr = arm_target(pinst) + target_addr.offset();
			}
			proc.free_inst(pinst);
		}

		// ldr ip, [pc, #k]; bx ip
		else if(kind() & Process::IS_BX_IP) {

			// look current and previous instruction words
			t::uint32 cur_word, pre_word;
			proc.get(address() - 4, pre_word);
			proc.get(address(), cur_word);

			// is it ldr ip, [pc, #k] with same condition ?
			if(((pre_word & 0x0ffff000) == 0x059fc000)
			&& ((pre_word & 0xf0000000) == (cur_word & 0xf0000000))) {
				// load address from M[pc + 8 + k]
				Address addr = address() + 4 + (pre_word & 0xfff);
				t::uint32 target;
				proc.get(addr, target);
				target_addr = target & 0xfffffffe;
			}
		}
#		endif

	// cleanup
	proc.free_inst(inst);
	return target_addr;
}


/**
 */
BranchInst::kind_t BranchInst::kind(void) {
	if(!(_kind & IS_CONTROL)) {
		_kind |= IS_CONTROL;

		// thumb: pop { ..., ri, ... }; bx ri
		if(_kind & Process::IS_THUMB_BX) {
			t::uint16 pre_half, cur_half;
			process().get(address(), cur_half);
			process().get(address() - 2, pre_half);
			int r = (cur_half >> 3) & 0xf;
			if(r < 8
			&& (pre_half & 0xff00) == 0xbc00
			&& (pre_half & (1 << r)))
				_kind |= IS_RETURN;
		}

		// mov lr, pc; mov pc, ri
		else if(size() == 4 && prevInst()->topAddress() == address()) {

			// get instruction words
			t::uint32 cword, pword;
			process().get(address(), cword);
			process().get(prevInst()->address(), pword);

			// check instructions
			if((pword & 0x0fffffff) == 0x01a0e00f				// mov pc, lr
			&& (pword & 0xf0000000) == (cword & 0xf0000000)		// same condition
			&& (cword & 0x0ffff000) == 0x01a0f000)
				_kind |= IS_CALL;
		}

	}
	return _kind;
}

otawa::Inst *BranchInst::target() {
	if (!isTargetDone) {
		isTargetDone = true;
		arm_address_t a = decodeTargetAddress();
		if (a)
			_target = process().findInstAt(a);
	}
	return _target;
}


otawa::Inst *Segment::decode(address_t address) {
	return proc.decode(address);
}


void Inst::semInsts (sem::Block &block) {

	// get the block
	arm_inst_t *inst = proc.decode_raw(address());
	if(inst->ident == ARM_UNKNOWN)
		return;
	arm_sem(inst, block);
	arm_free_inst(inst);

	// fix spurious instructions possibly generated with conditional instructions
	for(int i = 0; i < block.length(); i++)
		if(block[i].op == sem::CONT) {
			block.setLength(i);
			break;
		}
}


sem::cond_t Inst::condition(void) {
	arm_inst_t *inst = proc.decode_raw(address());
	sem::cond_t res;
	switch (arm_condition(inst)) {
	case 0: 	res = sem::EQ; 			break;
	case 1: 	res = sem::NE; 			break;
	case 2: 	res = sem::UGE; 		break;
	case 3: 	res = sem::ULT; 		break;
	case 8: 	res = sem::UGT; 		break;
	case 9:		res = sem::ULE; 		break;
	case 10:	res = sem::GE; 			break;
	case 11:	res = sem::LT; 			break;
	case 12:	res = sem::GT; 			break;
	case 13: 	res = sem::LE; 			break;
	case 14:	res = sem::NO_COND; 	break;
	default: 	res = sem::ANY_COND;	break;
	}
	arm_free_inst(inst);
	return res;
}


/****** loader definition ******/

// loader definition
class Loader: public otawa::Loader {
public:
	Loader(void): otawa::Loader(make("arm", OTAWA_LOADER_VERSION)
		.version(VERSION)
		.description("loader for ARM 32-bit architecture")
		.license(Manager::copyright)
		.alias("elf_40")
		.alias("arm2")) { }

	virtual CString getName(void) const { return "arm"; }

	virtual otawa::Process *load(Manager *man, CString path, const PropList& props) {
		otawa::Process *proc = create(man, props);
		if (!proc->loadProgram(path)) {
			delete proc;
			return 0;
		}
		else
			return proc;
	}

	virtual otawa::Process *create(Manager *man, const PropList& props) {
		return new Process(man, new Platform(props), props);
	}
};


// plugin definition
class Plugin: public otawa::ProcessorPlugin {
public:
	Plugin(void): otawa::ProcessorPlugin(make("otawa/arm", OTAWA_PROC_VERSION)
		.version(Version(VERSION))
		.description("plugin providing access to ARM specific resources")
		.license(Manager::copyright)) { }
};

} }		// otawa::arm2

otawa::arm2::Loader otawa_arm2_loader;
ELM_PLUGIN(otawa_arm2_loader, OTAWA_LOADER_HOOK);
otawa::arm2::Plugin arm2_plugin;
ELM_PLUGIN(arm2_plugin, OTAWA_PROC_HOOK);

