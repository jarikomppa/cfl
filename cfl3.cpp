    ////// ////// //     /////
   //     //     //         //
  //     ////   //       ///
 //     //     //         //
////// //     ////// /////

///////////////////////////////////////////////
// Copyright
///////////////////////////////////////////////
//
// Compressed File Library 3
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
// See cfl3.h for documentation
//
///////////////////////////////////////////////

// CFL3 needs fopen, fclose, fseek, ftell, fread, fwrite and memcpy
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfl3.h"


CFLResourceHandler * CFLResourceHandler::root=NULL;


CFLResource * CFLResource::load(FILE * f) 
{
    CFLResource * d=new CFLResource();
    if (d==NULL) return NULL;
    fread(&d->compressedSize,1,4,f);
    d->data=new char[d->compressedSize];
    if (d->data==NULL)
    {
        delete d;
        return NULL;
    }
    fread(d->data,1,d->compressedSize,f);
    return d;
}


void CFLResource::save(FILE * f) 
{
    fwrite(&compressedSize,1,4,f);
    fwrite(data,1,compressedSize,f);
}


CFLResource::CFLResource()
{
    data=NULL;
    compressedSize=0;
}


CFLResource::~CFLResource()
{
    if (data!=NULL)
        delete[] data;
}


CFLResourceHandler::CFLResourceHandler()
{
    enabled=1;
}


void CFLResourceHandler::registerHandler(CFLResourceHandler * handler)
{
    handler->next=root;
    root=handler;
}


CFLResourceHandler::~CFLResourceHandler()
{
}



CFLResourceHandler * CFLResourceHandler::findHandler(unsigned int tag) {
    CFLResourceHandler * walker;
    walker=root;
    while (walker) 
    {
        if (walker->tag==tag) 
            return walker;
        walker=walker->next;        
    }
    return NULL;
}


char * CFLResourceHandler::unhandle(const CFLResource *res, unsigned int compressionFlags)
{
#ifdef _DEBUG
    if (root==NULL)
    {
        int i=0,j=0;
        j/=i;
        // If you hit this check, it's because you haven't
        // linked any resource handlers to your project.
        // * You do not need to change anything in this file *
        // just add cfl3_none.cpp to your project and you're go. 
        // See the top of this file for more info.
    }
#endif
    CFLResourceHandler * h;    
    char * data=res->data;
    unsigned int datasize=res->compressedSize;
    char * d;
    unsigned int newsize;
    
    // encryption
    
    if (compressionFlags&0xff000000) 
    {
        h=findHandler(compressionFlags&0xff000000);
        if (h==NULL) 
            return NULL;
        h->reverseProcess(data,&d,datasize,newsize);
        data=d;
        datasize=newsize;
    }
    
    // compression
    
    h=findHandler(compressionFlags&0x0000ffff);
    // if not found, try to find the base variant:
    if (h==NULL) 
        h=findHandler(compressionFlags&0x000000ff);
    if (h==NULL) 
    {
        if (data!=res->data)
            delete[] data;
        return NULL;
    }
    h->reverseProcess(data,&d,datasize,newsize);
    if (data!=res->data)
        delete[] data;
    data=d;
    datasize=newsize;

    // preprocess

    if (compressionFlags&0x00ff0000) 
    {
        h=findHandler(compressionFlags&0x00ff0000);
        if (h==NULL) 
        {
            if (data!=res->data)
                delete[] data;
            return NULL;
        }
        h->reverseProcess(data,&d,datasize,newsize);
        if (data!=res->data)
            delete[] data;
        data=d;
        datasize=newsize;
    }
    return data;
}


