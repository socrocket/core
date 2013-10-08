#include "json_parser.h"

int main(int argc, char **argv)
{
	json_parser* json_parser1 = new json_parser();
	json_parser1->config("leon3mp.singlecore.json");
    std::vector<std::string> list;
    json_parser1->save();
	//getchar();
    return 0;
}
