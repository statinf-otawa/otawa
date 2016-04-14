/*
 *	label features implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#include <otawa/proc/Processor.h>
#include <otawa/program.h>
#include <otawa/prop/Identifier.h>

namespace otawa {

class LabelSetter: public Processor {
public:
	static p::declare reg;
	LabelSetter(p::declare& r = reg): Processor(r) {
	}

protected:

	virtual void processWorkSpace(WorkSpace *ws) {
		for(Process::FileIter file(ws->process()); file; file++)
			for(File::SymIter sym(file); sym; sym++)
				switch(sym->kind()) {
				case Symbol::FUNCTION: {
						Inst *i = ws->process()->findInstAt(sym->address());
						if(i) {
							FUNCTION_LABEL(i) = sym->name();
							SYMBOL(i).add(sym);
						}
					}
					break;
				case Symbol::LABEL: {
						Inst *i = ws->process()->findInstAt(sym->address());
						if(i) {
							LABEL(i) = sym->name();
							SYMBOL(i).add(sym);
						}
					}
					break;
				default:
					break;
				}
	}

};


/**
 * This feature ensures that label or symbol information has been linked to the
 * concerned instruction. It is not mandatory for OTAWA work but improve
 * the readability of error message or logs.
 *
 * @par Properties
 * @li @ref LABEL
 * @li @ref FUNCTION_LABEL
 * @li @ref SYMBOL
 */
p::feature LABEL_FEATURE("otawa::LABEL_FEATURE", new Maker<LabelSetter>());


/**
 */
p::declare LabelSetter::reg = p::init("otawa::LabelSetter", Version(1, 0, 0))
	.provide(LABEL_FEATURE)
	.maker<LabelSetter>();


/**
 * This property carries the symbol attached to an instruction.
 * Notice that an instruction may support several symbols.
 *
 * @par Hooks
 * @li @ref Inst
 *
 * @par Features
 * @li @ref LABEL_FEATURE
 *
 * @ingroup prog
 */
Identifier<Symbol *> SYMBOL("otawa::SYMBOL", 0);

/**
 * Property with this identifier is put on instructions or basic blocks which a symbol is known for.
 *
 * @par Hooks
 * @li @ref Inst
 *
 * @par Features
 * @li @ref LABEL_FEATURE
 *
 * @ingroup prog
 */
Identifier<String> LABEL("otawa::LABEL", "");


/**
 * This property is put on instruction. An instruction may accept many
 * properties of this type.
 *
 * @par Hooks
 * @li @ref Inst
 *
 * @par Features
 * @li @ref LABEL_FEATURE
 *
 * @ingroup prog
 */
Identifier<String> FUNCTION_LABEL("otawa::FUNCTION_LABEL", "");

} // otawa
