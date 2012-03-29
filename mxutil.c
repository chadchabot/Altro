/*
 *  mxtool.c
 *  Created by Chad Chabot on 12-02-02.
 *	Last updated:	March 24, 2012
 *
 *	Author:	Chad Chabot
 *			0580345
 *			chad@chadchabot.com
 *			http://www.chadchabot.com/
 */

#define printDebug 0
//	required to use fileno() without warnings
#define _POSIX_SOURCE 1
//	required to use strdup() without warnings 
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "mxutil.h"

/*	helper functions I have defined
 *	explanations are with the definitions below
 */
/*****	helper function prototypes begin *****/
int checkSubelements( XmElem *ep, int tag );
int checkSubelementForCode( XmElem *ep, char sub );
int checkSubelementForCodeCount( XmElem *ep, char sub );
int stringOnlyWhitespace( const char * input );
void initNodeToNull( XmElem * node );
int allNumerals( const char * string );
void printElem( XmElem * ep );
void printElemToFile( const XmElem *ep, FILE * output );
/*****	helper function prototypes end *****/


xmlSchemaPtr mxInit( const char *xsdfile ){
	#ifndef printDebug
		printf( "entering mxInit\n" );
	#endif
	//	what is this/does it do?
	xmlSchemaPtr schema = NULL;

	//	what is this/does it do?
 	xmlSchemaParserCtxtPtr ctxt = NULL;


	LIBXML_TEST_VERSION;
	xmlLineNumbersDefault( 1 );
	
	ctxt = xmlSchemaNewParserCtxt( xsdfile );	
	if( ctxt == NULL ){
		#ifndef printDebug
			printf( "ERROR: ctxt ptr is NULL\n" );
		#endif
		return NULL;
	}

	schema = xmlSchemaParse( ctxt );
	if( schema == NULL ){
		#ifndef printDebug
			printf( "ERROR: schema ptr is NULL" );
		#endif
		return NULL;
	}

	xmlSchemaFreeParserCtxt( ctxt );

	#ifndef printDebug
		printf( "exiting mxInit\n" );
	#endif

	return schema;
}


int mxReadFile( FILE *marcxmlfp, xmlSchemaPtr sp, XmElem **top ){
	
	xmlDocPtr xmlFile 		= NULL;
	xmlSchemaValidCtxtPtr ctxt 	= NULL;
	int fnStatus 		= 0;	/*	status of the function
								 *	0 = successfully read MARCXML file and validated it against the schema
								 *	1 = failed to parse XML file
								 *	2 = XML file did not match schema
								 */
	int validateResult 	= 0;

	#ifndef printDebug
		printf( "entering mxReadFile\n" );
	#endif

	if ( marcxmlfp == NULL ) {
		fnStatus = 0;
		return fnStatus;
	}
	
	//	open XML file to validate
	xmlFile = xmlReadFd( fileno(marcxmlfp), "", NULL, 0 );
	if( xmlFile == NULL ){
		fnStatus = 1;
		#ifndef printDebug
			printf( "xmlReadFd result: NULL\n" );
		#endif
		return fnStatus;
	}
	
	ctxt = xmlSchemaNewValidCtxt( sp );
	if( ctxt == NULL ){
		fnStatus = 1;
		#ifndef printDebug
			printf( "xmlSchemaNewValidCtxt result: 1\n" );
		#endif
		return fnStatus;
	}

	validateResult = xmlSchemaValidateDoc( ctxt, xmlFile );
	if( validateResult == 0 ){
		//	successful, proceed
		#ifndef printDebug
			printf( "xmlFile successfully validated\n" );
		#endif
		fnStatus = 0;
	}
	else if( validateResult > 0 ){
		//	error
		#ifndef printDebug
			printf( "xmlFile did not validate\n" ); 
		#endif
		fnStatus = 2;
	}
	else if( validateResult == -1 ){
		//	API error
		#ifndef printDebug
			printf( "validation API error" ); 
		#endif
		fnStatus = 2;
	}

	//	if fnStatus is not 0, do not go any further, 
	if ( fnStatus == 0 ){
	      //	get document root node
	      //	if root node exists, send it to mxMakeElem( )
	      xmlNodePtr rootNode = NULL;
	      rootNode = xmlDocGetRootElement( xmlFile );
	      if ( rootNode != NULL ){
		      //	send rootNode to mxMakeElem()
		      *top = mxMakeElem( xmlFile, rootNode );
	      }
		xmlFreeDoc( xmlFile );
	}

	if ( ctxt != NULL )
		xmlSchemaFreeValidCtxt( ctxt );
	
	#ifndef printDebug
		printf( "exiting mxReadFile\n" );
	#endif

	return fnStatus;
}

