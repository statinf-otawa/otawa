#include <otawa/app/Application.h>
#include <otawa/prog/WorkSpace.h>

using namespace otawa;


class SLook: public Application {
public:
	SLook(void): Application("slook", Version(1, 0, 0), "Symbol lookup from address") {
	}

protected:
	virtual void work(const string &entry, PropList &props) throw (elm::Exception) {
		Address::offset_t offset;
		entry >> offset;
		Symbol *sym = workspace()->process()->findSymbolAt(Address(offset));
		if(!sym)
			cout << "Symbol at " << Address(offset) << ": not found\n";
		else
			cout << "Symbol at " << Address(offset) << ": "
				 << '"' << sym->name() << "\", "
				 << sym->kind() << ' '
				 << sym->address() << ':' << sym->size()
				 << io::endl;
	}

};

OTAWA_RUN(SLook);
