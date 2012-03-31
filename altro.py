# -*- coding: utf-8 -*-
# File: altro.py
"""
 *  altro.py
 *  Created by Chad Chabot on .
 *	Last updated:	March 25, 2012
 *
 *	Author:	Chad Chabot
 *			0580345
 *			chad@chadchabot.com
 *			http://www.chadchabot.com/
 */
"""
import re
import sys
import os
#   shared object library to talk with the C functions
import Mx
import psycopg2

from Tkinter import *
import tkMessageBox
import tkFileDialog


#	python app components

###################################
#	imported modules
###################################
#	'global' data
###################################
#	helper classes and functions
###################################
#	main class
###################################
#	GUI unit
###################################
#	main class instantiation
###################################
#	test code
###################################

#	get command line args for db connection
args = sys.argv[1:]
if len(args) == 1:
    userName = args[0]
    hostname = "db.socs.uoguelph.ca"
    connectString = "host=" + hostname + " dbname=" + userName + " user=" + userName

elif len(args) == 2:
    userName = args[0]
    hostname = args[1]
    connectString = "host=" + hostname + " dbname=" + userName + " user=" + userName

elif len(args) == 3:
    userName = args[0]
    hostname = args[1]
    password = args[2]
    connectString = "host=" + hostname + " dbname=" + userName + " user=" + userName + " password=" + password

else:
    #	error
    print "Fatal error: Incorrect arguments supplied to altro.py."
    sys.exit(1)
#    connecting to db
#	build connection string
try:
    #connection = psycopg2.connect( dbname=userName, host=hostname, user=userName )
    connection = psycopg2.connect( str(connectString) )
except psycopg2, e:
    print str(e)
    print "Fatal error: Unable to connect to database [" + hostname+ "] with username [" + userName + "]."
    sys.exit(1)

cur = connection.cursor()
connection.set_isolation_level( psycopg2.extensions.ISOLATION_LEVEL_AUTOCOMMIT )
#cur.execute( "INSERT INTO chad (name, age) VALUES ('eric', 33);" )
#connection.commit()
#cur.close()
#connection.close()

root = Tk()

#	Global variables
numberOfRecords = 0

recordPtr = None
#	temporary file location
tempFolder = "xsdTempFiles"
tempDir = "./" + tempFolder + "/"


#class GUI:
#    def config():
#	  print "it works"

databasemenu = None
listbox = None
deleteButton = None
#	deleteButton = GUI()
undoButton = None
statusBarText = StringVar()

os.system( "mkdir " + tempFolder )

xsdFile = StringVar()




#   HELP window
def showAboutWindow():
    userResponse = tkMessageBox.showinfo(
        "About altro",
        "Altro is an XML file validation and exporting tool.\nAltro is compatible with the MARC21slim XML schema [http://www.loc.gov/standards/marcxml/schema/MARC21slim.xsd].\nAltro: Chad Chabot - 2012" )

#   KEEP/DISCARD window
def showKeepDiscardWindow():
	userResponse = tkMessageBox.showinfo(
		"Keep / Discard regex patterns",
		"Uses standard regex.h library search patterns.\n Usage: \"<field>=<regex>\"\nwhere <field> = a, p, or t, for author, publisher, or title, respectively.\n\n Examples: a=" )

def openFile():
	global numberOfRecords
	global recordPtr
	tempFileName =  tkFileDialog.askopenfilename(title="Open New XML File", filetypes=[("XML files", "*.xml"), ("Text files", "*.txt"), ("All Files", "*")], initialdir="./")
	#	send tempFileName as input parameter to review ( how to disable user interaction with review function? )
	#	the ordered records from review will populate the listbox in the main window
	if len( tempFileName ) == 0:
		#	catch the cancel/ESCAPE key entered
		updateStatusBar( "No file selected" )
	elif tempFileName:
		( status, recordPtr, numberOfRecords ) = Mx.readFile( tempFileName )
		for i in range( 0, numberOfRecords ):
			( title, author, pub, callnum ) = Mx.marc2bib( recordPtr, i )
			string = str( i ) + " " + author + " " +  title + " " + pub
			listbox.insert( END, string )
		updateStatusBar( str( numberOfRecords ) + " records found in file " + tempFileName )
	else:
		updateStatusBar( "There was an error selecting a file." )

