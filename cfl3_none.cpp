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
// This source file defines the do-nothing compressor, and it self-registers
// itself to cfl3. No header file is needed.
//

class CFLRH_None : public CFLResourceHandler
{
private:
    // Each extended class must have a private static instance of itself, like this:
    static CFLRH_None myself; 
protected:
    // Private constructor.
    // Constructor must:
    // - set tag and infostring
    // - call registerHandler(tag,this)
    CFLRH_None();
    // process (eg, compress, encrypt..) data 
    virtual void process(char * datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize);
    // reverse process data (eg. decompress, decrypt..) data
    virtual void reverseProcess(char *datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize);
};

CFLRH_None CFLRH_None::myself;

CFLRH_None::CFLRH_None()
{
    static const char sn[]="None";
    static const char ln[]="No compression";
    tag=CFLCOMPRESS_NONE;
    shortName=(char*)sn;
    infoString=(char*)ln;
    registerHandler(this);
}

void CFLRH_None::process(char * datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize)
{
    *dataout=new char[datainsize];
    if (*dataout==NULL)
    {
        dataoutsize=0;
        return;
    }
    memcpy(*dataout,datain,datainsize);
    dataoutsize=datainsize;
}


void CFLRH_None::reverseProcess(char *datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize)
{
    process(datain,dataout,datainsize,dataoutsize);
}
