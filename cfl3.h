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
//     warranty.  In no event will the authors be held liable for any damages
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
// What is CFL3?
///////////////////////////////////////////////
// 
// CFL3 is the third complete rewrite of my Compressed File Library library.
// CFL makes it possible to store several blocks of data, such as files,
// into one data file, from which they can be retrieved with a single call.
//
// In addition to making the resource loading easy, the stored data can be
// compressed and/or encrypted, and the compression can be defined for each
// file separately.
//
// CFL also enables you to have single executable file distribution - you
// can append the .cfl file after your executable, and CFL will happily
// read it from there.
//
// You can also use 'patch' CFL:s, eg. you can open several CFLs with the
// same resource names, and when retrieved only the latest opened resource
// will be loaded.
// 
// Using CFL3 is pretty straight-forward, and only major differences from
// CFL2 are:
// - Object-oriented design
// - You can now have several different directories in memory 
//   (for example coredata, leveldata etc) by creating several instances
//   of CFL.
// - compressors etc. are now implemented through pluggable class factories.
// - new file format
//
// Important note: you HAVE to link at least one resource handler with this
//                 library; preferably at least the "no compression" one.
//
// 
///////////////////////////////////////////////
// Creating CFL files
///////////////////////////////////////////////
//
// You can create CFL files in three different ways:
// - Programmatically using CFLMaker class (see below for details)
// - From command line using cflmake tool (see cflmake.cpp for details)
// - Visually in Windows using cflstudio (planned tool, unlikely to 
//   appear any time soon due to the fact that the author is tad bit busy)
//
//
///////////////////////////////////////////////
// Basic usage
///////////////////////////////////////////////
//
// You can create the CFL object by just new'ing it, or using the static
// Create functions to open an .cfl file while doing so.
// 
//   example 1:
//      CFL * filelib;
//      filelib=new CFL();
//      filelib->OpenLibrary("mylib.cfl");
//
//   example 2:
//      CFL * filelib;
//      filelib=CFL::Create("mylib.cfl");
//
// After opening the library, you can open others as well, and their 
// contents will be added to the directory. You do not need to care
// where the data comes from when you use it; CFL takes care of that.
//
// Getting the data is done in the following manner:
//   
//   example 3:
//      char * mydata;
//      mydata=filelib->getFile("mydata.dat");
//      useData(mydata);
//      delete[] mydata;
//
// You can also get file size etc. by using the CFL class methods.
//
//
///////////////////////////////////////////////
// CFL creator usage
///////////////////////////////////////////////
//
// Main reason for the creator is game saves, so let's say you have a huge
// adventure game with starting point game data in one CFL and then you alter
// the data on the fly - saving the differences into CFL. Loading a game
// can be as simple as first mounting the world data and then player data
// on top of it.
//
// Using the maker works like this:
//  
//   example 4:
//      CFLMaker * maker;
//      maker=CFLMaker::create("mysavefile.cfl");
//      maker->store("thisdata",thisdata,thisdatasize,compressFlags);
//      maker->store("thatdata",thatdata,thatdatasize,compressFlags);
//      maker->store("somedata",somedata,somedatasize,compressFlags);
//      maker->finish(compressFlags);
//
// The finish call also destructs the maker.
// Compress flags may be a combination of compressor, encrypter and preprocessor.
// Generally you only use a compressor, but you may need to create encrypters etc.
// for your own specific needs, so the functionality is there.
//
// If you specify CFLCOMPRESS_BEST as compress flag, CFL will try to compress
// the data with every single compressor it has available and finally compresses
// with the one that gave it the best compression ratio. (eg. it's slow, eats
// memory, but guarantees the best compression ratio).
//
//
///////////////////////////////////////////////
// Using encrypters
///////////////////////////////////////////////
//
// Before compressing or decompressing data with some encryption, you must
// set the key for the encrypter using the static function
//
// CFLResourceHandler::setKey(unsigned int tag, void * key);
//
// See documentation on each encrypter for details on the key.
// (Please note that encryption by no means makes your data
// secure, as you will need to provide the means to decrypt 
// it in order to use it!)
// 
// 
///////////////////////////////////////////////
// File format
///////////////////////////////////////////////
// 
// You should not need to know this, but here it is in any case..
//
// Header
//  Tag 'CFL3'
//  Offset of directory
//  size of decompressed library
// Resources
//  ..
//  ..
//  ..
// Directory
//  Compression type
//  Directory resource
// Footer
//  Offset of header
//  Tag '3CFL'
// 
//
///////////////////////////////////////////////

