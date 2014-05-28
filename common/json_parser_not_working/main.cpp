// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file main.cpp
/// 
///
/// @date 2013-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author 
///
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
/// @}