void mxTerm( xmlSchemaPtr sp ){
	xmlSchemaFree( sp );
	xmlSchemaCleanupTypes();
	xmlCleanupParser( );
}


XmElem *mxMakeElem( xmlDocPtr doc, xmlNodePtr node ){
	XmElem *newXmElem 	= NULL;
	newXmElem = malloc( sizeof( XmElem ) );
	assert( newXmElem );
	
//	newXmElem->nattribs 	= 0;
	int i 			= 0;

	initNodeToNull( newXmElem );
	
		
	#ifndef printDebug
		printf( "Entering mxMakeElem()\n" );
		if ( newXmElem->text == NULL)
			printf( "node->text is NULL\n" );
	#endif

	//	check node type
	//	move past any comment nodes
	while( node->type == XML_COMMENT_NODE ){
		//	comment nodes do not have any children
		if( node->next ){
			node = node->next;
		}
		else {
			//	Q: what should I return if the last node is a COMMENT node?
			//	return NULL? By this point newXmElem is allocated, but everything is NULL
			return newXmElem;
		}
	}

	//	get number of children
	newXmElem->nsubs = xmlChildElementCount( node );	
	//	get tag
	newXmElem->tag = strdup( ( const char *) node->name );
	//	get text

	//	replaced
	//	newXmElem->text = strdup( ( const char * ) xmlNodeListGetString( doc, node->xmlChildrenNode, 1 ) );
	//	with the following 4 lines because xmlNodeListGetString() allocates storage space, as does strdup,
	//	and it was causing memory leaks.
	xmlChar * nodeText = NULL;
	nodeText = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	
	//	ERROR from A1
	//	strdup segfaults with a NULL pointer; check for an empty text field in NODE
	if ( nodeText != NULL ){
		newXmElem->text = strdup( ( const char * ) nodeText );
		free( nodeText );
	}else {
		//	if NODE has no text, copy a 
//		newXmElem->text = malloc( sizeof( char ) );
//		strcpy( newXmElem->text, "" );
	}
	/*	TODO:	add conditions to all if statements, leading with value, not variable
	 *			example:	if ( 1 == stringOnlyWhitespace() )
	 */
	if ( stringOnlyWhitespace( (const char * ) newXmElem->text ) ){
		newXmElem->isBlank = 1;
	}
	else{
		newXmElem->isBlank = 0;
	}

	//	REFACTOR THIS!!! This loop is repeated, once to count, then once to grab/assign values.
	//	Can this be reduced to a single traversal of the tree?
	//	get attributes
	int attributeCount = 0;
	xmlAttrPtr attrTempNode = NULL;
	attrTempNode = node->properties;
	while( attrTempNode != NULL ){
		attributeCount++;
		attrTempNode = attrTempNode->next;
	}
	newXmElem->nattribs = attributeCount;

	//	instead of allocating memory at each level, do it at once as part of a block
	newXmElem->attrib = malloc( sizeof( char * ) * 2 * newXmElem->nattribs );	
	assert( newXmElem->attrib );

	//	remember the starting node for the attributes
	attrTempNode = node->properties;
	//  	while there are attributes to be found, get them!
	while( attrTempNode != NULL ){
        	//  get the name of the attribute
		( *newXmElem->attrib )[i][0] = strdup( ( const char * ) attrTempNode->name  );
        	//  get the value of the attribute
		(* newXmElem->attrib )[i][1] = ( char * ) xmlGetProp( node, attrTempNode->name  );
		attrTempNode = attrTempNode->next;
		i++;
	}
	i = 0; 	//	reset to 0 just in case...

	//	now go through and get the sub elements
	if ( newXmElem->nsubs > 0 ){
		//	allocate enough space for subelems
		newXmElem->subelem = malloc( sizeof( XmElem ) * newXmElem->nsubs );
		assert( newXmElem->subelem );
		//	get first child element
		xmlNodePtr firstChild = xmlFirstElementChild( node  );
		//	for each of the sub elements
		//	populate the newXmElem->subelem list
		for( i = 0; i < newXmElem->nsubs ; i++ ){
			(*newXmElem->subelem)[i] = mxMakeElem( doc, firstChild );
			firstChild = xmlNextElementSibling( firstChild );
		}
	}
	i = 0;	//	reset to 0 just in case...
	#ifndef printDebug
		printf( "Exiting mxMakeElem()\n" );
	#endif

	return newXmElem;
}

