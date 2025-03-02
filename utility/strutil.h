/*******************************************************************
 * File:            StringUtil.h
 * Description:     Some common utility functions for strings.
 * Author:          Subhasis Ray
 * E-mail:          ray.subhasis@gmail.com
 * Created:         2007-09-25 12:08:00
 ********************************************************************/
#ifndef _STRINGUTIL_H
#define _STRINGUTIL_H

#ifdef USE_FMT_LIB
#include <fmt/core.h>
#endif

#include <string>
#include <sstream>
#include <vector>

using namespace std;

namespace moose {
/** List of characters considered to be whitespace */
static const char* const DELIMITERS = " \t\r\n";

/** Splits given string into tokens */
void tokenize(const std::string& str, const std::string& delimiters,
              std::vector<std::string>& tokens);

/** trims the leading and trailing white spaces */
std::string trim(const std::string myString,
                 const std::string& delimiters = " \t\r\n");

/** Fix the user-given path whenever possible */
std::string fix(const std::string myString,
                const std::string& delimiters = " \t\r\n");

std::string& clean_type_name(std::string& arg);
bool endswith(const std::string& full, const std::string& ending);

/**
 * @brief Compares the two strings a and b for first n characters, ignoring
 * the case of the characters. Return 0 in case both are same upto first n
 * characters. Othere a non-zero value is returned. When n is smaller or
 * equal to the size of both strings, positive is return if a is larger than
 * b, or negative when a is smaller than b.
 *
 * When n is larger than size of a or b, non-zero values is returned when  a and
 * b are not equal upto min(n, min(a.size(), b.size())) characters.
 *
 * @param a First string.
 * @param b Second string.
 * @param n Compare up n characters/bytes.
 *
 */
int strncasecmp(const std::string& a, const std::string& b, size_t n);

/**
 * Generate random string of given length.
 */
std::string random_string(const unsigned size);

/* ---------------------------------------------------------------------*/
/**
 * @Synopsis  Normalize given path.
 *
 * @Param path
 *
 * @Returns
 */
/* ---------------------------------------------------------------------*/
std::string normalizePath(const std::string& path);

std::pair<std::string, std::string> splitPath(const std::string& path);

// Replace occurance of a substring in a given string.
void str_replace_all(std::string& str, const std::string& a,
                     const std::string& b);

// Is string a prefix of string b.
bool isPrefix(const std::string& a, const std::string& b);

std::string textwrap(const std::string& text, const std::string& prefix = "",
                     const size_t width = 70);

#ifdef USE_FMT_LIB
std::string boxed(const std::string& text, const size_t width = 70);

template <char U = '-'>
std::string underlined(const string& text)
{
    // This is bit clumsy to read.
    return fmt::format(string("{1:^{2}}\n{0:") + U + string("^{2}}\n"), "",
                       text, text.size());
}
#endif

std::string capitalize(const string& f);

void split(const string& text, const string& splitat, vector<string>& res);


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Converts a vector to string.
 *
 * @Param vec
 *
 * @Returns
 */
/* ----------------------------------------------------------------------------*/
template <typename T = double>
std::string vectorToCSV(const std::vector<T>& vec)
{
    std::stringstream ss;
    for(size_t i = 0; i < vec.size(); i++)
        ss << vec[i] << ',';
    auto res = ss.str();
    if(',' == res.back())
        res.pop_back();
    return res;
}

}  // namespace moose

#endif  //_STRINGUTIL_H
