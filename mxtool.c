/*
 *  mxtool.c
 *  Created by Chad Chabot on 12-02-02.
 *	Last updated:	February 13, 2012 23:15 EST
 *
 *	Author:	Chad Chabot
 *			0580345
 *			chad@chadchabot.com
 *			http://www.chadchabot.com/
 */

//	required to use strdup() without warnings 
#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#define mxtoolDebug 1

// #define XSDFILE "MARC21slim.xsd"
/*
 *	remember to run command "export MXTOOL_XSD=MARC21slim.xsd"
 *	prior to running mxtool
 */
#define XSDFILE getenv( "MXTOOL_XSD" )
#define DEVTTY	"/dev/tty"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <regex.h>
#include <assert.h>
#include <ctype.h>

#include "mxtool.h"
#include "mxutil.h"


void printHelp();
void printArgs();

/*	deprecated functions - BEGIN	*/
void bdataAddLastChar( BibData bdata );
char *copyBufferOrDefaultString( char * buffer, const char * defaultString );
/*	deprecated functions - END	*/

char * stripTrailingSpaces( char * string );
char * stripLeadingSpaces( char * string );

int libOrBibOutput( const XmElem * top, FILE * outfile, int libFormat );

int checkPunctuation( char * string );
int marc21Setup( FILE *inputStream, XmElem **top );
void subLastChar( char * string, char * charToAdd );
void printBibInfo( BibData bibInfo );
void freeBibInfo( BibData bibInfo );
static int cmpstringp(const void *p1, const void *p2);
void printElement( XmElem *ep );
void cleanUpMain( XmElem * top1, XmElem * top2 );

int main( int argc, char * argv[] ){
	
	int functionReturnStatus = 0;
	
	//	check to see if there are more than 2 input args
	if ( argc > 3 || argc < 2 ) {
		fprintf( stderr, "In main(): Too %s command line arguments.\n\n", ( argc > 3 ) ? "many": "few" );
		printHelp();
		return EXIT_FAILURE;
	}
	
	FILE * file1	= NULL;
	
	XmElem * top1 = NULL;	//	top node of the first xml file read in
	XmElem * top2 = NULL;	//	top node of the second xml file read in
	
	//	check to see if the first argument matches any of the valid args
	if ( strcmp( argv[1], "-review" ) == 0 ) {
		//	MARK:	-review arg
		//	check for more arguments
		if ( argc != 2 ) {
			//	ERROR if more args
			fprintf( stderr, "Too %s arguments to use -review function; try again\n\n", ( argc > 1 ) ? "many": "few" );
			printHelp();
			functionReturnStatus = EXIT_FAILURE;
		}
		else {
			//	enter Review function
			if ( marc21Setup( stdin, &top1 ) != 1) {
				fprintf( stderr, "In main(): unable to open [%s] schema file.\n", XSDFILE );			
				return EXIT_FAILURE;
			}
			else {
				functionReturnStatus = review( top1, stdout );
			}			
		}
	}else if ( strcmp( argv[1], "-cat" ) == 0 ) {
		//	MARK:	-cat tool
		if ( argc != 3 ) {
			fprintf( stderr, "Too %s arguments to use -cat function; try again\n\n", ( argc > 1 ) ? "many": "few" );
			printHelp();
		}
		else {
			file1	= fopen( argv[2], "r" );
			if ( file1 == NULL ) {
				fprintf( stderr, "In main(): unable to open file [%s].\n", argv[2] );
				functionReturnStatus = EXIT_FAILURE;
			}
			else {
				if ( marc21Setup( stdin, &top1 ) != 1) {
					fprintf( stderr, "In main(): unable to open [%s] schema file.\n", XSDFILE );			
					functionReturnStatus = EXIT_FAILURE;
				}
				else {
					//	two xml files coming in
					//	one from the command line arg
					//	one from stdin ( via piping and redirection )
					if ( marc21Setup( file1, &top2 ) != 1 ){
						functionReturnStatus = EXIT_FAILURE;
					}
					else {
						functionReturnStatus = concat( top1, top2, stdout );
					}
				}
				fclose( file1 );
			}
		}
	}else if ( strcmp( argv[1], "-keep" ) == 0 ){
		//	MARK:	-keep tool
		//		printf("\tkeep tool\n");
		if ( argc != 3 ) {
			fprintf( stderr, "Too %s arguments to use -keep function; try again\n\n", ( argc > 1 ) ? "many": "few" );
			printHelp();
			functionReturnStatus = EXIT_FAILURE;
		}else {
			//	setup XSD files and read in from pipe
			if ( marc21Setup( stdin, &top1 ) != 1) {
				fprintf( stderr, "In main(): unable to open [%s] schema file.\n", XSDFILE );			
				functionReturnStatus = EXIT_FAILURE;
			}else {
				functionReturnStatus = selects( top1, KEEP, argv[2], stdout );
			}
		}
	}else if ( strcmp( argv[1], "-discard" ) == 0 ) {
		//	MARK:	-discard tool
		//		printf("\tdiscard tool\n");
		if ( argc != 3 ) {
			fprintf( stderr, "Too %s arguments to use -discard function; try again\n\n", ( argc > 1 ) ? "many": "few" );
			printHelp();
			functionReturnStatus = EXIT_FAILURE;
		}else {
			//	setup XSD files and read in from pipe
			if ( marc21Setup( stdin, &top1 ) != 1) {
				//	TODO: exit?
				fprintf( stderr, "In main(): unable to open [%s] schema file.\n", XSDFILE );			
				functionReturnStatus = EXIT_FAILURE;
			}else {
				functionReturnStatus = selects( top1, DISCARD, argv[2], stdout );
			}
		}
	}else if ( strcmp( argv[1], "-bib" ) == 0 ) {
		//	MARK:	-bib tool
		if ( argc != 2 ) {
			fprintf( stderr, "Too %s arguments to use -bib function; try again\n\n", ( argc > 1 ) ? "many": "few" );
			printHelp();
			functionReturnStatus = EXIT_FAILURE;
		}else {
			if ( marc21Setup( stdin, &top1 ) != 1 ){
				fprintf( stderr, "In main(): unable to open [%s] schema file.\n", XSDFILE );
				functionReturnStatus = EXIT_FAILURE;
			}
			else {
				functionReturnStatus = bibFormat( top1, stdout );
			}
		}
	}else if ( strcmp( argv[1], "-lib" ) == 0 ) {
		//	MARK:	-lib tool
		if ( argc != 2 ) {
			fprintf( stderr, "Too %s arguments to use -lib function; try again\n\n", ( argc > 1 ) ? "many": "few" );
			printHelp();
		}
		else {
			if ( marc21Setup( stdin, &top1 ) != 1 ){
				fprintf( stderr, "In main(): unable to open [%s] schema file.\n", XSDFILE );
				functionReturnStatus = EXIT_FAILURE;
			}
			else {
				functionReturnStatus = libFormat( top1, stdout );
			}
		}
	}else {
		//	unrecognized input
		fprintf( stderr, "In main():\tunrecognized command line arguments.\n" );
		printHelp();
		functionReturnStatus = EXIT_FAILURE;
	}
	cleanUpMain( top1, top2 );
	return functionReturnStatus;
}