void mxCleanElem( XmElem *top ){
	int i = 0;

	//	for each subelem, recursively call mxCleanElem()
	//	begin freeing the tree structure from the bottom up
	if ( top->nsubs > 0 ) {
		for ( i = ( int ) top->nsubs ; i > 0 ; i-- ){
			mxCleanElem( ( *top->subelem )[i-1] );
		}
	}

	free( top->subelem );

	//	free text, tag and attrib pointers first
	if ( top->text != NULL ){
		/*	!!!:	two errors
		 *			Address 0x5bfd960 is 0 bytes after a block of size 0 free'd
		 *			Invalid free() / delete / delete[]
		 */
		free( top->text );
		top->text = NULL;	//	seems uneccesary, but was suggested by Ben, just in case...
	}
	if ( top->tag != NULL ){
		/*	!!!:	two errors
		 *			Address 0x5bfd960 is 0 bytes after a block of size 0 free'd
		 *			Invalid free() / delete / delete[]
		 */
		free( top->tag );
		top->tag = NULL;	//	seems uneccesary, but was suggested by Ben, just in case...
	}

	if ( top->nattribs > 0 ){
	//	remove all attributes of top
		for ( i = top->nattribs ; i > 0 ; i-- ){
			if ( ( *top->attrib )[i-1][0] != NULL ){
				free( ( *top->attrib )[i-1][0] );
			}
			if ( ( *top->attrib )[i-1][1] != NULL ){
				free( ( *top->attrib )[i-1][1] );
			}
		}
	}
	//	what about (*top->attrib)[n]? trying to free it was giving me lots of memory leaks
	//	but the line below seems to cover that problem.
	/*	!!!:	two errors
	 *			Address 0x5bfd960 is 0 bytes after a block of size 0 free'd
	 *			Invalid free() / delete / delete[]
	 */
	if ( top->attrib != NULL )
		free( top->attrib );
	
	if ( top != NULL ){
		free( top );
		top = NULL;
	}
}

int mxFindField( const XmElem *mrecp, int tag ){
	int fieldCount 	= 0;
	int i			= 0;
	//	for each subelement of mrcep
	for ( i = 0; i < ( int ) mrecp->nsubs ; i++ ){
		//	send subelement to checkSubelements()
		fieldCount += checkSubelements( ( *mrecp->subelem )[i], tag );
	}
	return fieldCount;
}

int mxFindSubfield( const XmElem *mrecp, int tag, int tnum, char sub ){
	int i, j, k 		= 0;
	int fieldCount 		= 0;
	int tagMatch		= 0;
	int tnumUpperBound 	= 0;

	tnumUpperBound = mxFindField( mrecp, tag );
	//	check to make sure that tnum is within range [ 1:n ]
	//	where n = number of times that tag appears in mrecp
	if ( tnum < 1 || tnum > tnumUpperBound ){
		return 0;
	}
	
	//	tnum is greater than 0; search through subelements
	//	for each subelement of mrcep
	for ( i = 0; i < ( int ) mrecp->nsubs ; i++ ){
		//	send subelement to checkSubelements()
		//	much cleaner than using (*((*mrecp->subelem)[i])->subelem)[j] at the next stage
		//	thanks, Richard!
		XmElem * datafieldPtr = (*mrecp->subelem)[i];
		for ( j = 0; j < datafieldPtr->nattribs; j++ ){
			//	check to make sure that (*datafieldPtr->attrib)[j][1] is in fact all digits
			if ( allNumerals( (*datafieldPtr->attrib)[j][1] ) ){
				if ( atoi( (*datafieldPtr->attrib)[j][1] ) == tag ){
					tagMatch++;
					if ( tagMatch == tnum ){
						//	this is the tnum-th instance of the tag
						//	now look through the subelements of this XmElem, and look for instances of 'sub'
						for ( k = 0; k < datafieldPtr->nsubs; k++)
							fieldCount += checkSubelementForCode( (*datafieldPtr->subelem)[k], sub );
					}
				}
			}
		}
	}

	return fieldCount;
}

