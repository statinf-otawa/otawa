/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	test/props/test_props.cpp -- OTAWA props module test.
 */

#include <elm/util/test.h>
#include <otawa/properties.h>
#include <elm/genstruct/Vector.h>
#include <otawa/ipet/IPET.h>

using namespace elm;
using namespace otawa;

Identifier<int> MY_ID("my_id", 666);
Identifier<CString> MY_STRING("my_string", "ok");

int main(void) {
	
	CHECK_BEGIN("otawa.props")
	PropList props;
	const PropList& cprops = props;
	Identifier<int> ID1("id1", 0), ID2("id2", 0);
	
	// Empty test
	CHECK(!props.hasProp(ID1));
	CHECK(!props.hasProp(ID2));
	CHECK(!cprops.hasProp(ID1));
	CHECK(!cprops.hasProp(ID2));
	
	// Simple getter
	props.set(ID1, 111);
	CHECK(props.hasProp(ID1));
	CHECK(props.get<int>(ID1) == 111);
	
	// Iterator getter
	int cnt = 0;
	for(PropList::Getter<int> prop(cprops, ID1); prop; prop++)
		cnt++;
	CHECK(cnt == 1);
	cnt = 0;
	for(PropList::Getter<int> prop(cprops, ID2); prop; prop++)
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
	for(PropList::Getter<int> prop(cprops, ID1); prop; prop++)
		cnt++;
	CHECK(cnt == 1);
	cnt = 0;
	for(PropList::Getter<int> prop(cprops, ID2); prop; prop++)
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
		genstruct::Vector<PropList> props;
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
	
	// Identifie find
	{
		AbstractIdentifier *id = AbstractIdentifier::find("otawa::ipet::wcet");
		CHECK(id == & otawa::ipet::WCET);
	}
	
	CHECK_END
	
	return 0;
}
