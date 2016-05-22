# gitid

## Description

A utility program that extracts the latest git tag name for the current HEAD and
its SHA-1 commit id and then recursively replaces all occurences of CVS-like 
markers in all source code files.

**USE IT AT YOUR OWN RISK**

The following markers are supported (case sensitive):

- `Id`
- `Name`

`$Id$` is replaced by `$Id: <SHA-1 commit id> $` and `$Name$` is replaced by 
`$Name: <git tag name> $`. If no tag is found, then `$Name$` is replaced by 
`$Name: WORKING $`.

Only files with the following extensions are elligible for keyword substitution:

        .adb, .ads, .c, .h, .cpp, .hpp, .cc, .java, .py, .js, .cs, .rc, .xml, .html, .htm, .xhtml

## Build instructions

The program has been built and tested under:
- Linux Mint 17.3 32bit using GCC 4.8.4 and libgit2 0.22.0.
- MS Windows using MinGW32 (gcc 4.8.1) and libgit2 0.22.0 (built with VS 2010 Express).

In order to build the program:

- Download and build [libgit2](https://libgit2.github.com/) (on Windows, on Linux you can use your package manager to install the library and headers).
- Compile with:
        gcc -o gitid.exe src\gitid.c src\git_helper.c -I<libgit2_include_dir_path> -L<libgit2_lib_dir_path> -lgit2

For example on Windows:

        gcc -o gitid.exe src\gitid.c src\git_helper.c -IC:\projects\libgit2-0.22.0\include -LC:\projects\libgit2-0.22.0\build\Debug -lgit2
        
And on Linux:

        gcc -o gitid src/gitid.c src/git_helper.c -I/usr/local/include -L/usr/local/lib -lgit2


## Usage
Put it on your system PATH and then call it in the directory where your source code resides.

## Notes
Each file is read in memory and then written back to disk for each keyword that is searched (currently 2 - Name and Id).
This means that
- **If something goes wrong it might mess up your source files!!!** 
- It could be very slow and memory intensive if you have many large files.


## License

The MIT License (MIT)

Copyright (c) 2016 Georgios Migdos

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

