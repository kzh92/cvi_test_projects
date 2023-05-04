#ifndef NODE_H
#define NODE_H

#ifndef NULL
#define NULL 0
#endif

template <class T>
struct Node {
    Node<T> *next;
    Node<T> *prev;
    T data;
    Node() {next = NULL; prev = NULL;}
};

#endif