////////////////////////////
const char *mxGetData( const XmElem *mrecp, int tag, int tnum, char sub, int snum ){
	int i, j, k		= 0;
	int tagMatch		= 0;
	const char * dataFound 	= NULL;
	int matches		= 0;
	
	//	check if tnum is out of range
	//	tnum < lowerBound OR tnum > upperBound
	if ( tnum < 1 || tnum > mxFindField( mrecp, tag ) )
		return NULL;
	
	//	if tnum > 10 there will be subelements of the XmElem
	//	ensure that the instance of sub that you're looking for is not less than
	//	the lower bound of 1
	if ( tnum > 10 && snum < 1 )
		return NULL;
	
	//	for each subelem in mrecp
	for ( i = 0; i < mrecp->nsubs ; i++ ){
		XmElem * datafieldPtr = (*mrecp->subelem)[i];
		for ( j = 0; j < datafieldPtr->nattribs; j++ ){
			//	CHECK FOR NUMERALS
			if ( allNumerals( (*datafieldPtr->attrib)[j][1] ) ) {
				if ( tag == atoi( (*datafieldPtr->attrib)[j][1] ) ){
					tagMatch++;
					if ( tagMatch == tnum ){
						if ( tag > -1 && tag < 10 ){
							//	get the data from this XmElem, since it is a control field
							dataFound = datafieldPtr->text;
						}
						else {
							XmElem * subFieldPtr = NULL;
							for ( k = 0; k < datafieldPtr->nsubs; k++){
								subFieldPtr = (*datafieldPtr->subelem)[k];
								matches += checkSubelementForCodeCount( subFieldPtr, sub );
								if ( matches == snum ){
#ifndef printDebug
									printf( "#matches = [%d]\ttext = [%s]\t ***\n", matches, subFieldPtr->text );
#endif
									dataFound = subFieldPtr->text;
									return dataFound;
								}
								//								printf( "no matches found\n");
							}
						}
					}
				}
			}
		}
	}
	
	return dataFound;
}

