/***
 *    Description:  Tests functions in global.h
 *
 *        Created:  2020-04-03

 *         Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
 *        License:  MIT License
 */

#include "global.h"

void test_normalize_path()
{
    string p1("//a/./b");
    auto p1fixes = moose::normalizePath(p1);
    cout << p1 << " fixed " << p1fixes << endl;
}

int main(int argc, const char *argv[])
{
    std::cout << "Testing normalize path" << std::endl;
    test_normalize_path();
    return 0;
}