CFLResource * CFLResourceHandler::handle(const char *data, unsigned int datasize, unsigned int &compressionFlags)
{
#ifdef _DEBUG
    if (root==NULL)
    {
        int i=0,j=0;
        j/=i;
        // If you hit this check, it's because you haven't
        // linked any resource handlers to your project.
        // * You do not need to change anything in this file *
        // just add cfl3_none.cpp to your project and you're go. 
        // See the top of this file for more info.
    }
#endif
    CFLResource * res=new CFLResource();
    if (res==NULL)
    {
        return NULL;
    }
    res->data=(char*)data;
    CFLResourceHandler * h;    
    unsigned int currentsize=datasize;
    char * d;
    unsigned int newsize;
    
    // preprocess
    
    if (compressionFlags&0x00ff0000) 
    {
        h=findHandler(compressionFlags&0x00ff0000);
        if (h==NULL) 
            return NULL;
        h->process(res->data,&d,currentsize,newsize);
        res->data=d;
        currentsize=newsize;
    }
    
    // compression
    
    if ((compressionFlags&0x0000ffff)==CFLCOMPRESS_BEST)
    {
        CFLResourceHandler * walker=root;
        int decided=0;
        unsigned int smallest=0xffffffff;
        char * smallest_data=NULL;
        while (walker)
        {
            unsigned int tempsize;
            char * tempdata;
            walker->process(res->data,&tempdata,currentsize,tempsize);            
            if (tempdata!=NULL && tempsize<smallest)
            {
                decided=walker->tag;
                smallest=tempsize;
                if (smallest_data!=NULL)
                    delete[] smallest_data;
                smallest_data=tempdata;
            } 
            else
            {
                delete[] tempdata;
            }
            walker=walker->next;
        }
        compressionFlags&=0xffff0000;
        compressionFlags|=decided;
        if (res->data!=data) {
            delete[] res->data;
        }
        res->data=smallest_data;
        currentsize=smallest;
    } 
    else // not _BEST
    {
        h=findHandler(compressionFlags&0x0000ffff);
        if (h==NULL) 
        {
            if (res->data!=data) 
                delete[] res->data;
            delete res;
            return NULL;
        }
        h->process(res->data,&d,currentsize,newsize);
        if (res->data!=data) {
            delete[] res->data;
        }
        res->data=d;
        currentsize=newsize;
    }

    // encryption

    if (compressionFlags&0xff000000) 
    {
        h=findHandler(compressionFlags&0xff000000);
        if (h==NULL) 
        {
            if (res->data!=data)
                delete[] res->data;
            delete res;
            return NULL;
        }
        h->process(res->data,&d,currentsize,newsize);
        if (res->data!=data)
            delete[] res->data;
        res->data=d;
        currentsize=newsize;
    }
    res->compressedSize=currentsize;
    return res;
}


const char * CFLResourceHandler::handlerShortName(unsigned int tag)
{
    static const char findbest_sn[]="best";
    if (tag==0xffff)
    {
        return findbest_sn;
    }
    CFLResourceHandler * h=findHandler(tag);
    if (h==NULL) return "Unknown";
    return h->shortName;    
}


const char * CFLResourceHandler::handlerInfoString(unsigned int tag)
{
    static const char findbest_info[]="Finds the best compressor by testing them all";
    if (tag==0xffff)
    {
        return findbest_info;
    }
    CFLResourceHandler * h=findHandler(tag);
    if (h==NULL) return "Unknown";
    return h->infoString;    
}


void CFLResourceHandler::enumerateHandlers(CFLEnumerator * enumerator)
{
    CFLResourceHandler * walker;
    walker=root;
    while (walker) 
    {
        enumerator->infoCallback(walker->tag,walker->infoString,walker->shortName);
        walker=walker->next;        
    }
}

void CFLResourceHandler::enableHandler(unsigned int tag, int state)
{
    CFLResourceHandler * h=findHandler(tag);
    if (h!=NULL) 
        h->enabled=state;
}

void CFLResourceHandler::setKey(unsigned int tag, void * key)
{
    CFLResourceHandler * h=findHandler(tag);
    if (h!=NULL) 
        h->setCipherKey(key);
}


void CFLResourceHandler::setCipherKey(void * key)
{
}


CFLLibrary::CFLLibrary()
{
    fileptr=NULL;
    filename=NULL;
    offset=0;
    next=NULL;
}


CFLLibrary::~CFLLibrary()
{
    if (filename)
        delete[] filename;
    if (fileptr)
        fclose(fileptr);
}


