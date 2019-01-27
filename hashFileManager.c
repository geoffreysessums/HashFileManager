/*******************************************************************************
    hashFileManager.c by Geoffrey Sessums
Purpose:
    The hashFileManager opens, creates (if necessary), and edits a hash file 
    (movie.dat) containing movie records. It also implements chaining to handle
    synonyms. The hashFileManager receives input from the hashFileDriver which 
    uses an input file (Input.txt) containing the commands create, open, insert,
    read, and dump (see command descriptions below).

    To help with debugging the hashFileDriver produces an output file 
    (Output.txt).
Command Parameters:
    hashFileDriver < infile > outfile

    Commands within infile:
    
    * comment       
        This is just a comment which helps explain what is being tested.
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
Results:
    All output is written to stdout.
    Prints each of the commands and their command parameters. Some of the 
    commands print information:  
        CREATE - prints the record size
        INSERT - prints the primary RBN
        READ   - prints the movie information (if found)
Returns:
    RC_OK               0   // normal 
    RC_FILE_EXISTS      1   // file already exists
    RC_REC_EXISTS       2   // record already exists
    RC_REC_NOT_FOUND    3   // record not found
    RC_FILE_NOT_FOUND   4   // file not found
    RC_HEADER_NOT_FOUND 5   // header not found
    RC_BAD_REC_SIZE     6   // invalid record size in info record
    RC_LOC_NOT_FOUND    7   // Location not found for read
    RC_LOC_NOT_WRITTEN  8   // Location not written for write

Notes:
 ******************************************************************************/
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "hashFileManager.h"

/****************************** hashCreate  ***********************************
int hashCreate(char szFileNm[], HashHeader *pHashHeader);
Purpose:
    This function creates a hash file containing only the HashHeader record.
Parameters:
    List each parameter on a separate line including data type name and
    description. Each item should begin with whether the parameter is passed in,
    out or both:
    I    char szFileNm[] filename 
    O    HashHeader *pHashHeader
Return value:
    RC_FILE_EXISTS returned if the hash file already exists
    RC_OK returned if HashFile was created successfully
*******************************************************************************/
int hashCreate(char szFileNm[], HashHeader *pHashHeader) 
{
    FILE *pFile;
    int iNumRecWritten;
    
    // Open an existing file as binary input, if file doesn't exist, return NULL
    pFile = fopen(szFileNm, "rb");
    
    // Close & return RC_FILE_EXISTS, if the hash file already exists
    if (pFile != NULL) 
    {
        fclose(pFile);
        return RC_FILE_EXISTS;
    }

    // Create a binary hash file
    if ((pFile = fopen(szFileNm, "wb+")) == NULL)
        errExit("failed to open hash file: %s\n", szFileNm);
    pHashHeader->iHighOverflowRBN = pHashHeader->iNumPrimary;
    iNumRecWritten = fwrite(pHashHeader, pHashHeader->iRecSize, 1L, pFile);
    assert(iNumRecWritten == 1);
    fclose(pFile);
    return RC_OK;
}

/****************************** hashOpen ***************************************
int hashOpen(char szFileNm[], HashFile *pHashFile);
Purpose:
    This function opens an existing hash file which must contain a HashHeader 
    record, and sets the pHashFile->pFile. 
    It returns the HashHeader record by setting pHashFile->hashHeader..
Parameters:
    List each parameter on a separate line including data type name and 
    description. Each item should begin with whether the parameter is passed in,
    out or both:
    I    char szFileNm
    I/O  Hashfile *pHashFile
Return value:
    RC_HEADER_NOT_FOUND returned if the read fails
    RC_OK returned if HashFile opened successfully
*******************************************************************************/
int hashOpen(char szFileNm[], HashFile *pHashFile) 
{
    int iRC;
    int iRecCount; // number of records read
    FILE *pFile;

    // Open the file, return RC_FILE_NOT_FOUND if it doesn't exist
    pFile = fopen(szFileNm, "rb+");    
    if (pFile == NULL)
    {
        return RC_FILE_NOT_FOUND;
    }
    // Read the HashHeader record and return it through the parameter
    pHashFile->pFile = pFile;
    iRecCount = fread(&pHashFile->hashHeader, sizeof(Movie)
                      , 1L, pHashFile->pFile);
    // If the read fails, return RC_HEADER_NOT_FOUND.
    if (iRecCount != 1)
        return RC_HEADER_NOT_FOUND;
    return RC_OK;
}

/****************************** readRec ****************************************
int readRec(HashFile *pHashFile, int iRBN, void *pRecord);
Purpose:
    This function reads a record at the specified RBN in the specified file. 
Parameters:
    I    HashFile *pHashFile    
    I    int iRBN
    I    void *pRecord
Return value:
    RC_OK returned if record was read successfully
*******************************************************************************/
int readRec(HashFile *pHashFile, int iRBN, void *pRecord) 
{
    int iRC;       // return code: 0 = success
    int iRecCount; // number of records sucessfully read
    long lRBA;    // relative byte address
    
	// Determine the RBA based on iRBN and pHashFile->hashHeader.iRecSize.
    lRBA = iRBN * pHashFile->hashHeader.iRecSize;
    // Find position in the file
    iRC = fseek(pHashFile->pFile, lRBA, SEEK_SET);
    //assert(iRC == 0);
    // Read the record at the location in the file, if location wasn't found 
    // then return RC_LOC_NOT_FOUND. Otherwise, return RC_OK.
    iRecCount = fread(pRecord, sizeof(Movie) 
                , 1L, pHashFile->pFile);
    if (iRecCount != 1)
        return RC_LOC_NOT_FOUND;
    return RC_OK;
}