def appendFile():
	global numberOfRecords
	global recordPtr
	tempFileName =  tkFileDialog.askopenfilename(title="Insert XML File", filetypes=[("XML files", "*.xml"), ("Text files", "*.txt"), ("All Files", "*")], initialdir="./")
	if len( tempFileName ) == 0:
		#	catch the cancel/ESCAPE key entered
		updateStatusBar( "No file selected" )
	elif tempFileName:
		( newFileName, recordPtr, numRecordsAppened ) = Mx.append( tempFileName )
		listbox.delete( 0, END )
		
		( status, recordPtr, numberOfRecords ) = Mx.readFile( newFileName )
		for i in range( 0, numberOfRecords ):
			( title, author, pub, callnum ) = Mx.marc2bib( recordPtr, i )
			string = str( i ) + " " + author + " " +  title + " " + pub
			listbox.insert( END, string )
		updateStatusBar( str( listbox.size() - numRecordsAppened ) + " records appended from file " + tempFileName )
	else:
		updateStatusBar( "There was an error selecting a file." )


def insertFile():
	global numberOfRecords
	global recordPtr
	tempFileName =  tkFileDialog.askopenfilename(title="Insert XML File", filetypes=[("XML files", "*.xml"), ("Text files", "*.txt"), ("All Files", "*")], initialdir="./")
	if len( tempFileName ) == 0:
		#	catch the cancel/ESCAPE key entered
		updateStatusBar( "No file selected" )
	elif tempFileName:
		( newFileName, recordPtr, numRecordsAppened ) = Mx.insert( tempFileName )
		listbox.delete( 0, END )
		
		( status, recordPtr, numberOfRecords ) = Mx.readFile( newFileName )
		for i in range( 0, numberOfRecords ):
			( title, author, pub, callnum ) = Mx.marc2bib( recordPtr, i )
			string = str( i ) + " " + author + " " +  title + " " + pub
			listbox.insert( END, string )
		updateStatusBar( str( numRecordsAppened ) + " records inserted from file " + tempFileName )
	else:
		updateStatusBar( "There was an error selecting a file." )

#   FILE Exit
def exitApp():
	userResponse = tkMessageBox.askyesno("Exit?", "Are you sure you would like to quit?")
	if userResponse:
		#	check for all open/temp files, close any that are open.
		#	free any open data structures by calling appropriate mxutil functions.
		#	then quit the python program.
		os.system( "rm -rf " + tempFolder + "/" )
		Mx.term()
		cur.close()
		connection.close()
		root.quit()
       
def ok( self ):
        self.top.destroy()

def saveAs():
	outputFilename = tkFileDialog.asksaveasfilename(title="Save file as:", filetypes=[("XML files", "*.xml"), ("All Files", "*")], initialdir="./")
	if len( outputFilename) == 0:
		#	catch the cancel/ESCAPE key entered
		updateStatusBar( "no file selected" )
	elif outputFilename:
		reclist = [ 0, 1 ]
		recordsWritten = Mx.writeFile( outputFilename, reclist )
		updateStatusBar( str( recordsWritten ) + " records written to file " + outputFilename)
	else:
		updateStatusBar( "error selecting file." )

def libOutput():
	outputFilename = tkFileDialog.asksaveasfilename(title="Save Library format to file:", filetypes=[("Text files", "*.txt"), ("All Files", "*")], initialdir="./")
	#print "saving file as: " + outputFilename
	if len( outputFilename) == 0:
		#	catch the cancel/ESCAPE key entered
		updateStatusBar( "no file selected" )
	elif outputFilename:
		recordsWritten =  Mx.libOrBib( outputFilename, 1 )
		updateStatusBar( str( listbox.size() ) + " records written to file\n" + outputFilename + "\nin library format" )
	else:
		updateStatusBar( "error selecting file." )

