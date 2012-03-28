/*
 *  mxtool.c
 *  Created by Chad Chabot on 12-02-02.
 *	Last updated:	February 13, 2012 12:22:33 AM EST
 *
 *	CIS*2750 W12 - A2
 *	Author:	Chad Chabot
 *			0580345
 *			chad@chadchabot.com
 *			http://www.chadchabot.com/
 */

#ifndef MXTOOL_H_
#define MXTOOL_H_ 1

#define TRUE	1
#define FALSE	0

#include "mxutil.h"


/* command execution functions

   Return values: EXIT_SUCCESS or EXIT_FAILURE (stdlib.h) */

int review( const XmElem *top, FILE *outfile );
int concat( const XmElem *top1, const XmElem *top2, FILE *outfile );
enum SELECTOR { KEEP, DISCARD };
int selects( const XmElem *top, const enum SELECTOR sel, const char *pattern, FILE *outfile );
int libFormat( const XmElem *top, FILE *outfile );
int bibFormat( const XmElem *top, FILE *outfile );


/* helper functions (will be tested individually) */

int match( const char *data, const char *regex );

typedef char *BibData[4];
enum BIBFIELD { AUTHOR=0, TITLE, PUBINFO, CALLNUM };
void marc2bib( const XmElem *mrec, BibData bdata );

void sortRecs( XmElem *collection, const char *keys[] );

int marc21Setup( FILE *inputStream, XmElem **top );
int libOrBibOutput( const XmElem * top, FILE * outfile, int libFormat );
#endif
