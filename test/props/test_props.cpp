/*
 *	CFG class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-18, IRIT UPS.
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
#include <otawa/dfa/BitSet.h>
#include <otawa/ipet/IPET.h>
#include <otawa/prop/info.h>
#include <otawa/prop/ContextualProperty.h>
#include "../../include/otawa/prop.h"

using namespace elm;
using namespace otawa;

Identifier<int> MY_ID("my_id", 666), OTHER_ID("other", 111);
Identifier<CString> MY_STRING("my_string", "ok");

int main(void) {
	
CHECK_BEGIN("otawa_props")
	{
		PropList props;
		const PropList& cprops = props;
		Identifier<int> ID1("id1", 0), ID2("id2", 0);

		// Empty test
		CHECK(!props.hasProp(ID1));
		CHECK(!props.hasProp(ID2));
		CHECK(!cprops.hasProp(ID1));
		CHECK(!cprops.hasProp(ID2));

		// Simple getter
		ID1(props) = 111;
		CHECK(props.hasProp(ID1));
		CHECK(ID1(props) == 111);

		// Iterator getter
		int cnt = 0;
		for(Identifier<int>::Getter prop(cprops, ID1); prop(); prop++)
			cnt++;
		CHECK(cnt == 1);
		cnt = 0;
		for(Identifier<int>::Getter prop(cprops, ID2); prop(); prop++)
			cnt++;
		CHECK(cnt == 0);

		// Functional getter
		CHECK(ID1(props) == 111);
		CHECK(ID2(props) == 0);
		CHECK(ID1(cprops) == 111);
		CHECK(ID2(cprops) == 0);

		// Functional setter
		ID1(props) = 666;
		CHECK(ID1(props) == 666);
		CHECK(ID2(props) == 0);
		CHECK(ID1(cprops) == 666);
		CHECK(ID2(cprops) == 0);
		cnt = 0;
		for(Identifier<int>::Getter prop(cprops, ID1); prop(); prop++)
			cnt++;
		CHECK(cnt == 1);
		cnt = 0;
		for(Identifier<int>::Getter prop(cprops, ID2); prop(); prop++)
			cnt++;
		CHECK(cnt == 0);

		// Property output
		{
			PropList props;
			MY_ID(props) = 111;
			MY_STRING(props) = "ko\ncoucoué";
			MY_STRING(props).add("ohe !");
			cout << props << io::endl;
		}

		// Assignment property to property
		{
			PropList props;
			ID1(props) = 666;
			ID2(props) = ID1(props);
			CHECK(ID1(props) == ID2(props));
			cout << "prop-to-prop: " << ID1(props) << " = " << ID2(props) << io::endl;
		}

		// Proplist in collection
		{
			Vector<PropList> props;
			for(int i = 0; i < 256; i++) {
				//cout << "-> " << i << io::endl;
				PropList propl;
				MY_ID(propl) = 111;
				props.add(propl);
			}
			bool checked = true;
			for(int i = 0; i < 256; i++)
				if(MY_ID(props[i]) != 111) {
					checked = false;
					break;
				}
			CHECK(checked);
		}

		// Identifier find
		{
			AbstractIdentifier *id = AbstractIdentifier::find("otawa::ipet::WCET");
			CHECK(id == & otawa::ipet::WCET);
		}
	}

	{
#		define PRINT(n) cout << #n << " = " << (n) << io::endl
		static Identifier<int> INT("", -1);
		AbstractIdentifier *pINT = &INT;
		PropList props;
		INT(props) = 111;

		PRINT(INT(props));
		PRINT((*(Identifier<int> *)pINT)(props));

		//INT.print(cout, props.getProp(&INT));
		pINT->print(cout, props.getProp(pINT)); cout << io::endl;
	}

	{
		cerr << "ipet::WCET = " << IDENTIFIER_LABEL(ipet::WCET) << io::endl;
		cerr << "ipet::TIME = " << IDENTIFIER_LABEL(ipet::TIME) << io::endl;
		cerr << "ipet::COUNT = " << IDENTIFIER_LABEL(ipet::COUNT) << io::endl;
	}

	{
		static Identifier<int> ID("ID", -1), BAD("BAD", -1), ID2("ID2", -1);
		PropList props;
		ContextualPath path;

		// simple case
		ID(props) = 111;
		ContextualProperty::printFrom(cerr, props);
		CHECK(ID(props) == 111);
		CHECK(BAD(props) == -1);
		CHECK(path(ID, props) == 111);
		CHECK(path(BAD, props) == -1);

		// simple path
		path.push(ContextualStep::FUNCTION, Address(1));
		path(ID, props) = 222;
		ContextualProperty::printFrom(cerr, props);
		CHECK(ID(props) == 111);
		CHECK(BAD(props) == -1);
		CHECK_EQUAL((int)path(ID, props), 222);
		CHECK(path(BAD, props) == -1);

		// double value
		path(ID2, props) = 333;
		ContextualProperty::printFrom(cerr, props);
		CHECK_EQUAL((int)ID2(props), -1);
		CHECK(path(ID2, props) == 333);

		// far value
		path.clear();
		path.push(ContextualStep::FUNCTION, Address(3));
		path.push(ContextualStep::FUNCTION, Address(2));
		path.push(ContextualStep::FUNCTION, Address(1));
		path(ID, props) = 444;
		ContextualProperty::printFrom(cerr, props);
		CHECK_EQUAL((int)ID(props), 111);
		CHECK_EQUAL(int(path(ID, props)), 444);
		CHECK_EQUAL(int(path(ID2, props)), 333);

		// blurred path value
		path.clear();
		path.push(ContextualStep::FUNCTION, Address(3));
		path.push(ContextualStep::FUNCTION, Address(1));
		path.ref(ID, props) = 555;
		ContextualProperty::printFrom(cerr, props);
		CHECK(path(ID, props) == 555);
		CHECK(path(ID2, props) == 333);

		// more precise path
		path.clear();
		path.push(ContextualStep::FUNCTION, Address(3));
		path.push(ContextualStep::CALL, Address(3));
		path.push(ContextualStep::FUNCTION, Address(2));
		path.push(ContextualStep::FUNCTION, Address(1));
		CHECK_EQUAL((int)ID(props), 111);
		CHECK(path(ID, props) == 444);
		CHECK(path(ID2, props) == 333);

		// removal test
		path(ID, props).remove();
		CHECK(path(ID, props) == 222);
	}

	{
		const int N = 10;
		PropList props;
		for(int i = 0; i < N; i++) {
			if(i % 2 == 1)
				OTHER_ID(props).add(i);
			else
				MY_ID(props).add(i);
		}

		dfa::BitSet set(N / 2);
		bool ok = true;
		for(auto i: MY_ID(props).all()) {
			if(i % 2 != 0 || set.contains(i / 2))
				ok = false;
			set.add(i / 2);
		}
		CHECK(ok);
		CHECK(set.isFull());
	}

CHECK_RETURN
}
