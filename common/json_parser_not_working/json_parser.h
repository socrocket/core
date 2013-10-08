#ifndef JSON_PARSER_H
#define JSON_PARSER_H


#include "block_allocator.h"
#include <vector>
#include <stdio.h>
#include <map>
//#include "greencontrol/config.h"


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

struct json_value {
  json_value() : parent(NULL), next_sibling(NULL), first_child(NULL), last_child(NULL) {}
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
    std::string print(json_value* value);
    //void save(std::string path, std::vector<std::string> paramList);
    void save();
private:
	std::vector<char> m_buffer;
	json_value *m_root;
    block_allocator* m_allocator;
//	gs::cnf::cnf_api* mApi;
	json_value *json_parse(char *source, char **error_pos, char **error_desc, int *error_line, block_allocator *allocator);	
	void createParamsList(json_value *value, json_value *parent);
	std::string getValuePath(json_value* temp);
	std::vector<std::pair<std::string, json_value*> > m_jsonValues;
	void read_source(const char *filename);
	void parse(char *source);
    json_value* createJsonTree(json_value* root, std::string jsonPath, std::string value = "0");
};
#endif
