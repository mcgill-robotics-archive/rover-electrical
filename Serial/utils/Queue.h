#pragma once
#include <Arduino.h>

template <class T>
class Queue
{
public:

    class QueueNode
    {
    public:
        QueueNode(T _data) : data(_data) {}

        QueueNode* next = nullptr;
        T data;
    };
    
    Queue() {}

    void enqueue(T& message)
    {
        QueueNode* node = new QueueNode(message);

        if (front == nullptr) // queue is empty
        {
            front = node;
            back = node;
        }
        else // not empty
        {
            back->next = node;
            back = node;
        }
    }


    T dequeue()
    {
        if (front == nullptr) return T();
        QueueNode* node = front;
        T result = node->data;
        
        front = front->next;
        delete node;

        return result;
    }

private:
    QueueNode* front;
    QueueNode* back;
};