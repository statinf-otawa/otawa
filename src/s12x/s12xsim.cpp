/**
 * $Id$
 * Copyright (c) 2007, IRIT - UPS <casse@irit.fr>
 *
 * Star12X temporal simulator.
 * This file is part of OTAWA
 *
 * OTAWA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * OTAWA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define ISS_DISASM
#include <emul.h>
#include <iss_include.h>
#include "s12x.h"

#ifndef NDEBUG
#	define ASSERTT(cnd, msg)	if(!(cnd)) onTimeError(time, _ << msg, inst)
#	define TRACE(msg)	//cerr << msg << io::endl
#else
#	define ASSERTT(cnd, msg)
#	define TRACE(msg)
#endif

//#define STATS

#ifdef STATS
class MemStats {
public:

	MemStats(void): count(0), resolved(0), file("mem.txt"), out(file) {
	}
	
	~MemStats(void) {
		elm::cerr << "[MEM] COUNT = " << count << elm::io::endl;
		elm::cerr << "[MEM] RESOLVED = " << resolved << elm::io::endl;
		file.close();
	}
	
	void add(void) {
		resolved++;
		count++;
	}
	
	void unresolve(otawa::Inst *inst) {
		out << inst << elm::io::endl;
		resolved--;
	}
	
private:
	int count, resolved;
	elm::io::OutFileStream file;
	elm::io::Output out;
};
static MemStats stats;
#endif

namespace otawa { namespace s12x {

// State class
class State: public otawa::sim::State {
public:

	State(WorkSpace *ws, state_t *state): _ws(ws), _state(state), _cycle(0) {
		ASSERT(ws);
		ASSERT(state);
	}
	
	State(const State& state)
		: _ws(state._ws), _state(state._state), _cycle(state._cycle) { }
	
	virtual State *clone(void) { return new State(*this); }
	virtual void run(sim::Driver& driver);
	virtual void stop(void) { _stop = true; }
	virtual void flush(void)  { }
	virtual int cycle(void) { return _cycle; }
	virtual void reset(void) { _cycle = 0; }
	
private:
	void interpretFull(Inst *inst);
	int interpret(CString expr, Inst *inst, instruction_t *_inst, bool x18);
	int access8(Inst *inst, instruction_t *_inst);
	int access16(Inst *inst, instruction_t *_inst, bool write);
	int access16_aligned(Inst *inst, instruction_t *_inst, bool write);
	int access16_code(Inst *inst, instruction_t *_inst);
	void onTimeError(cstring time, const string& msg, Inst *inst);
	Address getAccessAddress(Inst *inst, instruction_t *_inst, bool write);
	WorkSpace *_ws;
	state_t *_state;
	int _cycle;
	bool _stop;
};


/**
 */
Simulator::Simulator(void)
: sim::Simulator("s12x_simulator", Version(1, 0, 0), OTAWA_SIMULATOR_VERSION) {
}


/**
 */
sim::State *Simulator::instantiate(WorkSpace *ws, const PropList& props) {
		
	// Check consistency
	if(ws->process()->loader() != &s12x_plugin)
		throw otawa::Exception("s12x_simulator can only be used with the s12x loader !");
		
	// Compute the state
	return new State(ws, (state_t *)((Process *)ws->process())->state());
}


/**
 */
Simulator simulator;


/**
 */
void State::run(sim::Driver& driver)  {
	
	// Loop on instructions
	_stop = false;
	Inst *inst = 0;
	while(!_stop) {
		
		// Get next instruction
		inst = driver.nextInstruction(*this, inst);
		if(!inst)
			break; 
		
		// Interpret the instruction time sequence
		interpretFull(inst);

		// Terminate the instruction
		driver.terminateInstruction(*this, inst);
		inst = inst->nextInst();
	} 
}


/**
 * Intepret the time for the given instruction including the prefixes.
 * @param inst	Instruction to interpret.
 */
