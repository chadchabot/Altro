#! /usr/bin/env python

import time
from Tkinter import *
import tkMessageBox
#from Tkinter.ScrolledText import ScrolledText


textArea = None
querySelect = None
eb1 = None
eb2 = None
eb3 = None
eb4 = None
eb5 = None

def windowState():
    global queryWindow
    if queryWindow.state() == "withdrawn":
        print "it is withdrawn!"
    elif queryWindow.state() == "normal":
        print "it is visible!"

def closeWindow():
    global queryWindow
    print "Closing window"
    queryWindow.withdraw()
        

def submitQuery( option ):
    global textArea
    global querySelect
#    textArea.config( state=NORMAL )
#    option = querySelect.get()
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

"""
def changeFocus():
    global eb1, eb2, eb3, eb4, eb5
    global querySelect
    value = querySelect.get()
    if value == 1:
        eb1.focus_set()
    elif value == 2:
        eb2.focus_set()
    elif value == 3:
        eb3.focus_set()
    elif value == 4:
        eb4.focus_set()
    elif value == 5:
        eb5.focus_set()
"""
def changeFocus( entryBox ):
    entryBox.focus_set()


def helpWindow():
    userResponse = tkMessageBox.showinfo(
        "About db queries",
        "The database is structured as follows:\n\nTable name:\n\tbibrec\nColumns:\n\trec_id\t| primary key\n\tauthor\t| author name\n\ttitle\t\t| publication title\n\tpubinfo\t| publisher information\n\tcallnum\t| library reference #\n\tyear\t| year of publishing\n\txml\t\t| raw xml of MARC21 record" )

def enterPressed( variable ):
    global querySelect
    submitQuery( querySelect.get() )

def clearField( event ):
#    print "clearField called"
    global queryWindow, eb1 
#    print "return: event.widget is",event.widget
#    print "focus is:", queryWindow.focus_get()
    eb1.delete( 0, END )

queryWindow = Tk()
queryWindow.resizable(0,0)
queryWindow.title( "Query Menu" )
# four radio buttons
querySelect = IntVar()
#   NOTE:   add back 'takefocus=0' if the radiobuttons aren't part of the tab sequence
#           how to have radiobuttons show their highlightcolor or highlightbackground when they have focus?
rb1 = Radiobutton( queryWindow, text="1", value=1, variable=querySelect, command=lambda: changeFocus( eb1 ), takefocus=0 )
rb2 = Radiobutton( queryWindow, text="2", value=2, variable=querySelect, command=lambda: changeFocus( eb2 ), takefocus=0 )
rb3 = Radiobutton( queryWindow, text="3", value=3, variable=querySelect, command=lambda: changeFocus( eb3 ), takefocus=0 )
rb4 = Radiobutton( queryWindow, text="4", value=4, variable=querySelect, command=lambda: changeFocus( eb4 ), takefocus=0 )
rb5 = Radiobutton( queryWindow, text="5", value=5, variable=querySelect, command=lambda: changeFocus( eb5 ), takefocus=0 )
querySelect.set( 1 )


eb1 = Entry( queryWindow, bg="white" )
eb2 = Entry( queryWindow, bg="white" )
eb3 = Entry( queryWindow, bg="white" )
eb4 = Entry( queryWindow, bg="white" )
eb5Text = StringVar()
eb5Text.set( "SELECT " )
eb5 = Entry( queryWindow, bg="white", textvariable=eb5Text )
#eb5.see(END)

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

submitButton = Button( queryWindow, text="Submit SQL query", state=NORMAL, command=lambda: submitQuery( querySelect.get() ), takefocus=0 )
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


print "about to disappear!"
#queryWindow.withdraw()
windowState()
time.sleep(3)
queryWindow.deiconify()
windowState()
mainloop()