int review( const XmElem *top, FILE *outfile ){
	if ( top == NULL || outfile == NULL ) {
		fprintf( stderr, "In review(): %s.\n", top == NULL ? "XmElem *top is NULL": "FILE *outfile is NULL" );
		return EXIT_FAILURE;
	}
	
	FILE	*input		= fopen( DEVTTY, "r");   //open the terminal keyboard
	FILE	*output		= fopen( DEVTTY, "w");   //open the terminal screen
	
	if ( input == NULL || output == NULL ) {
		fprintf( stderr, "In review(): unable to open %s for %s.\n", DEVTTY, input == NULL ? "reading": "writing" );
		return EXIT_FAILURE;
	}
	
	system("stty -F /dev/tty -icanon"); 
	system("stty -F /dev/tty -echo");
	
	char	keyPressed;
	int		writeResult		= 0;		//	how many records were written to outfile
	int		moreRecords		= FALSE;
	int		currentSubelem	= 0;		//	tracks which subelem in top is active
	int		subelemsOutput	= 0;		//	tracks how many subelems have been written out
	int		returnStatus	= EXIT_SUCCESS;
	
	BibData	bibInfo;
	
	if ( top->nsubs > 0)
		moreRecords = TRUE;
	
	
	/*	save a new, temporary top which will have the exact
	 *	same values for its internal members, 
	 *	with the possibility of nsubs and subelem being different
	 *	based on the user's selections.
	 */
	XmElem * tempTop	= malloc( sizeof( XmElem ) );
	assert( tempTop );
	tempTop->tag		= top->tag;
	tempTop->text		= top->text;
	tempTop->isBlank	= top->isBlank;
	tempTop->nattribs	= top->nattribs;
	tempTop->attrib		= top->attrib;
	tempTop->nsubs		= top->nsubs;
	tempTop->subelem	= malloc( top->nsubs * sizeof( XmElem * ) );
	assert( tempTop->subelem );
	/*	TODO:	Is having the moreRecords flag a good idea?
	 *			Is there a better way to do this
	 *				- count how many records are there, and how many have been viewed?
	 *				- just keep going while( record->next ) or something similar?
	 */
	while ( moreRecords ) {
		marc2bib( ( *top->subelem )[currentSubelem], bibInfo );
		if ( bibInfo == NULL ) {
			//	there was an error, jump out?!
			//	TODO:	exit?
			returnStatus = EXIT_FAILURE;
		}else{
			fprintf( output, "%d. %s %s %s\n\n",currentSubelem +1, bibInfo[ AUTHOR ], bibInfo[ TITLE ], bibInfo[ PUBINFO ] );
			keyPressed = getc( input );
			//	echo keyPressed value for testing
			//		printf( "\t[%c]=>[%d]\n", keyPressed, (int) keyPressed );
			//	act on the keyPressed by the user
			switch ( (int) keyPressed ) {
				case 107:
					//	user pressed 'k'
					//	write out all remaining records to outfile
					//	do not show any more to stdout
					while ( currentSubelem < top->nsubs ) {
						( *tempTop->subelem )[subelemsOutput] = ( *top->subelem )[currentSubelem];
						subelemsOutput++;
						currentSubelem++;						
					}
					moreRecords = FALSE;
					break;
				case 100:
					//	user pressed 'd'
					//	discard all remaining records
					while ( currentSubelem < top->nsubs ) {
						currentSubelem++;						
					}
					moreRecords = FALSE;
					break;
				case 32:
					//	user pressed 'space'
					//	skip this record
					currentSubelem++;
					break;
				case 10:
					//	user pressed 'enter'
					//	copy record to STDOUT
					//	will be printed to outfile
					( *tempTop->subelem )[subelemsOutput] = ( *top->subelem )[currentSubelem];
					subelemsOutput++;
					currentSubelem++;
					break;
				default:
					//	display help keys
					fprintf( output, "\n%s%s%s%s%s%s",
							"Not a valid option. Your choices are:\n",
							"Enter: record is copied to stdout.\n",
							"Spacebar: record is skipped.\n",
							"'k' (keep): remaining records are copied and the program exits.\n",
							"'d' (discard): remaingin records are skipped and the program exits.\n",
							"Any other key displays help (lists these controls), the redisplays the current record.\n"
							);
					break;
			}
		}
		if ( bibInfo != NULL )
			freeBibInfo( bibInfo );
		if ( currentSubelem >= top->nsubs )
			moreRecords = FALSE;
	}
	/*	clean up tempTop;
	 *	the number of subelems ptrs allocated = nsubs,
	 *	but it has only subelemsOutput of them populated
	 *	realloc that many?
	 */
	tempTop->nsubs		= subelemsOutput;
	tempTop->subelem	= realloc( tempTop->subelem, tempTop->nsubs * sizeof( XmElem * ) );
	writeResult = mxWriteFile( tempTop, outfile );
	if ( -1 == writeResult )
		returnStatus = EXIT_FAILURE;
	if ( input != NULL )
		fclose( input );
	if ( output != NULL )
		fclose( output );
	//	TODO:	free some memory! tempTop and tempTop->subelem at least?
	
	system("stty -F /dev/tty icanon"); 
	system("stty -F /dev/tty echo");
	return returnStatus;
}

