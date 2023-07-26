#include <string.h>
#include "zlib.h"
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
// This source file defines the zlib compressor, and it self-registers
// itself to cfl3. No header file is needed.
//
// 7.5.2001: Changed to work with zlib 1.1.3

class CFLRH_Zlib : public CFLResourceHandler
{
private:
    // Each extended class must have a private static instance of itself, like this:
    static CFLRH_Zlib myself; 
protected:
    // Private constructor.
    // Constructor must:
    // - set tag and infostring
    // - call registerHandler(tag,this)
    CFLRH_Zlib();
    // process (eg, compress, encrypt..) data 
    virtual void process(char * datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize);
    // reverse process data (eg. decompress, decrypt..) data
    virtual void reverseProcess(char *datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize);
};

CFLRH_Zlib CFLRH_Zlib::myself;

CFLRH_Zlib::CFLRH_Zlib()
{
    static const char sn[]="ZLIB";
    static const char ln[]="ZLIB data compression library (C) 1995-1998 Jean-loup Gailly and Mark Adler";
    tag=CFLCOMPRESS_ZLIB;
    shortName=(char*)sn;
    infoString=(char*)ln;
    registerHandler(this);
}

void CFLRH_Zlib::process(char * datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize)
{
    // output buffer "must be at least 0.1% larger than sourceLen plus 12 bytes"
    dataoutsize=(unsigned long)(datainsize*1.01f)+12;    
    char * data=new char[dataoutsize];
    if (data==NULL)
    {
        *dataout=NULL;
        dataoutsize=0;
        return;
    }        
    if (compress(data,(unsigned long*)&dataoutsize,(const char*)datain,datainsize)>=0) // negative values are errors
    {
        // We also want to store the length in the beginning of the block, since we
        // need it when decompressing.
        *dataout=new char[dataoutsize+4];
        if (*dataout==NULL)
        {            
            dataoutsize=0;
            delete[] data;
            return;
        }        
        memcpy(*dataout,&datainsize,4);
        memcpy(*dataout+4,data,dataoutsize);        
        delete[] data;
    }
    else // failed
    {
        delete[] data;
        *dataout=NULL;
        dataoutsize=0;
    }
}

void CFLRH_Zlib::reverseProcess(char *datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize)
{
    memcpy(&dataoutsize,datain,4);    
    char * data=new char[dataoutsize];
    if (data==NULL)
    {
        *dataout=NULL;
        dataoutsize=0;
        return;
    } 
    // ZLIB 1.1.3 gives out Z_BUF_ERROR even if no error on decompress.
    int res=uncompress(data,(unsigned long*)&dataoutsize,(const char*)(datain+4),datainsize-4);
    if (res==Z_BUF_ERROR || res>=0) // negative values are errors
    {
        *dataout=new char[dataoutsize];
        if (*dataout==NULL)
        {            
            dataoutsize=0;
            delete[] data;
            return;
        }        
        memcpy(*dataout,data,dataoutsize);        
        delete[] data;
    }
    else // failed
    {
        delete[] data;
        *dataout=NULL;
        dataoutsize=0;
    }
}
