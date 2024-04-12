// queueT.cc - QueueT<T> template class method templates

// Copyright 1998-2010 Wind River Systems, Inc.

//
// modification history
// --------------------
// 02a,20mar98,pai  added Wind River coding conventions
// 01a,10jan98,pai  written
//

//
// DESCRIPTION
// This module contains the method template for the QueueT<T> template
// class.  QueueT<T> provides the following public methods: 
//
//           enQueue    - add an item to the queue.
//           deQueue    - remove an item from the queue.
//           emptyQueue - check for an empty queue.
//           fullQueue  - check for a full queue.
//           countQueue - check the number of elements in the queue.
//
// This is a very basic queue template.  The type stored in the queue,
// and the maximum number of elements to be stored in the queue are
// specified when the object is constructed.  For example:
//
//           QueueT<int> myQ (10);
//
// will construct a queue which can hold up to 10 integers.  If a queue
// size is not specified during construction, the queue will use a
// default size of 50 items.
//
// The copy constructor and assignment operator for QueueT<T> do not
// perform a deep copy.
//
// INCLUDE FILES:  queueT.h
//



////////////////////////////////////////////////////////////////////////////////
//
// QueueT<T>::QueueT - construct a QueueT<T> object of type T
//
// This constructor is also the default constructor for class QueueT<T>.
// An optional argument, qs, may be supplied to specify the maximum number
// of elements to hold in the queue.  If an argument is not supplied to the
// constructor, a QueueT<T> object will, by default, hold a maximum of 50
// elements.
//
// RETURNS:  N/A
//

template <class Item> inline
QueueT<Item>::QueueT (int qs) : qsize (qs)
    {
    head = tail = 0;
    items = 0;
    }

////////////////////////////////////////////////////////////////////////////////
//
// QueueT<T>::~QueueT - destroy a QueueT<T> object
//
// When QueueT<T> constructor is invoked, it will delete all of the memory
// allocated to store objects which have be placed in the queue.
//
// RETURNS:  N/A
//

template <class Item>
QueueT<Item>::~QueueT ()
    {
    Node * temp;
	
    while ( head != 0 )    // while queue not empty
        {
        temp = head;
        head = head->next;
        delete temp;
        }
    }

////////////////////////////////////////////////////////////////////////////////
//
// QueueT<T>::enQueue - add an item to a queue
//
// This method adds an item to a QueueT<T> object.  The argument, item,
// should be a reference to the type specified when the QueueT<T> object
// was constructed.
//
// RETURNS:  True, if the item was added to the queue.  Else, this method
//           returns False to indicate that the item could not be added to
//           the queue.
//

template <class Item>
Bool QueueT<Item>::enQueue (const Item & item)
    {
    if ( fullQueue() )
        {
        return False;
        }

    Node * add = new Node (item);
	
    if ( add == 0 )
        { 
        return False;
        }

    ++items;

    if ( head == 0 )
        head = add;
    else
        tail->next = add;

    tail = add;

    return True;
    }

////////////////////////////////////////////////////////////////////////////////
//
// QueueT<T>::deQueue - remove an item from a queue 
//
// This method removes an item to a QueueT<T> object and deletes the memory
// which was allocated to hold the item.   The argument, item, should be a
// reference to the type specified when the QueueT<T> object was constructed.
//
// RETURNS:  True, if the item was removed from the queue.  Else, this method
//           returns False to indicate that the item could not be removed from 
//           the queue.
//

template <class Item>
Bool QueueT<Item>::deQueue (Item & item)
    {
    if ( head == 0 )
        {
        return False;
        }
	
    item = head->item;
    --items;
    Node * temp = head;
    head = head->next;
    delete temp;

    if ( items == 0 )
        {
        tail = 0;
        }

    return True;
    }