/*	prints out the accepted command line argument formats
 *
 *	PRE		the entered command line args are incorrect
 *	POST	none
 */
void printHelp(){
	fprintf( stderr, "NAME\n\tmxtool -- open and act on MARCXML files to export reference lists\n\n" );
	fprintf( stderr, "SYNOPSIS\n\tmxtool [-OPTIONS [PARAMETERS] ]\n\n" );
	fprintf( stderr, "OPTIONS\n" );
	fprintf( stderr, "\t-review\n" );
	fprintf( stderr, "\t-cat XMLFILENAME\n" );
	fprintf( stderr, "\t-keep PATTERN\n" );
	fprintf( stderr, "\t-discard PATTERN\n" );
	fprintf( stderr, "\t-lib\n" );
	fprintf( stderr, "\t-bib\n" );
}

/*	prints out the command line args that the user has entered
 *
 */	
void printArgs( int numArgs, char * args[] ){
	int i = 0;
	for( i = 0; i < numArgs; i++ ){
		printf( "%s ", args[i] );
	}
	printf( "\n" );
}


/*	Used to parse and validate the xmlSchema as well as
 *	read in and validate an XML file against the schema
 *	
 *	PRE		inputStream is open for reading
 *			top is an un-allocated XmElem pointer
 *	POST	Returns 0 (false) if either the schema is not valid
 *			or if mxReadFile was unable to validate the XML file.
 *			Returns 1 (true) if successful
 */
int marc21Setup( FILE *inputStream, XmElem **top ){
	int mf = 0;
	
	//	get MARC 21 XML schema pattern
	//	and make sure it is valid
	xmlSchemaPtr schema = mxInit( XSDFILE );
	if ( schema == NULL ) {
		//	TODO: exit()? or return bad value?
#ifndef mxtoolDebug
		printf( "schema is NULL\n" );
#endif
		fprintf( stderr, "In marc21Setup(): schema is NULL, unable to validate schema.\n" );
		return 0;
	}
#ifndef mxtoolDebug
	printf( "schema is not NULL\n" );
#endif
	//	compare input file against schema against 
	//	if it checks out, continue
	mf = mxReadFile( inputStream, schema, top );
	if ( mf != 0 ) {
#ifndef mxtoolDebug
		printf( "mxReadFile() failed\n");
#endif
		//	TODO:	exit()? or return bad value?
		fprintf( stderr, "marc21Setup(): mxReadFile() returned [%d]. reason: \"%s\".\n", mf, mf == 1? "failed to parse XML file" : "XML file did not match schema" );
		//	exit( 1 );
		return 0;
	}
#ifndef mxtoolDebug
	printf( "mxReadFile() success\n");
#endif
	mxTerm( schema );
	
	return 1;
}