#ifndef CFL3_H_INCLUDED
#define CFL3_H_INCLUDED

#include <stdio.h>

// Resource handler flags
// Please note that most of these have not been really implemented,
// but are specified just in case someone wants to do so.
// If you implement some, and want to contribute the implementation, 
// feel free to email me.
enum CFLHANDLERFLAGS {
    CFLCOMPRESS_NONE  = 0x00000000, // No compression
    CFLCOMPRESS_ZLIB  = 0x00000001, // Zlib, normal compress
    CFLCOMPRESS_ZLIB9 = 0x00000901, // Zlib, max compress
    CFLCOMPRESS_LZSS  = 0x00000002, // LZSS
    CFLCOMPRESS_BZIP2 = 0x00000003, // BZip2
    CFLCOMPRESS_BEST  = 0x0000FFFF, // Tries out all registered compressors, uses best result

    CFLPREPROCESS_NONE    = 0x00000000, // No preprocess
    CFLPREPROCESS_DELTA8  = 0x00010000, // 8-bit delta encoding
    CFLPREPROCESS_DELTA16 = 0x00020000, // 16-bit delta encoding
    CFLPREPROCESS_DELTA32 = 0x00030000, // 32-bit delta encoding
    CFLPREPROCESS_BWT     = 0x00040000, // Burrow-Wheeler(sp?) transform
  
    CFLENCRYPT_NONE      = 0x00000000, // No encryption
    CFLENCRYPT_XOR       = 0x01000000, // Simple XOR crypt (generally stops casual hex-editor), key is one char.
    CFLENCRYPT_PRANDXOR  = 0x02000000, // XOR's every byte with data from pseudorandom generator, key is the random seed.
    CFLENCRYPT_STRINGXOR = 0x03000000, // XOR's every byte with a letter from entered string. Somewhat easy to crack if string is short, but is easy way to implement password protection.
    CFLENCRYPT_PGP       = 0x10000000, // Pretty Good Privacy
    CFLENCRYPT_GPG       = 0x20000000, // GPG
    CFLENCRYPT_DES       = 0x30000000, // Data Encryption Standard
    CFLENCRYPT_3DES      = 0x40000000, // Triple-DES
    CFLENCRYPT_BLOWFISH  = 0x50000000, // Blowfish
    CFLENCRYPT_IDEA      = 0x60000000, // IDEA
    CFLENCRYPT_RC4       = 0x70000000  // RC4
};


// Callback enumerator for handlers
class CFLEnumerator
{
public:
    virtual void infoCallback(unsigned int tag, const char * infostring, const char * shortname)=0;
};


// Resource header and handler
class CFLResource 
{
public:
    unsigned int compressedSize; // from beginning of block
    char * data;
public:  
    // Loads a resource from current file position
    static CFLResource * load(FILE * f);
    // Saves a resource to current file position
    void save(FILE * f);
    CFLResource();
    ~CFLResource();
};


// Class factory to handle compressors, preprocessors and encryptors.
class CFLResourceHandler 
{
private:
    // Each extended class must have a private static instance of itself, like this:
    static CFLResourceHandler myself; 
protected:
    // As root singleton object this one is created when some other is created.
    static CFLResourceHandler * rootObject;
    // Root of the linked list of resource handlers
    static CFLResourceHandler * root;
    // Pointer to the next handler, or NULL if at end of list.
    CFLResourceHandler * next;
    // Pointer to the printable info string.
    char * infoString;
    // Pointer to a printable short name (10 chars or less preferably)
    char * shortName;
    // Flag: is this handler enabled or not?
    int enabled;
    // Tag of the current handler. Tag format:
    // EPCC
    // |||+- Compressor
    // ||+-- Compressor variant
    // |+--- Preprocessor
    // +---- Encryptor
    // Each handler must leave the rest of the bits to zero (eg. 000Z, 00D0, 0X00)
    unsigned int tag;
protected:
    // Private constructor.
    // Constructor must:
    // - set tag and infostring
    // - call registerHandler(tag,this)
    CFLResourceHandler();
    // Static method to register handler
    static void registerHandler(CFLResourceHandler * handler);
    // process (eg, compress, encrypt..) data 
    virtual void process(char * datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize)=0;
    // reverse process data (eg. decompress, decrypt..) data
    virtual void reverseProcess(char *datain, char ** dataout, unsigned int datainsize, unsigned int & dataoutsize)=0;
    // Finds handler for a tag
    static CFLResourceHandler * findHandler(unsigned int tag);
    // Sets the cipher key.
    virtual void setCipherKey(void * key);
public:
    // destructor.
    virtual ~CFLResourceHandler();
    // decompress, decrypt and deprocess data
    static char * unhandle(const CFLResource *res, unsigned int compressionFlags);
    // compress, encrypt and process data. CompressionFlags may change during handling.
    static CFLResource * handle(const char *data, unsigned int datasize, unsigned int &compressionFlags);
    // get a printable string on a handler.
    static const char * handlerInfoString(unsigned int tag);
    // get a printable short name on a handler.
    static const char * handlerShortName(unsigned int tag);
    // enumerate handlers (for gui use generally)
    static void enumerateHandlers(CFLEnumerator * enumerator);
    // enable or disable handlers (for gui use generally)
    static void enableHandler(unsigned int tag, int state);
    // set key for encrypter
    static void setKey(unsigned int tag, void * key);
};


