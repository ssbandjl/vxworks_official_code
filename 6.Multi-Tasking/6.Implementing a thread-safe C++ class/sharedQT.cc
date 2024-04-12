// sharedQT.cc - SharedQT<T> class method templates

// Copyright 1998-2010 Wind River Systems, Inc.

//
// modification history
// --------------------
// 02a,30mar98,pai  added Wind River coding conventions
// 01a,24mar98,pai  written
//

//
// DESCRIPTION
// This class provides the following public methods:
//
//            -sharedEnQ
//            -sharedDeQ
//            -sharedEmptyQ
//            -sharedFullQ
//            -sharedCountQ
//
// This is a very basic shared queue template.  The type stored in the queue
// and the maximum number of elements to be stored in the queue are specified
// when the object is constructed.  For example:
//
//     SharedQT<int> myQ (10);
//
// will construct a queue which can hold up to 10 integers.  If a queue size
// is not specified during construction, the queue will use a default size of
// 50 items.
//
// This class is essentially a very thin wrapper around the QueueT<T> class.
// As the class name implies, the intent behind the sharedQT<T> class is to
// provide a thread safe container.  QueueT<T> does not provide a mutual
// exclusion mechanism.  SharedQT<T> attempts to make QueueT<T> thread safe
// by guarding calls to QueueT<T> methods with a mutex semaphore.
//
// It is assumed that multiple tasks will be trying to read from and write to
// a SharedQT<T> object.  A single mutex semaphore insures that when one task
// is in the middle of some queue operation, another task will not preempt the
// operation at an inappropriate moment.
//
// SharedQT<T>::SharedQT initializes the object's mutex semaphore using
// SEM_Q_PRIORITY and SEM_DELETE_SAFE options.  Pended task will be put on the
// ready queue on the basis of priority, and a task which owns the semaphore
// will be safe from unexpected deletion.  The SEM_DELETE_SAFE enables an
// implicit taskSafe() for each semTask(), and an implicit taskUnsafe() for
// each semGive().
//
// INCLUDE FILES:  sharedQT.h
//



////////////////////////////////////////////////////////////////////////////////
//
// SharedQT<T>::SharedQT - construct a SharedQT<T> object
//
// There is but one constructor in this class.  After the base class
// constructor is called, this constructor will initialize the mutex semaphore.
//
// If an argument, qs, is supplied to this constructor, the queue will be
// constructed to hold up to 'qs' number of elements.  If no argument is
// supplied to the constructor, the queue will, by default, allow up to 50
// items to be stored in the queue.
//
// RETURNS:  N/A
//

template <class Item>
SharedQT<Item>::SharedQT (int qs) : QueueT<Item> (qs)
    { 
    sharedQMutx = semMCreate (SEM_Q_PRIORITY | SEM_DELETE_SAFE); 
    }


////////////////////////////////////////////////////////////////////////////////
//
// SharedQT<T>::~SharedQT - destroy a shared queue object
//
// This destructor deletes the mutex semaphore.  The base class destructor
// will delete all of the memory allocated for queue items.
//
// RETURNS:  N/A
//

template <class Item>
SharedQT<Item>::~SharedQT ()
    {
    // It is a good idea to own a semaphore you intend to delete

    semTake (sharedQMutx, WAIT_FOREVER);
    
    // QueueT<T>::~QueueT is virtual; it will take care of
    // deleting memory allocated to hold items in the queue.

    semDelete (sharedQMutx);
    }


////////////////////////////////////////////////////////////////////////////////
//
// SharedQT<T>::sharedEnQ - add an item to a shared queue
//
// This method will attempt to take an internal semaphore before adding
// an item to the queue.  The argument passed to this method should be
// a reference to the type specified when the shared queue was constructed.
//
// RETURNS:  True, if the item was added to the queue.  False is returned
//           to indicate that the item could not be added to the queue.
//

template <class Item>
Bool SharedQT<Item>::sharedEnQ (const Item & item)
    {
    Bool status;

    //// GUARD ////
    semTake (sharedQMutx, WAIT_FOREVER);
    ///////////////
    
    status = enQueue (item);

    semGive (sharedQMutx);
    return status;
    }


////////////////////////////////////////////////////////////////////////////////
//
// SharedQT<T>::sharedDeQ - remove an item from a shared queue
//
// This method will remove an item from a shared queue and delete the memory
// allocated to store the item.  The dequeued item is placed in the argument
// for this method.  The method will not attempt to remove an item until it
// has taken an internal semaphore.
//
// RETURNS:  True, if the item was dequeued.  False is returned to indicate
//           that an item could not be dequeued.
// 

template <class Item>
Bool SharedQT<Item>::sharedDeQ (Item & item)
    {
    Bool status;

    //// GUARD ////
    semTake (sharedQMutx, WAIT_FOREVER);
    ///////////////
    
    status = deQueue (item);

    semGive (sharedQMutx);
    return status;
    }


////////////////////////////////////////////////////////////////////////////////
//
// SharedQT<T>::sharedEmptyQ - test for an empty shared queue
//
// This method tests a shared queue for an empty state.
//
// RETURNS:  True, if the shared queue is empty.  False is returned to
//           indicate that the queue is not empty.
//

template <class Item>
Bool SharedQT<Item>::sharedEmptyQ () const
    {
    Bool status;

    //// GUARD ////
    semTake (sharedQMutx, WAIT_FOREVER);
    ///////////////

    status = emptyQueue ();

    semGive (sharedQMutx);
    return status;
    }


////////////////////////////////////////////////////////////////////////////////
//
// SharedQT<T>::sharedFullQ - test for a full shared queue
//
// This method tests for a full queue. 
//
// RETURNS:  True, if the queue is full.  False is returned to indicate that
//           the queue is not full.
//

template <class Item>
Bool SharedQT<Item>::sharedFullQ () const
    {
    Bool status;

    //// GUARD ////
    semTake (sharedQMutx, WAIT_FOREVER);
    ///////////////

    status = fullQueue ();

    semGive (sharedQMutx);
    return status;
    }


////////////////////////////////////////////////////////////////////////////////
//
// SharedQT<T>::sharedCountQ - return the number of items in a shared queue
//
// This method will return the number of items currently stored in a queue.
//
// RETURNS:  The number of elements in the queue.
//

template <class Item>
int SharedQT<Item>::sharedCountQ () const
    {
    int sharedCount;


    //// GUARD ////
    semTake (sharedQMutx, WAIT_FOREVER);
    ///////////////

    sharedCount = countQueue ();
    
    semGive (sharedQMutx);
    return sharedCount;
    }