FILE * CFLLibrary::open()
{
    if (fileptr) 
        return fileptr;
    return fopen(filename,"rb");
}


void CFLLibrary::close(FILE * f)
{
    if (!fileptr) fclose(f);
}


int CFLDirectoryEntry::byteRequirement()
{
    int size=0;
    while (name[size]) size++;
    size+=4+4+4+2; // unpacksize, offset, compression, name lenght
    return size;
}


void CFLDirectoryEntry::store(char *buffer, int &offset)
{
    memcpy(buffer+offset,&this->unpackedSize,4);
    offset+=4;
    memcpy(buffer+offset,&this->offset,4);
    offset+=4;
    memcpy(buffer+offset,&this->compression,4);
    offset+=4;
    unsigned short namelen=0;
    while (name[namelen]) 
    {
        buffer[offset+2+namelen]=name[namelen];
        namelen++;
    }
    memcpy(buffer+offset,&namelen,2);
    offset+=namelen+2;    
}
 
 
CFLDirectoryEntry * CFLDirectoryEntry::restore(char *buffer, int &offset)
{
    CFLDirectoryEntry * e=new CFLDirectoryEntry();
    if (e==NULL)
    {
        return NULL;
    }
    memcpy(&e->unpackedSize,buffer+offset,4);
    offset+=4;
    memcpy(&e->offset,buffer+offset,4);
    offset+=4;
    memcpy(&e->compression,buffer+offset,4);
    offset+=4;
    unsigned short namelen=0;
    memcpy(&namelen,buffer+offset,2);
    offset+=2;
    e->name=new char[namelen+1];
    if (e->name==NULL)
    {
        delete e;
        return NULL;
    }
    memcpy(e->name,buffer+offset,namelen);
    e->name[namelen]=0;
    offset+=namelen;
    e->nameHash=calcHash(e->name);
    return e;
}


CFLDirectoryEntry::~CFLDirectoryEntry()
{
    delete[] name;
}


int CFLDirectoryEntry::equals(const char * name, int hash)
{
    if (nameHash!=hash) return 0;
    char * n1=(char*)name, * n2=this->name;
    while (*n1 && *n2 && *n1==*n2) 
    {
        n1++;
        n2++;
    }
    if (!*n1 && !*n2) 
        return 1;
    return 0;
}


int CFLDirectoryEntry::calcHash(const char * name)
{
    int hash=0;
    char * d=(char*)name;
    while (*d) 
    {
        int t=hash>>11;
        hash=hash<<(32-11);
        hash+=t;
        hash^=*d&0xff;
        d++;
    }
    return hash;
}


void CFLDirectoryEntry::replaceEntry(CFLDirectoryEntry * replacement)
{
    delete[] name;
    name=replacement->name;
    replacement->name=NULL;
    lib=replacement->lib;
    unpackedSize=replacement->unpackedSize;
    offset=replacement->offset;
    nameHash=replacement->nameHash;
    delete replacement;
}


CFLDirectoryEntry * CFL::findEntry(const char * name)
{
    CFLDirectoryEntry * walker;
    walker=rootdir;
    int hash=CFLDirectoryEntry::calcHash(name);
    while (walker)
    {
        if (walker->equals(name,hash)) 
            return walker;
        walker=walker->next;
    }
    return NULL;
}


CFL::CFL()
{
    rootlib=NULL;
    rootdir=NULL;    
}


CFL * CFL::create(const char * libfilename, int offset)
{
    CFL * c=new CFL();
    if (c==NULL)
    {
        return NULL;
    }
    if (!c->openLibrary(libfilename,offset))
    {
        delete c;
        return NULL;
    }
    return c;
}


CFL * CFL::create(const char * libfilename)
{
    return create(libfilename,0);
}


CFL * CFL::create(FILE * libfileptr)
{
    CFL * c=new CFL();
    if (c==NULL)
    {
        return NULL;
    }
    if (!c->openLibrary(libfileptr))
    {
        delete c;
        return NULL;
    }
    return c;
}


