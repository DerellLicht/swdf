//**********************************************************************************
//  swdf.cpp - Stalker equipment Degradation Fix
//  
//  Written by:  Derell Licht
//  build:  g++ -Wall -O2 -s swdf.cpp -o swdf.exe
//**********************************************************************************

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>  //  PATH_MAX

#ifndef PATH_MAX
#define PATH_MAX  260
#endif

typedef  unsigned char  uchar ;
typedef  unsigned int   uint ;
typedef  unsigned long  ulong ;

#define  LOOP_FOREVER   1
#define  HTAB     9
#define  SPC      32

//  this definition was excluded by WINNT.H
#define FILE_ATTRIBUTE_VOLID  0x00000008

#define  MAX_FILE_LEN   1024
#define  MAX_LINE_LEN   1024

WIN32_FIND_DATA fdata ; //  long-filename file struct

//  per Jason Hood, this turns off MinGW's command-line expansion, 
//  so we can handle wildcards like we want to.                    
//lint -e765  external '_CRT_glob' could be made static
//lint -e714  Symbol '_CRT_glob' not referenced
int _CRT_glob = 0 ;

uint filecount = 0 ;

//lint -esym(843, show_all)
bool show_all = true ;

//lint -esym(534, FindClose)  // Ignoring return value of function
//lint -esym(818, filespec, argv)  //could be declared as pointing to const
//lint -e10  Expecting '}'
//lint -esym(818, infile) ; variable could be declared as pointing to const
//lint -esym(534, swdf_func)  Ignoring return value of function 

union u64toul {
   ULONGLONG i ;
   ulong u[2] ;
};

//************************************************************
struct ffdata {
   uchar          attrib ;
   FILETIME       ft ;
   ULONGLONG      fsize ;
   char           *filename ;
   uchar          dirflag ;
   struct ffdata  *next ;
} ;
ffdata *ftop  = NULL;
ffdata *ftail = NULL;

//**********************************************************************
//  skips past leadings spaces and hard tabs in a line,
//  so program act on actual data.
//  Note that the line is not actually modified, this just returns a pointer,
//  so original line with spaces can be retained, if required.
//**********************************************************************
char const *strip_leading_spaces(char const * const str)
{
   if (str == 0)
      return 0;
   char const *tptr = str ;
   while (LOOP_FOREVER) {
      if (*tptr == 0)
         return tptr;
      if (*tptr != SPC  &&  *tptr != HTAB)
         return tptr;
      tptr++ ;
   }
}