def bibOutput():
	outputFilename = tkFileDialog.asksaveasfilename(title="Save Bibliography format to file:", filetypes=[("Text files", "*.txt"), ("All Files", "*")], initialdir="./")
	#print "saving file as: " + outputFilename
	if len( outputFilename) == 0:
		#	catch the cancel/ESCAPE key entered
		updateStatusBar( "no file selected" )
	elif outputFilename:
		Mx.libOrBib( outputFilename, 0 )
		updateStatusBar( str( listbox.size() ) + " records written to file\n" + outputFilename + "\nin bibliography format" )
	else:
		updateStatusBar( "error selecting file." )

def getEnvVar():
	tempFileName = os.environ[ 'MXTOOL_XSD' ]
	if len( tempFileName ) == 0:
		xsdFile = tkFileDialog.askopenfilename( title="Open XSD schema file", filetypes=[ ("XSD file", "*.xsd"), ("All files", "*")], initialdir="./" )
		os.environ[ 'MXTOOL_XSD' ] = str( xsdFile )
		print os.environ[ 'MXTOOL_XSD' ]
		updateStatusBar( Mx.getEnvVar( str( xsdFile ) ) )

def select( option ):
	global numberOfRecords
	global recordPtr
	value = radioSelect.get()
	if value == 1:
		field = 'a'
	elif value == 2:
		field = 't'
	else:
		field = 'p'
	
	pattern = field+"="+regexString.get()
	prevNumRecords = listbox.size()
	if option == 1:
		#	keep matches
		( status, newFileName ) = Mx.select( "k", pattern );
		
		( status, recordPtr, numberOfRecords ) = Mx.readFile( newFileName )
		listbox.delete( 0, END )
		for i in range( 0, numberOfRecords ):
			( title, author, pub, callnum ) = Mx.marc2bib( recordPtr, i )
			string = str( i ) + " " + author + " " +  title + " " + pub
			listbox.insert( END, string )
		updateStatusBar( str( numberOfRecords ) + " records kept, " + str( prevNumRecords - numberOfRecords ) +" remain from file " + newFileName )
	elif option == 0:
		#	keep matches
		( status, newFileName ) = Mx.select( "d", pattern );
		
		( status, recordPtr, numberOfRecords ) = Mx.readFile( newFileName )
		listbox.delete( 0, END )
		for i in range( 0, numberOfRecords ):
			( title, author, pub, callnum ) = Mx.marc2bib( recordPtr, i )
			string = str( i ) + " " + author + " " +  title + " " + pub
			listbox.insert( END, string )
		updateStatusBar( str( prevNumRecords - numberOfRecords ) + " records discarded, " + str( numberOfRecords ) + "remain from file " + newFileName )
	else:
		#	wtf?
		i = 1

def updateStatusBar( string ):
    statusBarText.set( string )

def purgeDb():
    userResponse = tkMessageBox.askyesno("Purge db?", "Are you sure that you want to purge all records from the database?")
    if userResponse:
	sqlCommand = "TRUNCATE TABLE bibrec"
	cur.execute( sqlCommand )
#	connection.commit()
	updateStatusBar( "Database table [bibrec] has been purged of all entries." )

def storeAllRecs():
    global numberOfRecords
    global recordPtr
    recordsWritten = 0
#    updateStatusBar( "storing [" + str( numberOfRecords ) + "] records" )
    #	grab records from the most current version of top
    for i in range( 0, numberOfRecords ):
	( title, author, pubinfo, callnum ) = Mx.marc2bib( recordPtr, i )
#	sqlCommand = "INSERT INTO bibrec (author,title,pubinfo,callnum) VALUES ('" + author + "','" + title + "','" + pubinfo + "','" + callnum + "');"
#	cur.execute( sqlCommand )
	cur.execute( "SELECT * FROM bibrec WHERE author=%s AND title=%s;", (author,title) )
	results = cur.fetchall()
	if len(results) == 0:
	    recordsWritten += 1
	    pubCopy = pubinfo
	    match = re.search(r"[0-9]{4}", pubCopy )
	    if match:
		year = match.group(0)
	    else:
		year = ""
	    tempFileName = Mx.getRawXml( recordPtr, i )
	    fp = open( tempFileName, "r" )
	    xml = fp.read()
	    fp.close()
	    os.system( "rm -rf " + tempFileName )
	    cur.execute( "INSERT INTO bibrec (author,title,pubinfo,callnum,year,xml) VALUES (%s,%s,%s,%s,%s,%s);", (author[:60],title[:120],pubinfo,callnum[:30],year,xml) ) 
    updateStatusBar( str( recordsWritten ) + " stored to database" )

