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
// This source file defines the 16-bit delta encoder, and it self-registers
// itself to cfl3. No header file is needed.
//

class CFLRH_Delta16 : public CFLResourceHandler
{
private:
    // Each extended class must have a private static instance of itself, like this:
    static CFLRH_Delta16 myself; 
protected:
    // Private constructor.
    // Constructor must:
    // - set tag and infostring
    // - call registerHandler(tag,this)
    CFLRH_Delta16();
    // process (eg, compress, encrypt..) data 
    virtual void process(char * datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize);
    // reverse process data (eg. decompress, decrypt..) data
    virtual void reverseProcess(char *datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize);
};

CFLRH_Delta16 CFLRH_Delta16::myself;

CFLRH_Delta16::CFLRH_Delta16()
{
    static const char sn[]="Delta16";
    static const char ln[]="16-bit delta encoding";
    tag=CFLPREPROCESS_DELTA16;
    shortName=(char*)sn;
    infoString=(char*)ln;
    registerHandler(this);
}

void CFLRH_Delta16::process(char * datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize)
{
    short * din=(short*)datain;
    short * dout=(short*)new char[datainsize];
    if (dout==NULL)
    {
        *dataout=NULL;
        dataoutsize=0;
        return;
    }
    *dataout=(char*)dout;
    unsigned int process=datainsize/2;
    if (process*2<datainsize)
    {
        for (unsigned int i=process*2;i<datainsize;i++)
            *(((char*)dout)+i)=*(((char*)din)+i);
    }

    dout[0]=din[0];
    for (unsigned int i=1;i<process;i++)
        dout[i]=din[i]-din[i-1];
    dataoutsize=datainsize;
}

void CFLRH_Delta16::reverseProcess(char *datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize)
{
    short * din=(short*)datain;
    short * dout=(short*)new char[datainsize];
    if (dout==NULL)
    {
        *dataout=NULL;
        dataoutsize=0;
        return;
    }
    *dataout=(char*)dout;
    unsigned int process=datainsize/2;
    if (process*2<datainsize)
    {
        for (unsigned int i=process*2;i<datainsize;i++)
            *(((char*)dout)+i)=*(((char*)din)+i);
    }
    dout[0]=din[0];
    for (unsigned int i=1;i<process;i++)
        dout[i]=din[i]+dout[i-1];
    dataoutsize=datainsize;
}
