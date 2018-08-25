/***
 *       Filename:  Parser.h
 *
 *    Description:  Parser class. Similar API as muParser.
 *
 *         Author:  Dilawar Singh <dilawars@ncbs.res.in>
 *   Organization:  NCBS Bangalore
 *
 *        License:  GNU GPL3
 */

#ifndef PARSER_H
#define PARSER_H

#include "../external/tinyexpr/tinyexpr.h"

#include <string>
#include <exception>
#include <map>
#include <iostream>

using namespace std;

namespace moose {
    namespace Parser {

        struct ParserException : public std::exception 
        {
            ParserException( const string msg ) : msg_(msg) { ; }

            string GetMsg()
            {
                return msg_;
            }

            string msg_;
        };

        typedef ParserException exception_type;
        typedef double value_type;
        typedef map<string, value_type> varmap_type;
    }

class MooseParser
{
    public:
        MooseParser();
        ~MooseParser();

        void DefineVar( const char* varName, moose::Parser::value_type* val);

        void DefineVar( const string& varName, moose::Parser::value_type& val);

        void DefineFun( const char* funcName, moose::Parser::value_type (&func)(moose::Parser::value_type) );

        void SetExpr( const string& expr );

        moose::Parser::value_type Eval( ) const;

        Parser::varmap_type GetVar() const;

        void SetVarFactory( const char* varName, void* data );

        void DefineConst( const string& constName, moose::Parser::value_type& value );

        void DefineConst( const char* constName, const moose::Parser::value_type& value );

        moose::Parser::value_type Diff( const moose::Parser::value_type a, const moose::Parser::value_type b) const;

        Parser::varmap_type GetConst( ) const;
        Parser::varmap_type GetUsedVar( );
        void ClearVar( );
        const string GetExpr( ) const;
        void SetVarFactory( double* (*fn)(const char*, void*), void *);


    private:
        /* data */
        string expr_;
        moose::Parser::value_type value=0.0;
        Parser::varmap_type var_map_;
        Parser::varmap_type const_map_;
        Parser::varmap_type used_vars_;

        /* tiny expr */
};

} // namespace moose.

#endif /* end of include guard: PARSER_H */
