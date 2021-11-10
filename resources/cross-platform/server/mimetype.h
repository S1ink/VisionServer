#pragma once

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <wchar.h>
#include <assert.h>
#include <wctype.h>
#include <limits.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MEGAMIMES_H_
#define _MEGAMIMES_H_

    enum {
        EXTENSION_POS,
        MIMENAME_POS,
        MIMETYPE_POS,
        MAGICNUMBER_POS,
        COMPONENTS_NUMBER,
    };

#ifdef __unix__
#define FILE_PATH_SEP '/'
#else 
#define FILE_PATH_SEP '\\'
#endif 
#ifdef _WIN32 
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL _Thread_local 
#endif 

#define MAX_LINE_SIZE 256 

    /**
     * 	MegaFileInfo is the structure that conains information about a file.  The
     *  structure is returned by  getMegaFileInfo. It contains all the details
     *  that is needed about a file.
     */

    extern const char* MegaMimeTypes[][COMPONENTS_NUMBER];

    typedef struct
    {
        char* mBaseDir;
        char* mBaseName;
        char* mExtension;
        long long mFileSize;

        const char* mMimeType;
        const char* mMimeName;

        bool 	mTextFile;
        const char* mTextEncoding;

    } MegaFileInfo;


    /**
      @brief Get The Mime Type Of A File Or A File Extension

      The function gets the mime type of the file specified by the path.
      The function does not check if the file exists or not. It just uses the file's basename
      including the extension to lookup the mime type. The filename can also be an extension
      only, which is preceeded by a ".". Example (.mp3, mega.mp3, /usr/lib/mega.mp3 ) will all
      return the same thing.

      @param pFileName
        The name of the file whose mime type should be determined
      @return
        the mimetype of the file or NULL if no mimetype is known
        for the file. The string returned should be freed
    */
    const char* getMegaMimeType(const char* pFileName);


    /**
      @brief Get The Extensions associated with Mime Types

      The function gets the file extensions for the mimetype. The mimetype should be
      in the form type/sub-type, otherwise it is invalid. An optional version can be
      added to the string. Any other trailing details are ignored. * character can be used to match
      everything. Eg `video/` returns all video file extensions and `*` returns all file extensions

      @param pMimeType
        The mimetype whose extension is to be determined

      @return
        An array of extension names. The array is terminated by a NULL. The function
        returns NULL if no extensions were found for the mime types given.
    */
    const char** getMegaMimeExtensions(const char* pMimeType);

    /**
      @brief Get information about a file.

      Gets information about the file, such as the encoding, the mime name  the mime type and the the file size.

      @param pFilePath
        The name of the file whose information is to be determined.

      @return
        the information about the file. The structure returned should be freed
        with freeMegaFileInfo. The function returns NULL if the file does not exists, or the user does not
        have permissions to read from the file
    */
    MegaFileInfo* getMegaFileInformation(const char* pFilePath);

    /**
      @brief Determines if a file is a text file

      Returns whether the file is a text file or not. The function
      is a really slow function, as it scans through every byte, checking
      to see if it is a control character or not. The encoding is taken into consideration.

      @param path
        The path name for the file. If the file des not exist, false is returned.
      @return
        whether the file is a text file or not. Returns false if the file does not eixist.
    */
    bool isTextFile(const char* path);

    /**
      @brief Determines if ia file is a binary file

      Returns whether the file is a binary file or not

      @param path
        The path name for the file.
      @return
        whether the file is a binary file or not. Reutrns false if the file does not exist.

    */
    bool isBinaryFile(const char* path);

    /**
      @brief Gets the encoding for a text file.
      @pre The file is assumed to be a text file. Use isTextFile() to check the file before calling
            this function.

      The function gets the encoding of the file. The string can be UTF-8, UTF-8, UTF-16LE, UTF-16
      UTF-32, UTF-32BE, UTF-32LE. The function does not check if the file is binary file or a text file.
      It is assumend that you are really sure that it is a binary file. You can use  isTextFile() to
      first check if it is a text file, before using the function.

      @param path
        The path name for the file
      @return
        the file encoding or empty string if the file does not exist or cannot be read.
    */
    const char* getMegaTextFileEncoding(const char* path);

    /**
      @brief Frees a MegaFileInfo structure

      The function deallocates and destroys a MegaFileInfo structure. If the structure is NULL, the function
      simply and does nothing.

      @param pData
        The MegaFileInfo structure to be freed
    */

    void freeMegaFileInfo(MegaFileInfo* pData);

    /**
      @brief Frees a string

      The function deallocates and frees a string returned by any of the functions. The difference between
      this function and the std library  free , is that this function does nothing if its
      argument is NULL

      @param pData
        The string to be freed
    */
    void freeMegaString(char* pData);

    /**
      @brief Frees an array of strings.

      The function frees the dynamically allocated array of dynamically allocated strings returned by
       getMegaMimeExtensions. The function simply does nothing if its argument is NULL

      @param pData
        The string array to be freed.
    */
    void freeMegaStringArray(char** pData);


#endif

#ifdef __cplusplus
}
#endif