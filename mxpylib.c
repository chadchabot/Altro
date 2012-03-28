/*
 *  mxtool.c
 *  Created by Chad Chabot on 12-02-02.
 *	Last updated:	March 25, 2012
 *
 *	Author:	Chad Chabot
 *			0580345
 *			chad@chadchabot.com
 *			http://www.chadchabot.com/
 */

#include <stdio.h>
#include <stdlib.h>
#include <Python.h>
#include <strings.h>
#include <assert.h>
#include <ctype.h>


#include "mxtool.h"
#include "mxutil.h"

xmlSchemaPtr schemaPtr = NULL;
char * XSDFILE;
XmElem * collection;
XmElem * top;
XmElem * top2;

PyObject * Mx_append ( PyObject * self, PyObject * args ){
	const char * filename;
	PyArg_ParseTuple( args, "s", &filename );
	int functionStatus = 0;		// the result of mxReadFile()
	FILE * xmlFile = NULL;
	FILE * outputFile = NULL;
	xmlFile = fopen( filename, "r" );
	//	check for xmlFile to be available
	if ( xmlFile == NULL ){
	  //	file did not open for reading
	} else {
	  //	file is ready for reading
	  //	send it to mxReadFile to be validated
		if ( 1 != marc21Setup( xmlFile, &top2 ) ){
			//	error opening schema file
			//	return an error
			fclose( xmlFile );
			return Py_BuildValue ( "ski", "Unable to validate schema.", NULL, 0 );
		} else {
			outputFile = fopen( "insertTempFile.xml", "w" );
			functionStatus = concat( top, top2, outputFile );
			fclose( outputFile );
			return Py_BuildValue ( "ski", "insertTempFile.xml", top, top->nsubs );
		}
	}
	return Py_BuildValue ( "ski", outputFile, top, 0 );
}


PyObject * Mx_insert ( PyObject * self, PyObject * args ){
	const char * filename;
	PyArg_ParseTuple( args, "s", &filename );
	int functionStatus = 0;		// the result of mxReadFile()
	FILE * xmlFile = NULL;
	FILE * outputFile = NULL;
	xmlFile = fopen( filename, "r" );
	//	check for xmlFile to be available
	if ( xmlFile == NULL ){
	  //	file did not open for reading
	} else {
	  //	file is ready for reading
	  //	send it to mxReadFile to be validated
		if ( 1 != marc21Setup( xmlFile, &top2 ) ){
			//	error opening schema file
			//	return an error
			fclose( xmlFile );
			return Py_BuildValue ( "ski", "Unable to validate schema.", NULL, 0 );
		} else {
			outputFile = fopen( "insertTempFile.xml", "w" );
			functionStatus = concat( top2, top, outputFile );
			fclose( outputFile );
			return Py_BuildValue ( "ski", "insertTempFile.xml", top2, top2->nsubs );
		}
	}
	return Py_BuildValue ( "ski", outputFile, top, 0 );
}

PyObject * Mx_getEnv ( PyObject * self, PyObject * args ){
	const char * fileName;
	PyArg_ParseTuple( args, "s", &fileName );
//	xmlSchemaPtr schemaPtr = NULL;
	schemaPtr = mxInit( fileName );
	if ( schemaPtr == NULL ){
		return Py_BuildValue ( "s", "schema is BAD" );
	} else {
		//	schema is valid
		return Py_BuildValue ( "s", "schema is GOOD" );
	}
}

PyObject * Mx_readFile ( PyObject * self, PyObject * args ){
	const char * filename;
	PyArg_ParseTuple( args, "s", &filename );
	
	FILE * xmlFile = NULL;
	xmlFile = fopen( filename, "r" );
	//	check for xmlFile to be available
	if ( xmlFile == NULL ){
	  //	file did not open for reading
	} else {
	  //	file is ready for reading
	  //	send it to mxReadFile to be validated
		if ( 1 != marc21Setup( xmlFile, &top ) ){
			//	error opening schema file
			//	return an error
			fclose( xmlFile );
			return Py_BuildValue ( "iki", 0, NULL, 0 );
		}
	}
	fclose( xmlFile );
	return Py_BuildValue ( "iki", 1, top, top->nsubs );
}

