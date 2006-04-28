/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	test/ipet/test_hard.cpp -- test for hardware features.
 */

#include <stdlib.h>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hardware/CacheConfiguration.h>
#include <otawa/ilp.h>

using namespace elm;
using namespace otawa;

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
	
	Cache::info_t info = {
		1,
		10,
		4,
		6,
		2,
		Cache::LRU,
		Cache::WRITE_THROUGH,
		false
	};
	Cache data_cache(info);
	CacheConfiguration cache_conf(0, &data_cache);
	Manager manager;
	PropList props;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
	props.set<CacheConfiguration *>(Platform::ID_Cache, &cache_conf);
	
	try {
		
		// Load program
		if(argc < 2) {
			cerr << "ERROR: no argument.\n"
				 << "Syntax is : test_ipet <executable>\n";
			exit(2);
		}
		FrameWork *fw = manager.load(argv[1], props);
		assert(fw);
		
		// Display information
		Platform *pf = fw->platform();
		cout << "Platform : " << pf->identification().name() << '\n';
		
		// Display registers
		for(int i = 0; i < pf->banks().count(); i++) {
			hard::RegBank *bank = pf->banks()[i];
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
		
		// Display cache
		const CacheConfiguration& cconf(pf->cache());
		display_cache_level(1, cconf.instCache(), cconf.dataCache());
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	catch(ProcessorException e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

