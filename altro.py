# -*- coding: utf-8 -*-
# File: altro.py
"""
 *  altro.py
 *  Created by Chad Chabot on .
 *	Last updated:	April 2, 2012
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

queryWindow = None

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


##### queryWindow globals
querySelect = IntVar()
textArea = None
eb1 = None
eb2 = None
eb3 = None
eb4 = None
eb5 = None


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

def showQueryWindow():
	global queryWindow
	if queryWindow.state() == "withdrawn":
		#	show query window
		queryWindow.deiconify()
	






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
databasemenu.add_command ( label = "Query", command = showQueryWindow )
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

#	code for queryWindow BEGIN

def callback( event ):
    global textArea
    global querySelect
#    textArea.config( state=NORMAL )
    option = querySelect.get()
    if option == 1:
        if len( eb1.get() ) != 0 :
            sqlcmd = "SELECT * FROM bibrec WHERE author=" + eb1.get() + " ORDER BY author, title;"
        else:
            sqlcmd = "nothing entered"
    elif option == 2:
        sqlcmd = "SELECT COUNT (year) FROM bibrec WHERE year >=" + eb2.get() +";"
    elif option == 3:
#        sqlcmd = eb3.get()
        sqlcmd = "SELECT author, COUNT(title) FROM bibrec GROUP BY author;"
    elif option == 4:
        sqlcmd = eb4.get()
    else:
        sqlcmd = eb5.get()
    textArea.insert( END, "you chose " + str( option ) + " " + sqlcmd + "\n\\------------------------------------------------------------------------------/\n" )
    textArea.see( END )
#    textArea.config( state=DISABLED )

def clearPanel():
    global textArea
#    textArea.config( state=NORMAL )
    textArea.delete( 1.0, END )
#    textArea.config( state=DISABLED )

def changeFocus( choice ):
    global eb1, eb2, eb3, eb4, eb5
    if choice == 1:
        eb1.focus_set()
    elif choice == 2:
        eb2.focus_set()
    elif choice == 3:
        eb3.focus_set()
    elif choice == 4:
        eb4.focus_set()
    elif choice == 5:
        eb5.focus_set()

def helpWindow():
    userResponse = tkMessageBox.showinfo(
        "About db queries",
        "The database is structured as follows:\n\nTable name:\n\tbibrec\nColumns:\n\trec_id\t| primary key\n\tauthor\t| author name\n\ttitle\t\t| publication title\n\tpubinfo\t| publisher information\n\tcallnum\t| library reference #\n\tyear\t| year of publishing\n\txml\t\t| raw xml of MARC21 record" )

def enterPressed( variable ):
    callback( variable )

def clearField( event ):
    #print "clearField called"
    global queryWindow, eb1 
    #print "return: event.widget is",event.widget
    #print "focus is:", queryWindow.focus_get()
    eb1.delete( 0, END )

def windowState():
    global queryWindow
    if queryWindow.state() == "withdrawn":
        print "it is withdrawn!"
    elif queryWindow.state() == "normal":
        print "it is visible!"

def closeWindow():
    global queryWindow
    # print "Closing queryWindow"
    queryWindow.withdraw()

queryWindow = Tk()
queryWindow.resizable(0,0)
queryWindow.title( "Query Menu" )
# four radio buttons

#   NOTE:   add back 'takefocus=0' if the radiobuttons aren't part of the tab sequence
#           how to have radiobuttons show their highlightcolor or highlightbackground when they have focus?
rb1 = Radiobutton( queryWindow, text="1", value=1, variable=querySelect, command=lambda: changeFocus(1), takefocus=0 )
rb2 = Radiobutton( queryWindow, text="2", value=2, variable=querySelect, command=lambda: changeFocus(2), takefocus=0 )
rb3 = Radiobutton( queryWindow, text="3", value=3, variable=querySelect, command=lambda: changeFocus(3), takefocus=0 )
rb4 = Radiobutton( queryWindow, text="4", value=4, variable=querySelect, command=lambda: changeFocus(4), takefocus=0 )
rb5 = Radiobutton( queryWindow, text="5", value=5, variable=querySelect, command=lambda: changeFocus(5), takefocus=0 )
querySelect.set( 1 )


eb1 = Entry( queryWindow, bg="white" )
eb2 = Entry( queryWindow, bg="white" )
eb3 = Entry( queryWindow, bg="white" )
eb4 = Entry( queryWindow, bg="white" )
eb5Text = StringVar()
eb5 = Entry( queryWindow, bg="white", textvariable=eb5Text )
eb5Text.set( "SELECT " )

lb1 = Label( queryWindow, text="Find all books by author:" )
lb2 = Label( queryWindow, text="How many books were published since the year: " )
lb3 = Label( queryWindow, text="List authors and # of books: " )
lb4 = Label( queryWindow, text="Canned 4" )
lb5 = Label( queryWindow, text="Enter any SQL command:" )

helpButton = Button( queryWindow, text="Help", state=NORMAL, command=helpWindow, takefocus=0 )
helpButton.grid( row=0, column=2, sticky=N+S+E+W)


rb1.grid( row=1, column=0 )
lb1.grid( row=1, column=1 )
eb1.grid( row=1, column=2 )

rb2.grid( row=2, column=0 )
lb2.grid( row=2, column=1 )
eb2.grid( row=2, column=2 )

rb3.grid( row=3, column=0 )
lb3.grid( row=3, column=1 )
eb3.grid( row=3, column=2 )

rb4.grid( row=4, column=0 )
lb4.grid( row=4, column=1 )
eb4.grid( row=4, column=2 )

rb5.grid( row=5, column=0 )
lb5.grid( row=5, column=1 )
eb5.grid( row=5, column=2 )

submitButton = Button( queryWindow, text="Submit SQL query", state=NORMAL, command=lambda: callback( querySelect ), takefocus=0 )
submitButton.grid( row=6, column=0, columnspan=3, sticky=N+S+E+W)
queryWindow.bind( '<Return>', enterPressed )
queryWindow.bind( '<Key-Escape>', clearField )
queryWindow.protocol( "WM_DELETE_WINDOW", closeWindow )

scroller = Scrollbar( queryWindow, orient = VERTICAL )
textArea = Text( queryWindow, width=80, height=30, wrap=WORD, borderwidth=5, relief=GROOVE, takefocus=0, yscrollcommand=scroller.set )
textArea.grid( row=7, column=0, columnspan=3 )

scroller.config( command=textArea.yview )
scroller.grid( row=7, column=3, sticky=N+S+E+W )

clearButton = Button( queryWindow, text="Clear result window", state=NORMAL, command=clearPanel, takefocus=0 )
clearButton.grid( row=8, column=0, columnspan=3, padx=5, pady=5 )

eb1.focus_set()



#	code for queryWindow END
queryWindow.withdraw()
mainloop()