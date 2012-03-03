/*	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	This file is part of OTAWA
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

#include <otawa/util/Dominance.h>
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/ilp.h>
#include <otawa/ipet.h>
#include <otawa/cfg/Edge.h>
#include <otawa/util/LoopInfoBuilder.h>

#include <otawa/hard/BHT.h>
#include <otawa/branch/BranchProblem.h>
#include <otawa/branch/CondNumber.h>

namespace otawa {

MUSTBranch::MUSTBranch(const int _size,  WorkSpace *_fw, const int _A, const int _row) 
:	
	fw(_fw), row(_row),
	bot(_size, _A),
	ent(_size, _A)
{
		ent.empty();	
}
	
MUSTBranch::~MUSTBranch() {
	
}
const MUSTBranch::Domain& MUSTBranch::bottom(void) const {
		return bot;
}
const MUSTBranch::Domain& MUSTBranch::entry(void) const {
		return ent;
}

void MUSTBranch::update(Domain& out, const Domain& in, BasicBlock* bb) {
		cerr << "FATAL: PERSProblem is not to be used directly, use MUSTPERS instead.\n";
		ASSERT(false);
}
	


MAYBranch::MAYBranch(const int _size,  WorkSpace *_fw, const int _A, const int _row) 
:	
	fw(_fw), row(_row),
	bot(_size, _A),
	ent(_size, _A)
{
		ent.empty();	
}
	
MAYBranch::~MAYBranch() {
	
}
const MAYBranch::Domain& MAYBranch::bottom(void) const {
		return bot;
}
const MAYBranch::Domain& MAYBranch::entry(void) const {
		return ent;
}

void MAYBranch::update(Domain& out, const Domain& in, BasicBlock* bb) {
		cerr << "FATAL: PERSProblem is not to be used directly, use MUSTPERS instead.\n";
		ASSERT(false);
}


	
	

	
PERSBranch::PERSBranch(const int _size,  WorkSpace *_fw, const int _A, const int _row) 
:	fw(_fw), row(_row),
	bot(_size, _A),
	ent(_size, _A)
{
		bot.setToBottom();
		ent.empty();


}
	
PERSBranch::~PERSBranch() {

}
const PERSBranch::Domain& PERSBranch::bottom(void) const {
		return bot;
}

const PERSBranch::Domain& PERSBranch::entry(void) const {
		return ent;
}

void PERSBranch::update(Domain& out, const Domain& in, BasicBlock* bb)  {
		cerr << "FATAL: PERSProblem is not to be used directly, use MUSTPERS instead.\n";
		ASSERT(false);
}
	

elm::io::Output& operator<<(elm::io::Output& output, const PERSBranch::Domain& dom) {
	dom.print(output);
	return output;
}

	
BranchProblem::BranchProblem(const int _size,  WorkSpace *_fw,  const int _A, const int _row) 
:	mustProb(_size,  _fw, _A, _row),
	persProb(_size,  _fw, _A, _row),
	mayProb(_size, _fw, _A, _row),
	bot(_size, _A), fw(_fw), row(_row),
	ent(_size, _A)
	{
		
		persProb.assign(bot.pers, persProb.bottom());
		mustProb.assign(bot.must, mustProb.bottom());
		
		persProb.assign(ent.pers, persProb.entry());
		mustProb.assign(ent.must, mustProb.entry());
}
	

const BranchProblem::Domain& BranchProblem::bottom(void) const {
		return bot;
}
const BranchProblem::Domain& BranchProblem::entry(void) const {
		return ent;
}
		


void BranchProblem::update(Domain& out, const Domain& in, BasicBlock* bb) {
	assign(out, in);
	Inst *last = bb->lastInst();
	
	if (last != NULL) {
  		if ((hard::BHT_CONFIG(fw)->line(last->address())) == row) {
	      	if (branch::COND_NUMBER(bb) != -1) {
	      		//cout << "Inject cond number " << branch::COND_NUMBER(bb) << " for BB " << bb->number() << "\n";
  				out.inject(branch::COND_NUMBER(bb));
          	}
  		}
  	}
  	/*cout << "For BB: " << bb->number() << " IN=";
  	in.print(cout);
  	cout << " OUT=";
  	out.print(cout);
  	cout << "\n";*/
  	 
}

}




