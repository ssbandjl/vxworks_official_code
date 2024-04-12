/* privateCode.h - header file for making data writable from routine only */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.    
01a,???  written.
*/

#define MAX 1024

typedef struct myData
	{
	char stuff[MAX];
	int moreStuff;
	} MY_DATA;

