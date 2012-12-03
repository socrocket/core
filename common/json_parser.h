// ********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the
// European Space Agency or of TU-Braunschweig.
// ********************************************************************
// Title:      json_parser.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Timo Veit
// Reviewed:
// ********************************************************************

#ifndef JSON_PARSER_H
#define JSON_PARSER_H


#include "block_allocator.h"
#include <vector>
#include <stdio.h>
#include <map>
#include "greencontrol/config.h"


enum json_type
{
        JSON_NULL,
        JSON_OBJECT,
        JSON_ARRAY,
        JSON_STRING,
        JSON_INT,
        JSON_FLOAT,
        JSON_BOOL,
};
struct json_value
{
			json_value *parent;
			json_value *next_sibling;
			json_value *first_child;
			json_value *last_child;


			char *name;
			union
			{
					char *string_value;
					int int_value;
					float float_value;
			};


			json_type type;
};

class json_parser
{
public:
	json_parser();
	~json_parser();
	void config(const char *filename);
private:
	std::vector<char> m_buffer;
	json_value *m_root;
	gs::cnf::cnf_api* mApi;
	json_value *json_parse(char *source, char **error_pos, char **error_desc, int *error_line, block_allocator *allocator);	
	void createParamsList(json_value *value, json_value *parent);
	std::string getValuePath(json_value* temp);
	std::vector<std::pair<std::string, json_value*> > m_jsonValues;
	void read_source(const char *filename);
	void parse(char *source);
};
#endif
