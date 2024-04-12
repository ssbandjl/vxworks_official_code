// testQ.h - testQ.cc header file

// Copyright 1998-2010 Wind River Systems, Inc.

//
// modification history
// --------------------
// 01a,25mar98,pai  written
//


#ifndef __INCtestQh
#define __INCtestQh



// Includes

#include   "cpstring.h"
#include   "sharedQT.h"
#include   "taskLib.h"



// Defines

#define    READER_NAME    "tQueRead"      // Reader task name
#define    WRITER_NAME    "tQueWrite"     // Writer task name
#define    READER_PRI     150             // Reader task priority
#define    WRITER_PRI     150             // Writer task priority
#define    READER_STACK   30000           // Reader task stack size
#define    WRITER_STACK   30000           // Writer task stack size


// Forward declarations

void testQ        (void);                 // Entry point to the queue test
void StringReader (SharedQT<String> *);   // Entry point to reader task
void StringWriter (SharedQT<String> *);   // Entry point to writer task



#endif // __INCtestQh