void State::interpretFull(Inst *inst) {
	
	// Get information
	code_t buffer[20];
	instruction_t *_inst;
	iss_fetch(inst->address().address(), buffer);
	_inst = iss_decode(_state, inst->address().address(), buffer);
	//cerr << "time = " << iss_table[_inst->ident].time << io::endl;
	CString time = iss_table_time[iss_table[_inst->ident].time];
	bool x18 = (buffer[0] >> 24) == 0x18;
	
	// Process prefix
	int res = 0;
	switch(time[0]) {
		
	// time[time_select]
	case '!': {
			int cnt = iss_table[_inst->ident].time_select;
			int pos = 0;
			while(cnt) {
				pos = time.indexOf(',', pos + 1);
				ASSERTT(pos >= 0, "exp " <<
					iss_table[_inst->ident].time_select << " miss");
				cnt--;
			}
			res = interpret(time.substring(pos + 1), inst, _inst, x18);
		}
		break;

	// time[time_select * 5 + time_select2]
	case '$':  {
			int cnt = iss_table[_inst->ident].time_select * 5
				+ iss_table[_inst->ident].time_select2;
			int pos = 0;
			while(cnt) {
				pos = time.indexOf(',', pos + 1);
				ASSERTT(pos >= 0, "exp " << (iss_table[_inst->ident].time_select
					* 5 + iss_table[_inst->ident].time_select2) << " miss");
				cnt--;
			}
			res = interpret(time.substring(pos + 1), inst, _inst, x18);
		}
		break;
	
	// time[0] + time[1] * COUNT + time[2]
	case '*': {
		
			// Prefix
			int t1 = interpret(time.substring(1), inst, _inst, x18);
			
			// Body
			int p1 = time.indexOf(',');
			ASSERTT(p1 >= 0, "exp2 miss");
			int t2 = interpret(time.substring(p1 + 1), inst, _inst, x18);
			
			// Suffix
			int p2 = time.indexOf(',', p1 + 1);
			ASSERTT(p2 >= 0, "exp3 miss");
			int t3 = interpret(time.substring(p2 + 1), inst, _inst, x18);
			
			// Time computation
			int cnt = COUNT(inst);
			ASSERTP(cnt < 0, "fuzzy instruction without count");
			res = t1 + t2 * cnt + t3;
		}
		break;
	
	// Conditional branch
	case '?': {
			int pos = time.indexOf(',');
			ASSERTT(pos >= 1, "exp2 miss");
			int t1 = interpret(time.chars() + 1, inst, _inst, x18);
			int t2 = interpret(time.chars() + pos + 1, inst, _inst, x18);
			res = t1 > t2 ? t1 : t2;
		}
		break;

	// Dependent next instruction in 0x18
	case '+': {
			Inst *next = inst->nextInst();
			if(next) {
				code_t buffer2[20];
				iss_fetch(next->address().address(), buffer2);
				if((buffer2[0] >> 24) == 0x18) {
					int pos = time.indexOf(',');
					ASSERTT(pos >= 0, "exp2 miss");
					res = interpret(time.substring(pos + 1), inst, _inst, x18);
					break;
				}		
			}
			res = interpret(time.substring(1), inst, _inst, x18);
		}
		break;

	// IT dependent (not managed) 	
	case '%':
		res = interpret(time.substring(1), inst, _inst, x18);
		break;
	
	// Wait instruction
	case '#': {
			int wait = WAIT(inst);
			ASSERTP(wait < 0, "undefined wait time at " << inst->address());
			res = wait; 
		}
		break;
	
	// Simple expression
	default:
		res = interpret(time, inst, _inst, x18);
	}
	
	// Cleanup
	_cycle += res;
	TRACE(inst->address() << ":" << inst << ", time=\"" << time
		<< "\" -> " << res << " (" << iss_table[_inst->ident].time_select
		<< ", " << iss_table[_inst->ident].time_select2 << ")");
	iss_free(_inst);
}


/**
 * Interpret a simple time string.
 * @param time	String expression to interpret.
 * @param inst	OTAWA instruction.
 * @param _inst	GLISS instruction.
 */
