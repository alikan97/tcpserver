#include <cstdlib>
#include <iostream>
#include <queue>
#include <cstring>
#include <thread> 
#include <mutex>

std::mutex mtx;

typedef struct message_t
{
    std::string message;
    std::string datetime;
}* message;

void func (std::queue<message_t> *mq)
{
    mtx.lock();
    if (!mq->empty()) {
        std::cout << "Messages in the queue " << mq->front().message << std::endl;
    } else {
        message_t myMessage = { "First Message", "20220/1/03"};
        mq->push(myMessage);
    }
    mtx.unlock();
}

int main (int argc, char** argv)
{
    int maxSize = 100;
    std::queue<message_t> myQueue;

    // std::strncpy(buff, first, (size_t)strlen(first)+1);   // strcpy will overwrite what was previouslyt in the buffer, +1 will allocate one more room for null termination
    // std::strcat(buff, second);

    std::thread th1(func, &myQueue);
    std::thread th2(func, &myQueue);
    std::thread th3(func, &myQueue);

    th1.join();
    th2.join();
    th3.join();
}