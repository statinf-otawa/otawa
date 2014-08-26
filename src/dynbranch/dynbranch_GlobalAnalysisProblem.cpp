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

namespace global {

GlobalAnalysisProblem::GlobalAnalysisProblem(WorkSpace* workspace, bool v, Domain entry) :
		verbose(v),
		ws(workspace),
		ent(entry)
{
	// Bottom
	Domain b ;
	b.setBottom(true) ;
	bot = b ;
}

const Domain& GlobalAnalysisProblem::bottom() {
	return bot ;
}

const Domain& GlobalAnalysisProblem::entry() {
	return ent ;
}

void GlobalAnalysisProblem::updateEdge(Edge *edge, Domain& d) {
	//TODO
}

void GlobalAnalysisProblem::lub(Domain& a,const Domain& b) const {
	//cout << " LUB DANS " << &a << " valant " << a << " AVEC " << &b << " valant " << b << endl ;
	if (b.isBottom()) { //A U Bottom = A   Nothing to do on A
		return ;
	}

	if (a.isBottom()) { // BOttom U B = B
		a = b ;
		return ;
	}

	for(Domain::MutableIterator it(a); it; it++) {
		if(b.hasKey((*it).fst)) ;
			it.set(merge((*it).snd, *b.get((*it).fst)));
	}

	for(Domain::Iterator it(b); it; it++) {
		if(!a.hasKey((*it).fst))
			a.remove(it);
	}
}

void GlobalAnalysisProblem::assign(Domain& a,const Domain &b) const {
	//cout << " ASSIGN DE " << &b << " valant " << b << " DANS " << &a << " valant " << a << endl << " FIN ASSIGN " << endl ;
	a.clear();
	for(Domain::Iterator it(b); it; it++)
		a.put((*it).fst, (*it).snd);
}

bool GlobalAnalysisProblem::equals(const Domain &a, const Domain &b) const {
	return a == b ;
}

void GlobalAnalysisProblem::widening(Domain &a,Domain b) const {
	/* The Widening operation is gonna be really basic
	 * It put TOP to all the value which has been modified during the iteration
	 */

	// select entries to delete
	genstruct::Vector<MemID> to_delete;
	for(Domain::Iterator it(a); it; it++) {
		Option<PotentialValue> val = b.get((*it).fst);

		// if it doesn't exist in b, it shouldn't exist in A either!
		if(!val)
			to_delete.add((*it).fst);

		// we must ensure they are the same
		else if(*val != (*it).snd)
			to_delete.add((*it).fst);
	}

	// delete them
	for(genstruct::Vector<MemID>::Iterator it(to_delete); it; it++)
		a.remove(*it);
}

void GlobalAnalysisProblem::update(Domain& out, const Domain& in, BasicBlock *bb) {
	out = in ;

	// process each instruction in turn
	for(BasicBlock::InstIter inst(bb) ; inst ; inst++) {
		//  cout << "\t" << *inst << endl ;

		// get semantic instructions
		sem::Block block;
		inst->semInsts(block);

		// process the semantic instruction
		for(sem::Block::InstIter semi(block) ; semi ; semi++) {
			sem::inst inst = *semi ;
			//  cout << "\t\t" << inst << endl ;

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
				MemID i = elm::Pair<Memtype,int>(REG, inst.d()) ;
				out.remove(i) ;
				break ;
			}

			case sem::SETI:		// d <- cst
			{
				PotentialValue pv ;
				pv.insert(inst.cst()) ;
				MemID i = elm::Pair<Memtype,int>(REG, inst.d()) ;
				out.put(i, pv);
				break ;
			}

			case sem::SET:		// d <- a
			{
				MemID id1 = elm::Pair<Memtype,int>(REG, inst.d());
				MemID id2 = elm::Pair<Memtype,int>(REG, inst.a());
				Option<PotentialValue> val = out.get(id2);
				if(val)
					out.put(id1, *val);
				else	// we dont know, so its TOP
					out.remove(id1) ;
				break ;
			}

			case sem::ADD:		// d <- a + b
			{
				MemID idret = elm::Pair<Memtype,int>(REG, inst.d());
				MemID ida = elm::Pair<Memtype,int>(REG, inst.a());
				MemID idb = elm::Pair<Memtype,int>(REG, inst.b());
				Option<PotentialValue> vala = out.get(ida);
				Option<PotentialValue> valb = out.get(idb);
				if(vala && valb)
					out.put(idret, *vala + *valb);
				else
					out.remove(idret);
				break ;
			}

			case sem::STORE:		// MEMb(a) <- d
			{
				MemID ida = elm::Pair<Memtype,int>(REG, inst.a()) ;
				MemID idd = elm::Pair<Memtype,int>(REG, inst.d()) ;

				Option<PotentialValue> addr = out.get(ida);
				if(addr) {
					if((*addr).length() == 1) { //We can't store to multiple addresses
						int m = (*addr).get(0) ;
						MemID mem = elm::Pair<Memtype,int>(MEM, m) ;

						Option<PotentialValue> vd = out.get(idd);
						if(vd)
							out.put(mem, *vd);
						else
							out.remove(mem) ;
					}
					else
						cout << " Warning : we can't store to multiple potential memory addresses " << endl ;
				}
				else
					cout << "WARNING: we can't find the memory address for store" << endl ;
				break ;
			}

			case sem::LOAD:		// d <- MEMb(a)
			{
				MemID ida = elm::Pair<Memtype,int>(REG, inst.a());
				MemID idreg = elm::Pair<Memtype,int>(REG, inst.d());

				Option<PotentialValue> addr = out.get(ida);
				if(addr) {
					if((*addr).length() == 1) { //We can't load from multiple addresses
						int m = (*addr).get(0) ;
						MemID idmem = elm::Pair<Memtype,int>(MEM, m) ;
						Option<PotentialValue> va = out.get(idmem);
						if(va)	// this value is not a TOP
							out.put(idreg, *va);
						else {
							// Here is a decision , do we try to load the value from memory or we put top?
							// The mode should be specified in a header
							if(GLOBAL_MEMORY_LOADER) {
								PotentialValue res;
								t::uint32 val = 0;		// must be fixed
								ws->process()->get(m, val);
								res.insert(val);
								out.put(idreg, res);
							}
							else
								out.remove(idreg) ;
						}
					} else
						out.remove(idreg) ;

				} else {
					out.remove(idreg) ;
					cout << "WARNING: we can't find the memory address (load)" << endl ;
				}
				break ;
			}

			case sem::SHL:		// d <- unsigned(a) << b
			{
				MemID idret = elm::Pair<Memtype,int>(REG, inst.d());
				MemID ida = elm::Pair<Memtype,int>(REG, inst.a());
				MemID idb = elm::Pair<Memtype,int>(REG, inst.b());
				Option<PotentialValue> va = out.get(ida);
				Option<PotentialValue> vb = out.get(idb);
				if(va && vb)
					out.put(idret,  *va << *vb);
				else
					out.remove(idret) ;
				break ;
			}

			/*
            IF,			// continue if condition a is meet in register b, else jump c instructions
            SUB,		// d <- a - b
            SHR,		// d <- unsigned(a) >> b
            ASR,		// d <- a >> b
            NEG,		// d <- -a
            NOT,		// d <- ~a
            AND,		// d <- a & b
            OR,			// d <- a | b
            MUL,		// d <- a * b
            MULU,		// d <- unsigned(a) * unsigned(b)
            DIV,		// d <- a / b
            DIVU,		// d <- unsigned(a) / unsigned(b)
            MOD,		// d <- a % b
            MODU		// d <- unsigned(a) % unsigned(b)
              break ;
			 */
			default:
				if (verbose) {cout << "Warning : Unsupported instruction " << inst.op << endl ; }
				break;
			}
			//      cout << out ;
		}
	}
}

} // global
