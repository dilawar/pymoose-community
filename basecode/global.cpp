/*
 * ==============================================================================
 *
 *       Filename:  global.cpp
 *
 *    Description:  Some global declarations.
 *
 *        Version:  1.0
 *        Created:  Tuesday 29 April 2014 10:18:35  IST
 *       Revision:  0.1
 *       Compiler:  gcc/g++
 *
 *         Author:  Dilawar Singh
 *   Organization:  Bhalla's lab, NCBS Bangalore
 *
 * ==============================================================================
 */

#include "global.h"
#include <numeric>
#include <regex>

#include <sys/stat.h>
#include <sys/types.h>

/*-----------------------------------------------------------------------------
 *  This variable keep track of how many tests have been performed.
 *
 *  Check header.h for macros tbegin and tend which uses it.
 *-----------------------------------------------------------------------------*/
unsigned int totalTests = 0;

stringstream errorSS;

clock_t simClock = clock();

namespace moose
{


map<string, valarray<double>> solverProfMap = {{"Ksolve", {0.0, 0}},
    {"HSolve", {0.0, 0}}
};


/* Check if path is OK */
int checkPath(const string& path)
{
    if (path.size() < 1)
        return EMPTY_PATH;

    if (path.find_first_of(" \\!") != std::string::npos)
        return BAD_CHARACTER_IN_PATH;

    if (path[path.size() - 1] != ']')
    {
        return MISSING_BRACKET_AT_END;
    }
    return 0;
}

/* Join paths */
string joinPath(string& pathA, string& pathB)
{
    pathA = fixPath(pathA);
    string newPath = pathA + "/" + pathB;
    return fixPath(newPath);
}

/* Fix given path */
string fixPath(const string& path)
{
    string x(path);
    int pathOk = moose::checkPath(x);
    if (pathOk == 0)
        return x;
    else if (pathOk == MISSING_BRACKET_AT_END)
        return x + "[0]";
    return x;
}

// MOOSE suffixes [0] to all elements to path. Remove [0] with null
// character whenever possible. For n > 0, [n] should not be touched. Its
// the user job to take the pain and write the correct path.
string createMOOSEPath(const string& path)
{
    string s = path; /* Local copy */
    // Remove [0] from paths. They will be annoying for normal users.
    std::string::size_type n = 0;
    string zeroIndex("[0]");
    while ((n = s.find(zeroIndex, n)) != std::string::npos)
        s.erase(n, zeroIndex.size());
    return s;
}

/**
 * @brief Create directories recursively needed to open the given file p.
 *
 * @param path When successfully created, returns created path, else
 * convert path to a filename by replacing '/' by '_'.
 */
bool createParentDirs(const string& path)
{
    // Remove the filename from the given path so we only have the
    // directory.
    string p = path;
    bool failed = false;
    size_t pos = p.find_last_of('/');
    if (pos != std::string::npos)
        p = p.substr(0, pos);
    else /* no parent directory to create */
        return true;

    if (p.size() == 0)
        return true;

    string command("mkdir -p ");
    command += p;
    int ret = system(command.c_str());
    struct stat info;
    if (stat(p.c_str(), &info) != 0)
    {
        LOG(moose::warning, "cannot access " << p);
        return false;
    }
    else if (info.st_mode & S_IFDIR)
    {
        LOG(moose::info, "Created directory " << p);
        return true;
    }
    else
    {
        LOG(moose::warning, p << " is no directory");
        return false;
    }
    return true;
}

/*  Flatten a dir-name to return a filename which can be created in pwd . */
string toFilename(const string& path)
{
    string p = path;
    std::replace(p.begin(), p.end(), '/', '_');
    std::replace(p.begin(), p.end(), '\\', '_');
    return p;
}

/*  return extension of a filename */
string getExtension(const string& path, bool without_dot)
{
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos)
        return "";

    if (without_dot)
        return path.substr(dotPos + 1);

    return path.substr(dotPos);
}

/*  returns `basename path`  */
string pathToName(const string& path)
{
    return path.substr(path.find_last_of('/'));
}

/*  /a[0]/b[1]/c[0] -> /a/b/c  */
string moosePathToUserPath(string path)
{
    // Just write the moose path. Things becomes messy when indexing is
    // used.
    return createMOOSEPath(path);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Generate a suitable column name based on given path. Replace
 * [\d+] with \d+; and remove [\0+]. Replace / with given delim (default '.')
 *
 * @Param path (string).
 * @Param delim (char)
 * @Param maxLevel  (int). Max number of parents to include. Default 1.
 *
 * @Returns  Reformatted path suitable for column name.
 *
 * Example:
 * -------
 *      /a[12]/d[0]/e[1]/d[0] -> e1.d
 *      /a[12]/d[0] -> e12.d
 *      /a[00]/b[0] -> a.b
 */
/* ----------------------------------------------------------------------------*/
string moosePathToColumnName(const string& path, char delim, size_t maxParents)
{
    string s(path);
    static std::regex e0("\\[(0+)\\]");    // Remove [0+]
    static std::regex e1("\\[(\\d+)\\]");  // Replace [xyz] by xyz
    s = std::regex_replace(s, e0, "");
    s = std::regex_replace(s, e1, "$1");

    string s2;
    string colname = "";
    size_t nBreak = 0;

    // Keep as many parents as required by maxParents. Using reverse magic.
    for (auto rit = s.rbegin(); rit != s.rend(); rit++)
    {
        if (*rit == '/')
        {
            colname = s2 + delim + colname;
            s2 = "";
            nBreak += 1;
            if (nBreak == (1 + maxParents))
                break;
        }
        else
            s2 = *rit + s2;
    }
    colname.pop_back();
    return colname;
}


/*  Return formatted string
 *  Precision is upto 17 decimal points.
 */
string toString(double x)
{
    char buffer[50];
    sprintf(buffer, "%.17g", x);
    return string(buffer);
}


void addSolverProf(const string& name, double time, size_t steps)
{
    solverProfMap[name] =
        solverProfMap[name] + valarray<double>({time, (double)steps});
}

void printSolverProfMap()
{
    for (auto& v : solverProfMap)
        cout << '\t' << v.first << ": " << v.second[0] << " sec ("
             << v.second[1] << ")" << endl;
}


}
