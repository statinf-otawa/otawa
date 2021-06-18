/*
 *	test_clp test program
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2012, IRIT UPS.
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

#include <elm/test.h>
#include <otawa/data/clp/ClpValue.h>

using namespace elm;
using namespace otawa;

clp::Value none = clp::Value::none;
clp::Value all = clp::Value::all;
typedef clp::Value value;
const clp::intn_t inf = clp::MAXn;
const clp::intn_t ninf = clp::MINn;

inline clp::Value val(clp::intn_t l, clp::intn_t d, clp::intn_t n) {
	return clp::Value(clp::VAL, l, d, n);
}

inline bool test_le(value v, clp::intn_t k, value w) {
	value b = v;
	v.le(k);
	if(v != w)
		cerr << "\tbefore  : " << b << io::endl  
			 << "\tobtained: " << v << io::endl
			 << "\twaited  : " << w << io::endl; 
	return v == w;
}

int main(void) {
	CHECK_BEGIN("CLP Value")

	// equality
	{
		CHECK(clp::Value::all == clp::Value::all);
		CHECK(clp::Value::none == clp::Value::none);
		CHECK(clp::Value::all != clp::Value::none);
		clp::Value v1(clp::VAL, 5, 3, 10);
		clp::Value v2(clp::VAL, 3, 2, 10);
		CHECK(v1 == v1);
		CHECK(v1 != v2);
		CHECK(v1 != clp::Value::all);
		CHECK(v1 != clp::Value::none);
	}

	// Xwrap tests
	{
		clp::Value v1(clp::VAL, 10, 1, 100);
		clp::Value v2(clp::VAL, 10, 1, clp::UMAXn);
		clp::Value v3(clp::VAL, 10, 1, clp::MAXn - 10);
		clp::Value v4(clp::VAL, 10, 1, clp::MAXn);
		clp::Value v5(clp::VAL, 10, 1, clp::UMAXn - 10);
		CHECK(!v1.swrap());
		CHECK(v2.swrap());
		CHECK(!v3.swrap());
		CHECK(v4.swrap());
		CHECK(v5.swrap());
		CHECK(!v1.uwrap());
		CHECK(v2.uwrap());
		CHECK(!v3.uwrap());
		CHECK(!v4.uwrap());
		CHECK(!v5.uwrap());
	}

	// intersection test
	{
		clp::Value v1(clp::VAL, -10, 1, 20);
		clp::Value v2(clp::VAL, 10, 1, clp::MAXn - 10);
		v1.inter(v2);
		CHECK_EQUAL(v1, clp::Value(10));
	}


	// ge tests
	{
		// T and _
		clp::Value v = clp::Value::all;
		v.ge(10);
		CHECK_EQUAL(clp::Value(clp::Value::all).ge(10), clp::Value(clp::VAL, 10, 1, 0x7FFFFFF5));
		v = clp::Value::none;
		v.ge(10);
		CHECK_EQUAL(clp::Value(clp::Value::none).ge(10), clp::Value::none);

		// constant
		v = clp::Value(clp::VAL, 15, 0, 0);
		v.ge(10);
		CHECK(v == clp::Value(clp::VAL, 15, 0, 0));
		v = clp::Value(clp::VAL, 5, 0, 0);
		v.ge(10);
		CHECK(v == clp::Value::none);

		// positive delta
		v = clp::Value(clp::VAL, -10, 1, 15);
		v.ge(10);
		CHECK(v == clp::Value::none);
		v = clp::Value(clp::VAL, 15, 1, 10);
		v.ge(10);
		CHECK(v == clp::Value(clp::VAL, 15, 1, 10));
		v = clp::Value(clp::VAL, 0, 1, 20);
		v.ge(10);
		CHECK_EQUAL(v, clp::Value(clp::VAL, 10, 1, 10));

		// negative delta
		v = clp::Value(clp::VAL, 0, -1, 10);
		v.ge(10);
		CHECK_EQUAL(v, clp::Value::none);
		v = clp::Value(clp::VAL, 25, -1, 10);
		v.ge(10);
		CHECK_EQUAL(v, clp::Value(clp::VAL, 25, -1, 10));
		v = clp::Value(clp::VAL, 15, -1, 10);
		v.ge(10);
		CHECK_EQUAL(v, clp::Value(clp::VAL, 15, -1, 5));
	}

	// le tests
	{
		// T and _
		CHECK(test_le(all, 10, clp::Value(clp::VAL, 0x80000000, 1, 0x8000000a)));
		CHECK(test_le(::none, 10, ::none));

		// constant
		CHECK(test_le(val(5, 0, 0), 10, val(5, 0, 0)));
		CHECK(test_le(val(5, 0, 0), 0, ::none));

		// without wrap
		CHECK(test_le(val(16, 4, 4), 10, ::none));				// case a (positive)
		CHECK(test_le(val(32, -4, 4), 10, ::none));				// case a (negative)
		CHECK(test_le(val(16, 4, 4), 100, val(16, 4, 4)));		// case c (positive)
		CHECK(test_le(val(32, -4, 4), 100, val(32, -4, 4)));	// case c (negative)
		CHECK(test_le(val(16, 4, 4), 32, val(16, 4, 4)));		// case b (positive)
		CHECK(test_le(val(32, -4, 4), 32, val(32, -4, 4)));		// case b (negative)
		CHECK(test_le(val(16, 4, 4), 31, val(16, 4, 3)));		// case b (positive)
		CHECK(test_le(val(32, -4, 4), 31, val(28, -4, 3)));		// case b (negative)
		
		// with wrap
		CHECK(test_le(val(16, 4, inf), 10, ::none));				// case a (only positive)
		CHECK(test_le(val(-32, -4, inf), 0, val(-32, -4, (ninf + 32) / -4)));	// case c (only negative)
		CHECK(test_le(val(-32, 4, inf * 2), 0, val(-32, 4, 8)));		// case b (positive)
		CHECK(test_le(val(32, -4, inf * 2), 0, val(0, -4, clp::uintn_t(ninf) / 4)));	// case b (negative)
		
		// specific
		CHECK(test_le(val(0, 1, 0xffffffff), 0xff, val(0, 1, 0xff)));
	}

	// geu tests
	{
		// T and _
		CHECK_EQUAL(clp::Value(clp::Value::all).ge(10), clp::Value(clp::VAL, 10, 1, 0x7FFFFFF5));
		clp::Value v;
		CHECK_EQUAL(clp::Value(clp::Value::none).geu(10), clp::Value::none); 

		// constant
		v = clp::Value(clp::VAL, 15, 0, 0);
		v.geu(10);
		CHECK(v == clp::Value(clp::VAL, 15, 0, 0));
		v = clp::Value(clp::VAL, 5, 0, 0);
		v.geu(10);
		CHECK(v == clp::Value::none);

		// positive delta
		v = clp::Value(clp::VAL, 0, 1, 5);
		v.geu(10);
		CHECK(v == clp::Value::none);
		v = clp::Value(clp::VAL, 15, 1, 10);
		v.geu(10);
		CHECK_EQUAL(v, clp::Value(clp::VAL, 15, 1, 10));
		v = clp::Value(clp::VAL, 0, 1, 20);
		v.geu(10);
		CHECK_EQUAL(v, clp::Value(clp::VAL, 10, 1, 10));

		// negative delta
		v = clp::Value(clp::VAL, 5, -1, 5);
		v.geu(10);
		CHECK_EQUAL(v, clp::Value::none);
		v = clp::Value(clp::VAL, 25, -1, 10);
		v.geu(10);
		CHECK_EQUAL(v, clp::Value(clp::VAL, 25, -1, 10));
		v = clp::Value(clp::VAL, 15, -1, 10);
		v.geu(10);
		CHECK_EQUAL(v, clp::Value(clp::VAL, 15, -1, 5));
	}

	// leu tests
	{
		// T and _
		clp::Value v = clp::Value::all;
		v.leu(10);
		CHECK_EQUAL(clp::Value(clp::Value::all).leu(10), clp::Value(clp::VAL, 0x0, 1, 0xa));
		v = clp::Value::none;
		v.leu(10);
		CHECK(v == clp::Value::none);

		// constant
		v = clp::Value(clp::VAL, 15, 0, 0);
		v.leu(20);
		CHECK(v == clp::Value(clp::VAL, 15, 0, 0));
		v = clp::Value(clp::VAL, 15, 0, 0);
		v.leu(10);
		CHECK(v == clp::Value::none);

		// positive delta
		v = clp::Value(clp::VAL, 15, 1, 5);
		v.leu(10);
		CHECK(v == clp::Value::none);
		v = clp::Value(clp::VAL, 0, 1, 5);
		v.leu(10);
		CHECK(v == clp::Value(clp::VAL, 0, 1, 5));
		v = clp::Value(clp::VAL, 0, 1, 20);
		v.leu(10);
		CHECK_EQUAL(v, clp::Value(clp::VAL, 0, 1, 10));

		// negative delta
		v = clp::Value(clp::VAL, 20, -1, 5);
		v.leu(10);
		CHECK_EQUAL(v, clp::Value::none);
		v = clp::Value(clp::VAL, 5, -1, 5);
		v.leu(10);
		CHECK_EQUAL(v, clp::Value(clp::VAL, 0, 1, 5));
		v = clp::Value(clp::VAL, 15, -1, 10);
		v.leu(10);
		CHECK_EQUAL(v, clp::Value(clp::VAL, 5, 1, 5));
	}
	
	// le tests
	{
		clp::Value v = clp::Value::all;
		v = clp::Value(clp::VAL, 0x7FFFE775, -1, 0xFFFFFFFF);
		v.le(0xffffe775);
		CHECK_EQUAL(v, clp::Value(clp::VAL, -0x188b, -0x1, 0x7fffe775));
	}

	// substractions
	{
		CHECK_EQUAL(val(8, -1, 0xFFFFFFFF) - val(1, 0, 0), val(7, -1, 0xFFFFFFFF)); // inf1- k
		CHECK_EQUAL(val(2, 3, 2) - val(8, 1, 0xFFFFFFFF), val(0, -1, 0xFFFFFFFF)); // xxx - inf2
		CHECK_EQUAL(val(8, 1, 0xFFFFFFFF) - val(2, 3, 2), val(0, 1, 0xFFFFFFFF)); // inf1 - xxx
		CHECK_EQUAL(val(8, 1, 1) - val(2, 3, 2), val(0, 1, 7)); // xxx - yyy
		CHECK_EQUAL(val(2, 3, 0xFFFFFFFF) - val(8, 1, 0xFFFFFFFF), all); // inf1 - inf2
		CHECK_EQUAL(val(2, -3, 2) - val(8, -1, 0xFFFFFFFF), val(-12, 1, 0xFFFFFFFF));
		CHECK_EQUAL(val(8, -1, 0xFFFFFFFF) - val(2, -3, 2), val(12, -1, 0xFFFFFFFF));
		CHECK_EQUAL(val(8, -1, 1) - val(2, -3, 2), val(5, 1, 7));
		CHECK_EQUAL(val(7, 1, 1) - val(-4, 3, 2), val(5, 1, 7));
		CHECK_EQUAL(val(2, -3, 0xFFFFFFFF) - val(8, -1, 0xFFFFFFFF), all);
		CHECK_EQUAL(val(2, 3, 0xFFFFFFFF) - val(8, -1, 0xFFFFFFFF), val(-6, 1, 0xFFFFFFFF));
		CHECK_EQUAL(val(2, -3, 0xFFFFFFFF) - val(8, 1, 0xFFFFFFFF), val(-6, -1, 0xFFFFFFFF));
		CHECK_EQUAL(val(2, 3, 0xFFFFFFFF) - val(8, -1, 1), val(-6, 1, 0xFFFFFFFF));
		CHECK_EQUAL(val(2, -3, 0xFFFFFFFF) - val(8, 1, 1), val(-6, -1, 0xFFFFFFFF));
		CHECK_EQUAL(val(2, 3, 2) - val(8, -1, 1), val(-6, 1, 7));
		CHECK_EQUAL(val(2, -3, 2) - val(8, 1, 1), val(-6, -1, 7));
		CHECK_EQUAL(val(2, 3, 0xFFFFFFFF) - val(8, -1, 0xFFFFFFFF), val(-6, 1, 0xFFFFFFFF));
		CHECK_EQUAL(val(2, -3, 0xFFFFFFFF) - val(8, 1, 1), val(-6, -1, 0xFFFFFFFF));
		CHECK_EQUAL(val(2, -3, 2) - val(8, 1, 0xFFFFFFFF), val(-6, -1, 0xFFFFFFFF));
		CHECK_EQUAL(val(2, 3, 2) - val(8, -1, 0xFFFFFFFF), val(-6, 1, 0xFFFFFFFF));
		CHECK_EQUAL(val(0, 1, 0xFFFFFFFE) - val(0, -2, 0x7FFFFFFE), val(0, 1, 0xFFFFFFFF)); // testing the overflow
	}

	// checking intersections
	{
		clp::Value v = val(1, 1, 0x7FFFFFFE);
		clp::Value u = val(-1, -1, 0x7FFFFFFE);
		v.inter(u);
		CHECK_EQUAL(v, clp::Value::none);

		v = val(1, 1, 0x7FFFFFFF);
		u = val(-1, -1, 0x7FFFFFFF);
		v.inter(u);
		CHECK_EQUAL(v, val(0x80000000, 0, 0));

		v = val(2, -1, -1);
		u = val(2, 1, 1);
		v.inter(u);
		CHECK_EQUAL(v, val(0x2, 1, 0x1));

		v = val(2, -2, -1); // even though v circulates all even values
		u = val(2, 1, 1);
		v.inter(u); // when intersect with u, only "2" matches
		CHECK_EQUAL(v, val(0x2, 0, 0));

		v = clp::Value::none; // none intersect with anything is none
		u = val(2, 1, 1);
		v.inter(u);
		CHECK_EQUAL(v, clp::Value::none);

		CHECK_EQUAL(val(2,-1,-1).inter(val(2,0,0)), val(2, 0, 0));
		CHECK_EQUAL(val(2,0,-0).inter(val(2,-1,-1)), val(2, 0, 0));
		CHECK_EQUAL(val(3,0,0).inter(val(2,2,1000)), clp::Value::none);		
		
		CHECK_EQUAL(val(0x800004ac,0,0).inter(val(-0x7ffffb54, -0x1ffffee4, 0x1)), val(0x800004ac,0,0));
		CHECK_EQUAL(val(0x6784a1ea,0,0).inter(val(-0x29d12fb8, 0x2, 0x48aae8d1)), val(0x6784a1ea,0,0));	
	}
	
	// checking join
	{
		CHECK_EQUAL(val(0x84c4, 0, 0).join(val(0x84c4, -112, 0xffffffff)), val(0x84c4, -112, 0xffffffff));
		CHECK_EQUAL(val(0x2, 1, 0xffffffff).join(val(0x0, 8, 0x1)), val(0, 1, 0xffffffff));
		CHECK_EQUAL(val(0x20094c, 0x8, 0x1).join(val(0x200944, 0, 0)), val(0x200944, 0x8, 0x2));
	}
	
	// checking shr
	{
		CHECK_EQUAL(val(0x8, -0x1, 0xffffffff).shr(val(1, 0, 0)), val(4, -1, 0xffffffff)); // [-inf, 8] >> 2 = { -inf, ..... , 3, 4 }
		CHECK_EQUAL(val(0x9, -0x1, 0xfffffffd).shr(val(1, 0, 0)), val(0x4, -0x1, 0x7FFFFFFF) /* >> with value with inf mtimes */ );
		CHECK_EQUAL(val(0x2, 0x2, 11).shr(val(2, 0, 0)), val(0, 1, 6));
	}
	
	// checking +
	{
		CHECK_EQUAL(val(0x8, -0x4, 0xffffffff) + val(0x0, 0x1, 0x1), val(0x9, -0x1, 0xffffffff) /* adding with value with inf mtimes */);
	}
	
	// checking and
	{
		CHECK_EQUAL(val(0xa7, -1, 0x63)._and(val(0xc0000, 0, 0)), val(0, 0, 0));
		CHECK_EQUAL(val(7, -1, 3)._and(val(0xc0000, 0, 0)), val(0, 0, 0));
		CHECK_EQUAL(val(0x0, 0x1, 0x2)._and(val(0xa0000002, 0, 0)), val(0, 2, 1));
		CHECK_EQUAL(val(0xff, 0, 0)._and(val(-0xff, 0x1, 0x1fe)), val(0, 1, 0xff));
	}
	
	// checking widening
	{
		CHECK_EQUAL(val(0, 0, 0).widening(val(0, 1, 0xffffffff)), val(0, 1, 0xffffffff));
		CHECK_EQUAL(val(-1, -1, 0xffffffff).widening(val(0x0, -1, 0xffffffff)), clp::Value::all);
		CHECK_EQUAL(val(1,0,0).widening(val(2,0,0)), val(1, 1, 0xffffffff));
		CHECK_EQUAL(val(0x6000000C,4,0xffffffff).widening(val(0x6000004c,0,0)), val(0x6000000c, 4, 0xffffffff));
		CHECK_EQUAL(val(0x6000000C,4,7).widening(val(0x6000004c,0,0)), val(0x6000000c, 4, 0x10));
		CHECK_EQUAL(val(0x6000000C,4,0x20).widening(val(0x6000004c,0,0)), val(0x6000000c, 4, 0x20));
		CHECK_EQUAL(val(0x60000004,8,0xFFFFFFFF).widening(val(0x6000000C,4,0xFFFFFFFF)), val(0x60000004, 4, 0xFFFFFFFF));
		CHECK_EQUAL(val(2,0,0).widening(val(3,1,0xFFFFFFFF)), val(2, 1, 0xFFFFFFFF));
	}
	
	CHECK_RETURN
}
