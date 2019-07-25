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
 * @class BaseBundle
 * Bundles are group of instructions that are executed together and in parallel
 * inside VLIW microprocessors. This class allows grouping instructions
 * composing a bundle and handling it as a single instruction. This is useful
 * for some low-level analyzes requiring to consider the group of instruction
 * as a single one. For non-VLIW instruction set, the bundle will only match
 * one instruction.
 *
 * Notice that this class is usually not used as is because an iterator
 * on the instruction composing the bundle is required. Instead, one can use
 * the class Bundle (for bundle anywhere in memory) or BasicBlock::Bundle
 * to visit bundles in a BB (using BasicBlock::BundleIter).
 *
 * @param I	Type of the iterator to look up instructions of the bundle.
 * @ingroup prog
 */

/**
 * @fn BaseBundle::Bundle(const I& i);
 * Build a bundle from the first instruction of the bundle.
 * @param i	Iterator on the first instruction.
 */

/**
 * @fn Address BaseBundle::address(void) const;
 * Get the starting address of the bundle.
 * @return	Bundle starting address.
 */

/**
 * @fn int BaseBundle::size(void) const;
 * Get the size of the bundle, that is, the sum of the sizes of instructions
 * composing the bundle.
 * @return	Bundle size.
 */

/**
 * @fn Address BaseBundle::topAddress(void) const;
 * Get the address following the last byte of the bundle.
 * @return	Bundle top address.
 */

/**
 * @fn Inst::kind_t BaseBundle::kind(void) const;
 * Get the kind of the bundle, i.e. the bit-to-bit OR of
 * the kinds of the instructions composing the bundle.
 * @param	Bundle kind.
 */

/**
 * @fn Inst *BaseBundle::target(void) const;
 * Get the target of the bundle if it contains a branch
 * (the first found branch instruction) or null address if
 * the bundle doesn't contain branch or if the branch is indirect.
 * @return	Target branch.
 */

/**
 * @fn void BaseBundle::readRegSet(RegSet& set) const;
 * Get the set of read registers, i.e. the union of registers
 * read by the instruction composing the bundle.
 * @param set	Register set to fill.
 */

/**
 * @fn void BaseBundle::writeRegSet(RegSet& set) const;
 * Get the set of written registers, i.e. the union of registers
 * written by the instruction composing the bundle.
 * @param set	Register set to fill.
 */

/**
 * @fn void BaseBundle::semInsts(sem::Block& block) const;
 * Get the semantic instructions of the bundle, that is, a composition
 * of the semantic instructions of the machine instructions composing
 * the bundle such that:
 * @li register read is performed in parallel,
 * @li register write is performed in parallel.
 */


/**
 * @fn void BaseBundle::semKernel(sem::Block& block) const;
 * Get the kernel of semantics instructions of the bundle.
 * Only the computation kernel of semantic instructions is
 * provided without the code dedicated to the condition support.
 * @param block	To fill the semantic instructions in.
 */


/**
 * @fn Iter BaseBundle::insts(void) const;
 * Get an iterator on the bundle instructions.
 * @return	Bundle instruction iterator.
 */

/**
 * @fn Iter BaseBundle::operator*(void) const;
 * Same as insts().
 */


/**
 * @class Bundle
 *
 * This BaseBundle derived class works on bundles obtained from the traversal of instructions
 * in the process.
 *
 * @ingroup prog
 */
}	// otawa
