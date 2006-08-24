#include <otawa/prop/Identifier.h>
#include <elm/utility.h>

using namespace elm;
using namespace otawa;

static class ConstIdentifierHashKey: public HashKey<const Identifier *> {
	virtual unsigned long hash(const Identifier *v) {
		return (unsigned long)v;
	};
	virtual bool equals(const Identifier *key1, const Identifier * key2) {
		return key1 == key2;
	};
} constidentifier_hkey_obj;

template <> HashKey<const Identifier *>& HashKey<const Identifier *>::def = constidentifier_hkey_obj;

