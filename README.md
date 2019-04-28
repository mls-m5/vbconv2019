Lasersköld vb6 converter
=========================

A converter for vb6 code to buildable c++.

Usage
---

### Generate cpp files
`./cppgen`

this prints help text that will get you going

####example usage:
`./cppgen pathtoobj.cls -o path/to/targetobj -l`

The program will fix line endings on the output file no matter what ending you write.

### Build the project
Make sure to include the file `vbheader.h` located in the include folder.
g++ -c object.cpp -o object.o -I/path/to/include/

Build
----
This project uses the matmake build system. Download and install from
<https://github.com/mls-m5/matmake>

then run
matmake


Licence
-----
As of now (2019-04-18) this project is private. If you are interested in using it contact me.


Known limitations
-----
* Because of the limited parsing some functions must be called with arguments. For example when a function is called inside a if-condition the program cannot determine if it should call a function that is typed without arguments. A solution to this is to change the vbcode to allways have parenthesis after funciton calls (method calls is however handled allready).


