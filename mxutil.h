/********
 mxutil.h -- header file for mxutil.c
 Last updated:  26-Jan-12, Version 9001
 
 Chad Chabot
 0580345
 ********/
#ifndef MXUTIL_H
#define MXUTIL_H 1

#include <stdio.h>
#include <libxml/xmlschemastypes.h>

// XmElem is a container for a generic XML element
typedef struct XmElem XmElem;	// lets us avoid coding "struct"
struct XmElem {					// fixed-length container for a generic XML element
    char *tag;					// <tag>
    char *text;					// any text between <tag> and </tag>, or NULL for <tag/>
    int isBlank;				// flag: 0 if text has some non-whitespace
    int nattribs;				// no. of attributes (can be 0)
    char *(*attrib)[][2];		// ->array of attributes (can be NULL)
								//   [0]->attribute; [1]->value
    unsigned long nsubs;		// no. of subelements (can be 0)
    XmElem *(*subelem)[];		// ->array of subelements (can be NULL)
};


// public interface (copied from A1 spec)
xmlSchemaPtr mxInit( const char *xsdfile );
int mxReadFile( FILE *marcxmlfp, xmlSchemaPtr sp, XmElem **top );
int mxFindField( const XmElem *mrecp, int tag );
int mxFindSubfield( const XmElem *mrecp, int tag, int tnum, char sub );
const char *mxGetData( const XmElem *mrecp, int tag, int tnum, char sub, int snum );
void mxCleanElem( XmElem *top );
void mxTerm( xmlSchemaPtr sp );

// internal function (given here because the autotester will call it)
XmElem *mxMakeElem( xmlDocPtr doc, xmlNodePtr node );
int mxWriteFile( const XmElem *top, FILE *mxfile );

#endif
