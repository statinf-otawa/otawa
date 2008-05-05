/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	TextDecoder class interface
 */
#ifndef OTAWA_PROG_TEXT_DECODER_H
#define OTAWA_PROG_TEXT_DECODER_H

#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// TextDecoder class
class TextDecoder: public Processor {
public:
	static TextDecoder _;
	TextDecoder(void);
	static Identifier<bool> FOLLOW_PATHS;
	virtual void configure(const PropList& props);

protected:
	virtual void processWorkSpace(WorkSpace *fw);
	bool follow_paths;
private:
	const PropList *conf_props;
};

// Features
extern Feature<TextDecoder> DECODED_TEXT;

} // otawa

#endif	// OTAWA_PROG_TEXT_DECODER_H
