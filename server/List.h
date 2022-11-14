/*
* File: List.h
* Author: Ali
*
* Created on 17 April 2022
*/

#ifndef LIST_H
#define LIST_H

class List {
    private:
	typedef struct node
	{
	    int data;
		int id; 		// treated as index
	    node *next;
	}* nodePtr;

	nodePtr head;
	nodePtr curr;
	nodePtr temp;
	int currId;

    public:	// This is where the functions go
	List();
	void AddFirst(int addData);
	void AddNode(int addData);
	void DeleteNode(int delData);
	int removeLast();
	bool removeFirst();
	bool removeAt(int index);
	int peekLast();
	void PrintList();
	int getSize();
	bool isEmpty();
	void reverse();
	int getHead();
};
#endif
