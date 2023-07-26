#include <string.h>
#include "cfl3.h"

///////////////////////////////////////////////
// Creating resource handler plugins
///////////////////////////////////////////////
// 
// Different compressors, encrypters, and preprocessors are plugged
// into cfl3 using class factory mechanism.
//
// No changes to cfl3.cpp and/or cfl3.h are needed.
//
// This source file defines the 8-bit delta encoder, and it self-registers
// itself to cfl3. No header file is needed.
//

class CFLRH_Delta8 : public CFLResourceHandler
{
private:
    // Each extended class must have a private static instance of itself, like this:
    static CFLRH_Delta8 myself; 
protected:
    // Private constructor.
    // Constructor must:
    // - set tag and infostring
    // - call registerHandler(tag,this)
    CFLRH_Delta8();
    // process (eg, compress, encrypt..) data 
    virtual void process(char * datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize);
    // reverse process data (eg. decompress, decrypt..) data
    virtual void reverseProcess(char *datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize);
};

CFLRH_Delta8 CFLRH_Delta8::myself;

CFLRH_Delta8::CFLRH_Delta8()
{
    static const char sn[]="Delta8";
    static const char ln[]="8-bit delta encoding";
    tag=CFLPREPROCESS_DELTA8;
    shortName=(char*)sn;
    infoString=(char*)ln;
    registerHandler(this);
}

void CFLRH_Delta8::process(char * datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize)
{
    char * din=datain;
    char * dout=new char[datainsize];
    if (dout==NULL)
    {
        *dataout=NULL;
        dataoutsize=0;
        return;
    }
    *dataout=dout;
    int process=datainsize;
    dout[0]=din[0];
    for (int i=1;i<process;i++)
        dout[i]=din[i]-din[i-1];
    dataoutsize=datainsize;
}

void CFLRH_Delta8::reverseProcess(char *datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize)
{
    char * din=datain;
    char * dout=new char[datainsize];
    if (dout==NULL)
    {
        *dataout=NULL;
        dataoutsize=0;
        return;
    }
    *dataout=dout;
    int process=datainsize;
    dout[0]=din[0];
    for (int i=1;i<process;i++)
        dout[i]=din[i]+dout[i-1];
    dataoutsize=datainsize;
}