//**********************************************************************************
//  build a linked-list of all matching filenames
//**********************************************************************************
int read_files(char *filespec)
{
   int done, fn_okay ;
   HANDLE handle;
   ffdata *ftemp;

   handle = FindFirstFile (filespec, &fdata);
   //  according to MSDN, Jan 1999, the following is equivalent
   //  to the preceding... unfortunately, under Win98SE, it's not...
   // handle = FindFirstFileEx(target[i], FindExInfoStandard, &fdata, 
   //                      FindExSearchNameMatch, NULL, 0) ;
   if (handle == INVALID_HANDLE_VALUE) {
      return -errno;
   }

   //  loop on find_next
   done = 0;
   while (!done) {
      if (!show_all) {
         if ((fdata.dwFileAttributes & 
            (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)) != 0) {
            fn_okay = 0 ;
            goto search_next_file;
         }
      }
      //  filter out directories if not requested
      if ((fdata.dwFileAttributes & FILE_ATTRIBUTE_VOLID) != 0)
         fn_okay = 0;
      else if ((fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
         fn_okay = 1;
      //  For directories, filter out "." and ".."
      else if (fdata.cFileName[0] != '.') //  fn=".something"
         fn_okay = 1;
      else if (fdata.cFileName[1] == 0)   //  fn="."
         fn_okay = 0;
      else if (fdata.cFileName[1] != '.') //  fn="..something"
         fn_okay = 1;
      else if (fdata.cFileName[2] == 0)   //  fn=".."
         fn_okay = 0;
      else
         fn_okay = 1;

      if (fn_okay) {
         // printf("DIRECTORY %04X %s\n", fdata.attrib, fdata.cFileName) ;
         // printf("%9ld %04X %s\n", fdata.file_size, fdata.attrib, fdata.cFileName) ;
         filecount++;

         //****************************************************
         //  allocate and initialize the structure
         //****************************************************
         // ftemp = new ffdata;
         ftemp = (struct ffdata *) malloc(sizeof(ffdata)) ;
         if (ftemp == NULL) {
            return -errno;
         }
         memset((char *) ftemp, 0, sizeof(ffdata));

         //  convert filename to lower case if appropriate
         // if (!n.ucase)
         //    strlwr(fblk.name) ;

         ftemp->attrib = (uchar) fdata.dwFileAttributes;

         //  convert file time
         // if (n.fdate_option == FDATE_LAST_ACCESS)
         //    ftemp->ft = fdata.ftLastAccessTime;
         // else if (n.fdate_option == FDATE_CREATE_TIME)
         //    ftemp->ft = fdata.ftCreationTime;
         // else
         //    ftemp->ft = fdata.ftLastWriteTime;
         ftemp->ft = fdata.ftLastAccessTime;

         //  convert file size
         u64toul iconv;
         iconv.u[0] = fdata.nFileSizeLow;
         iconv.u[1] = fdata.nFileSizeHigh;
         ftemp->fsize = iconv.i;

         ftemp->filename = (char *) malloc(strlen ((char *) fdata.cFileName) + 1);
         strcpy (ftemp->filename, (char *) fdata.cFileName);

         ftemp->dirflag = ftemp->attrib & FILE_ATTRIBUTE_DIRECTORY;

         //****************************************************
         //  add the structure to the file list
         //****************************************************
         if (ftop == NULL) {
            ftop = ftemp;
         }
         else {
            ftail->next = ftemp;
         }
         ftail = ftemp;
      }  //  if file is parseable...

search_next_file:
      //  search for another file
      if (FindNextFile (handle, &fdata) == 0)
         done = 1;
   }

   FindClose (handle);
   return 0;
}

//*****************************************************************
//  lines which will be modified by this program
//*****************************************************************
static char const * const tgtstr[] = {
//  weapon tags
"condition_queue_shot_dec",
"condition_shot_dec",
//  armour tags
"burn_immunity",
"chemical_burn_immunity",
"explosion_immunity",
"fire_wound_immunity",
"radiation_immunity",
"shock_immunity",
"strike_immunity",
"telepatic_immunity",
"wound_immunity",
NULL };

//*****************************************************************
//  scan all lines in one file.
//  Original file is renamed to <filename.ext.bak> before reading.
//  Modified filename will have original <filename.ext>
//*****************************************************************
int swdf_func(char *infile)
{
   char workfile[MAX_FILE_LEN+1] = "";
   char backfile[MAX_FILE_LEN+1];

   if (infile == NULL  ||  *infile == 0) {
      puts("invalid input file");
      return -1;
   }
   strcpy(workfile, infile);
   sprintf(backfile, "%s.bak", workfile);
   int result = rename(workfile, backfile);
   if (result != 0) {
      printf("rename: %s: %s\n", workfile, strerror(errno));
      return -1;
   }

   puts("");
   puts("==========================================");
   printf("original (backup) file: %s\n", backfile);
   printf("modified file: %s\n", workfile);
   FILE* infd = fopen(backfile, "rt");
   if (infd == NULL) {
      printf("%s: %s\n", backfile, strerror(errno));
      return -errno;
   }
   FILE* outfd = fopen(workfile, "wt");
   if (outfd == NULL) {
      printf("%s: %s\n", workfile, strerror(errno));
      return -errno;
   }

//condition_shot_dec      = 0.0001 ;увеличение износа при каждом выстреле
   char inpstr[MAX_LINE_LEN+1];
   char outpstr[MAX_LINE_LEN+1];
   while (fgets(inpstr, MAX_LINE_LEN, infd) != NULL) {
      //  skip any leading whitespace
      char const *hd = strip_leading_spaces(inpstr);

      bool found = false ;
      uint idx ;
      for (idx=0; tgtstr[idx] != NULL; idx++) {
         if (strncmp(hd, tgtstr[idx], strlen(tgtstr[idx])) == 0) {
            char const *tl = strchr(hd, '=');
            if (tl == NULL) {
               puts(inpstr);
               printf("invalid string format!!  aborting...\n");
               break;
            }
            sprintf(outpstr, "%s = 0.0 ;%s", tgtstr[idx], tl);
            printf("%s", outpstr);
            fputs(outpstr, outfd);
            found = true ;
            break;
         }
      }
      //  everything *except* target strings should just get copied to output
      if (!found) {
         fputs(inpstr, outfd);
      }
   }
   fclose(infd);
   fclose(outfd);
   return 0;
}

//**********************************************************************************
char file_spec[PATH_MAX+1] = "" ;

int main(int argc, char **argv)
{
   int idx ;
   for (idx=1; idx<argc; idx++) {
      char *p = argv[idx] ;
      strncpy(file_spec, p, PATH_MAX);
      file_spec[PATH_MAX] = 0 ;
   }

   if (file_spec[0] == 0) {
      puts("Usage: swdf <filespec>");
      return -1;
   }

   int result = read_files(file_spec);
   if (result < 0) {
      printf("filespec: %s, %s\n", file_spec, strerror(-result));
      return -result;
   }

   //  parse each file and apply the working function
   printf("filespec: %s, %u found\n", file_spec, filecount);
   if (filecount > 0) {
      for (ffdata *ftemp = ftop; ftemp != NULL; ftemp = ftemp->next) {
         // printf("%s\n", ftemp->filename);
         swdf_func(ftemp->filename);
      }
   }
   return 0;
}

