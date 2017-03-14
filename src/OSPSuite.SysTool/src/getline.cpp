


#ifndef SAFE_STL

#include <string>
#ifdef USE_DOT_H
    #include <iostream.h>
#else
    #include <iostream>
    using namespace std;
#endif


#if !defined( __BORLANDC__ ) || __BORLANDC__ < 0x0530


// The recent compilers don't seem to have this working correctly.
// So here's my own. Use this on Borland 5.0 and Visual.
// You can remove these lines of code when the compilers catch up.
istream & getline( istream & in, string & str, char delim )
{
    char ch;
    str = "";     // empty string, will build one char at-a-time
    
    while( in.get( ch ) && ch != delim )
        str += ch;
    
    return in;
}

istream & getline( istream & in, string & str )
{
    return getline( in, str, '\n' );
}

#endif

#endif