void marc2bib( const XmElem * mrec, BibData bdata ){
	char *	stringA		= NULL;
	char *	stringB		= NULL;
	char *	stringC		= NULL;
	char *	stringP		= NULL;
	char *	string4		= NULL;
	
	//	check for AUTHOR
	if ( 0 < mxFindField( mrec, 100 ) ) {
		if ( 0 < mxFindSubfield( mrec, 100, 1, 'a' ) ){
			stringA = (char * ) mxGetData( mrec, 100, 1, 'a', 1 );
		} else {
			stringA = NULL;
		}
		asprintf( &bdata[ AUTHOR ], "%s", NULL == stringA ? "" : stringA );
		
	} else if ( 0 < mxFindField( mrec, 130 ) ) {
		if ( 0 < mxFindSubfield( mrec, 130, 1, 'a' ) ){
			stringA = (char * ) mxGetData( mrec, 130, 1, 'a', 1 );
		} else {
			stringA = NULL;
		}
		asprintf( &bdata[ AUTHOR ], "%s", NULL == stringA ? "" : stringA );
	} else {
		asprintf( &bdata[ AUTHOR ], "%s", "na" );
	}
	
	//	check for TITLE
	if ( 0 < mxFindField( mrec, 245 ) ) {
		if ( 0 < mxFindSubfield( mrec, 245, 1, 'a' ) ){
			stringA = (char * ) mxGetData( mrec, 245, 1, 'a', 1 );
		} else {
			stringA = NULL;
		}
		
		if ( 0 < mxFindSubfield( mrec, 245, 1, 'p' ) ){
			stringP = (char * ) mxGetData( mrec, 245, 1, 'p', 1 );
		} else {
			stringP = NULL;
		}
		
		if ( 0 < mxFindSubfield( mrec, 245, 1, 'b' ) ){
			stringB = (char * ) mxGetData( mrec, 245, 1, 'b', 1 );
		} else {
			stringB = NULL;
		}
		asprintf( &bdata[ TITLE ], "%s%s%s", NULL == stringA ? "" : stringA,
				 NULL == stringP ? "" : stringP,
				 NULL == stringB ? "" : stringB );
	} else {
		asprintf( &bdata[ TITLE ], "%s", "na" );
	}
	
	//	check for PUBINFO
	if ( 0 < mxFindField( mrec, 260 ) || 0 < mxFindField( mrec, 250 ) ) {
		if ( 0 < mxFindSubfield( mrec, 260, 1, 'a' ) ){
			stringA = (char * ) mxGetData( mrec, 260, 1, 'a', 1 );
		} else {
			stringA = NULL;
		}
		
		if ( 0 < mxFindSubfield( mrec, 260, 1, 'b' ) ){
			stringB = (char * ) mxGetData( mrec, 260, 1, 'b', 1 );
		} else {
			stringB = NULL;
		}
		
		if ( 0 < mxFindSubfield( mrec, 260, 1, 'c' ) ){
			stringC = (char * ) mxGetData( mrec, 260, 1, 'c', 1 );
		} else {
			stringC = NULL;
		}
		
		if ( 0 < mxFindSubfield( mrec, 250, 1, 'a' ) ){
			string4 = (char * ) mxGetData( mrec, 250, 1, 'a', 1 );
		} else {
			string4 = NULL;
		}
		asprintf( &bdata[ PUBINFO ], "%s%s%s%s", NULL == stringA ? "" : stringA,
				 NULL == stringB ? "" : stringB,
				 NULL == stringC ? "" : stringC,
				 NULL == string4 ? "" : string4);
		
	} else {
		asprintf( &bdata[ PUBINFO ], "%s", "na" );
	}
	
	
	//	check for CALLNUM
	if ( 0 < mxFindField( mrec, 90 ) ) {
		if ( 0 < mxFindSubfield( mrec, 90, 1, 'a' ) ){
			stringA = (char * ) mxGetData( mrec, 90, 1, 'a', 1 );
		} else {
			stringA = NULL;
		}
		
		if ( 0 < mxFindSubfield( mrec, 90, 1, 'b' ) ){
			stringB = (char * ) mxGetData( mrec, 90, 1, 'b', 1 );
		} else {
			stringB = NULL;
		}
		asprintf( &bdata[ CALLNUM ], "%s%s", NULL == stringA ? "" : stringA,
				 NULL == stringB ? "" : stringB );
	} else if ( 0 < mxFindField( mrec, 50 ) ) {
		if ( 0 < mxFindSubfield( mrec, 50, 1, 'a' ) ){
			stringA = (char * ) mxGetData( mrec, 50, 1, 'a', 1 );
		} else {
			stringA = NULL;
		}
		
		if ( 0 < mxFindSubfield( mrec, 50, 1, 'b' ) ){
			stringB = (char * ) mxGetData( mrec, 50, 1, 'b', 1 );
		} else {
			stringB = NULL;
		}
		asprintf( &bdata[ CALLNUM ], "%s%s", NULL == stringA ? "" : stringA,
				 NULL == stringB ? "" : stringB );
	} else {
		asprintf( &bdata[ CALLNUM ], "%s", "na" );
	}
}



/*	Funnels all members of a BibData struct to subLastChar()
 *	to check for/add a "." at the end of each field.
 *	
 *	PRE		bdata is an allocated and filled BibData struct
 *	POST	bdata's members are altered based on what happened
 *			in subLastChar().
 */
void bdataAddLastChar( BibData bdata ){
	char period[] = ".";
	subLastChar( bdata[ TITLE ], period );
	subLastChar( bdata[ AUTHOR ], period );
	subLastChar( bdata[ CALLNUM ], period );
	subLastChar( bdata[ PUBINFO ], period );
}

/********
 *	A poorly named function that will check from the last printable character
 *	of string is the character specifed by charToAdd.
 *	If charToAdd is not found, it will add it at the end of string.
 *	Only called by bdataAddLastChar.
 *
 *	PRE:	string is not NULL and of length > 1
 *			charToAdd is a char, and not NULL
 *	POST:	string will have an ending character which matches charToAdd
 ********/
