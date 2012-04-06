/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	test/ipet/test_hard.cpp -- test for hardware features.
 */

#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/ilp.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/hard/Processor.h>
#include <otawa/hard/Platform.h>

using namespace elm;
using namespace otawa;
using namespace otawa::hard;

static CString reg_kinds[] = {
	"none",
	"addr",
	"int",
	"float",
	"bits"
};

static void display_cache(const Cache *cache) {
	cout << "\tsize = " << cache->cacheSize() << '\n';
	cout << "\tblock size = " << cache->blockSize() << '\n';
	cout << "\tassociativity = " << cache->wayCount() << '\n';
	cout << "\treplace policy = "
		 << elm::serial2::Enum<Cache::replace_policy_t>::type.nameOf(cache->replacementPolicy())
		 << io::endl;
	cout << "\twrite policy = "
		 << elm::serial2::Enum<Cache::write_policy_t>::type.nameOf(cache->writePolicy())
		 << io::endl;
	cout << "\tmiss penalty = " << cache->missPenalty() << io::endl;
}

static void display_cache_level(int level, const Cache *icache,
const Cache *dcache) {
	if(icache || dcache) {
		if(icache == dcache) {
			cout << "Cache Level " << level << " unified\n";
			display_cache(icache);
			icache = icache->nextLevel();
			dcache = icache;
		}
		else {
			cout << "Cache Level " << level << '\n';
			if(icache) {
				cout << "\tInstruction Cache\n";
				display_cache(icache);
				icache = icache->nextLevel();
			}
			if(dcache) {
				cout << "\tData Cache\n";
				display_cache(dcache);
				dcache = dcache->nextLevel();
			}
		}
		display_cache_level(level + 1, icache, dcache);
	}
}

int main(int argc, char **argv) {

	Manager manager;
	PropList props;
	PROCESSOR_PATH(props) = "../../data/procs/op1.xml";
	CACHE_CONFIG_PATH(props) = "../../data/caches/inst-64x16x1.xml";

	try {

		// Load program
		if(argc < 2) {
			cerr << "ERROR: no argument.\n"
				 << "Syntax is : test_ipet <executable>\n";
			return 2;
		}
		WorkSpace *fw = manager.load(argv[1], props);
		assert(fw);

		// Display information
		cout << "PLATFORM INFORMATION\n";
		Platform *pf = fw->platform();
		cout << "Platform : " << pf->identification().name() << '\n';
		cout << '\n';

		// Display registers
		cout << "REGISTERS\n";
		for(int i = 0; i < pf->banks().count(); i++) {
			const hard::RegBank *bank = pf->banks()[i];
			cout << "Bank " << bank->name() << ", "
				 << bank->size() << "bits, "
				 << bank->count() << " registers, "
				 << reg_kinds[bank->kind()];
			for(int j = 0; j < bank->registers().count(); j++) {
				if(j % 8 == 0)
					cout << "\n\t";
				else
					cout << ", ";
				cout << bank->registers()[j]->name();
			}
			cout << '\n';
		}
		cout << '\n';

		// Display cache
		cout << "CACHE CONFIGURATION\n";
		const CacheConfiguration& cconf(pf->cache());
		display_cache_level(1, cconf.instCache(), cconf.dataCache());
		cout << '\n';

		// Display some instructions
		fw->require(DECODED_TEXT);
		cout << "READ/WRITTEN REGS TEST\n";
		String label("main");
		Inst *inst = fw->process()->findInstAt("main");
		//fw->findLabel(label));
		if(!inst)
			throw new otawa::Exception(CString("no main in this file ?"));
		for(int i = 0; i < 10; i++, inst = inst->nextInst()) {
			cout << '\n' << inst->address() << ": "
				 << inst << " (" << io::hex(inst->kind()) << ")\n";
			const elm::genstruct::Table<hard::Register *>& reads = inst->readRegs();
			cout << "\tread registers : ";
			for(int i = 0; i < reads.count(); i++)
				cout << reads[i] << ' ';
			cout << '\n';
			const elm::genstruct::Table<hard::Register *>& writes = inst->writtenRegs();
			cout << "\twritten registers : ";
			for(int i = 0; i < writes.count(); i++)
				cout << writes[i] << ' ';
			cout << '\n';
		}
		cout << io::endl;


		// Processor load test
		cout << "Processor load test\n";
		//pf->loadProcessor("proc.xml");
		const hard::Processor *proc = pf->processor();
		if(!proc)
			cout << "NO PROCESSOR !\n";
		else {
			cout << "arch = " << proc->getArch() << io::endl;
			cout << "model = " << proc->getModel() << io::endl;
			cout << "builder = " << proc->getBuilder() << io::endl;
			cout <<"stages =\n";
			const elm::genstruct::Table<hard::Stage *>& stages = proc->getStages();
			for(int i = 0; i< stages.count(); i++) {
				cout << '\t'
					 << stages[i]->getName() << " "
					 << stages[i]->getType() << " "
				     << stages[i]->getWidth() << " "
				     << stages[i]->getLatency() << " "
				     << io::pointer(stages[i]) << io::endl;
				const elm::genstruct::Table<hard::FunctionalUnit *>& fus = stages[i]->getFUs();
				if(fus) {
					cout << "\tfus=\n";
					for(int i = 0; i < fus.count(); i++)
						cout << "\t\t" << fus[i]->getName() << ' '
							 << fus[i]->getWidth() << ' '
							 << fus[i]->getLatency() << ' '
							 << fus[i]->isPipelined() << io::endl;
				}
				const elm::genstruct::Table<hard::Dispatch *>& dispatch = stages[i]->getDispatch();
				if(dispatch) {
					cout << "\tdispatch=\n";
					for(int i = 0; i < dispatch.count(); i++)
						cout << "\t\t" << dispatch[i]->getType() << ' '
							 << dispatch[i]->getFU()->getName() << io::endl;
				}
			}
			cout << "queues =\n";
			const elm::genstruct::Table<hard::Queue *>& queues = proc->getQueues();
			for(int i = 0; i< queues.count(); i++) {
				cout << '\t'
					 << queues[i]->getName() << " "
				     << queues[i]->getSize() << " "
				     << queues[i]->getInput()->getName() << " ("
				     << io::pointer(queues[i]->getInput()) << ") "
				     << queues[i]->getOutput()->getName() << " ("
				     << io::pointer(queues[i]->getOutput()) << ")" << io::endl;
				const elm::genstruct::Table<hard::Stage *>& intern = queues[i]->getIntern();
				if(intern) {
					cout << "\tintern=\n";
					for(int i = 0; i < intern.count(); i++)
						cout << "\t\t" << intern[i]->getName()
							 << " (" << io::pointer(intern[i]) << ')' << io::endl;
				}
			}
		}
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << '\n';
		return 1;
	}
	return 0;
}

