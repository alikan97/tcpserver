#include <cstdlib>
#include "List.h"
#include <iostream>

int main (int argc, char** argv)
{
    List nums;
    nums.AddNode(1);
    nums.AddNode(2);
    nums.AddNode(3);
    nums.AddNode(4);
    nums.AddNode(5);
    nums.AddNode(6);

    nums.PrintList();

    nums.reverse();
    nums.PrintList();
}