void subLastChar( char * string, char * charToAdd ){
	//	make sure that the character is actually only one character long
	if ( strlen( charToAdd ) == 1 ) {
		int i	= 0;
		for ( i = ( strlen( string ) -1 ) ; i >= 0; i-- ) {
			if ( string[i] == charToAdd[0] ) {
				break;
			}
			else {
				//	!!!:	invalid write of size 1
				if ( (int)string[i] > 31 && (int)string[i] < 127) {
					string = strcat( string, charToAdd );
					break;
				}
			}
		}
	}
	else {
		//	print to stderr?
		//	no return value required… but I should do something?
	}
}

/********
 *	Will take in a bibInfo object (should it be a BibData * instead?)
 *	and free each of the components, provided the have not already been freed
 *	
 *	PRE:	bibInfo is not NULL
 *	POST:	all members of bibInfo have been free'd
 ********/
void freeBibInfo( BibData bibInfo ){ 
	if ( bibInfo[ AUTHOR ] != NULL )
		free( bibInfo[ AUTHOR ] );
	if ( bibInfo[ TITLE ] != NULL )
		free( bibInfo[ TITLE ] );
	if ( bibInfo[ CALLNUM ] != NULL )
		free( bibInfo[ CALLNUM ] );
	if ( bibInfo[ PUBINFO ] != NULL )
		free( bibInfo[ PUBINFO ] );
}

/********
 *	Takes in two strings, and will return either buffer or defaultString
 *	based on whether buffer is NULL or not.
 *	
 *	PRE:	buffer is a char * that may or may not be NULL
 *			defaultString is a char * that is not NULL.
 *	POST:	a non NULL char * is returned.
 ********/
char *copyBufferOrDefaultString( char * buffer, const char * defaultString ){
	/*	TODO:	pre- and post-conditions
	 *			usage & reason for existing
	 *	???:	this returns gibberish. Fix before using
	 */
	if ( buffer != NULL ) {
		return buffer;
	}else {
		return ( char * ) defaultString;
	}
}

/********
 *	Prints out, with field names, all members of a BibData struct
 *	
 *	PRE:	bibInfo is not NULL
 *	POST:	stuff is printed out…
 ********/
void printBibInfo( BibData bibInfo ){
	//	TODO:	pre and post conditions; usage
	printf( "title: [%s]\nauthor: [%s]\npubinfo: [%s]\ncallnum: [%s]\n",
		   bibInfo[ TITLE ]		== NULL ? "NULL" : bibInfo[ TITLE ],
		   bibInfo[ AUTHOR ]	== NULL ? "NULL" : bibInfo[ AUTHOR ],
		   bibInfo[ PUBINFO ]	== NULL ? "NULL" : bibInfo[ PUBINFO ],
		   bibInfo[ CALLNUM ]	== NULL ? "NULL" : bibInfo[ CALLNUM ]);
}

int match( const char * data, const char * regex ){
	if ( data == NULL || regex == NULL ) {
		//	should never exit here, but just in case
		fprintf( stderr, "In match():\t%s was a NULL string\n", data == NULL ? "DATA" : "REGEX" );
		return 0;
	}	
	regex_t pattern;
	int		match		= 0;
	
	//	compile regular expression
	if ( regcomp( &pattern, regex, 0 ) == 0 ) {
		//	execute regex search
		match = regexec( &pattern, data, 0, NULL, 0);
		
		if ( !match ) {
			regfree( &pattern );
			return 1;
		}
		regfree( &pattern );
		return 0;
	}
	else {
		//	unable to compile regular expression
		//	no error message required! Handled by calling function.
		fprintf( stderr, "In match():\tunable to compile regular express from \"%s\" string\n", regex );
		return 0;
	}
}

int concat( const XmElem *top1, const XmElem *top2, FILE *outfile ){
	//	take the records from top1 and top2, and add them to a new XmElem * joinedTop
	//	create a new top, joinedTop
	//	copy the elements of top1 to joinedTop
	//	create a new collection element – subelem[0] of joinedTop – with the same contents as (*top1->subelem)[0]
	int i = 0;
	int j = 0;
	int numRecordsWritten = 0;
	
	XmElem * joinedTop	= malloc( sizeof( XmElem ) );
	assert( joinedTop );
	joinedTop->tag		= top1->tag;
	joinedTop->text		= top1->text;
	joinedTop->isBlank	= top1->isBlank;
	joinedTop->nattribs	= top1->nattribs;
	joinedTop->attrib	= top1->attrib;
	joinedTop->nsubs	= top1->nsubs + top2->nsubs;
	joinedTop->subelem	= malloc( ( top1->nsubs + top2->nsubs ) * sizeof( XmElem * ) );
	assert( joinedTop->subelem );
	//	add records from top1
	for ( i = 0; i < top1->nsubs; i++ )
		( *joinedTop->subelem )[i] = (* top1->subelem )[i];
	
	//	add records from top2
	for ( j = 0; j < top2->nsubs; j++, i++ )
		( *joinedTop->subelem )[i] = ( *top2->subelem )[j];
	
	//	now all records are part of joinedTop
	numRecordsWritten = mxWriteFile( joinedTop, outfile );
	
	/*	the only thing malloc'd in this function is the array of XmElem ptrs
	 *	this is freeing the array, not the array members:
	 *	those are still "owned" and reachable via top1, top2.
	 */
	if ( NULL != joinedTop->subelem )
		free( joinedTop->subelem );
	
	if ( numRecordsWritten == joinedTop->nsubs ) {
		if ( NULL != joinedTop )
			free( joinedTop );
		return EXIT_SUCCESS;
	}else {
		if ( NULL != joinedTop )
			free( joinedTop );
		return EXIT_FAILURE;
	}
}

