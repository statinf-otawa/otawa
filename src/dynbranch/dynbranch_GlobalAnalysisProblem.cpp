/*
 *	GlobalAnalysisProblem class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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
#include "GlobalAnalysisProblem.h"
#include <elm/log/Log.h> // to use the debugging messages

#define DEBUG_MEM(x)

using namespace elm::log;
using namespace elm::color;

namespace otawa { namespace dynbranch {

GlobalAnalysisProblem::GlobalAnalysisProblem(WorkSpace* workspace, bool v, Domain & entry) : verbose(v), ws(workspace) {
	istate = dfa::INITIAL_STATE(workspace);

	// initial the BOT state
	bot.setBottom(true);
	bot.setFastState(entry.getFastState());
	bot.setState(entry.getFastState()->bot);

	// initial the TOP state
	topd.setBottom(false);
	topd.setFastState(entry.getFastState());
	topd.setState(entry.getFastState()->top);

	// initial the ENTRY state
	ent.setBottom(false); // not bottom
	ent.setFastState(entry.getFastState());
	ent.setState(entry.getFastState()->bot);
	PotentialValue pvEntry;
	pvEntry.insert(ws->process()->defaultStack());
	ent.setReg(ws->process()->platform()->getSP()->platformNumber(), pvEntry);

	// setting up temp regs
	PotentialValue pv;
	_tempRegs = new Vector<PotentialValue>(workspace->process()->platform()->regCount());
	for(int i = 0; i < workspace->process()->platform()->regCount(); i++)
		_tempRegs->add(pv);
}

GlobalAnalysisProblem::~GlobalAnalysisProblem(void) {
	delete _tempRegs;
}

const Domain& GlobalAnalysisProblem::bottom() {
	return bot ;
}

const Domain& GlobalAnalysisProblem::top() {
	return topd ;
}

const Domain& GlobalAnalysisProblem::entry() {
	return ent ;
}

void GlobalAnalysisProblem::updateEdge(Edge *edge, Domain& d) {
	// this is for filtering purpose......, no need to implement for now
	//TODO
}

void GlobalAnalysisProblem::lub(Domain& a,const Domain& b) const {
	if (b.isBottom()) //A U Bottom = A   Nothing to do on A
		return;
	if (a.isBottom()) { // BOttom U B = B
		a = b;
		return;
	}
	a.lub(b);
}

void GlobalAnalysisProblem::assign(Domain& a,const Domain &b) const {
	a.copy(b); // just point to the same state variable for fast state
}

bool GlobalAnalysisProblem::equals(const Domain &a, const Domain &b) const {
	return a.equals(b);
}

void GlobalAnalysisProblem::widening(otawa::Block* ob, Domain& a, Domain b) const {
	// if a has something but not in b, remove it (make it TOP)
	// if a and b both have something but of different value, remove it (make it TOP)
	a.widening(b);
}

void GlobalAnalysisProblem::update(Domain& out, const Domain& in, Block *b) {
	// initially the output state equals to the input state
	cerr << "DEBUG: updating " << b << io::endl;
	out = in ;

	if(!b->isBasic()) // only process the BB
		return;

	BasicBlock *bb = b->toBasic();

	// process each instruction in turn
	for(BasicBlock::InstIter inst(bb) ; inst ; inst++) {
		// get semantic instructions
		sem::Block block;
		inst->semInsts(block);

		// process the semantic instruction
		for(sem::Block::InstIter semi(block) ; semi ; semi++) {
			sem::inst inst = *semi ;

			// unsupported instructions without side-effects
			switch(inst.op) {
			case sem::NOP:
			case sem::TRAP:		// perform a trap
			case sem::CONT:		// continue in sequence with next instruction
			case sem::BRANCH:		// perform a branch on content of register a
				break ;

			// unsupported instructions with side-effects
			case sem::SCRATCH:	// d <- T
			case sem::CMP:		// d <- a ~ b
			case sem::CMPU:		// d <- a ~u b
			case sem::SETP:		// page(d) <- cst
			{
				setReg(out, inst.d(), PotentialValue::top);
				break ;
			}

			case sem::SETI:		// d <- cst
			{
				PotentialValue pv ;
				pv.insert(inst.cst()) ;
				setReg(out, inst.d(), pv);
				break;
			}

			case sem::SET:		// d <- a
			{
				const PotentialValue& vala = readReg(out, inst.a());
				setReg(out, inst.d(), vala);
				break ;
			}

			case sem::ADD:		// d <- a + b
			{
				const PotentialValue& vala = readReg(out, inst.a());
				const PotentialValue& valb = readReg(out, inst.b());
				if(vala.length() && valb.length()) // when both of the lengths are larger than 0
				{
					PotentialValue sum = vala + valb;
					setReg(out, inst.d(), sum);
				}
				else {
					// removing the entry (because we don't know the results, so we make an assumption that it is TOP)
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
				break ;
			}

			case sem::STORE:		// MEMb(a) <- d
			{
				const PotentialValue& address = readReg(out, inst.a());
				if(address.length() == 1) {
					elm::t::uint32 addressToStore = address[0];
					const PotentialValue& data = readReg(out, inst.d());
					out.storeMemory(addressToStore, data);
				}
#ifdef DYNBRANCH_EXPERIMENT
				else if(address.length() > 1) {
					if(verbose) { elm::cerr << "WARNING: we can't store to multiple potential memory addresses!" << io::endl; }
					assert(0); // just in case, want to see
					for (PotentialValue::Iterator pi(address); pi; pi++) {
						elm::t::uint32 addressToStore = *pi;
						PotentialValue temp = out.loadMemory(addressToStore);
						const PotentialValue& data = readReg(out, inst.d());
						temp.insert(data);
						out.storeMemory(addressToStore, temp);
					}
				}
				else { // no address found
					if(verbose) elm::cerr << "WARNING: we can't find the memory address for store" << io::endl;
				}
#endif
				break ;
			}

			case sem::LOAD:		// d <- MEMb(a)
			{
				const PotentialValue& address = readReg(out, inst.a());
				if(address.length() == 1) {
					elm::t::uint32 addressToLoad = address[0];
					const PotentialValue& data = out.loadMemory(addressToLoad);

					// If we couldn't find the memory info from the Global State, lets try to see if this info exists in the Read Only Region
					if(data.length() == 0) {
						if(istate && istate->isInitialized(addressToLoad)) {
							t::uint32 dataFromMemDirectory;
							ws->process()->get(addressToLoad, dataFromMemDirectory);
							PotentialValue pv;
							pv.insert(dataFromMemDirectory);
							setReg(out, inst.d(), pv);
						}
						else
							setReg(out, inst.d(), data);
					}
					else
						setReg(out, inst.d(), data);
				}
#ifndef DYNBRANCH_EXPERIMENT
				else {
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
#else
				else if(address.length() > 1) {
					if(verbose)
						elm::cerr << "WARNING: we can't load from multiple potential memory addresses!" << io::endl;
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP

					bool firstValue = true;
					PotentialValue temp; // empty value
					for(int currAddr = 0; currAddr < address.length(); currAddr++) {
						elm::t::uint32 addressToLoad = address[currAddr];
						const PotentialValue& data = out.loadMemory(addressToLoad); // try to load the value from the Global analysis
						if((data.length() == 0) && (GLOBAL_MEMORY_LOADER)) {	// if the value is empty, try to read from the initialized memory
							if(istate && istate->isInitialized(addressToLoad)) {
								t::uint32 dataFromMemDirectory;
								ws->process()->get(addressToLoad, dataFromMemDirectory);
								DEBUG_MEM(elm::cout << Debug::debugPrefix(__FILE__, __LINE__,__FUNCTION__) << "    " << IGre << "dataFromMemDirectory = " << hex(dataFromMemDirectory) << RCol << io::endl;)
								PotentialValue pv;
								temp.insert(dataFromMemDirectory);
							}
						}
						else {
							for(PotentialValue::Iterator pi(data); pi; pi++)
								temp.insert(*pi);
						}
					}
					setReg(out, inst.d(), temp); // put the collected data
				} // end else if(address.length() > 1) {
				else { // no address found
					if(verbose)
						elm::cerr << "WARNING: we can't find the memory address to load" << io::endl;
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
#endif
				break;
			}

			case sem::SHL:		// d <- unsigned(a) << b
			{
				const PotentialValue& vala = readReg(out, inst.a());
				const PotentialValue& valb = readReg(out, inst.b());
				if(vala.length() && valb.length()) // when both of the lengths are larger than 0
				{
					PotentialValue result = vala << valb;
					setReg(out, inst.d(), result);
				}
				else {
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
				break ;
			}

			case sem::SUB:		// d <- a - b
			{
				const PotentialValue& vala = readReg(out, inst.a());
				const PotentialValue& valb = readReg(out, inst.b());
				if(vala.length() && valb.length()) // when both of the lengths are larger than 0
				{
					PotentialValue sum = vala - valb;
					setReg(out, inst.d(), sum);
				}
				else {
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
				break ;
			}

			case sem::AND:		// d <- a & b
			{
				const PotentialValue& vala = readReg(out, inst.a());
				const PotentialValue& valb = readReg(out, inst.b());
				if(vala.length() && valb.length()) // when both of the lengths are larger than 0
				{
					PotentialValue sum = vala & valb;
					setReg(out, inst.d(), sum);
				}
				else {
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
				break ;
			}

			case sem::ASR:		// d <- a >> b
			{
				const PotentialValue& vala = readReg(out, inst.a());
				const PotentialValue& valb = readReg(out, inst.b());
				if(vala.length() && valb.length()) // when both of the lengths are larger than 0
				{
					PotentialValue sum = vala >> valb;
					setReg(out, inst.d(), sum);
				}
				else {
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
				break ;
			}

			case sem::SHR:		// d <- unsigned(a) >> b
			{
				const PotentialValue& vala = readReg(out, inst.a());
				const PotentialValue& valb = readReg(out, inst.b());
				if(vala.length() && valb.length()) // when both of the lengths are larger than 0
				{
					PotentialValue result = logicalShiftRight(vala, valb);
					setReg(out, inst.d(), result);
				}
				else {
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
				break ;
			}

			case sem::OR:		// d <- a | b
			{
				const PotentialValue& vala = readReg(out, inst.a());
				const PotentialValue& valb = readReg(out, inst.b());
				if(vala.length() && valb.length()) // when both of the lengths are larger than 0
				{
					PotentialValue result = vala | valb;
					setReg(out, inst.d(), result);
				}
				else {
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
				break ;
			}

			case sem::XOR:		// d <- a ^ b
			{
				const PotentialValue& vala = readReg(out, inst.a());
				const PotentialValue& valb = readReg(out, inst.b());
				if(vala.length() && valb.length()) // when both of the lengths are larger than 0
				{
					PotentialValue result = vala ^ valb;
					setReg(out, inst.d(), result);
				}
				else {
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
				break ;
			}

			case sem::NOT:		// d <- ~a
			{
				const PotentialValue& vala = readReg(out, inst.a());
				if(vala.length()) // when both of the lengths are larger than 0
				{
					PotentialValue result = ~vala;
					setReg(out, inst.d(), result);
				}
				else {
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
				break ;
			}

			case sem::MUL:
			{
				const PotentialValue& vala = readReg(out, inst.a());
				const PotentialValue& valb = readReg(out, inst.b());
				if(vala.length() && valb.length()) // when both of the lengths are larger than 0
				{
					PotentialValue result = vala * valb;
					setReg(out, inst.d(), result);
				}
				else {
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
				break ;
			}

			case sem::MULH: // d <- (a * b) >> bitlength(d)
			{
				const PotentialValue& vala = readReg(out, inst.a());
				const PotentialValue& valb = readReg(out, inst.b());
				if(vala.length() && valb.length()) // when both of the lengths are larger than 0
				{
					PotentialValue result = MULH(vala,valb);
					setReg(out, inst.d(), result);
				}
				else {
					setReg(out, inst.d(), PotentialValue::top); // because we don't know the results, so we make an assumption that it is TOP
				}
				break ;
			}
			/*
            NEG,		// d <- -a
            NOT,		// d <- ~a
            AND,		// d <- a & b
            OR,			// d <- a | b
            MULU,		// d <- unsigned(a) * unsigned(b)
            DIV,		// d <- a / b
            DIVU,		// d <- unsigned(a) / unsigned(b)
            MOD,		// d <- a % b
            MODU		// d <- unsigned(a) % unsigned(b)
              break ;
			 */
			case sem:: IF:
				break;
			default:
				elm::cerr << "WARNING: Unsupported instruction " << inst.op << " of " << inst << io::endl;
				elm::cerr << "i.a() = " << readReg(out, inst.a()) << io::endl;
				elm::cerr << "i.b() = " << readReg(out, inst.b()) << io::endl;
				assert(0); // need to think about the implementation here! or we just leave it TOP?
				break;
			} // end switch(inst.op) {
		} // end of each semantic instruction
	} // end of each instruction
} // end of the BB

}} // namespace otawa { namespace dynbranch {
