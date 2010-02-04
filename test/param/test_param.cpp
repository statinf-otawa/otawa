/*
 * test_param.cpp
 *
 *  Created on: 3 févr. 2010
 *      Author: casse
 */

#include <otawa/app/Application.h>
#include <otawa/proc/ParamProcessor.h>
#include <otawa/proc/ParamFeature.h>

using namespace elm;
using namespace otawa;

// declarations
Identifier<int> PARAM1("PARAM1", 0);
Identifier<string> PARAM2("PARAM2", "");

class MyProcessor: public Processor {
public:
	MyProcessor(cstring name, Version version, const ActualFeature& feature);

protected:
	virtual void processFrameWork(WorkSpace *fw);
private:
	const ActualFeature& feat;
	Identifier<string>& d;
};

ParamProcessor MY_PROCESSOR("MY_PROCESSOR", Version(1, 0, 0), new ParamProcessor::Maker<MyProcessor>());

ParamFeature MY_FEATURE("MY_FEATURE", MY_PROCESSOR);

ParamIdentifier<string> DATA1(MY_FEATURE, "DATA1");


// usage
MyProcessor::MyProcessor(cstring name, Version version, const ActualFeature& feature)
	: feat(feature), d(DATA1[feature]) { }


void MyProcessor::processFrameWork(WorkSpace *ws) {
	StringBuffer buf;
	for(int i = 0; i < PARAM1(feat); i++)
		buf << PARAM2(feat);
	d(ws) = buf.toString();
}


// test
class TestParam: public Application {
public:
	TestParam(void): Application("test_param", Version(1, 0, 0)) { }

protected:

	virtual void work (const string &entry, PropList &props) throw (elm::Exception) {

		PropList fprops;
		PARAM2(fprops) = "hello ";
		PARAM1(fprops) = 3;
		ActualFeature *feature = MY_FEATURE.instantiate(fprops);
		require(*feature);
		Identifier<string>& d = DATA1[feature];
		cout << "RESULT = " << d(workspace()) << io::endl;

		PropList fprops2;
		PARAM2(fprops2) = "bye ";
		PARAM1(fprops2) = 4;
		ActualFeature *feature2 = MY_FEATURE.instantiate(fprops2);
		require(*feature2);
		Identifier<string>& d2 = DATA1[feature2];
		cout << "RESULT = " << d2(workspace()) << io::endl;

		PropList fprops3;
		PARAM2(fprops3) = "hello ";
		PARAM1(fprops3) = 3;
		ActualFeature *feature3 = MY_FEATURE.instantiate(fprops3);
		cout << (void *)feature << " = " << (void *)feature3 << io::endl;
	}
};

int main(int argc, char **argv) {
	TestParam app;
	return app.run(argc, argv);
}
