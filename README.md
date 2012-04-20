README
=============
________________

##About Altro

This application was a course project for CIS 2750 at the University of Guelph.
CIS 2750 [colloquially known as *The Angel of Death* @ UofG] is "Software Systems Development and Integration".
It's meant to test students as coders, problem solvers, managers, and spec readers.
It is a course that historically has a very high crash-and-burn rate.

There are four components to the assignment application:
1.  utility layer – mxutil.c, written in C
2.  I/O layer – mxtool.c, written in C
3.  GUI – altro.py, written in Python using Tkinter
4.  db integration – improving altro.py, using PostgreSQL

"Altro" is the name of the completed application.

The target systems ran Fedora, and as of now there is no system detection in place to modify settings/params based on OS type/version.
That means it may not run on your machine without some modification.

##How to run Altro

Run `$ make` and hopefully things work out for you.

There are some tests included in the makefile that will run Valgrind with specified files and output reports.
If I knew more Perl, I would have just automated it that way.

_______
##Licence

I don't know what the appropriate licensing for this project is, so until I decide, the licence is this:
Be cool. Check out my code, play with it, perhaps even improve it!
If you're going to sell it or use it to make money, ask first, and we'll work that out.
If it's being given away, maintain proper attribution.

I'll probably just GPL it, along with whatever other inconsequential repos I have available to the public, but not just yet.