int selects( const XmElem *top, const enum SELECTOR sel, const char *pattern, FILE *outfile ){
	
	//	if no records exist to match against
	//	technically, this isn't an error…
	//	so EXIT_FAILURE would be incorrect
	if ( top->nsubs == 0){
		return EXIT_SUCCESS;
	}
	if ( pattern[1] != '=' ) {
		fprintf( stderr, "In selects():\tregex pattern [%s] does not conform to format %s.\n",
				pattern,
				"\"<field>=<regex>\"\n\t\twhere <field> is (a)uthor, (p)ublisher, or (t)tile" );
		return EXIT_FAILURE;
	}
	
	char * regexString		= (char *)(pattern+2);	//	use pointer arithmetic to avoid copying the regex pattern
	int fieldToFind			= (int) pattern[0];
	int writeResult			= 0;
	int matchFound			= 0;
	int	subelemsRead		= 0;
	int	subelemsWriting		= 0;
	int subelemCounter		= 0;
	
	BibData	bibInfo;
	/*	save a new, temporary top which will have the exact
	 *	same values for its internal members, 
	 *	with the possibility of nsubs and subelem being different
	 *	based on the user's selections.
	 */
	XmElem * tempTop	= malloc( sizeof( XmElem ) );
	assert( tempTop );
	tempTop->tag		= top->tag;
	tempTop->text		= top->text;
	tempTop->isBlank	= top->isBlank;
	tempTop->nattribs	= top->nattribs;
	tempTop->attrib		= top->attrib;
	tempTop->nsubs		= top->nsubs;
	tempTop->subelem	= malloc( top->nsubs * sizeof( XmElem * ) );
	assert( tempTop->subelem );
	switch ( fieldToFind ) {
		case 97:
			//	looking for author
			/*	TODO:	clean this up; it should be refactored into a more compact function
			 */
			for ( subelemCounter = 0; subelemCounter < top->nsubs ; subelemCounter++) {
				marc2bib( ( *top->subelem)[ subelemCounter ], bibInfo );
				if ( bibInfo == NULL ) {
					//	clean up tempTop
					return EXIT_FAILURE;
				}
				//				printBibInfo( bibInfo );
				matchFound = match( bibInfo[ AUTHOR ], regexString );
				if ( matchFound != 0 ) {
					if ( sel == KEEP ) {
						//	add this record to tempTop, at position subelemsWriting
						( *tempTop->subelem )[ subelemsWriting ] = ( *top->subelem )[ subelemCounter ];
						//	increment numRecordsToWrite
						subelemsWriting++;
					}
					//
				}else {
					//	if sel == KEEP
					//	do nothing
					if ( sel == DISCARD ) {
						//	add this record to tempTop, at position subelemsWriting
						( *tempTop->subelem )[ subelemsWriting ] = ( *top->subelem )[ subelemCounter ];
						//	increment numRecordsToWrite
						subelemsWriting++;
					}
				}
				subelemsRead++;
				if ( bibInfo != NULL )
					freeBibInfo( bibInfo );
			}
			break;
		case 116:
			//	looking for author
			for ( subelemCounter = 0; subelemCounter < top->nsubs ; subelemCounter++) {
				marc2bib( ( *top->subelem)[ subelemCounter ], bibInfo );
				if ( bibInfo == NULL ) {
					//	clean up tempTop
					return EXIT_FAILURE;
				}
				//				printBibInfo( bibInfo );
				matchFound = match( bibInfo[ TITLE ], regexString );
				if ( matchFound != 0 ) {
					if ( sel == KEEP ) {
						//	add this record to tempTop, at position subelemsWriting
						( *tempTop->subelem )[ subelemsWriting ] = ( *top->subelem )[ subelemCounter ];
						//	increment numRecordsToWrite
						subelemsWriting++;
					}
					//
				}else {
					//	if sel == KEEP
					//	do nothing
					if ( sel == DISCARD ) {
						//	add this record to tempTop, at position subelemsWriting
						( *tempTop->subelem )[ subelemsWriting ] = ( *top->subelem )[ subelemCounter ];
						//	increment numRecordsToWrite
						subelemsWriting++;
					}
				}
				subelemsRead++;
				if ( bibInfo != NULL )
					freeBibInfo( bibInfo );
			}
			break;
		case 112:
			//	looking for author
			for ( subelemCounter = 0; subelemCounter < top->nsubs ; subelemCounter++) {
				marc2bib( ( *top->subelem)[ subelemCounter ], bibInfo );
				if ( bibInfo == NULL ) {
					//	clean up tempTop
					return EXIT_FAILURE;
				}
				//printBibInfo( bibInfo );
				matchFound = match( bibInfo[ PUBINFO ], regexString );
				if ( matchFound != 0 ) {
					if ( sel == KEEP ) {
						//	add this record to tempTop, at position subelemsWriting
						( *tempTop->subelem )[ subelemsWriting ] = ( *top->subelem )[ subelemCounter ];
						//	increment numRecordsToWrite
						subelemsWriting++;
					}
					//
				}else {
					//	if sel == KEEP
					//	do nothing
					if ( sel == DISCARD ) {
						//	add this record to tempTop, at position subelemsWriting
						( *tempTop->subelem )[ subelemsWriting ] = ( *top->subelem )[ subelemCounter ];
						//	increment numRecordsToWrite
						subelemsWriting++;
					}
				}
				subelemsRead++;
				if ( bibInfo != NULL )
					freeBibInfo( bibInfo );
			}
			break;
		default:
			//	unrecognized search field
			fprintf( stderr, "In selects():\tregex pattern [%s] does not conform to format %s.\n",
					pattern,
					"\"<field>=<regex>\"\n\t\twhere <field> is (a)uthor, (p)ublisher, or (t)tile" );
			return EXIT_FAILURE;
			break;
	}
	
	//	now realloc the size of tempTop->subelem
	tempTop->nsubs		= subelemsWriting;
	tempTop->subelem	= realloc( tempTop->subelem, tempTop->nsubs * sizeof( XmElem * ) );
	writeResult = mxWriteFile( tempTop, outfile );
	
	/*	???:	why do the following 4 lines give
	 *			me invalid reads?
	 */
	if ( tempTop->subelem != NULL )
		free( tempTop->subelem );
	if ( tempTop != NULL )
		free( tempTop );
	
	return EXIT_SUCCESS;
}

