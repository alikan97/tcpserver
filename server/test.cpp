#include <cstdlib>
#include <iostream>
#include <queue>
#include <cstring>

typedef struct message_t
{
    std::string message;
    std::string datetime;
}* message;

int main (int argc, char** argv)
{
    std::queue<message> myQueue;
    message frs = new message_t;
    frs->message = "First message";
    frs->datetime = "2022/04/18";
    myQueue.push(frs);

    std::cout << myQueue.back()->message;
}