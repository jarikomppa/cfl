#include <stdio.h>
#include <stdlib.h>
#include "cfl3.h"

///////////////////////////////////////////////
// Copyright
///////////////////////////////////////////////
//
// Compressed File Library 3 test tool
// Copyright (c) 2001 Jari Komppa
//
//
///////////////////////////////////////////////
// License
///////////////////////////////////////////////
// 
//     This software is provided 'as-is', without any express or implied
//     warranty.    In no event will the authors be held liable for any damages
//     arising from the use of this software.
// 
//     Permission is granted to anyone to use this software for any purpose,
//     including commercial applications, and to alter it and redistribute it
//     freely, subject to the following restrictions:
// 
//     1. The origin of this software must not be misrepresented; you must not
//        claim that you wrote the original software. If you use this software
//        in a product, an acknowledgment in the product documentation would be
//        appreciated but is not required.
//     2. Altered source versions must be plainly marked as such, and must not be
//        misrepresented as being the original software.
//     3. This notice may not be removed or altered from any source distribution.
// 
// (eg. same as ZLIB license)
// 
//
///////////////////////////////////////////////
//
// This is a tool application for debugging CFL and resource handler plugins.
//
///////////////////////////////////////////////

static const int TESTDATA_BYTES=1024*1024;

CFLMaker * maker;
CFL * cfl;
char * testdata;

class handler_enum : public CFLEnumerator
{
public:
    int stage;
    virtual void infoCallback(unsigned int tag, const char * infostring, const char * shortname);
};

void test_data(const char * fname)
{
    printf("\n- Testing '%s'.. ",fname);
    if (!cfl->fileExists(fname))
    {
        printf("error: file not found");
        return;
    }
    printf("file ok");
    int size;
    char * d;
    d=cfl->getFile(fname,size);
    if (size!=TESTDATA_BYTES)
    {
        printf(", size mismatch (reported %d)",size);
        return;
    }
    printf(", size ok");
    for (int i=0;i<TESTDATA_BYTES;i++)
    {
        if ((d[i]&0xff)!=(testdata[i]&0xff))
        {
            printf(", data mismatch (at position %d)",i);
            return;
        }
    }
    printf(", data ok");
    delete[] d;
}

void handler_enum::infoCallback(unsigned int tag, const char * infostring, const char * shortname)
{
    if (stage==0)
    {
        printf("\n- Processing with '%s'.. ",shortname);
        int size=maker->store(shortname,testdata,TESTDATA_BYTES,tag);
        printf("%d bytes (%3.3f%%)",size,size*100.0f/TESTDATA_BYTES);
    } 
    else
    {
        test_data(shortname);
    }
}

void generate_testdata()
{
    testdata=new char[TESTDATA_BYTES];
    int a,b,c;
    a=10;
    b=20;
    c=30;
    for (int i=0;i<TESTDATA_BYTES;i++)
    {
        c+=b>>12;
        b+=a>>12;
        a+=1;
        testdata[i]=c>>12;
    }
}

int main(int parc, char ** pars)
{
    setbuf(stdout,NULL);
    printf("TestCFL\nCFL testing tool\n\n");
    printf("\n- Generating test data.. ");
    generate_testdata();
    printf("%d bytes",TESTDATA_BYTES);
    printf("\n- Creating CFL..");
    maker=CFLMaker::create("testcfl.cfl");
    handler_enum e;
    e.stage=0;
    CFLResourceHandler::enumerateHandlers(&e);
    printf("\n- Processing with 'find best'.. ");
    int size=maker->store("find best",testdata,TESTDATA_BYTES,CFLCOMPRESS_BEST);
    printf("%d bytes (%3.3f%%)",size,size*100.0f/TESTDATA_BYTES);
    printf("\n- Closing CFL");
    maker->finish(CFLCOMPRESS_BEST);
    printf("\n- Mounting CFL.. ");    
    cfl=CFL::create("testcfl.cfl");
    if (cfl==NULL)
    {
        printf("Failed!");
        delete[] testdata;
        return 1;
    }
    printf("OK");
    e.stage=1;
    CFLResourceHandler::enumerateHandlers(&e);
    test_data("find best");
    printf("\n- Closing CFL");
    delete cfl;    
    printf("\n(you may delete testcfl.cfl)");
    printf("\n");
    delete[] testdata;
    return 0;
}