/****************************** writeRec ***************************************
int writeRec(HashFile *pHashFile, int iRBN, void *pRecord);
Purpose:
    This function writes a record to the specified RBN in the specified file.
Parameters:
    I/O    HashFile *pHashFile
    I    int iRBN
    I    void *pRecord 
Return value:
    RC_OK returned if write was successful
    RC_LOC_NOT_WRITTEN returned if fwrite fails
*******************************************************************************/
int writeRec(HashFile *pHashFile, int iRBN, void *pRecord) 
{
    int iRC;               // return code of fseek: 0 = success
    long lRBA;             // relative byte address
    int iNumRecWritten;    // number of written records
    
    // Determine the RBA based on iRBN and pHashFile->hashHeader.iRecSize.
    lRBA = iRBN * pHashFile->hashHeader.iRecSize;
    // Find position in the file
    iRC = fseek(pHashFile->pFile, lRBA, SEEK_SET);
    assert(iRC == 0);
	// Use fwrite to write that record to the file.
	// If the fwrite fails, return RC_LOC_NOT_WRITTEN.  Otherwise, return RC_OK.
    iNumRecWritten = fwrite(pRecord, sizeof(Movie)
                , 1L, pHashFile->pFile);
    if (iNumRecWritten != 1)
       return RC_LOC_NOT_WRITTEN; 
    return RC_OK;
}

/****************************** movieInsert ************************************
int movieInsert(HashFile *pHashFile, Movie *pMovie);
Purpose:
    This function inserts a movie into the specified file.
Parameters:
    I/O    HashFile *pHashFile 
    I    Movie *pMovie 
Return value:
    RC_REC_EXISTS returned if the new movie we're 
        attempting to insert already exists
    RC_SYNONYM returned if the new movie we're attempting to insert hashed to 
        same location resulting in a collision
Notes:
    This function inserts a movie into the specified file.
	    Determine the RBN using the driver's hash function.
	    Use readRec to read the record at that RBN.  
	    If that location doesn't exist or the record at that location has a 
        szMovieId[0] == '\0':
	        Sets its iNextChain to 0.
	        Write this new movie record at that location using writeRec.
	    If that record exists and that movie's szMovieId matches, then 
        return RC_REC_EXISTS.  (Do not update it.)

	    Otherwise, it is a synonym to the movie in the primary area:
	        Determine if it exists in the synonym chain.  If it does already 
            exist, return RC_REC_EXISTS.  (Do not update it.)

	        Otherwise, insert it in the overflow area and set the last record 
            in its chain to point to this new one. Note that if this is the 
            first synonym, the primary record's iNextChain should point to this
            new record.  

	        When inserting onto the end of the synonym chain, make certain you 
            re-write that last chain record, changing its iNextChain.

	        If you write it to the overflow area:
	            Where is it written?
	            Besides writing the new overlfow record, what else must be 
                written?
*******************************************************************************/
int movieInsert(HashFile *pHashFile, Movie *pMovie) 
{
    int iRC;               // return coe of readRec: RC_OK = success
    int iRBN;              // relative block number in HashFile
    int iNextRBN;          // relative block number in iNextChain
    int iHighOverflowRBN;  // relative block number of highest record in overflow
                           // area
    Movie tmpMovie;        // record read within HashFile

    // Determine the RBN using the driver's hash function and read the record
    // at that location 
    iRBN = hash(pMovie->szMovieId, pHashFile->hashHeader.iNumPrimary);
    iRC = readRec(pHashFile, iRBN, &tmpMovie); 
    // If that location doesn't exist or the record isn't there, then set its
    // iNextChain to 0 and write the new movie record to that location.
    if (iRC == RC_LOC_NOT_FOUND || tmpMovie.szMovieId[0] == '\0')
    {
        pMovie->iNextChain = 0;
        writeRec(pHashFile, iRBN, pMovie);
    }
	// If that record exists and that movie's szMovieId matches, return 
    // RC_REC_EXISTS. (Do not update it.)
    else if (strcmp(tmpMovie.szMovieId, pMovie->szMovieId) == 0)
    {
        return RC_REC_EXISTS;
    } 
    else    // We have sysnonym to the movie in the primary area
    {
        iNextRBN = tmpMovie.iNextChain;

        while (iNextRBN != 0)
        {
            readRec(pHashFile, iNextRBN, &tmpMovie);
            // if pMovie matches movie within synonym chain, return it exists
            if (strcmp(tmpMovie.szMovieId, pMovie->szMovieId) == 0)
                return RC_REC_EXISTS;
            // update iNextRBN
            iNextRBN = tmpMovie.iNextChain;
        }
        // Note: iNextRBN is 0 here 
        iHighOverflowRBN = pHashFile->hashHeader.iHighOverflowRBN + 1;
        pHashFile->hashHeader.iHighOverflowRBN = iHighOverflowRBN; 
        writeRec(pHashFile, 0, &pHashFile->hashHeader);
        pMovie->iNextChain = 0;
        writeRec(pHashFile, iHighOverflowRBN, pMovie);
        // get RBN of record with movie read
        movieRead(pHashFile, &tmpMovie, &iRBN);
        tmpMovie.iNextChain = iHighOverflowRBN; 
        // re-write the record with iNextChain pointing to RBN or new record
        writeRec(pHashFile, iRBN, &tmpMovie);
    }
    return RC_OK;
}

