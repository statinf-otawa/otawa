/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test.cc -- test program.
 */

#include <stdio.h>
#include <otawa/instruction.h>
#include <otawa/program.h>
#include <otawa/manager.h>
using namespace otawa;

//#define PATH	"gliss-ppc/prog/primes"
#define PATH	"hept_loader/hello"

namespace otawa { namespace gliss {
	extern otawa::Loader& loader;
} } // otawa::gliss;

int main(void) {
	
	// Load the file
	Manager *man = new Manager();
	PropList args;
	args.set<Loader *>(Loader::ID_Loader, &otawa::gliss::loader); 
	FrameWork *fw = man->load(PATH, args);
	
	// Display the file
	for(Iterator<File *> file(fw->files()); file; file++) {
		printf("FILE: %s\n", &file->name());
		
		for(Iterator<Segment *> seg(file->segments()); seg; seg++) {
			printf("\tSEGMENT: %s [%p:%08x] %s %s \n", &seg->name(), seg->address(), seg->size(),
				(seg->flags() & Segment::EXECUTABLE) ? "EXECUTABLE" : "",
				(seg->flags() & Segment::WRITABLE) ? "WRITABLE" : "");
			
			for(Iterator<ProgItem *> item(seg->items()); item; item++) {
				printf("\t\tITEM: %s [%p:%08x] \n", &item->name(), item->address(), item->size());
				
				if(seg->flags() & Segment::EXECUTABLE) {
					Code *code = (Code *)*item;
					for(Inst *inst = code->first(); !inst->atEnd(); inst = inst->next())
						printf("\t\t\t%p: \n", inst->address());
				}
			}
		}
	}
}