def storeSelected():
    global numberOfRecords
    global recordPtr
    recordsWritten = 0
    #listCount = listbox.curselection()
    listCount = map( int, listbox.curselection() )
    for i in listCount:
    #	grab records from the most current version of top
	( title, author, pubinfo, callnum ) = Mx.marc2bib( recordPtr, i )
#	sqlCommand = "INSERT INTO bibrec (author,title,pubinfo,callnum) VALUES ('" + author + "','" + title + "','" + pubinfo + "','" + callnum + "');"
#	cur.execute( sqlCommand )
	cur.execute( "SELECT * FROM bibrec WHERE author=%s AND title=%s;", (author,title) )
	results = cur.fetchall()
	if len(results) == 0:
	    recordsWritten += 1
	    pubCopy = pubinfo
	    match = re.search(r"[0-9]{4}", pubCopy )
	    if match:
		year = match.group(0)
	    else:
		year = ""
	    tempFileName = Mx.getRawXml( recordPtr, i )
	    fp = open( tempFileName, "r" )
	    xml = fp.read()
	    fp.close()
	    os.system( "rm -rf " + tempFileName )
	    cur.execute( "INSERT INTO bibrec (author,title,pubinfo,callnum,year,xml) VALUES (%s,%s,%s,%s,%s,%s);", (author[:60],title[:120],pubinfo,callnum[:30],year,xml) )
    updateStatusBar( str( recordsWritten ) + " stored to database" )

def buttonState(variable):
      global listbox
      global deleteButton
      listCount = listbox.curselection()
      if listbox.size() > 0:
	  databasemenu.entryconfig(0,state="normal")
	  if len(listCount) != 0:
	      deleteButton.config( state=NORMAL )
	      databasemenu.entryconfig(1,state="normal")
	  else:
	      deleteButton.config( state=DISABLED )
     	      databasemenu.entryconfig(1,state="disabled")
      else:
	  databasemenu.entryconfig(0,state="disabled")
	  databasemenu.entryconfig(1,state="disabled")