int CFL::openLibrary(const char * libfilename)
{
    return openLibrary(libfilename,0);
}


int CFL::openLibrary(const char * libfilename, int offset)
{
    int tag=0;
    int offs=0;
    int dirsize=0;
    FILE * f;
    f=fopen(libfilename,"rb");
    if (f==NULL) return 0;

    // check given offset first

    fseek(f,offset,SEEK_SET);
    fread(&tag,1,4,f);
    if (tag!=0x334c4643) // 'CFL3'    
    {
        // okay, so tag wasn't there. Check footer next
        fseek(f,-4,SEEK_END);
        fread(&tag,1,4,f);
        if (tag!=0x4c464333) 
        {
            // nope, give up.
            fclose(f);
            return 0;
        }
        fseek(f,-8,SEEK_END);
        fread(&offs,1,4,f);
        fseek(f,-offs,SEEK_END);
        fread(&tag,1,4,f);
        if (tag!=0x334c4643) // 'CFL3'    
        {
            // footer matches but header doesn't. Give up.
            fclose(f);
            return 0;            
        }
    }
    // If we get this far, we've just passed the header CFL3 tag.
    offset=ftell(f)-4;
    fread(&offs,1,4,f);
    fread(&dirsize,1,4,f);
    fseek(f,offset+offs,SEEK_SET); // go to the beginning of directory
    CFLResource * directory;
    int directoryCompression;
    fread(&directoryCompression,1,4,f);
    directory=CFLResource::load(f);    
    fclose(f);
    if (directory==NULL)
    {
        return 0;
    }
    char * d=CFLResourceHandler::unhandle(directory,directoryCompression);
    delete directory;
    if (d==NULL)
    {
        // Decompressing the directory failed. (probably unsupported format)
        return 0;    
    }
    
    // so now we're pretty certain that this is a good cfl file..
    
    CFLLibrary * lib=new CFLLibrary();
    if (lib==NULL)
    {
        delete[] d;
        return 0;
    }
    int fn=0;
    while (libfilename[fn]) fn++;
    lib->filename=new char[fn+1];
    if (lib->filename==NULL)
    {
        delete[] d;
        delete lib;
        return 0;
    }
    fn=0;
    while (libfilename[fn]) 
    {
        lib->filename[fn]=libfilename[fn];
        fn++;
    }
    lib->filename[fn]=0;
    lib->offset=offset;
    lib->next=rootlib;
    rootlib=lib;
    
    offs=0;
    while (offs<dirsize)
    {
        // restore-call updates offs
        CFLDirectoryEntry * e=CFLDirectoryEntry::restore(d,offs);
        if (e==NULL)
        {
            delete[] d;
            return 1; // partial success
        }
        e->lib=lib;        
        CFLDirectoryEntry * old=findEntry(e->name);
        if (old!=NULL)
        {
            old->replaceEntry(e);
        }
        else
        {
            e->next=rootdir;
            rootdir=e;
        }
    }
    delete[] d;
    // success!
    return 1;
}


