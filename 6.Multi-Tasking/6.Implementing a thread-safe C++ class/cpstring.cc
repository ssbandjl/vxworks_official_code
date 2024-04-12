// cpstring.cc - String class method definitions

// Copyright 1998-2010 Wind River Systems, Inc.

//
// modification history
// --------------------
// 02a,30mar98,pai  added Wind River coding conventions
// 01a,10jan98,pai  written
// 

//
// DESCRIPTION
// This module provides method definitions for a String class.
//
// INCLUDE FILES:  cpstring.h
//
 


// includes

#include "cpstring.h"



////////////////////////////////////////////////////////////////////////////////
// Private methods


////////////////////////////////////////////////////////////////////////////////
//
// String::strcpy - copy one string to another string
//
// This routine provides exactly the same functionality as strcpy() from
// the ANSI standard C library.  The string pointed to by source is copied
// to the storage pointed to by target, including a terminating NUL character.  
//
// RETURNS:  The address of the target string.
//

char * String::strcpy (char * target, const char * source) const
    {
    char * sbase = target;

    while ( *target++ = *source++ )
        ;  // do nothing
    return (sbase);
    }

////////////////////////////////////////////////////////////////////////////////
//
// String::strlen - determine the length of a string
//
// This routine provides exactly that same functionality as strlen() from
// the ANSI standard C library.  The argument, str, should point to a NUL
// terminated string.  This routine will return the number of characters
// in the string pointed to by str, up to, and not including, the NUL
// character.
//
// RETURNS:  The number of characters, not including NUL, in the string
//           pointed to by str.
//
 
unsigned String::strlen (const char * str) const
    {
    unsigned ssize = 0;

    while ( *str++ )
        ++ssize;
    return (ssize);
    }

////////////////////////////////////////////////////////////////////////////////
//
// String::strcmp - compare two strings
//
// This routine provides exactly the same functionality as strcmp() in the
// ANSI standard C library.  Both arguments should point to NUL terminated
// strings.  The strings are compared, character by character.
//
// RETURNS:
//    < 0  if the string, s, is before, t, in the machine sort order.
//      0  if s == t in the machine sort order.
//    > 0  if the string, s, is after, t, in the machine sort order.
//

int String::strcmp (const char * s, const char * t) const
    {
    while ( *s++ == *t++ )
        if ( *(s - 1) == '\0') break;
    return ( *(--s) - *(--t) );
    }


////////////////////////////////////////////////////////////////////////////////
// Public methods


////////////////////////////////////////////////////////////////////////////////
//
// String::String - construct a String from a C-string
//
// This constructor constructs a String object from a NUL terminated C-string.
//
// RETURNS:  N/A
//

String::String (const char * s)
    {
    len = strlen (s);
    str = new char[len + 1];
    strcpy (str, s);
    }

////////////////////////////////////////////////////////////////////////////////
//
// String::String - default constructor
//
// The default constructor for String will construct a String object which
// is initially empty.
//
// RETURNS:  N/A
//

String::String ()
    {
    len = 0;
    str = new char[1];
    str[0] = '\0';
    }

////////////////////////////////////////////////////////////////////////////////
//
// String::String - copy constructor
//
// The copy constructor for String will construct a String object from
// another String object.
//
// RETURNS:  N/A
//

String::String (const String & st)
    {
    len = st.len;
    str = new char[len + 1];
    strcpy (str, st.str);
    }

////////////////////////////////////////////////////////////////////////////////
//
// String::~String - String destructor
//
// The string destructor, when invoked, will delete memory allocated for
// strings.
//
// RETURNS:  N/A
//

String::~String ()
    {
    delete [] str;
    }

////////////////////////////////////////////////////////////////////////////////
//
// String::operator= - assign a String to a String
//
// This assignment operator will assign one String object to another String
// object.
//
// RETURNS:  A reference to the invoking object ( *this ).
//
 
String & String::operator=(const String & st)
    {
    if ( this == &st )
        {
        return *this;
        }

    delete [] str;
    len = st.len;
    str = new char[len + 1];
    strcpy (str, st.str);

    return *this;
    }

////////////////////////////////////////////////////////////////////////////////
//
// String::operator= - assign a C string to a String object
//
// This assignment operator will assign a C string to a String object.
//
// RETURNS:  A reference to the invoking object ( *this ).
//
 