class CFLLibrary
{
public:
    // file pointer for libraries that are opened with file pointer for some reason.
    // normally NULL; cfl doesn't keep files open for no reason.
    FILE *fileptr;
    // filename to the library, or NULL if fileptr is not NULL.
    char *filename;
    // offset to the library, from start of file (0 for standalone cfl's)
    int offset;
    // pointer to the next library
    CFLLibrary *next;
    CFLLibrary();
    ~CFLLibrary();
    // opens the library file if fileptr is NULL, otherwise returns it
    FILE * open();
    // closes the pointer if fileptr is NULL, otherwise does nothing
    void close(FILE * f);
};


class CFLDirectoryEntry
{
public:
    // name is stored as short int size + char string
    char *name;
    unsigned int compression;
    unsigned int unpackedSize;
    unsigned int offset;
    // these are calculated on the run:
    int nameHash;
    CFLLibrary *lib;
    // pointer to the next entry
    CFLDirectoryEntry *next;
    // methods:
    // used to calculate required buffer size for directory
    int byteRequirement(); 
    // used to encode entry to buffer
    void store(char *buffer, int &offset); 
    // used to load entry from buffer
    static CFLDirectoryEntry * restore(char *buffer, int &offset); 
    ~CFLDirectoryEntry();
    int equals(const char * name, int hash);
    static int calcHash(const char * name);
    // replaces this entry with new one.
    void replaceEntry(CFLDirectoryEntry * replacement);
};


class CFL {
protected:
    CFLLibrary * rootlib;
    CFLDirectoryEntry * rootdir;
    CFLDirectoryEntry * findEntry(const char * name);
public:
    // Constructor
    CFL();
    // Create CFL and open lib from a file (checks beginning and end of file)
    static CFL * create(const char * libfilename);
    // Create CFL and open lib from a file (checks offset and end of file)
    static CFL * create(const char * libfilename, int offset);
    // Create CFL and open lib from a file pointer (pointer must point at start of lib)
    static CFL * create(FILE * libfileptr);
    // Open lib from a file (checks beginning and end of file)
    int openLibrary(const char * libfilename);
    // Open lib from a file (checks offset and end of file)
    int openLibrary(const char * libfilename, int offset);
    // Open lib from a file (checks offset and end of file)
    int openLibrary(FILE * libfileptr);
public:
    // Get file data
    char * getFile(const char *name);
    // Get file data and data length
    char * getFile(const char *name, int &size);
    // Get file into preallocated buffer, returns bytes written
    int getFile(const char * name, char * buffer, int maxsize);
    // Get uncompressed file size in bytes
    int getFileSize(const char *name);  
    // Returns nonzero if file exists in directory
    int fileExists(const char *name);
    // Gets a file pointer at the beginning of (presumably) compressed data
    // client must use dropFilePtr to close the file pointer!
    FILE * getFilePtr(const char *name);
    // drop file pointer
    void dropFilePtr(FILE * f);
    // get file offset to the beginning of (presumably) compressed data
    int getFileOfs(const char *name);
    // return printable string information on compression on this file
    const char * compressionInfoString(const char *name);
    // return printable string information on encryption on this file
    const char * encryptionInfoString(const char *name);
    // return printable string information on preprocessor on this file
    const char * preprocessorInfoString(const char *name);
    ~CFL();
};

class CFLMaker 
{
protected:
    CFLMaker();
    ~CFLMaker();
    CFLDirectoryEntry * rootdir;
    FILE * f;
public:
    static CFLMaker * create(const char *targetfilename);  
    int store(const char *fname, const char *data, int datasize, unsigned int compressionFlags);
    int finish(unsigned int libraryCompressionFlags); 
};

#endif // CFL3_H_INCLUDED