/****************************** movieRead **************************************
int movieRead(HashFile *pHashFile, Movie *pMovie, int *piRBN);
Purpose:
    This function reads the specified movie by its szMovieId.
Parameters:
    I    HashFile *pHashFile 
    I    Movie *pMovie 
Return value:
    RC_REC_NOT_FOUND returned it record wasn't found
    RC_OK returned if movie was read successfully
Notes:
    This function reads the specified movie by its szMovieId.
    We are now also returning the RBN for where it was written and also handling
    synonyms.
	Determine the primary RBN using the driver's hash function.
	Use readRec to read the record at that RBN. 
	If the movie at that location matches the specified szMovieId, return the 
    movie via pMovie, set *piRBN, and return RC_OK.

	Otherwise, it is a synonym to the movie in the primary area:
	    Determine if it exists in the synonym chain.  If it does exist, return 
        the movie via pMovie, set *piRBN to its actual RBN,  and return RC_OK.
	    Otherwise, return RC_REC_NOT_FOUND.
*******************************************************************************/
int movieRead(HashFile *pHashFile, Movie *pMovie, int *piRBN)
{
    int iRC;           // return code of readRec: RC_OK = success
    int iRBN;          // relative block number in HashFile
    int iNextRBN;      // relative block number in iNextChain
    Movie tmpMovie;    // record read within hash file

    // Determine the RBN using the driver's hash function and read the record
    // at that RBN
    iRBN = hash(pMovie->szMovieId, pHashFile->hashHeader.iNumPrimary);
    iRC = readRec(pHashFile, iRBN, &tmpMovie); 
    /*** Insert return code check here ***/

	// If the movie at that location matches the specified szMovieId, return the 
    // movie via pMovie, set *piRBN, and return RC_OK.
    if (strcmp(tmpMovie.szMovieId, pMovie->szMovieId) == 0)
    {
        *pMovie = tmpMovie;
        *piRBN = iRBN;
        return RC_OK;
    }
    else // pMovie is a synonym to the movie in the primary area
    {
        iNextRBN = tmpMovie.iNextChain;
        while (iNextRBN != 0 && iRC == RC_OK)
        {
            iRC = readRec(pHashFile, iNextRBN, &tmpMovie);
            // if pMovie matches movie within synonym chain, return it exists
            if (strcmp(tmpMovie.szMovieId, pMovie->szMovieId) == 0)
            {
                *pMovie = tmpMovie;
                *piRBN = iNextRBN;
                return RC_OK;
            }
            // update iNextRBN
            iNextRBN = tmpMovie.iNextChain;
        } 
    }
    return RC_REC_NOT_FOUND;
}

/****************************** movieUpdate ************************************
int movieUpdate(HashFile *pHashFile, Movie *pMovie);
Purpose:
    This function reads the specified movie by its szMovieId.
Parameters:
    I    HashFile *pHashFile 
    I    Movie *pMovie 
Return value:
Notes:
    This function reads the specified movie by its szMovieId. If found, it 
    updates the contents of the movie without losing the record's iNextChain. 
    If not found, it returns RC_REC_NOT_FOUND.
*******************************************************************************/
int movieUpdate(HashFile *pHashFile, Movie *pMovie)
{
    int iRC;           // return code of readRec: RC_OK = success
    int iRBN;          // relative block number in HashFile
    int iNextRBN;      // relative block number in iNextChain
    Movie tmpMovie;    // record read within hash file

    tmpMovie = *pMovie;
    iRC = movieRead(pHashFile, &tmpMovie, &iRBN);
    if (iRC == RC_OK)
    {
        pMovie->iNextChain = tmpMovie.iNextChain;
        writeRec(pHashFile, iRBN, pMovie);
        return RC_OK;
    }
    else
        return RC_REC_NOT_FOUND;
}

/****************************** movieDelete ************************************
int movieDelete(HashFile *pHashFile, Movie *pMovie); 
Purpose:
    This function deletes movies from the hash file.
Parameters:
    I    HashFile *pHashFile 
    I    Movie *pMovie 
Return value:
Notes:
    If you did not do the extra credit, create a simple function that just 
    returns RC_NOT_IMPLEMENTED.
*******************************************************************************/
int movieDelete(HashFile *pHashFile, Movie *pMovie)
{
    return RC_NOT_IMPLEMENTED;

}