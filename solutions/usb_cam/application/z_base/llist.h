#include "node.h"

#ifndef LLIST_H
#define LLIST_H

#ifndef NULL
#define NULL 0
#endif

#define HEAD 0
#define TAIL 1

template <class T>
class LList {
private:
    Node<T> *head, *tail;
    int size;
    int maxSize;
    Node<T> *cursor;
public:
    LList();
    ~LList();
    void addBefore(T data, int index);
    void addAfter(T data, int index);
    void addBeforeCursor(T data);
    void addAfterCursor(T data);
    void addToEnd(T data);
    void addToFront(T data);
    bool contains(T element);
    T remove(int index);
    T removeAtCursor();
    bool isEmpty() {return size==0;}
    int getSize() {return size;}
    T operator[](int index);
    T stepForward();
    T stepBack();
    void moveCursor(int position);
};

template <class T>
LList<T>::LList() {
    size = 0;
    head = new Node<T>();
    tail = new Node<T>();
    head->next = tail;
    tail->prev = head;
    head->prev = NULL;
    tail->next = NULL;
    cursor = NULL;
}

template <class T>
LList<T>::~LList() {
}

template <class T>
void LList<T>::addBefore(T data, int index)
{
    if(index>size)
        return;

    Node<T> *node = new Node<T>();
    Node<T>* p;

    p = head->next;

    for(int i=0; i<index; i++)
        p = p->next;

    node->data = data;

    node->next = p;
    node->prev = p->prev;
    p->prev->next = node;
    p->prev = node;

    size++;
}

template <class T>
void LList<T>::addAfter(T data, int index)
{
    if(index>size)
        return;

    Node<T> *node = new Node<T>();
    Node<T>* p;

    p = head->next;

    for(int i=0; i<index; i++)
        p = p->next;

    node->data = data;

    node->prev = p;
    node->next = p->next;
    p->next->prev = node;
    p->next = node;

    size++;
}

template <class T>
void LList<T>::addBeforeCursor(T data)
{
    if(cursor==NULL)
        return;

    Node<T> *node = new Node<T>();
    Node<T>* p;

    p = cursor;

    node->data = data;

    node->next = p;
    node->prev = p->prev;
    p->prev->next = node;
    p->prev = node;

    size++;
}

template <class T>
void LList<T>::addAfterCursor(T data)
{
    if(cursor==NULL)
        return;

    Node<T> *node = new Node<T>();
    Node<T>* p;

    p = cursor;

    node->data = data;

    node->prev = p;
    node->next = p->next;
    p->next->prev = node;
    p->next = node;

    size++;
}

template <class T>
void LList<T>::addToEnd(T data) {
    Node<T> *node = new Node<T>();

    node->data = data;

    node->next = tail;
    node->prev = tail->prev;
    node->prev->next = node;
    tail->prev = node;

    if(cursor==NULL)
        cursor = head;

    size++;
}

template <class T>
void LList<T>::addToFront(T data) {
    Node<T> *node = new Node<T>();

    node->data = data;

    node->prev = head;
    node->next = head->next;
    node->next->prev = node;
    head->next = node;

    if(cursor==NULL)
        cursor = head;

    size++;
}

template <class T>
bool LList<T>::contains(T element) {
    moveCursor(HEAD);
    T an_element;

    while(an_element = stepForward())
    {
        if(an_element==element)
            return true;
    }
    return false;
}

template <class T>
T LList<T>::remove(int index) {
    if(isEmpty() || index>size)
        return NULL;

    T data;
    Node<T>* node;
    Node<T>* p;

    p = head->next;

    for(int i=0; i<index; i++)
        p = p->next;

    data = p->data;

    p->prev->next = p->next;
    p->next->prev = p->prev;

    size--;

    delete p;

    return data;
}

template <class T>
T LList<T>::removeAtCursor() {
    if(isEmpty() || cursor==tail || cursor==head || cursor==NULL)
        return NULL;

    T data;
    Node<T>* node;
    Node<T>* p;

    p = cursor;

    if(cursor->prev!=head)
        cursor = cursor->prev;
    else if(cursor->next!=tail)
        cursor = cursor->next;
    else
        cursor = NULL;

    data = p->data;

    p->prev->next = p->next;
    p->next->prev = p->prev;

    size--;

    delete p;

    return data;
}

template <class T>
T LList<T>::operator[](int index)
{
    return NULL;
}

template <class T>
T LList<T>::stepForward()
{
    if(cursor==NULL)
        return NULL;

    if(cursor->next != tail)
    {
        cursor = cursor->next;
        return cursor->data;
    }
    else
        return NULL;
}

template <class T>
T LList<T>::stepBack()
{
    if(cursor==NULL)
        return NULL;

    if(cursor != head)
    {
        cursor = cursor->prev;
        return cursor->data;
    }
    else
        return NULL;
}

template <class T>
void LList<T>::moveCursor(int position)
{
    if(cursor==NULL)
        return;

    if(position==HEAD)
        cursor = head;
    else if(position==TAIL)
        cursor = tail;
}
#endif