int CFL::openLibrary(FILE * libfileptr)
{
    int tag=0;
    int offs=0;
    int dirsize=0;
    int offset;
    FILE * f=libfileptr;
    if (f==NULL) return 0;

    // check given offset first

    fread(&tag,1,4,f);
    if (tag!=0x334c4643) // 'CFL3'    
    {
        // okay, so tag wasn't there. Check footer next
        fseek(f,-4,SEEK_END);
        fread(&tag,1,4,f);
        if (tag!=0x4c464333) 
        {
            // nope, give up.
            fclose(f);
            return 0;
        }
        fseek(f,-8,SEEK_END);
        fread(&offs,1,4,f);
        fseek(f,-offs,SEEK_END);
        fread(&tag,1,4,f);
        if (tag!=0x334c4643) // 'CFL3'    
        {
            // footer matches but header doesn't. Give up.
            fclose(f);
            return 0;            
        }
    }
    // If we get this far, we've just passed the header CFL3 tag.
    offset=ftell(f)-4;
    fread(&offs,1,4,f);
    fread(&dirsize,1,4,f);
    fseek(f,offset+offs,SEEK_SET); // go to the beginning of directory
    CFLResource * directory;
    int directoryCompression;
    fread(&directoryCompression,1,4,f);
    directory=CFLResource::load(f);    
    if (directory==NULL)
    {
        return 0;
    }
    char * d=CFLResourceHandler::unhandle(directory,directoryCompression);
    delete directory;
    if (d==NULL)
    {
        // Decompressing the directory failed. (probably unsupported format)
        return 0;    
    }
    
    // so now we're pretty certain that this is a good cfl file..
    
    CFLLibrary * lib=new CFLLibrary();
    if (lib==NULL)
    {
        delete[] d;
        return 0;
    }
    lib->fileptr=f;
    lib->offset=offset;
    lib->next=rootlib;
    rootlib=lib;
    
    offs=0;
    while (offs<dirsize)
    {
        // restore-call updates ofs
        CFLDirectoryEntry * e=CFLDirectoryEntry::restore(d,offs);
        if (e==NULL)
        {
            delete[] d;
            return 1; // partial success
        }
        e->lib=lib;        
        CFLDirectoryEntry * old=findEntry(e->name);
        if (old!=NULL)
        {
            old->replaceEntry(e);
        }
        else
        {
            e->next=rootdir;
            rootdir=e;
        }
    }
    delete[] d;
    // success!
    return 1;
}


char * CFL::getFile(const char *name)
{
    int s;
    return getFile(name,s);
}


char * CFL::getFile(const char *name, int &size)
{
    CFLDirectoryEntry * e=findEntry(name);
    size=0;
    if (e==NULL) 
        return NULL;
    size=e->unpackedSize;
    CFLResource * r;
    FILE * f;
    f=e->lib->open();
    fseek(f,e->lib->offset+e->offset,SEEK_SET);
    r=CFLResource::load(f);
    e->lib->close(f);
    char * d=CFLResourceHandler::unhandle(r,e->compression);
    delete r;
    return d;    
}


int CFL::getFile(const char * name, char * buffer, int maxsize)
{
    int s;
    char * d=getFile(name,s);
    if (d==NULL) return 0;
    if (s>maxsize) s=maxsize;
    memcpy(buffer,d,s);
    delete[] d;
    return s;
}


int CFL::getFileSize(const char *name)
{
    CFLDirectoryEntry * e=findEntry(name);
    if (e==NULL) return 0;
    return e->unpackedSize;    
}


int CFL::fileExists(const char *name)
{
    CFLDirectoryEntry * e=findEntry(name);
    if (e==NULL) return 0;
    return 1;    
}


FILE * CFL::getFilePtr(const char *name)
{
    FILE * f;
    CFLDirectoryEntry * e=findEntry(name);
    if (e==NULL) 
        return NULL;
    f=e->lib->open();
    fseek(f,e->lib->offset+e->offset,SEEK_SET);
    return f;
}

void CFL::dropFilePtr(FILE * f)
{
    CFLLibrary * walker;
    walker=rootlib;
    while (walker)
    {
        if (walker->fileptr==f)
            return;
        walker=walker->next;
    }
    fclose(f);
}


int CFL::getFileOfs(const char *name)
{
    CFLDirectoryEntry * e=findEntry(name);
    if (e==NULL) 
        return 0;
    return e->lib->offset+e->offset;
}


const char * CFL::compressionInfoString(const char *name)
{
    CFLDirectoryEntry * e=findEntry(name);
    if (e==NULL) 
        return NULL;
    return CFLResourceHandler::handlerInfoString(e->compression&0x0000ffff);
}


const char * CFL::encryptionInfoString(const char *name)
{
    CFLDirectoryEntry * e=findEntry(name);
    if (e==NULL) 
        return NULL;
    return CFLResourceHandler::handlerInfoString(e->compression&0xff000000);
}


