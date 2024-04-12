// sharedQT.h - sharedQT<T> template class header

// Copyright 1998-2010 Wind River Systems, Inc.

//
// modification history
// --------------------
// 02a,30mar98,pai  added Wind River coding conventions
// 01a,24mar98,pai  written
//



#ifndef __INCsharedQTh
#define __INCsharedQTh



// includes

#include "semLib.h"
#include "queueT.h"



template <class Item>
class SharedQT : public QueueT <Item>
    {
    private:

        SEM_ID  sharedQMutx;        // Access guard

    public:

       ~SharedQT ();
        SharedQT (int qs = Q_SIZE);

        Bool sharedEnQ     (const Item & item);
        Bool sharedDeQ     (Item & item);
        Bool sharedEmptyQ  () const;
        Bool sharedFullQ   () const;
        int  sharedCountQ  () const;
    };


// method templates
////////////////////////////////////////////////////////////////////////////////

#include "sharedQT.cc"



#endif // __INCsharedQTh

