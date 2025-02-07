/*
 *	rtti module
 *	Copyright (c) 2018, IRIT UPS <casse@irit.fr>
 *
 *	LBlockBuilder class interface
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <elm/rtti/Class.h>
#include <elm/rtti/type_of.h>

#include <otawa/prop.h>
#include <otawa/program.h>

using namespace elm;

namespace otawa {

static rtti::Class<Address> address_class(rtti::make("otawa::Address"));
rtti::Type& Address::__type = address_class;

// PropList class
static rtti::Class<PropList> prop_list_class(rtti::make("otawa::PropList")
	.construct<PropList>("new")
	.op("clearProps", &PropList::clearProps));
rtti::Type& PropList::__type = prop_list_class;

// ProgItem class
static rtti::Class<ProgItem, PropList, rtti::no_inst> prog_item_class(rtti::make("otawa::ProgItem")
	.op("size", &ProgItem::size)
	/*.op("address", &ProgItem::address)*/);
rtti::Type& ProgItem::__type = prog_item_class;

// Inst class
static rtti::Class<Inst, ProgItem, rtti::no_inst> inst_class(rtti::make("otawa::Inst"));
rtti::Type& Inst::__type = inst_class;

// Segment class
static rtti::Class<Segment, PropList, rtti::no_inst> segment_class(rtti::make("otawa::Segment")
	.op("name", &Segment::name)
	.iter<ProgItem *, Segment::ItemIter, Segment>("items"));
rtti::Type& Segment::__type = segment_class;

// File class
static rtti::Class<File, PropList, rtti::no_inst> file_class(rtti::make("otawa::File")
	.op("name", &File::name)
	.iter<Segment *, File::SegIter, File>("segments"));
rtti::Type& File::__type = file_class;

// Process class
static rtti::Class<Process, PropList, rtti::no_inst> process_class(rtti::make("otawa::Process")
	.op("program_name", &Process::program_name)
	.iter<File *, Process::FileIter, Process>("files"));
rtti::Type& Process::__type = process_class;

// Workspace class
static rtti::Class<WorkSpace, PropList, rtti::no_inst> workspace_class(rtti::make("otawa::WorkSpace")
	.op("process", &WorkSpace::process));
rtti::Type& WorkSpace::__type = workspace_class;


// Manager class
//static WorkSpace *(Manager::*manager_load)(const elm::sys::Path&, const PropList&) = &Manager::load;
static rtti::Class<Manager> __class(rtti::make("otawa::Manager")
	.op("def", Manager::def)
	/*.op("load", manager_load)*/);
rtti::Type& Manager::__type = __class;

}	// otawa