PyObject * Mx_marc2bib ( PyObject * self, PyObject * args ){
	XmElem * collection;
	int recno = 0;
	PyArg_ParseTuple( args, "ki", (unsigned long * ) &collection, &recno );
	BibData bdata;
	marc2bib( (* collection->subelem )[recno], bdata );
	return Py_BuildValue ( "ssss", bdata[ TITLE ], bdata[ AUTHOR ], bdata[ PUBINFO ], bdata[ CALLNUM ] );
}

PyObject * Mx_writeFile ( PyObject * self, PyObject * args ){
	char * filename;
	PyObject * reclist;
	
//	PyArg_ParseTuple( args, "skO", &filename, (unsigned long * ) &collection, &reclist );
	PyArg_ParseTuple( args, "sO", &filename, &reclist );

//	tempCollection = top;
	int writeStatus = 0;
	
	FILE * outputFile = NULL;
	outputFile = fopen( filename, "w" );
	if ( NULL == outputFile ){
		//	error
		//printf("unable to open file for writing\n");
	} else {
		//printf( "writing out file with [%ld] records in top\n", top->nsubs );
		writeStatus = mxWriteFile( top, outputFile );
	}
	fclose( outputFile );
	return Py_BuildValue ( "i", writeStatus );
}


PyObject * Mx_init ( PyObject * self, PyObject * args ){
	XSDFILE = getenv( "MXTOOL_XSD" );
	schemaPtr = mxInit( XSDFILE );
	if ( schemaPtr == NULL ){
		//	error, fix this
		return Py_BuildValue ( "i", 0 );
	} else {
		return Py_BuildValue ( "i", 1 );
	}
}

PyObject * Mx_term ( PyObject * self, PyObject * args ){
	if ( NULL != top)
		mxCleanElem( top );
	if ( NULL != top2 )
		mxCleanElem( top2 );
	if ( NULL != schemaPtr )
		mxTerm( schemaPtr );
	return Py_BuildValue ( "i", 1 );}

PyObject * Mx_libOrBib ( PyObject * self, PyObject * args ){
	char * outFileName;
	int libFormatFlag = 0;	//	needs a better explanation
	PyArg_ParseTuple( args, "si", &outFileName, &libFormatFlag );
	FILE * outFilePtr;
	int writeStatus = 0;
	outFilePtr = fopen( outFileName, "w" );
	if ( NULL == outFilePtr ){
		//	error
	} else {
		writeStatus = libOrBibOutput( top, outFilePtr, libFormatFlag );
	}
	fclose( outFilePtr );
	return Py_BuildValue ( "i", writeStatus );
}

PyObject * Mx_select ( PyObject * self, PyObject * args ){
	char * pattern;
	char selector;
	int functionStatus = 0;
	FILE * outFile = fopen( "xsdTempFiles/selectTemp.xml", "w" );
	PyArg_ParseTuple( args, "cs", &selector, &pattern );
	printf( "[%c] and [%s]\n", selector, pattern );
	
	if ( selector == 'k' ){
		functionStatus = selects( top, KEEP, pattern, outFile );
	} else {
		functionStatus = selects( top, DISCARD, pattern, outFile );
	}
	
	fclose( outFile );
	return Py_BuildValue ( "is", functionStatus, "xsdTempFiles/selectTemp.xml" );

}

/*
    //	common 
    PyArg_ParseTuple( args, "isk", &myInt, &string, &myPointer )
  
    //	? when returning a pointer, cast it as an ( int long ) pointer name
    //		to get the memory address and python will find/map to it
    Py_BuildValue ( "b", false/true ); //	to use it to output boolean values; include bool.h
*/
static PyMethodDef MxMethods[] = {

	{
		"getEnvVar", Mx_getEnv, METH_VARARGS
	},
	{
		"readFile", Mx_readFile, METH_VARARGS
	},
	{
		"init", Mx_init, METH_VARARGS
	},
	{
		"marc2bib", Mx_marc2bib, METH_VARARGS
	},
	{
		"libOrBib", Mx_libOrBib, METH_VARARGS
	},
	{
		"writeFile", Mx_writeFile, METH_VARARGS
	},
	{
		"term", Mx_term, METH_VARARGS
	},
	{
		"insert", Mx_insert, METH_VARARGS
	},
	{
		"append", Mx_append, METH_VARARGS
	},
	{
		"select", Mx_select, METH_VARARGS
	},
	{
		NULL, NULL
	}
};

/*** Initialize the module. Called when import statement in python executes***/
void initMx()
{
	/* needs module name, module method array to export */
	Py_InitModule("Mx", MxMethods);
}
