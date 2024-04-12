// cpstring.h - String class header file

// Copyright 1998-2010 Wind River Systems, Inc.

//
// modification history
// --------------------
// 02a,30mar98,pai  added Wind River coding conventions
// 01a,10jan98,pai  written
//


#ifndef __INCcpstringh
#define __INCcpstringh



// includes

#include "iostream.h"
#include "bool.h"



class String
    {
    private:

        char *   str;
        int      len;

        char *   strcpy (char * target, const char * source) const;
        unsigned strlen (const char * str) const;
        int      strcmp (const char * s, const char * t) const;

    public:

        String  (const char * s);
        String  ();
        String  (const String & st);
       ~String  ();

        int Length () { return len; }


        // overloaded operators

        String & operator=(const String & st); // assignment operator
        String & operator=(const char * s);
        String & operator+=(const String & st);
        String & operator+=(const char * s);

        // friends

        friend Bool operator>(const String & st1, const String & st2);
        friend Bool operator<(const String & st1, const String & st2);
        friend Bool operator==(const String & st1, const String & st2);
        friend ostream & operator<<(ostream & os, const String & st);
        friend istream & operator>>(istream & is, String & st);

    };



#endif  // __INCcpstringh