int mxWriteFile( const XmElem *top, FILE *mxfile ){

	//	what are error conditions for mxWriteFile()?
		//	if the file write is interrupted/unable to write because of filesystem change - network resource unavailable, unplugged USB drive, etc.
	if( 0 >= fprintf( mxfile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n" ) )
		return -1;
	fprintf( mxfile, "<!-- Output by mxutil library ( Chad Chabot ) -->\r\n" );
	
	printElemToFile( top, mxfile );
	
	return top->nsubs;
}

/********
 //	used by mxFindFile()
 //	counts the number of nodes that have a tag matching 'tag'
 //	PRE: 	ep is a XmElem with data, tag is an integer from [1:n]
 //	POST:	an integer counting the number of nodes that have a tag matching 'tag'
 ********/
//	TODO:	may be able to refactor this with a FLAG to work with both mxFindField() and mxFindSubfield()?
int checkSubelements( XmElem *ep, int tag ) {
	int matches = 0;
	for ( int i=0; i < ep->nattribs; i++ ){
		//	CHECK FOR NUMERALS
		//	FIXED SUITE #8, ERROR TEST #E2, bug
		if ( strcmp( (*ep->attrib)[i][0], "tag" ) == 0 ){
			if ( allNumerals( (*ep->attrib)[i][1] ) ) {
				if ( atoi( (*ep->attrib)[i][1] ) == tag )
					//			printElem ( ep );
					matches++;
				
			}
		}
	}
	return matches;
}

/********
 //	used by mxFindSubfield()
 //	very similar to checkSubelements()
 //	will hopefully be refactored out in the next step
 //	PRE: 	a newly malloc'd XmElem, and a character
 //	POST:	a count of the number of times that sub shows up in subelement tags
 ********/
int checkSubelementForCode( XmElem *ep, char sub ) {
	int matches 	= 0;
	for ( int i=0; i < ep->nattribs; i++ ){
		//	compare the first character of the attribute value against the character
		if ( (*ep->attrib)[i][1][0] == sub ){
			matches++;
		}
	}
	return matches;
}

/********
 //	used by mxGetData()
 //	checkSubelementForCodeCount() looks at the 
 //	PRE:	ep points to a valid XmElem and ep is not of controlfield type.
 //			sub is a character and snum is an integer that is greater than 9
 //	POST:	a pointer to the character that matches sub.
 ********/
int checkSubelementForCodeCount( XmElem *ep, char sub ) {
	for ( int i=0; i < ep->nattribs; i++ ){
		//	compare the first character of the attribute value against the character
		if ( (*ep->attrib)[i][1][0] == sub ){
			#ifndef printDebug
				printf( "\tfound [%c]!\n", sub );
			#endif
			return 1;
		}
	}
//	printf( "[%s]\n", dataFound );
	return 0;
}


//  	checks to see if a string consists of only whitespace characters
//  	returns 1 if true, 0 if false
//	PRE: 	input is a string that exists somewhere in memory
//	POST:	an int signalling whether input consists of only white space
int stringOnlyWhitespace( const char * input ){
    if ( input == NULL )
	    return 1;
    
    int length = strlen( input );
    int i = 0;
//    printf( "\tchecking [%s] for whitespace\n", input );
    for ( i = 0; i < length; i++ ){
        if( !isspace( input[i] ) ){
//	    printf( "\t\t**** NON WHITESPACE FOUND ****\n" );
            return 0;
        }
    }
    return 1;
}

/********
 //	a silly little helper function to initialize all components
 //	of an XmElem node to NULL or 0, based on field type.
 //	I don't think it's necessary or having an effect
 //	PRE: 	a newly malloc'd XmElem
 //	POST:	an XmElem that has all values set to NULL or 0
 ********/
void initNodeToNull( XmElem * node ){
	node->tag		= NULL;
	node->text		= NULL;
	node->isBlank 	= 0;
	node->nattribs	= 0;
	node->attrib	= NULL;
	node->nsubs		= 0;
	node->subelem	= NULL;
}


/********
 //	checks to make sure that before converting from a string to an integer
 //	– using atoi( string ) – that 'string' does in fact consist of only digits
 //	PRE:	'string' is a valid string
 //			sub is a character and snum is an integer that is greater than 9
 //	POST:	a pointer to the character that matches sub.
 ********/
int allNumerals( const char * string ){
    int i = 0;
	
	//  what if string is of length zero?
    if ( strlen( string ) == 0 )
        return 0;

    for ( i = 0; i < strlen( string ); i++ ){
        if ( !isdigit( string[i] ) ){
			//	found a non-numeric character; FALSE
            return 0;
        }
    }
	//	no non-numeric characters found; TRUE
    return 1;
}


void printElem( XmElem * ep ){
    printf( "[%s] tag\n", ep->tag );
    
    for ( int i=0; i<ep->nattribs; i++ )
      printf( "\t[%s]=[%s]\n", (*ep->attrib)[i][0], (*ep->attrib)[i][1] );
    
    if ( ep->isBlank == 0 )
      printf( "[%s] text\n", ep->text );
}


void printElemToFile( const XmElem *ep, FILE * output )
{
	int static 	depth=0;
	int		subfield = 0;

	for ( int i=0; i<depth; i++ ) fprintf( output, "\t" );
	
	if ( strcmp( ep->tag, "collection" ) == 0 ) {
		fprintf( output, "<marc:collection xmlns:marc=\"http://www.loc.gov/MARC21/slim\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.loc.gov/MARC21/slim\nhttp://www.loc.gov/standards/marcxml/schema/MARC21slim.xsd\">\n" );
	}
	else {
		if ( 0 == strcmp( ep->tag, "subfield" ) || 0 == strcmp( ep->tag, "leader" ) || 0 == strcmp( ep->tag, "controlfield" ) )
			subfield = 1;
		// print tag, attributes, and text	
		fprintf( output, "<marc:%s", ep->tag );
		
		for ( int i=0; i < ep->nattribs; i++ )
			fprintf( output, " %s=\"%s\"", (*ep->attrib)[i][0], (*ep->attrib)[i][1] );
		
		if ( 1 == subfield ) {
/*			fprintf( output, ">%s",
					ep->isBlank ?
					( ep->text ? "" : "" )
					//	special character conversion here.
					: (char *) xmlEncodeSpecialChars( NULL, (const xmlChar *) ep->text ) );
			variable = (char *) xmlEncodeSpecialChars( NULL, (const xmlChar *) ep->text ) );
			
			free( variable );
			fprintf( output, "</marc:%s>\n", ep->tag );
*/
			if ( ep->isBlank ) {
				fprintf( output, ">%s", ep->text ? "" : "" );
			}
			else {
				char * encodedText = NULL;
				encodedText = (char *) xmlEncodeSpecialChars( NULL, (const xmlChar *) ep->text );
				fprintf( output, ">%s", encodedText );
				free( encodedText );
			}
			fprintf( output, "</marc:%s>\n", ep->tag );
		}
		else {
			fprintf( output, ">\n" );
		}	
	}
	
	// print subelements
	for ( int i=0; i < ep->nsubs; i++ ) {
		++depth;
		printElemToFile( (*ep->subelem)[i], output );
		--depth;
	}
	//	if subfield, print on a single line
	if ( subfield != 1 ) {
		for ( int i=0; i<depth; i++ ) fprintf( output, "\t" );
		fprintf( output, "</marc:%s>\n", ep->tag );
	}
}
