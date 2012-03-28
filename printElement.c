/********
printElement.c
Last updated:  3-Jan-12, Version 1

Useful function to print a tree of XmElem and subelements.
Output format:

EL <tag attributes>/text/
 EL... subelements

If isBlank flag is set...
 AND there is some whitespace text (text != NULL) -> prints /BLANK/
 ELSE -> prints /NULL/.
********/

#include "mxutil.h"
#include <string.h>

// print element and its subelements recursively
void printElement( XmElem *ep )
{
  int static depth=0;
/*  
  if ( depth == 0 ) {
    XmElem *rec = (*ep->subelem)[0];
    printf( "TAG/%s/, TEXT(%d)/%s/, ISBLANK/%d/, NATTRIBS/%d/, ATTRIB/%s/, NSUBS/%d/\n\n",
	    rec->tag, strlen(rec->text), rec->text, rec->isBlank, rec->nattribs,
	    (rec->attrib ? "PTR" : "NULL"), rec->nsubs,
	    (rec->subelem ? "PTR " : "NULL") );
  }
*/  
  for ( int i=0; i<depth; i++ ) printf( " " );
  
  // print tag, attributes, and text
  printf( "EL <%s", ep->tag );
 
  for ( int i=0; i < ep->nattribs; i++ )
    printf( " %s=\"%s\"", (*ep->attrib)[i][0], (*ep->attrib)[i][1] );
  
  printf( ">/%s/\n",
	  ep->isBlank ?
	      ( ep->text ? "BLANK" : "NULL" )
	      : ep->text );

  // print subelements
  for ( int i=0; i < ep->nsubs; i++ ) {
    ++depth;
    printElement( (*ep->subelem)[i] );
    --depth;
  }
}