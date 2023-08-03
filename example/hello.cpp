#include <stdio.h>
#include <stdlib.h>
#include "cfl.h"

int main(void)
{
    CFL * cfl = new CFL();
    if (!cfl) 
    {
        printf("Unable to crate CFL object.");
        return -1;
    }        
    if (!cfl->openLibrary("hello.cfl"))
    {
        printf("Unable to open CFL file.");
        delete cfl;
        return -2;
    }    
    if (cfl->fileExists("hello.txt"))
    {
        printf("Cfl does not contain hello.txt file.");
        delete cfl;
        return -3;
    }
    printf("%s",cfl->getFile("hello.txt"));
    delete cfl;
    return 0;
}