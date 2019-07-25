/*
 *	$Id$
 *	Documentation of Resource class.
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

#include <otawa/parexegraph/Resource.h>

/**
 * @class Resource
 * Generic representation of a resource in the processor (pipeline stage, functional unit, slot in an instruction queue, register value, etc...). It is used to implement the block cost computation algorithm described in: "A Context-Parameterized Model for Static Analysis of Execution Times", by Christine Rochange and Pascal Sainrat, in Transactions on High-Performance Embedded Architectures and Compilers, 2(3), Springer, October 2007.
 */

/**
 * @fn Resource::Resource(elm::String name, resource_type_t type, int index);
 * @param name Resource name (used for debugging purposes).
 * @param type Resource type
 * @param index Resource rank in the list of resources attached to a ParExeGraph. Resources are numbered in the order they are created.
 */

/**
 * @fn elm::String Resource::name();
 * @return Name of the resource.
 */

/**
 * @fn resource_type_t Resource::type();
 * @return Type of the resource.
 */

/**
 * @fn index Resource::index();
 * @return Index of the resource (i.e. its rank in the list of resources related to a ParExeGraph).
 */

namespace otawa{

	ResourceList::ResourceList(WorkSpace *ws, ParExeProc *proc){
	    int resource_index = 0;
	    //bool is_ooo_proc = false;

	    // build the start resource
	    StartResource * new_resource = new StartResource((elm::String) "start", resource_index++);
	    _resources.add(new_resource);

	    // build resource for stages and FUs
	    for (ParExePipeline::StageIterator stage(proc->pipeline()) ; stage() ; stage++) {
			if (stage->category() != ParExeStage::EXECUTE) {
				for (int i=0 ; i<stage->width() ; i++) {
					StringBuffer buffer;
					buffer << stage->name() << "[" << i << "]";
					StageResource * new_resource = new StageResource(buffer.toString(), *stage, i, resource_index++);
					_resources.add(new_resource);
				}
			}
			else { // EXECUTE stage
				if (stage->orderPolicy() == ParExeStage::IN_ORDER) {
					for (int i=0 ; i<stage->numFus() ; i++) {
						ParExePipeline * fu = stage->fu(i);
						ParExeStage *fu_stage = fu->firstStage();
						for (int j=0 ; j<fu_stage->width() ; j++) {
							StringBuffer buffer;
							buffer << fu_stage->name() << "[" << j << "]";
							StageResource * new_resource = new StageResource(buffer.toString(), fu_stage, j, resource_index++);
							_resources.add(new_resource);
						}
					}
				}
				//else
				//	is_ooo_proc = true;
			}
	    }

	    // build resources for queues
	    for (ParExeProc::QueueIterator queue(proc) ; queue() ; queue++) {
			int num = queue->size();
			for (int i=0 ; i<num ; i++) {
				StringBuffer buffer;
				buffer << queue->name() << "[" << i << "]";
				StageResource * upper_bound;
				for (Vector<Resource *>::Iter resource(_resources) ; resource() ; resource++) {
					if (resource->type() == Resource::STAGE) {
						if (((StageResource *)(*resource))->stage() == queue->emptyingStage()) {
							if (i < queue->size() - ((StageResource *)(*resource))->stage()->width() - 1) {
								if (((StageResource *)(*resource))->slot() == ((StageResource *)(*resource))->stage()->width()-1) {
									upper_bound = (StageResource *) (*resource);
								}
							}
							else {
								if (((StageResource *)(*resource))->slot() == i - queue->size() + ((StageResource *)(*resource))->stage()->width()) {
									upper_bound = (StageResource *) (*resource);
								}
							}
						}
					}
				}
				ASSERT(upper_bound);
				// build the queue resource
				QueueResource * new_resource = new QueueResource(buffer.toString(), *queue, i, resource_index++, upper_bound, proc->pipeline()->numStages());
				_resources.add(new_resource);
			}
	    }

	    // build resources for registers
	    const otawa::hard::Platform::banks_t & reg_banks  =  ws->platform()->banks() ;
		elm::cout << reg_banks.count() << " banks to consider \n";
		for (int b=0 ; b<reg_banks.count() ; b++) {
			otawa::hard::RegBank * bank = (otawa::hard::RegBank *) reg_banks.get(b);
			elm::cout << reg_banks.count() << " bank" << b << " has " << bank->count() << " registers\n";
			for (int r=0 ; r<bank->count() ; r++)
			{
				StringBuffer buffer;
				buffer << bank->name() << r;
				RegResource * new_resource = new RegResource(buffer.toString(), bank, r, resource_index++);
				_resources.add(new_resource);
			}
		}



	    // build the resources for out-of-order execution
//	    if (is_ooo_proc) {																														// ======= TO BE CHECKED (OOO PROC)
//			int i = 0;
//			for (InstIterator inst(_sequence) ; inst ; inst++) {
//				StringBuffer buffer;
//				buffer << "extconf[" << i << "]";
//				ExternalConflictResource * new_resource = new ExternalConflictResource(buffer.toString(), inst, resource_index++);
//				_resources.add(new_resource);
//				StringBuffer another_buffer;
//				another_buffer << "intconf[" << i << "]";
//				InternalConflictResource * another_new_resource = new InternalConflictResource(another_buffer.toString(), inst, resource_index++);
//				_resources.add(another_new_resource);
//				i++;
//			}
//	    }
//
  }

} // namespace otawa
