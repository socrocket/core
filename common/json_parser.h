// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file json_parser.h
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Timo Veit
///

#ifndef COMMON_JSON_PARSER_H_
#define COMMON_JSON_PARSER_H_

#include <stdio.h>
#include <greencontrol/config.h>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include "common/block_allocator.h"

enum json_type {
  JSON_NULL,
  JSON_OBJECT,
  JSON_ARRAY,
  JSON_STRING,
  JSON_INT,
  JSON_FLOAT,
  JSON_BOOL,
};

struct json_value {
  json_value *parent;
  json_value *next_sibling;
  json_value *first_child;
  json_value *last_child;

  char *name;
  union {
    char *string_value;
    int int_value;
    float float_value;
  };
  json_type type;
};

class json_parser {
  public:
    json_parser();
    ~json_parser();
    void config(const char *filename);
    void save(std::string path, gs::cnf::cnf_api *api);
  private:
    std::vector<char> m_buffer;
    json_value *m_root;
    gs::cnf::cnf_api *mApi;
    json_value*json_parse(char *source, char **error_pos, char **error_desc, int *error_line,
    block_allocator *allocator);
    void createParamsList(json_value *value, json_value *parent);
    std::string getValuePath(json_value *temp);
    std::vector<std::pair<std::string, json_value *> > m_jsonValues;
    void read_source(const char *filename);
    void parse(char *source);
};

#endif  // COMMON_JSON_PARSER_H_
/// @}