const char * CFL::preprocessorInfoString(const char *name)
{
    CFLDirectoryEntry * e=findEntry(name);
    if (e==NULL) 
        return NULL;
    return CFLResourceHandler::handlerInfoString(e->compression&0x00ff0000);
}


CFL::~CFL()
{
    CFLLibrary * libwalker=rootlib;
    while (libwalker)
    {
        CFLLibrary * last=libwalker;
        libwalker=libwalker->next;
        delete last;
    }
    CFLDirectoryEntry * dirwalker=rootdir;
    while (dirwalker)
    {
        CFLDirectoryEntry * last=dirwalker;
        dirwalker=dirwalker->next;
        delete last;
    }
}


CFLMaker::CFLMaker()
{
    rootdir=NULL;
    f=NULL;
}


CFLMaker::~CFLMaker()
{
    CFLDirectoryEntry * dirwalker=rootdir;
    while (dirwalker)
    {
        CFLDirectoryEntry * last=dirwalker;
        dirwalker=dirwalker->next;
        delete last;
    }
}


CFLMaker * CFLMaker::create(const char *targetfilename)
{
    CFLMaker * m=new CFLMaker();
    if (m==NULL)
    {
        return NULL;
    }
    m->f=fopen(targetfilename,"wb");
    if (m->f==NULL) 
        return NULL;
    int data=0x334c4643;
    fwrite(&data,1,4,m->f); // header
    fwrite(&data,1,4,m->f); // placeholder for directory offset
    fwrite(&data,1,4,m->f); // placeholder for directory decompressed size    
    return m;
}


int CFLMaker::store(const char *fname, const char *data, int datasize, unsigned int compressionFlags)
{
    CFLDirectoryEntry * dir=new CFLDirectoryEntry();
    if (dir==NULL)
    {
        return 0;
    }
    dir->offset=ftell(f);
    dir->compression=compressionFlags;
    dir->unpackedSize=datasize;
    int namelen=0;
    while (fname[namelen]) namelen++;
    dir->name=new char[namelen+1];
    if (dir->name==NULL)
    {
        delete dir;
        return 0;
    }
    namelen=0;
    while (fname[namelen]) 
    {
        dir->name[namelen]=fname[namelen];
        namelen++;
    }
    dir->name[namelen]=0;
    CFLResource * res=CFLResourceHandler::handle(data,datasize,dir->compression);    
    // if compression fails, return 0;
    if (res==NULL)
    {
        delete dir;
        return 0;
    }
    res->save(f);
    int packedsize=res->compressedSize;
    delete res;
    dir->next=rootdir;
    rootdir=dir;
    return packedsize;
}


int CFLMaker::finish(unsigned int libraryCompressionFlags)
{
    int libofs=ftell(f);
    
    int unpackedDirSize=0;
    CFLDirectoryEntry * dirwalker=rootdir;
    while (dirwalker)
    {
        unpackedDirSize+=dirwalker->byteRequirement();
        dirwalker=dirwalker->next;
    }
    char * rawdir=new char[unpackedDirSize];
    // memory allocation failed; return 0
    if (rawdir==NULL)
    {
        return 0;
    }
    dirwalker=rootdir;
    unpackedDirSize=0;
    while (dirwalker)
    {
        dirwalker->store(rawdir,unpackedDirSize);
        dirwalker=dirwalker->next;
    }
    CFLResource * resdir=CFLResourceHandler::handle(rawdir,unpackedDirSize, libraryCompressionFlags);
    // compression failed; return 0
    if (resdir==NULL)
    {
        delete[] rawdir;
        return 0;
    }
    delete[] rawdir;    
    fwrite(&libraryCompressionFlags,1,4,f);
    resdir->save(f);
    delete resdir;
    int offsetOfHeader=ftell(f)+8;
    fwrite(&offsetOfHeader,1,4,f);
    int tag=0x4c464333; // '3CFL'
    fwrite(&tag,1,4,f);
    fseek(f,4,SEEK_SET);
    fwrite(&libofs,1,4,f); 
    fwrite(&unpackedDirSize,1,4,f);
    fclose(f);
    delete this;
    return 1;
}