int State::interpret(CString time, Inst *inst, instruction_t *_inst, bool x18) {
	int res = 0;
	bool o_found = false;
	bool o_to_p = true;

	// Process the string
	bool end = false;
	for(int i = 0; !end; i++)
		switch(time[i]) {
			
		// End
		case '\0': case ',':
			end = true;
			break;
			
		// single internal cycle
		case 'f': case 'g': case 'n':
			res++;
			break;
		
		// 8-bits read / write
		case 'i': case 'r': case 'u': case 't':
		case 's': case 'w': case 'x':
			res += access8(inst, _inst);
			break;
		
		// 16-bits aligned
		case 'P':
			res += access16_code(inst, _inst);
			break;		
		case 'V':
			res += access16_aligned(inst, _inst, false);
			break;

		// 16-bits free
		case 'I': case 'R': case 'U': case 'T': 
			res += access16(inst, _inst, false);
			break;		
		case 'S': case 'W':
			res += access16(inst, _inst, true);
			break;
		
		// Special
		case 'O':
			if(!o_found) {
				o_found = true;
				if(!x18) {
					if((inst->address().address() & 0x1) && (inst->size() & 0x1))
						res += access16_code(inst, _inst);
					else
						res++;
				}
				else {
					if(inst->address().address() & 0x1) {
						o_to_p = false;
						res += access16_code(inst, _inst);
					}
					else
						res++;
				}
			}
			else {
				ASSERTT(x18, "many O out of 0x18 instruction");
				if(o_to_p)
					res += access16_code(inst, _inst);
				else
					res++;
			}
			break;
		
		// Else error
		default:
			ASSERTT(false, "unknown symbol \"" << time[i] << '"');
		}
	
	// Return result
	TRACE(inst->address() << ":" << inst->size() << ":" << inst
		<< (x18 ? " x18" : "") << "\ttime = \"" << time << "\" = " << res);
	return res;
}


/**
 * Compute the time for a simple 8-bit access.
 * @param inst	OTAWA instruction.
 * @param _inst	GLISS instruction.
 * @return		Time of the access.
 */
int State::access8(Inst *inst, instruction_t *_inst) {
	// !!TODO!! Customize with memory description.
	return 1;
}


/**
 * Compute the time for a free 16-bit access.
 * @param inst	OTAWA instruction.
 * @param _inst	GLISS instruction.
 * @return		Time of the access.
 */
int State::access16(Inst *inst, instruction_t *_inst, bool write) {
	// !!TODO!! Customize with memory description.
	// !!TODO!! Worst case : external unaligned.
	// Must be detailed with alignement and type of memory.
	Address addr = getAccessAddress(inst, _inst, write);
	/*if(addr.isNull() || addr.address() & 0x1)
		return 2;
	else*/
		return 1;
}


/*
 * Compute the time for a free 16-bit access.
 * @param inst	OTAWA instruction.
 * @param _inst	GLISS instruction.
 * @return		Time of the access.
 */
int State::access16_aligned(Inst *inst, instruction_t *_inst, bool write) {
	// !!TODO!! Customize with memory description.
	// !!TODO!!Must be detailed with alignement and type of memory.
	return 1;
}


/*
 * Compute the time for a free 16-bit access for code.
 * @param inst	OTAWA instruction.
 * @param _inst	GLISS instruction.
 * @return		Time of the access.
 */
int State::access16_code(Inst *inst, instruction_t *_inst) {
	// !!TODO!!Must be detailed with alignement and type of memory.
	return 1;
}


/**
 * Function called when an expression error is found.
 * @param time	Errored time expression.
 * @param msg	Message to display.
 * @param inst	Errored instruction.
 * Raise an assertion failure.
 */
void State::onTimeError(cstring time, const string& msg, Inst *inst) {
	ASSERTP(false, "time expression error: " << msg << " for \"" << time
		<< "\" at " << inst->address() << ":" << inst);
}


/**
 * Get the accessed address of the given instruction.
 * @param inst	OTAWA instruction.
 * @param _inst	GLISS matching instruction.
 * @param write	Type of access: false = read, true = write.
 * @return		Accessed address or null address if cannot be determined.
 */
