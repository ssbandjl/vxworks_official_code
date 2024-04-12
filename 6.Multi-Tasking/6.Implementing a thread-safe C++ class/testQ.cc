// testQ.cc - Test the shared queue template class

// Copyright 1998-2010 Wind River Systems, Inc.

//
// modification history
// --------------------
// 01a,25mar98,pai  written
//

//
// DESCRIPTION
//
// This program demonstrates using a thread safe C++ class called
// SharedQT<T>.  Reader and writer tasks, running at the same priority,
// will be started to do reading and writing on the same queue object.
//
// NOTES:  The project files are:
//
//     bool.h
//     cpstring.h    cpstring.cc
//     queueT.h      queueT.cc
//     sharedQT.h    sharedQT.cc
//     testQ.h       testQ.cc
//
// Use the project Makefile to build.  From the command line:
//
// % make tQue.out
//
// Load tQue.out from WindSh, and start the test program:
//
// --> ld < tQue.out
// --> sp testQ
//
// The output will be displayed on the system console.  One can change
// the project name by editing the PROJECT macro in the Makefile.  The
// target CPU type and debug options can also be easily modified in the
// Makefile using the CPU and DEBUG_OPT macros.
//
// INCLUDE FILES:  testQ.h
//



// Includes

#include "testQ.h"



// Defines

#define  WAVE_LENGTH  20



// Globals

const char * gWaves [WAVE_LENGTH] =
    {
    "     ///     ///     ///     ///     ///",
    "  /////   /////   /////   /////   /////",
    " /////   /////   /////   /////   /////",
    " /////   /////   /////   /////   /////",
    "//////  //////  //////  //////  //////",
    "//////  //////  //////  //////  //////",
    " /////   /////   /////   /////   /////",
    " /////   /////   /////   /////   /////",
    "  /////   /////   /////   /////   /////",
    "   ////    ////    ////    ////    ////",
    "    ////    ////    ////    ////    ////",
    "     ////    ////    ////    ////    ////",
    "      ////    ////    ////    ////    ////",
    "       ////    ////    ////    ////    ////",
    "        ///     ///     ///     ///     ///",
    "        ///     ///     ///     ///     ///",
    "         //      //      //      //      //",
    "         //      //      //      //      //",
    "         //      //      //      //      //",
    "        //      //      //      //      //"
    };




////////////////////////////////////////////////////////////////////////////////
//
// testQ - construct a shared queue, start reader and writer tasks
//
// testQ is the entry point to this program.  This routine does three
// things of interest to the queue test:
//
// 1.  construct a shared queue object which is large enough to hold
//     WAVE_LENGTH size elements.
//
// 2.  start a reader task and a writer task.  Each task is passed the
//     address of the queue object constructed in this routine.  As the
//     task names imply, the reader task will continuously read from the
//     queue object; the writer task will continuously write to the queue
//     object.
//
// 3.  testQ lowers its own priority and then ... just hangs out in the
//     ready queue.  This is important.  If testQ were to return, the shared
//     queue object, gStringQ, would be destroyed (it is local to testQ).
//     The reader and writer tasks would be subsequently left reading and
//     writing using some completely invalid object.
//
//     testQ will not return.  Because testQ lowers its own priority to 255,
//     it will be blocked by higher priority tasks.  Be aware that testQ is
//     not deletion safe if started from WindSh using the sp() primitive.
//
//     Internally, SharedQT<T> uses a mutex semaphore with the SEM_DELETE_SAFE
//     option.  As a result, when the reader or writer tasks are in possession
//     of the semaphore, they will be safe from deletion.  When the reader
//     or writer tasks release the semaphore, they will be unsafe from
//     deletion.
//
// RETURNS:  Never
//

void testQ (void)
    {
    // construct to hold just enough elements
    
    static SharedQT<String> gStringQ (WAVE_LENGTH);


    taskSpawn ((char *) READER_NAME, READER_PRI, VX_FP_TASK, READER_STACK,
               (FUNCPTR) StringReader, (int) &gStringQ, 0,0,0,0,0,0,0,0,0);

    taskSpawn ((char *) WRITER_NAME, WRITER_PRI, VX_FP_TASK, WRITER_STACK,
               (FUNCPTR) StringWriter, (int) &gStringQ, 0,0,0,0,0,0,0,0,0);

    taskPrioritySet (taskIdSelf (), 255);
    }


////////////////////////////////////////////////////////////////////////////////
//
// StringReader - continuously reads from the shared queue
//
// StringReader continuously cycles trying to pull items off of the shared
// queue, and then printing the item.
//
// RETURNS:  Never
//

void StringReader (SharedQT<String> * string)
    {
    
    String inComing;
    String notify ("Read value: ");


    FOREVER
        {
        if ( string->sharedDeQ (inComing) )
            {
            cout << inComing << endl;
            }
        else if ( string->sharedEmptyQ () )
            {
            cout << notify << "NONE" << endl; 

            ////////////////////////////////////////////////////////////////////
            // The queue is empty.  So, give up the CPU so the writer will have
            // an opportunity to run; a writer is running at the same priority.

            taskDelay (0);
            }  
        }

    }


////////////////////////////////////////////////////////////////////////////////
//
// StringWriter - continuously writes to the shared queue
//
// StringWriter continuously cycles through the global gWaves array and
// puts gWaves elements on the shared queue.
//
// RETURNS:  Never
//

void StringWriter (SharedQT<String> * string)
    {

    static int idx      = 0;
    String     outGoing = gWaves[idx];


    FOREVER
        {
        if ( string->sharedEnQ (outGoing) )
            {
            ////////////////////////////////////////////////////////////////////
            // The queue preserves the order of elements we put into it, but it
            // will not preserve the order of this index (idx).  We only want to
            // increment our index into gWaves if we've actually succeeded at
            // putting the current gWaves[idx] item in the queue.

            if ( (++idx) == WAVE_LENGTH )
                idx = 0;
            
            outGoing = gWaves[idx];
            }
        else
            {
            ////////////////////////////////////////////////////////////////////
            // The queue is full.  So, give up the CPU so the reader will have
            // an opportunity to run; a reader is running at the same priority.

            taskDelay (0); 
            }
        }
    
    }