def dbOpen():
    #
    tempFileName = tempFolder + "/dbOpenTemp.xml"
    tempFp = open( tempFileName, "w" )
    cur.execute( "SELECT xml FROM bibrec ORDER BY author, title;" )
    results = cur.fetchall()
    for rec in results:
	tempFp.write( rec[0] )
    #	for line in file, preface each line with "\t"
    tempFp.close()
    outFpName = tempFolder + "/dbtemp.xml"
    outFp = open( outFpName, "w+" )
    outFp.write( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n" )
    outFp.write( "<!-- Output by mxutil library ( Chad Chabot ) -->\r\n" )
    outFp.write( "<marc:collection xmlns:marc=\"http://www.loc.gov/MARC21/slim\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.loc.gov/MARC21/slim\nhttp://www.loc.gov/standards/marcxml/schema/MARC21slim.xsd\">\n" )

    tempFp = open( tempFileName,"r")
    tempLines = tempFp.readlines()
    for line in tempLines:
	#	get line and append "\t"
	outFp.write( "\t" + line )

    outFp.write( "</marc:collection>\n" )
    outFp.close()

    ( status, recordPtr, numberOfRecords ) = Mx.readFile( outFpName )
    listbox.delete( 0, END )
    for i in range( 0, numberOfRecords ):
	( title, author, pub, callnum ) = Mx.marc2bib( recordPtr, i )
	string = str( i ) + " " + author + " " +  title + " " + pub
	listbox.insert( END, string )
	listbox.itemconfig(listbox.size() -1 , bg='red', fg='white')
    updateStatusBar( str( numberOfRecords ) + " records loaded from db" )

    os.system( "rm -f " + tempFileName )
    os.system( "rm -f " + outFpName )

def dbInsert():
    #
    tempFileName = tempFolder + "/dbOpenTemp.xml"
    tempFp = open( tempFileName, "w" )
    cur.execute( "SELECT xml FROM bibrec ORDER BY author, title;" )
    results = cur.fetchall()
    for rec in results:
	tempFp.write( rec[0] )
    #	for line in file, preface each line with "\t"
    tempFp.close()
    outFpName = tempFolder + "/dbtemp.xml"
    outFp = open( outFpName, "w+" )
    outFp.write( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n" )
    outFp.write( "<!-- Output by mxutil library ( Chad Chabot ) -->\r\n" )
    outFp.write( "<marc:collection xmlns:marc=\"http://www.loc.gov/MARC21/slim\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.loc.gov/MARC21/slim\nhttp://www.loc.gov/standards/marcxml/schema/MARC21slim.xsd\">\n" )

    tempFp = open( tempFileName,"r")
    tempLines = tempFp.readlines()
    for line in tempLines:
	#	get line and append "\t"
	outFp.write( "\t" + line )

    outFp.write( "</marc:collection>\n" )
    outFp.close()

    ( status, recordPtr, numberOfRecords ) = Mx.readFile( outFpName )
    for i in range( 0, numberOfRecords ):
	( title, author, pub, callnum ) = Mx.marc2bib( recordPtr, i )
	string = str( i ) + " " + author + " " +  title + " " + pub
	listbox.insert( 0, string )
	listbox.itemconfig(0, bg='red', fg='white')
    updateStatusBar( str( numberOfRecords ) + " records loaded from db" )

    os.system( "rm -f " + tempFileName )
    os.system( "rm -f " + outFpName )

def dbAppend():
    #
    tempFileName = tempFolder + "/dbOpenTemp.xml"
    tempFp = open( tempFileName, "w" )
    cur.execute( "SELECT xml FROM bibrec ORDER BY author, title;" )
    results = cur.fetchall()
    for rec in results:
	tempFp.write( rec[0] )
    #	for line in file, preface each line with "\t"
    tempFp.close()
    outFpName = tempFolder + "/dbtemp.xml"
    outFp = open( outFpName, "w+" )
    outFp.write( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n" )
    outFp.write( "<!-- Output by mxutil library ( Chad Chabot ) -->\r\n" )
    outFp.write( "<marc:collection xmlns:marc=\"http://www.loc.gov/MARC21/slim\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.loc.gov/MARC21/slim\nhttp://www.loc.gov/standards/marcxml/schema/MARC21slim.xsd\">\n" )

    tempFp = open( tempFileName,"r")
    tempLines = tempFp.readlines()
    for line in tempLines:
	#	get line and append "\t"
	outFp.write( "\t" + line )

    outFp.write( "</marc:collection>\n" )
    outFp.close()

    ( status, recordPtr, numberOfRecords ) = Mx.readFile( outFpName )
    for i in range( 0, numberOfRecords ):
	( title, author, pub, callnum ) = Mx.marc2bib( recordPtr, i )
	string = str( listbox.size() + 1 ) + " " + author + " " +  title + " " + pub
	listbox.insert( END, string )
	listbox.itemconfig( listbox.size() - 1, bg='red', fg='white')
    updateStatusBar( str( numberOfRecords ) + " records loaded from db" )

    os.system( "rm -f " + tempFileName )
    os.system( "rm -f " + outFpName )


def queryMenu():
    queryWindow = Toplevel()
    queryWindow.title( "Query Menu" )
    

#-------------------    start the GUI code  -------------------#
min_x = 475
min_y = 380

#root = Tk()
root.title( "altro" )
root.minsize( min_x, min_y )
#           width, height, x-pos, y-pos
root.geometry( "1024x768+10+10" )


Mx.init()

# create a toolbar
toolbar = Frame(root)

menubar = Menu( root )

#   this creates a menu tied to the top of the screen
#   FILE menubar item
filemenu = Menu( menubar, tearoff = 0 )
filemenu.add_command ( label = "Open", command = openFile )
filemenu.add_command ( label = "Insert", command = insertFile )
filemenu.add_command ( label = "Append", command = appendFile )
filemenu.add_command ( label = "Save as…", command = saveAs )
filemenu.add_separator()
filemenu.add_command ( label = "Exit", command = exitApp )
menubar.add_cascade( label = "File", menu = filemenu )

#   PRINT menubar item
printmenu = Menu( menubar, tearoff = 0 )
printmenu.add_command ( label = "Library", command = libOutput )
printmenu.add_command ( label = "Bibliography", command = bibOutput )
menubar.add_cascade( label = "Print", menu = printmenu )


#   HELP menubar item
helpmenu = Menu( menubar, tearoff = 0 )
helpmenu.add_command ( label = "About", command = showAboutWindow )
helpmenu.add_command ( label = "Keep/Discard", command = showKeepDiscardWindow )
menubar.add_cascade( label = "Help", menu = helpmenu )

#	DATABASE menubar item
databasemenu = Menu( menubar, tearoff = 0 )
databasemenu.add_command ( label = "Store all", command = storeAllRecs )
databasemenu.add_command ( label = "Store Selected", command = storeSelected )
databasemenu.add_command ( label = "Open", command = dbOpen )
databasemenu.add_command ( label = "Insert", command = dbInsert )
databasemenu.add_command ( label = "Append", command = dbAppend )
databasemenu.add_command ( label = "Query", command = lambda: updateStatusBar("db - query") )
databasemenu.add_separator()
databasemenu.add_command ( label = "Purge", command = purgeDb )
menubar.add_cascade ( label = "Database", menu = databasemenu )

root.config( menu = menubar )

#	add listbox
listboxFrame = Frame( root )
scrollbarVertical = Scrollbar( listboxFrame, orient=VERTICAL )
scrollbarHorizontal = Scrollbar( listboxFrame, orient=HORIZONTAL )

listbox = Listbox( listboxFrame, selectmode=EXTENDED, bg="white", width=300, yscrollcommand=scrollbarVertical.set, xscrollcommand=scrollbarHorizontal.set )
scrollbarVertical.config( command=listbox.yview )
scrollbarVertical.pack( side=RIGHT, fill=Y )
scrollbarHorizontal.config( command=listbox.xview )
scrollbarHorizontal.pack( side=BOTTOM, fill=X )


listbox.pack( anchor=N, fill=BOTH, expand=1 )
listboxFrame.pack( anchor=N, fill=BOTH, expand=1 )
listbox.bind('<Button-1>', buttonState )

#	add regex input box
#Label( controlPanel, text="REGEX pattern:").pack(side=LEFT, padx=5, pady=10 )

#	add control panel radio buttons
controlPanel = Frame( root, width=475, height = 50 )
deleteButton = Button( controlPanel, text="Delete selected records", state=DISABLED, command=lambda: updateStatusBar('Delete button pressed') )
deleteButton.pack( anchor=W )

undoButton = Button( controlPanel, text="UNDO last action", state=DISABLED, command=lambda: updateStatusBar('UNDO button pressed') )
undoButton.pack( anchor=W )

keepButton = Button( controlPanel, text="Keep pattern", state=NORMAL, command=lambda: select( 1 ) ).pack( anchor=E, side=TOP )
discardButton = Button( controlPanel, text="Discard pattern", state=NORMAL, command=lambda: select( 0 ) ).pack( anchor=E, side=TOP )

radioSelect = IntVar()
Radiobutton( controlPanel, text="author", value=1, variable=radioSelect ).pack(anchor=W)
Radiobutton( controlPanel, text="title", value=2, variable=radioSelect ).pack(anchor=W)
Radiobutton( controlPanel, text="publisher data", value=3, variable=radioSelect ).pack(anchor=W)
radioSelect.set( 3 )

regexString = StringVar()
Entry( controlPanel, bg="white", width=40, textvariable=regexString ).pack( anchor=W )
regexString.set( "type regex here" )



controlPanel.pack( anchor=S, fill=BOTH )


#	status bar
status = Label(root, height=3, textvariable=statusBarText, bd=1, relief=SUNKEN, anchor=S)
status.pack(side=BOTTOM, fill=X)

mainloop()