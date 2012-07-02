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

#include <otawa/data/clp/ClpValue.h>
#include <elm/util/test.h>

using namespace elm;
using namespace otawa;

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
		CHECK(v == clp::Value::all);
		v = clp::Value::none;
		v.ge(10);
		CHECK(v == clp::Value::none);

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
		clp::Value v = clp::Value::all;
		v.le(10);
		CHECK(v == clp::Value::all);
		v = clp::Value::none;
		v.le(10);
		CHECK(v == clp::Value::none);

		// constant
		v = clp::Value(clp::VAL, 5, 0, 0);
		v.le(10);
		CHECK(v == clp::Value(clp::VAL, 5, 0, 0));
		v = clp::Value(clp::VAL, 15, 0, 0);
		v.le(10);
		CHECK(v == clp::Value::none);

		// negative delta
		v = clp::Value(clp::VAL, 25, -1, 10);
		v.le(10);
		CHECK_EQUAL(v, clp::Value::none);
		v = clp::Value(clp::VAL, 5, -1, 10);
		v.le(10);
		CHECK_EQUAL(v, clp::Value(clp::VAL, -5, 1, 10));
		v = clp::Value(clp::VAL, 15, -1, 10);
		v.le(10);
		CHECK_EQUAL(v, clp::Value(clp::VAL, 5, 1, 5));
	}

	// geu tests
	{
		// T and _
		clp::Value v = clp::Value::all;
		v.geu(10);
		CHECK(v == clp::Value::all);
		v = clp::Value::none;
		v.geu(10);
		CHECK(v == clp::Value::none);

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
		CHECK(v == clp::Value(clp::VAL, 15, 1, 10));
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
		CHECK(v == clp::Value::all);
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

	CHECK_END
	return 0;
}