int checkPunctuation( char * string ) {
	int i = strlen( string ) - 1;
	
	while ( i >= -1 ) {
		if ( isspace( string[i] ) ){
			i--;
		}
		else {
			if ( !ispunct( string[i] ) ) {
				//				printf( "no punctuation in [%s]\n", string );
				return 1;
			}
			else {
				//				printf("punctuation in [%s] @ [%d] position: [%c]\n", string, i + 1, string[i] );
				return 0;
			}
		}
	}
	//	printf( "no punctuation in [%s]\n", string );
	return 1;
}

char * stripTrailingSpaces( char * string ){
	int i = strlen( string ) - 1;
	char * outputString = strdup( string );
	//	TODO:	figure out how to copy from the input string to the output string
	while ( 0 <= i) {
		if ( !isspace( outputString[i] ) ){
			break;
		} else {
			outputString[i] = '\0';
			i--;
		}
	}
	return outputString;
}

/*	start by using pointer arithmatic to move starting position of sting forward,
 *	send string+offset to strip trailing spaces, and then copy/return the first N characters in string+offset
 *
 */
char * stripLeadingSpaces( char * string ){
	int i = 0;
	while ( 0 < strlen( string ) ) {
		if ( !isspace( string[i] ) )
			break;
		else
			i++;
	}
	return stripTrailingSpaces( string+i );
}

int bibFormat( const XmElem *top, FILE *outfile ){
	return libOrBibOutput( top, outfile, 0 );
}

int libFormat( const XmElem *top, FILE *outfile ){
	return libOrBibOutput( top, outfile, 1 );
}

int libOrBibOutput( const XmElem * top, FILE * outfile, int libFormat ){
	
	if ( top == NULL ) {
		fprintf( stderr, "In %s(): parameter XmElem * top was NULL.\n", 1 == libFormat ? "libFormat" : "bibFormat" );
		return EXIT_FAILURE;
	}
	if ( outfile == NULL ) {
		fprintf( stderr, "In %s(): parameter FILE * outfile was NULL.\n", 1 == libFormat ? "libFormat" : "bibFormat" );
		return EXIT_FAILURE;
	}
	
	const char *keys[top->nsubs];
	BibData bibInfo;
	
	int i = 0;
	int numRecords = top->nsubs;
	
	//	get bibInfo for each record
	for ( i = 0; i < numRecords; i++ ) {
		marc2bib( (*top->subelem)[i], bibInfo );
		if ( bibInfo == NULL ) {
			fprintf( stderr, "Error generating bibInfo of [%d]th subelem in %s\n", i, 1 == libFormat ? "libFormat" : "bibFormat" );
			return EXIT_FAILURE;
		}
		if ( 1 == libFormat ) {
			keys[ i ] = strdup( bibInfo[ CALLNUM ] );
		}else {
			keys[ i ] = strdup( bibInfo[ AUTHOR ] );
		}
		
		//		printBibInfo( bibInfo );
		freeBibInfo ( bibInfo );
		//		printf( "*[%s] key created\n", keys[ i ]);
	}
	
	//	don't send top in to sortRecs. Copy all of top's direct members, no descendants.
	//	TODO:	create duplicateXmElem() function in mxutil.c
	//	duplicate XmElem node
	XmElem * sortedTop = malloc( sizeof( XmElem ) );
	assert( sortedTop );
	sortedTop->tag		= top->tag;
	sortedTop->text		= top->text;
	sortedTop->isBlank	= top->isBlank;
	sortedTop->nattribs	= top->nattribs;
	sortedTop->attrib	= top->attrib;
	sortedTop->nsubs	= top->nsubs;
	sortedTop->subelem	= malloc( top->nsubs * sizeof( XmElem * ) );
	
	for ( i = 0; i < sortedTop->nsubs; i++) {
		( *sortedTop->subelem )[ i ] = ( *top->subelem )[ i ];
	}
	
	sortRecs( sortedTop, keys );
	//	free the keys array
	for (i = 0; i < numRecords; i++)
		free( (char * )keys[i] );
	
	//	output marc2Bib records and echo them
	for ( i = 0; i < numRecords; i++ ) {
		marc2bib( (*sortedTop->subelem)[i], bibInfo );
		if ( bibInfo == NULL ) {
			return EXIT_FAILURE;
		}
		char * callnum	= NULL;
		char * author	= NULL;
		char * title	= NULL;
		char * pubinfo	= NULL;
		
		if ( 1 == libFormat) {
			callnum	= stripLeadingSpaces( bibInfo[ CALLNUM ] );
			author	= stripLeadingSpaces( bibInfo[ AUTHOR ] );
			title	= stripLeadingSpaces( bibInfo[ TITLE ] );
			pubinfo = stripLeadingSpaces( bibInfo[ PUBINFO ] );
			
			fprintf( outfile, "%s%s %s%s %s%s %s%s\n",
						callnum,	1 == checkPunctuation( callnum )	? "." : "",
						author,		1 == checkPunctuation( author )		? "." : "",
						title,		1 == checkPunctuation( title )		? "." : "",
						pubinfo,	1 == checkPunctuation( pubinfo )	? "." : "" );			
		} else {
			author	= stripLeadingSpaces( bibInfo[ AUTHOR ] );
			title	= stripLeadingSpaces( bibInfo[ TITLE ] );
			pubinfo = stripLeadingSpaces( bibInfo[ PUBINFO ] );
			
			fprintf( outfile, "%s%s %s%s %s%s\n",
						author,		1 == checkPunctuation( author )		? "." : "",
						title,		1 == checkPunctuation( title )		? "." : "",
						pubinfo,	1 == checkPunctuation( pubinfo )	? "." : "" );
		}
		
		//		printBibInfo( bibInfo );
		freeBibInfo ( bibInfo );
		if ( NULL != callnum )
			free( callnum );
		if ( NULL != author )
			free( author );
		if ( NULL != title )
			free( title );
		if ( NULL != pubinfo )
			free( pubinfo );
	}
	
	if ( NULL != sortedTop->subelem )
		free( sortedTop->subelem );
	
	if ( sortedTop != NULL )
		free( sortedTop );
	
	return EXIT_SUCCESS;
}

