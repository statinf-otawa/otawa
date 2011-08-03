/*
 *	$Id$
 *	Interface to the EGDisplay class.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#ifndef _EGDISPLAY_H
#define _EGDISPLAY_H_

#include "ExecutionGraph.h"

namespace otawa { namespace exegraph2 {

class EGDisplay {

public:
	EGDisplay(ExecutionGraph *graph, elm::io::Output& out_file,const string& info = ""){}
	EGDisplay(ExecutionGraph *graph, const Path& out_file_path,const string& info = ""){}
};

class EGGenericDotDisplay : public EGDisplay {
public:
	EGGenericDotDisplay(ExecutionGraph *graph, elm::io::Output& out_file, const string& info = "");
	EGGenericDotDisplay(ExecutionGraph *graph, const Path& out_file_path, const string& info = "");
};

class EGDisplayFactory {
public:
	 virtual void newEGDisplay(ExecutionGraph *graph, elm::io::Output& out_file, const string& info = "") = 0;
	 virtual void newEGDisplay(ExecutionGraph *graph, const Path& out_file_path, const string& info = "") = 0;
};

class EGGenericDotDisplayFactory : public EGDisplayFactory {
public:
	void newEGDisplay(ExecutionGraph *graph, elm::io::Output& out_file, const string& info = ""){
		new EGGenericDotDisplay(graph, out_file, info);
	}
	void newEGDisplay(ExecutionGraph *graph, const Path& out_file_path, const string& info = ""){
			new EGGenericDotDisplay(graph, out_file_path, info);
		}
};

} // namespace exegraph2
} // namespace otawa
#endif // _EGDISPLAYER_H_



