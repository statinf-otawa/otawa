/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	test/props/test_props.cpp -- OTAWA props module test.
 */

#include <elm/util/test.h>
#include <otawa/properties.h>

using namespace elm;
using namespace otawa;

GenericIdentifier<int> MY_ID("my_id", 666);
GenericIdentifier<CString> MY_STRING("my_string", "ok");

int main(void) {
	
	CHECK_BEGIN("otawa.props")
	PropList props;
	const PropList& cprops = props;
	GenericIdentifier<int> ID1("id1", 0), ID2("id2", 0);
	
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
		MY_STRING(props) += "ohe !";
		cout << props;
	}
	
	CHECK_END
	
	return 0;
}
