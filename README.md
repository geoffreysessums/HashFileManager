# Project: HashFileManager

## Description

HashFileManager creates (if necessary), opens, and edits a hashed file 
containing movie records using simple chaining to handle synonyms. The 
hashFileManager receives input from the hashFileDriver which uses an 
input file (Input.txt) containing the commands create, open, insert, read,
and dump (see command descriptions below).

### Commands within the input file to help explain what is being tested

    CREATE MOVIE fileName numPrimary
        Create the specified hash file if it doesn't already exist
        using hashCreate.
        The file will have numPrimary records in the primary area.
        Record 0 contains information about this hash file.
        The size of each record is the size of a Movie structure.
    OPEN MOVIE fileName
        Opens the specified file using hashOpen.  Place the returned
        FILE pointer in the driver's HashFile array.
    INSERT MOVIE movieId,title,genre,rating,minutes
        Uses sscanf to get those attributes and populate a MOVIE structure.
        It invokes movieInsert to insert the movie into the hash file.
    READ MOVIE movieId
        Invokes movieRead to read the movie from the hash file and prints
        that movie.
    PRINTALL MOVIE fileName
        Prints the contents of the specified file.
    NUKE MOVIE fileName
        Removes the specified file.

### HashFileManager produces the following results

All output is written to stdout.
Prints each of the commands and their command parameters. Some of the
commands print information:  

    CREATE - prints the record size
    INSERT - prints the primary RBN
    READ   - prints the movie information (if found)

### HashFileManager produces the following return codes

    RC_OK               0   // normal
    RC_FILE_EXISTS      1   // file already exists
    RC_REC_EXISTS       2   // record already exists
    RC_REC_NOT_FOUND    3   // record not found
    RC_FILE_NOT_FOUND   4   // file not found
    RC_HEADER_NOT_FOUND 5   // header not found
    RC_BAD_REC_SIZE     6   // invalid record size in info record
    RC_LOC_NOT_FOUND    7   // Location not found for read
    RC_LOC_NOT_WRITTEN  8   // Location not written for write

## Installation

**(Driver not included)**
This project demonstrates how one might implement a hash file manager, how
I approach solving problems, the complexity of projects I've worked on, and
my documentation style.

## Usage

**The syntax for executing the program on the command-line:**
hashFileDriver < infile > outfile

## Credits

Authors: Geoffrey Sessums

## License

MIT License

Copyright (c) 2018 Geoffrey Sessums

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