Address State::getAccessAddress(Inst *inst, instruction_t *_inst, bool write) {
	#ifdef STATS
		stats.add();
	#endif
	switch(_inst->ident) {
		
	/* opr16a: Absolute address 16-bits */
	case ID_ADCA_: case ID_ADCB_: case ID_ADDA_: case ID_ADDB_:
	case ID_ADDD_: case ID_ADDX_: case ID_ADDY_:
	case ID_ADED_: case ID_ADEX_: case ID_ADEY_:
	case ID_ANDA_: case ID_ANDB_: case ID_ANDX_: case ID_ANDY_:
	case ID_ASL_: case ID_ASLW_: case ID_ASR_: case ID_ASRW_:
	case ID_BCLR__0: case ID_BITA__1: case ID_BITB__1:
	case ID_BITX_: case ID_BITY_:
	case ID_BRCLR_: case ID_BRSET_:
	case ID_BSET__0: case ID_BTAS__0:
	case ID_CLR_: case ID_CLRW_:
	case ID_CMPA_: case ID_CMPB_:
	case ID_COM_: case ID_COMW_:
	case ID_CPD_:
	case ID_CPED_: case ID_CPES_: case ID_CPEX_: case ID_CPEY_:
	case ID_CPS_: case ID_CPX_: case ID_CPY_:
	case ID_DEC_: /* case ID_DECW_:*/
	case ID_EMACS_:
	case ID_EORA__0: case ID_EORB__0: case ID_EORX__0: case ID_EORY__0:
	case ID_INC_: case ID_INCW_:
	case ID_LDAA_: case ID_LDAB_:
	case ID_LDD_: case ID_LDS_: case ID_LDX_: case ID_LDY_:
	/*case ID_LSL_: case ID_LSLW_:*/
	case ID_LSR_: case ID_LSRW_:
	case ID_NEG_: case ID_NEGW_:
	case ID_ORAA__1: case ID_ORAB__1: case ID_ORX__1: case ID_ORY__1:
	case ID_ROL_: case ID_ROLW_: case ID_ROR_: case ID_RORW_:
	case ID_SBCA_: case ID_SBCB_:
	case ID_SBED_: case ID_SBEX_: case ID_SBEY_:
	case ID_STAA_: case ID_STAB_: case ID_STD_:
	case ID_STS_: case ID_STX_: case ID_STY_:
	case ID_SUBA_: case ID_SUBB_:
	case ID_SUBD_: case ID_SUBX_: case ID_SUBY_:
	case ID_TST_: case ID_TSTW_:
		return _inst->instrinput[0].val.uint16;

	/* opr8a: absolute address 8-bits */
	case ID_ADCA__0: case ID_ADCB__0: case ID_ADDA__0: case ID_ADDB__0:
	case ID_ADDD__0: case ID_ADDX__0: case ID_ADDY__0:
	case ID_ADED__0: case ID_ADEX__0: case ID_ADEY__0:
	case ID_ANDA__0: case ID_ANDB__0: case ID_ANDX__0: case ID_ANDY__0:
	case ID_BCLR_: case ID_BITA__0: case ID_BITB__0:
	case ID_BITX__0: case ID_BITY__0:
	case ID_BRCLR__0: case ID_BRSET__0:
	case ID_BSET_: case ID_BTAS_:
	case ID_CMPA__0: case ID_CMPB__0:
	case ID_CPD__0:
	case ID_CPED__0: case ID_CPES__0: case ID_CPEX__0: case ID_CPEY__0:
	case ID_CPS__0: case ID_CPX__0: case ID_CPY__0:
	case ID_EORA__1: case ID_EORB__1: case ID_EORX__1: case ID_EORY__1:
	case ID_LDAA__0: case ID_LDAB__0:
	case ID_LDD__0: case ID_LDS__0: case ID_LDX__0: case ID_LDY__0:
	case ID_ORAA__0: case ID_ORAB__0: case ID_ORX__0: case ID_ORY__0:
	case ID_SBCA__0: case ID_SBCB__0:
	case ID_SBED__0: case ID_SBEX__0: case ID_SBEY__0:
	case ID_STAA__0: case ID_STAB__0: case ID_STD__0:
	case ID_STS__0: case ID_STX__0: case ID_STY__0:
	case ID_SUBA__0: case ID_SUBB__0:
	case ID_SUBD__0: case ID_SUBX__0: case ID_SUBY__0:
		return _inst->instrinput[0].val.uint8;
	
	/* mov family */
	case ID_MOVB_: case ID_MOVW_:
		return 	_inst->instrinput[1].val.uint16;
	case ID_MOVB__0:  case ID_MOVW__0:
		if(!write)
			return _inst->instrinput[0].val.uint16;
		else
			return _inst->instrinput[1].val.uint16;
	
	/* default */
	default:
		#ifdef STATS
			stats.unresolve(inst);
		#endif
		return Address::null;
	}
}

} } // otawa::s12x
