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
#include <otawa/prog/WorkSpace.h>

namespace otawa {

/**
 * @lass LabelInfo
 * Interface of @ref LABEL_FEATURE.
 * @ingroup prog
 */

///
LabelInfo::~LabelInfo() {}

/**
 * @fn string LabelInfo::labelFor(Inst *inst);
 * Obtain a label for the given instruction. Use the executable label if
 * available, else built it from the address. This label is unique for the
 * instruction.
 * @param inst		Instruction to find label for.
 * @return			Label for the instruction.
 */

/**
 * @fn Address LabelInfo::addressOf(string name);
 * Compute the address of a label.
 * @param name	Label name.
 * @return		Address of the label or a null address.
 */

	
///	
class LabelSetter: public Processor, public LabelInfo {
public:
	static p::declare reg;
	LabelSetter(p::declare& r = reg): Processor(r) { }

	void *interfaceFor(const AbstractFeature& feature) override {
		if(&feature == &LABEL_FEATURE)
			return static_cast<LabelInfo *>(this);
		else
			return nullptr;
	}
	
	string labelFor(Inst *inst) override {
		string l= FUNCTION_LABEL(inst);
		if(l)
			return l;
		l = LABEL(inst);
		if(l)
			return l;
		auto s = SYMBOL(inst);
		if(s != nullptr)
			return s->name();
		return _ << "_x" << inst->address();
	}

	Address addressOf(string name) override {
		return workspace()->process()->findLabel(name);
	}

protected:

	virtual void processWorkSpace(WorkSpace *ws) {
		for(Process::FileIter file(ws->process()); file(); file++)
			for(File::SymIter sym(*file); sym(); sym++)
				switch(sym->kind()) {
				case Symbol::FUNCTION: {
						Inst *i = ws->process()->findInstAt(sym->address());
						if(i) {
							FUNCTION_LABEL(i) = sym->name();
							SYMBOL(i).add(*sym);
						}
					}
					break;
				case Symbol::LABEL: {
						Inst *i = ws->process()->findInstAt(sym->address());
						if(i) {
							LABEL(i) = sym->name();
							SYMBOL(i).add(*sym);
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
p::interfaced_feature<LabelInfo> LABEL_FEATURE("otawa::LABEL_FEATURE", new Maker<LabelSetter>());


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
p::id<Symbol *> SYMBOL("otawa::SYMBOL", 0);

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
p::id<String> LABEL("otawa::LABEL", "");


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
p::id<String> FUNCTION_LABEL("otawa::FUNCTION_LABEL", "");

} // otawa
