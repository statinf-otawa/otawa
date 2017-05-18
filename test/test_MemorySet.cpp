/*
 *	MemorySet class unit testing
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2015, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#include <elm/test.h>
#include <otawa/dfa/MemorySet.h>

using namespace otawa;
using namespace otawa::dfa;

int main(void) {
CHECK_BEGIN("MemorySet")

	MemorySet set;
	typedef MemorySet::t t;
	MemArea a1(0x100, Address(0x200));	// --XXXX-------------
	MemArea a2(0x400, Address(0x500));	// -------------XXXX--
	MemArea a3(0x300, Address(0x350));	// --------XX---------
	MemArea a4(0x150, Address(0x450));	// ---XXXXXXXXXXXX----
	MemArea a5(0x050, Address(0x150));	// -XXX---------------
	MemArea a6(0x450, Address(0x550));  // ---------------XXX-
	MemArea a7(0x120, Address(0x180));			// ---XX--------------

	// empty test
	CHECK_EQUAL(set.empty, set.empty);
	CHECK(!(Address(0x150) % set.empty));

	// addition tests
	{
		// simple addition
		t m1 = set.add(set.empty, a1);
		CHECK_EQUAL(m1, m1);
		CHECK(Address(0x150) % m1);
		CHECK(Address(0x100) % m1);
		CHECK(!(Address(0x200) % m1));
		CHECK(!(Address(0x450) % m1));
		cerr << "DEBUG: a1 = " << a1 << ", " << a1.isEmpty() << io::endl;
		CHECK(a1 % m1);
		CHECK(!(a2 % m1));
		CHECK(!(a4 % m1));
		CHECK(!(a5 % m1));

		// disjoined addition
		t m2 = set.add(m1, a2);
		CHECK_EQUAL(m2, m2);
		CHECK(Address(0x150) % m1);
		CHECK(Address(0x450) % m2);
		CHECK(!(Address(0x300) % m2));
		CHECK(!(Address(0x50) % m2));
		CHECK(!(Address(0x550) % m2));

		// joined addition
		t m3 = set.add(m1, a4);
		CHECK(Address(0x150) % m3);
		CHECK(Address(0x200) % m3);
		CHECK(Address(0x250) % m3);
		CHECK(Address(0x400) % m3);
		CHECK(!(Address(0x50) % m3));
		CHECK(!(Address(0x500) % m3));
		CHECK(!(Address(0x450) % m3));

		// free all
		set.free(m1);
		set.free(m2);
		set.free(m3);
	}

	// remove test
	{
		// remove in middle
		t m1 = set.add(MemorySet::empty, MemArea(0x100, 0x100));
		t m2 = set.remove(m1, MemArea(0x120, 0x60));
		CHECK(Address(0x110) % m2);
		CHECK(Address(0x190) % m2);
		CHECK(!(Address(0x150) % m2));
		CHECK(!(Address(0x50) % m2));
		CHECK(!(Address(0x250) % m2));
		set.free(m1);
		set.free(m2);

		// remove before
		m1 = set.add(MemorySet::empty, MemArea(0x100, 0x100));
		m2 = set.remove(m1, MemArea(0x50, 0x100));
		CHECK(Address(0x150) % m2);
		CHECK(Address(0x180) % m2);
		CHECK(!(Address(0x100) % m2));
		CHECK(!(Address(0x50) % m2));
		CHECK(!(Address(0x200) % m2));
		CHECK(!(Address(0x300) % m2));
		set.free(m1);
		set.free(m2);

		// remove after
		m1 = set.add(MemorySet::empty, MemArea(0x100, 0x100));
		m2 = set.remove(m1, MemArea(0x150, 0x100));
		CHECK(Address(0x100) % m2);
		CHECK(Address(0x120) % m2);
		CHECK(!(Address(0x50) % m2));
		CHECK(!(Address(0x150) % m2));
		CHECK(!(Address(0x200) % m2));
		CHECK(!(Address(0x300) % m2));
		set.free(m1);
		set.free(m2);

		// remove between
		m1 = set.add(MemorySet::empty, MemArea(0x100, 0x100));
		m2 = set.add(m1, MemArea(0x300, 0x100));
		t m3 = set.remove(m2, MemArea(0x220, 0x280));
		CHECK(Address(0x100) % m3);
		CHECK(Address(0x150) % m3);
		CHECK(Address(0x300) % m3);
		CHECK(Address(0x350) % m3);
		CHECK(!(Address(0x50) % m3));
		CHECK(!(Address(0x250) % m3));
		CHECK(!(Address(0x450) % m3));
		set.free(m1);
		set.free(m2);
		set.free(m3);

		// remove across
		m1 = set.add(MemorySet::empty, MemArea(0x100, 0x100));
		m2 = set.add(m1, MemArea(0x300, 0x100));
		m3 = set.remove(m2, MemArea(0x150, 0x200));
		CHECK(!(Address(0x50) % m3));
		CHECK(Address(0x100) % m3);
		CHECK(Address(0x120) % m3);
		CHECK(!(Address(0x150) % m3));
		CHECK(!(Address(0x200) % m3));
		CHECK(!(Address(0x300) % m3));
		CHECK(Address(0x350) % m3);
		CHECK(Address(0x380) % m3);
		CHECK(!(Address(0x400) % m3));
		CHECK(!(Address(0x450) % m3));
	}

	// equality test
	{
		// different source
		t m1 = set.add(set.empty, MemArea(0x100, 0x100));
		t m2 = set.add(set.empty, MemArea(0x100, 0x100));
		CHECK_EQUAL(m1, m2);
		set.free(m1);
		set.free(m2);

		// sparse set
		m1 = set.add(set.empty, MemArea(0x100, 0x100));
		m2 = set.add(m1, MemArea(0x300, 0x100));
		t m3 = set.add(m1, MemArea(0x300, 0x100));
		CHECK_EQUAL(m2, m3);
		set.free(m1);
		set.free(m2);
		set.free(m3);

		// join set
		m1 = set.add(set.empty, MemArea(0x100, 0x200));
		m2 = set.add(set.empty, MemArea(0x100, 0x100));
		m3 = set.add(m2, MemArea(0x200, 0x100));
		CHECK_EQUAL(m1, m3);
		set.free(m1);
		set.free(m2);
		set.free(m3);
	}

	// join tests
	{

		// sparse join
		t m1 = set.add(set.empty, MemArea(0x100, 0x100));
		t m2 = set.add(set.empty, MemArea(0x300, 0x100));
		t m3 = set.join(m1, m2);
		CHECK(Address(0x100) % m3);
		CHECK(Address(0x150) % m3);
		CHECK(Address(0x300) % m3);
		CHECK(Address(0x350) % m3);
		CHECK(!(Address(0x50) % m3));
		CHECK(!(Address(0x200) % m3));
		CHECK(!(Address(0x250) % m3));
		CHECK(!(Address(0x400) % m3));
		CHECK(!(Address(0x450) % m3));
		set.free(m1);
		set.free(m2);
		set.free(m3);

		// melting join
		m1 = set.add(set.empty, MemArea(0x100, 0x200));
		m2 = set.add(set.empty, MemArea(0x200, 0x200));
		m3 = set.join(m1, m2);
		CHECK(Address(0x100) % m3);
		CHECK(Address(0x200) % m3);
		CHECK(Address(0x300) % m3);
		CHECK(!(Address(0x50) % m3));
		CHECK(!(Address(0x400) % m3));
		CHECK(!(Address(0x500) % m3));
		set.free(m1);
		set.free(m2);
		set.free(m3);

		// interleaved join
		m1 = set.add(set.empty, MemArea(0x100, 0x100));
		m2 = set.add(m1, MemArea(0x500, 0x100));
		m3 = set.add(set.empty, MemArea(0x300, 0x100));
		t m4 = set.add(m3, MemArea(0x600, 0x100));
		t m5 = set.join(m2, m4);
		CHECK(!(Address(0x50) % m5));
		CHECK(Address(0x150) % m5);
		CHECK(!(Address(0x250) % m5));
		CHECK(Address(0x350) % m5);
		CHECK(!(Address(0x450) % m5));
		CHECK(Address(0x550) % m5);
		CHECK(Address(0x650) % m5);
		CHECK(!(Address(0x750) % m5));
		set.free(m1);
		set.free(m2);
		set.free(m3);
		set.free(m4);
		set.free(m5);

		// empty is null element for join
		t t1 = set.add(set.empty, MemArea(0x100, 0x100));
		t t2 = set.add(t1, MemArea(0x300, 0x100));
		t t3 = set.join(set.empty, t2);
		CHECK_EQUAL(t2, t3);
		t t4 = set.join(t2, set.empty);
		CHECK_EQUAL(t2, t4);
		set.free(t1);
		set.free(t2);
		set.free(t3);
		set.free(t4);
	}

CHECK_END
}
