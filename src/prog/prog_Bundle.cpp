/*
 *	Bundle class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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

#include <otawa/prog/Bundle.h>

namespace otawa {

/**
 * @class Bundle
 * Bundles are group of instructions that are executed together and in parallel
 * inside VLIW microprocessors. This class allows grouping instructions
 * composing a bundle and handling it as a single instruction. This is useful
 * for some low-level analyzes requiring to consider the group of instruction
 * as a single one. For non-VLIW instruction set, the bundle will only match
 * one instruction.
 *
 * @ingroup prog
 */

/**
 * @fn bundle::Bundle(Inst *inst);
 * Build a bundle from the first instruction of the bundle.
 * @param inst	First instructrion of the bundle.
 */

/**
 * @fn Address Bundle::address(void) const;
 * Get the starting address of the bundle.
 * @return	Bundle starting address.
 */

/**
 * Get the size of the bundle, that is, the sum of the sizes of instructions
 * composing the bundle.
 * @return	Bundle size.
 */
int Bundle::size(void) const {
	int s = 0;
	for(Iter i(*this); i; i++)
		s += i->size();
	return s;
}

/**
 * @fn Address Bundle::topAddress(void) const;
 * Get the address following the last byte of the bundle.
 * @return	Bundle top address.
 */

/**
 * Get the kind of the bundle, i.e. the bit-to-bit OR of
 * the kinds of the instructions composing the bundle.
 * @param	Bundle kind.
 */
Inst::kind_t Bundle::kind(void) const {
	Inst::kind_t k = 0;
	for(Iter i(*this); i; i++)
		k |= i->kind();
	return k;
}

/**
 * Get the target of the bundle if it contains a branch
 * (the first found branch instruction) or null address if
 * the bundle doesn't contain branch or if the branch is indirect.
 * @return	Target branch.
 */
Inst *Bundle::target(void) const {
	for(Iter i(*this); i; i++)
		if(i->isBranch()) {
			if(i->isIndirect())
				return null<Inst>();
			else
				return i->target();
		}
	return null<Inst>();
}

/**
 * Get the set of read registers, i.e. the union of registers
 * read by the instruction composing the bundle.
 * @param set	Register set to fill.
 */
void Bundle::readRegSet(RegSet& set) const {
	for(Iter i(*this); i; i++)
		i->readRegSet(set);
}

/**
 * Get the set of written registers, i.e. the union of registers
 * written by the instruction composing the bundle.
 * @param set	Register set to fill.
 */
void Bundle::writeRegSet(RegSet& set) const {
	for(Iter i(*this); i; i++)
		i->writeRegSet(set);
}

/**
 * Get the semantic instructions of the bundle, that is, a composition
 * of the semantic instructions of the machine instructions composing
 * the bundle such that:
 * @li register read is performed in parallel,
 * @li register write is performed in parallel.
 */
void Bundle::semInsts(sem::Block& block) const {
	int tmp = -1;
	for(Iter i(*this); i; i++)
		tmp = i->semInsts(block, tmp);
	tmp = -1;
	for(Iter i(*this); i; i++)
		tmp = i->semWriteBack(block, tmp);
}

/**
 * @class Bundle::Iter;
 * Iterator on the instructions composing the bundle.
 */

/**
 * @fn Iter Bundle::insts(void) const;
 * Get an iterator on the bundle instructions.
 * @return	Bundle instruction iterator.
 */

/**
 * @fn Iter Bundle::operator*(void) const;
 * Same as insts().
 */

}	// otawa