void sortRecs( XmElem *collection, const char *keys[] ){
	int i = 0;
	int j = 0;
	int keysVisited[ collection->nsubs ];
	char	* sortedKeys[ collection->nsubs ];	//	destination for the copy of keys[]; will be sorted
	XmElem	* records[ collection->nsubs ];	//	used to make sure that no pointers to collection->subelems are lost
	
	for (i = 0; i < collection->nsubs; i++){
		//		printf( "\t[%s]\n", keys[ i ] );
		sortedKeys[ i ] = strdup( keys[ i ] );
		//	save pointers to each collection->subelem
		records[ i ] = (*collection->subelem)[ i ];
	}
	
	//	check to make sure that there are some records within collection
	if ( (int) collection->nsubs > 1 ) {
		qsort( sortedKeys, collection->nsubs, sizeof( char * ), cmpstringp );
		
		//	now keys maps to records 1:1
		//	sortedKeys is the order that we want, so move through each key in sortedKeys
		//	for ( i in sortedKeys )
		//	for ( j in key )
		//	when key == sortedKey
		//	grab the record associated with key
		//	(*collection->subelem)[ i ] = record[j]
		for ( i = 0; i < collection->nsubs; i++ ) {
			//		printf( "\t[%s]\n", sortedKeys[ i ] );
			for ( j = 0;  j < collection->nsubs; j++ ) {
				if ( strcmp( sortedKeys[ i ], keys[ j ] ) == 0 && 1 != keysVisited[ j ]) {
					//					printf( "\t*sortedKeys[%d] = [%s] matches keys[%d] = [%s]\n", i, sortedKeys[i], j, keys[j]);
					(*collection->subelem)[ i ] = records[ j ];
					keysVisited[ j ] = 1;
					break;
				}
			}
		}
	}
	//	free copy of keys
	for (i = 0; i < collection->nsubs; i++)
		free( sortedKeys[ i ] );
}

//	from man qsort page
static int cmpstringp(const void *p1, const void *p2) {
	/* The actual arguments to this function are "pointers to
	 pointers to char", but strcmp(3) arguments are "pointers
	 to char", hence the following cast plus dereference */
	
	//	case shouldn't matter?
	return strcasecmp(* (char * const *) p1, * (char * const *) p2);
}

/*	print element and its subelements recursively
 *	copied from printElement.c
 *	provided by Dr. William Gardner
 *	http://www.uoguelph.ca/~gardnerw/
 *
 *	contents of printElement.c follow below
 */

/******** printElement.c begins ********/
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
/******** printElement.c ends ********/

/********
 *	since top1 and top2 were originally assigned values in many different places
 *	I didn't want to have to keep track of which was active or not, and litter
 *	the code with if statements
 *
 *	PRE:	top1, top2 are XmElem ptrs or unknown provenance
 *	POST:	if either/both top1, top2 were allocated, they are now free
 ********/
void cleanUpMain( XmElem * top1, XmElem * top2 ){
	if ( top1 != NULL )
		mxCleanElem( top1 );
	if ( top2 != NULL )
		mxCleanElem( top2 );
}
