// queueT.h - QueueT<T> template class header file

// Copyright 1998-2010 Wind River Systems, Inc.

//
// modification history
// --------------------
// 02a,20mar98,pai  added Wind River coding conventions
// 01a,10jan98,pai  written
//



#ifndef __INCqueueTh
#define __INCqueueTh


// includes

#include "bool.h"



template <class Item>
class QueueT
    {
    protected:

        enum {Q_SIZE = 50};

    private:

        class Node         // Node - nested class declaration
            {
            public:

            Item   item;
            Node * next;
            Node (const Item & i) : item (i), next (0) {}
            };
	

        Node *    head;
        Node *    tail;
        int       items;   // current number of queue items
        const int qsize;   // maximum number of queue items

		
        // prevent the compiler from generating
        // copy constructor and assignment operator

        QueueT (const QueueT & q) : qsize (0) {}
        QueueT & operator=(const QueueT & q) { return *this; }


    public:

        virtual ~QueueT ();
        QueueT (int qs = Q_SIZE);

        Bool   emptyQueue () const { return (items == 0) ? True : False; }
        Bool   fullQueue  () const { return (items == qsize) ? True : False; }
        int    countQueue () const { return items; }
        Bool   enQueue    (const Item & item);
        Bool   deQueue    (Item & item);
    };
		

// method templates
////////////////////////////////////////////////////////////////////////////////

#include "queueT.cc"


#endif // __INCqueueTh