String & String::operator=(const char * s)
    {
    delete [] str;
    len = strlen (s);
    str = new char[len + 1];
    strcpy (str, s);
    
    return *this;
    }

////////////////////////////////////////////////////////////////////////////////
//
// String::operator+= - append a String to a String
//
// This operator will append a String object to a String object.  Given two
// String objects:
//
// String obj1 = "Hello, ";
// String obj2 = "World!";
//
// obj1 += obj2;
//
// will result in the string, "Hello, World!", being stored in obj1. 
//
// RETURNS:  A reference to the invoking object.
//

String & String::operator+=(const String & st)
    {
    char * tempString = new char[len + st.len + 1];

    strcpy (tempString, str);
    strcpy ((tempString + len), st.str);


    // get rid of the old string

    delete [] str;
    len = strlen (tempString);
    str = new char[len + 1];

	
    // copy the temp string to this string, and get rid of the temp string

    strcpy (str, tempString);
    delete [] tempString;

    return *this;
    }

////////////////////////////////////////////////////////////////////////////////
//
// String::operator+= - append a C string to a String
//
// This operator will append a C strintg to a String object.  Given a C string 
// and a String object:
//
// String obj1 = "Hello, ";
// char * obj2 = "World!";
//
// obj1 += obj2;
//
// will result in the string, "Hello, World!", being stored in obj1. 
//
// RETURNS:  A reference to the invoking object.
//

String & String::operator+=(const char * s)
    {
    char * tempString = new char[len + strlen(s) + 1];

    strcpy (tempString, str);
    strcpy ((tempString + len), s);


    // get rid of the old string

    delete [] str;
    len = strlen (tempString);
    str = new char[len + 1];

	
    // copy the temp string to this string, and get rid of the temp string

    strcpy (str, tempString);
    delete [] tempString;

    return *this;
    }


////////////////////////////////////////////////////////////////////////////////
// Friends


////////////////////////////////////////////////////////////////////////////////
//
// operator> - compare two String objects
//
// This operator, a friend method, compares two String objects.
//
// RETURNS:
//    True, if st1 is greater than (follows) st2 in the machine sort order.
//    False, if st1 is not greater than st2 in the machine sort order.
//

Bool operator>(const String & st1, const String & st2)
    {

    if ( strcmp (st1.str, st2.str) > 0 )
        {
        return True;
        }
    else
        {
        return False;
        }
    }

////////////////////////////////////////////////////////////////////////////////
//
// operator< - compare two String objects
//
// This operator, a friend method, compares two String objects.
//
// RETURNS:
//    True, if st1 is less than (precedes) st2 in the machine sort order.
//    False, if st1 is not less than st2 in the machine sort order.
//

Bool operator<(const String & st1, const String & st2)
    {
    if ( strcmp (st1.str, st2.str) < 0 )
        {
        return True;
        }
    else
        {
        return False;
        }
    }

////////////////////////////////////////////////////////////////////////////////
//
// operator== - compare two String objects
//
// This operator, a friend method, compares two String objects.
//
// RETURNS:
//    True, if st1 and st2 are identicle in the machine sort order.
//    False, if st1 and st2 are not identicle in the machine sort order.
//

Bool operator==(const String & st1, const String & st2)
    {
    if ( strcmp (st1.str, st2.str) == 0 )
        {
        return True;
        }
    else
        {
        return False;
        }
    }

////////////////////////////////////////////////////////////////////////////////
//
// operator<< - write a String object to the standard ostream object
//
// This operator, a friend method, inserts a String into an ostream object.
//
// RETURNS:  A reference to an ostream object.
//

ostream & operator<<(ostream & os, const String & st)
    {
    os << st.str;
    return os;
    }

////////////////////////////////////////////////////////////////////////////////
//
// operator>> - read a String object from the standard istream object
//
// This operator, a friend method, extracts a String from an istream object.
// This overloaded extraction operator for the String class is somewhat
// limited; it will only read up to 80 characters from the standard input
// stream.
//
// RETURNS:  A reference to an istream object. 
//

istream & operator>>(istream & is, String & st)
    {
    char temp[80];

    if ( is.getline (temp, 80) && temp[0] != '\0' )
        st = temp;

    // If user entered nothing, may have to reset the
    // failbit if compiler impliments draft standard

    if ( cin.fail () )
        cin.clear ();

    return is;
    }

