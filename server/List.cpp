#include "List.h"
#include <cstdlib>
#include <iostream>

/*
	Implementation of singly linked list
*/

using namespace std;

List::List()
{
	head = NULL;
	curr = NULL;
	temp = NULL;
	currId = 0;
}

void List::AddNode(int addData)
{
	nodePtr n = new node;
	n->next = NULL;
	n->data = addData;
	n->id = ++currId;

	if (head != NULL)
	{
		curr = head;
		while (curr->next != NULL) {
			curr = curr->next;
		}
		curr->next = n;
	} else {
		head = n;
	}
}

void List::DeleteNode(int delData)
{
	nodePtr delPtr = NULL; // nodePtr is naturally nodePtr* as defined in List.h
	temp = head;
	curr = head;

	while (curr != NULL && curr -> data != delData) // keep temp 1 iteration behind curr
	{
		temp = curr;
		curr = curr->next;
	}
	if (curr != NULL)
	{
		cout << delData << " was not found in the List" << std::endl;
	} else {
		delPtr = curr;
		curr = curr->next;
		temp->next = curr;
		delete delPtr;
		cout << delData << " was successfully removed from the list" << std::endl;
	}
}

void List::PrintList()
{
	curr = head;
	int index = 0;
	while (curr != NULL)
	{
		index++;
		cout << curr->data << " found on node " << curr->id << endl;
		curr = curr->next;
	}
}

int List::getSize()
{
	return currId;
}

bool List::isEmpty()
{
	if (currId > 0) {
		return false;
	}
	return true;
}

void List::AddFirst(int addData)
{
	nodePtr n = new node;
	if (head != NULL)
	{
		n->data = addData;
		n->id = ++currId;
		n->next = head;
		head = n;
	} else {
		head = n;
	}
}

int List::removeLast()
{
	if (head != NULL)
	{
		if (head->next == NULL)
		{
			head = NULL;
		}
		curr = head;
		while (curr->next != NULL)
		{
			temp = curr;
			curr = curr->next;
		}
		int tempData;
		tempData = temp->next->data;
		temp->next = NULL;
		return temp->data;
	}
	cout << "Head is null, consider adding an element" << endl;
	return -1;
}

bool List::removeFirst()
{
	if (head != NULL)
	{
		if (head->next == NULL)
		{
			int tempData = head->data;
			head = NULL;
			cout << "Removed head, linked list is now empty" << endl;
			return true;
		} else {
			temp = head;
			head = head->next;
			delete temp;
			return true;
		}
	}
	cout << "Head is null, consider adding an element" << endl;
	return false;
}

// Does not work properly, relies on id to be sequential
bool List::removeAt(int index)
{
	if (!(index > getSize()))
	{
		if (head != NULL)
		{
			if (index == head->id)
			{
				bool isDeleted = removeFirst();
				cout << "Removed first element" << endl;
				return true;
			}
			curr = head;

			while (curr->next->next != NULL && curr->next->id != index)
			{
				curr = curr->next;
			}

			nodePtr post = curr->next->next;
			delete curr->next;
			curr->next = post;
			cout << "Removed element" << endl;
			return true;
		}
		cout << "Head is null, add an element first" << endl;
		return false;
	}
	cout << "Index greater than capacity" << endl;
	return false;
}

int List::peekLast()
{
	if (head != NULL)
	{
		curr = head;
		while (curr->next != NULL)
		{
			curr = curr->next;
		}
		return curr->data;
	}
	cout << "Head is null" << endl;
	return -1;
}

void List::reverse()
{
	if (head != NULL)
	{
		nodePtr prev = NULL, next = NULL;
		curr = head;

		while (curr != NULL)
		{
			// Store next
			next = curr->next;
			// Reverse current nodes pointer
			curr->next = prev;

			// Move pointers one position ahead
			prev = curr;
			curr = next;
		}
		head = prev;
	}
}