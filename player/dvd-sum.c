/******************************************************************************
 * Written by Scott Bucholtz
 * last modified 1/23/02
 ******************************************************************************
 * This program should always determine which DVD title contains the movie
 * if the IFO parser fails.
 ******************************************************************************
 */

#include <stdio.h>
/* Begin include for str* functions */
#include <string.h>
/* End include for str* functions */
/* Begin include for *alloc functions */
#include <stdlib.h>
/* End include for *alloc functions */
/* Begin includes for *dir */
#include <sys/types.h>
#include <dirent.h>
/* End includes for *dir */
/* Begin includes for stat */
#include <sys/stat.h>
#include <unistd.h>
/* End includes for stat */


const int SIZE=100;  /* I'm hard-coding this because it's just easier. */
const char *PROC="/proc/mounts";  /* This is the file to search for the DVD's mount point */
const char *MNTPT="/dvd";  /* This is the default mount point of the DVD */

int
main(int argc, char **argv)
{
  FILE *mounts;  /* This is a pointer to the file PROC */
  DIR *directory;  /* This is a pointer to the directory open for reading */
  struct dirent *directory_entry;  /* This is the structure which will contain the current listing inside the directory */
  struct stat entry_info;  /* This is the structure which will contain information about the current listing inside the directory */
  int idx=0,  /* This is a counter for the index of the GMP array */
    current=0,  /* This tells us which collection of VOB files we're looking at */
    blah=1,  /* This is where I store the specific VOB file number -- it is not importaint */
    largest;  /* This is where I will put the number of the index of teh GMP array which is largest */
  char *mntPtr;  /* This is the pointer where I store the mount points as I'm searching through the file PROC */
  char *dvd_dir=NULL;  /* This will point to the mount point of the DVD */
  char *video_dir="/VIDEO_TS";  /* This is the directory inside the DVD where the VOB files are located */
  char *entry_name=NULL;  /* This will hold the actual name of the entry (i.e., filename) for the stat calls */
  unsigned long long title_size[SIZE];  /* This is the array which contains the sizes of the collection of VOB files -- I don't like hard-coding the size but actually, it's not too bad */
  
  if( argc!=1 ) {
    printf("This program returns the location of the actual movie on a mounted DVD\n");
    printf("and is used by dvdplay.\n\n");
    printf("NOTE:  you must have your DVD mounted with the UDF filesystem and be using the\n");
    printf("proc filesystem.\n");
    exit(1);
  }
  
  /* Here is where I find the mount point of the DVD */
  if( (mounts=fopen(PROC, "r"))==NULL ) {
    fprintf(stderr, "Cannot open file %s.  Using default mount point %s...\n", PROC, MNTPT);
    fclose(mounts);
  } else {
    entry_name=realloc(entry_name,SIZE);
    while( fgets(entry_name,SIZE-1,mounts)!=NULL ) {
      strtok(entry_name," ");
      mntPtr=strtok(NULL," ");
      if( strcmp(strtok(NULL," "),"udf")==0 ) {
	blah=0;
	break;  /* We've found the mount point so now we can stop */
      }
      entry_name=realloc(entry_name,SIZE);
      entry_name[0]='\0';
    }
    fclose(mounts);
    if(blah) fprintf(stderr, "Cannot determine mount point from file %s.  Using default mount point %s...\n", PROC, MNTPT);
  }
  if(blah) {  /* If we didn't find the mount point then revert to default (will probably terminate shortly) */
    dvd_dir=realloc(dvd_dir,strlen(MNTPT)+1);
    strcpy(dvd_dir,MNTPT);
  } else {
    dvd_dir=realloc(dvd_dir,strlen(mntPtr)+1);
    strcpy(dvd_dir,mntPtr);
  }    
  free(mntPtr);  /* Clearing memory */
  
  /* Set the directory to the VOB files */
  dvd_dir=realloc(dvd_dir,strlen(dvd_dir)+strlen(video_dir)+2);  /* Leave room for a "/" at the end */
  strcat(dvd_dir,video_dir);
  strcat(dvd_dir,"/");  /* ...just to be safe... */
  if( (directory=opendir(dvd_dir))==NULL ) {  /* Validate reading of the directory */
    fprintf(stderr, "Cannot read from directory %s\nBummer.\n", dvd_dir);
    closedir(directory);
    exit(-1);
  }

  /* Initialize title sizes */
  while(current<SIZE) title_size[current++]=0;
  /* Parse the directory for valid VOB files and add their sizes to respective titles */
  while( (directory_entry=readdir(directory))!=NULL ) {
    if( (sscanf(directory_entry->d_name,"VTS_%d_%d.VOB",&current,&blah)==2) && (strstr(directory_entry->d_name,"VOB")!=NULL) ) {
      entry_name=realloc(entry_name,strlen(dvd_dir)+strlen(directory_entry->d_name)+1);
      strcpy(entry_name,dvd_dir);  /* So basically, for each entry, I'm creating the string:  entry_name */
      strcat(entry_name,directory_entry->d_name); /* and it will hold the exact path to the filenames for stat. */
      if( stat(entry_name, &entry_info)==0 ) {
	if( current!=idx ) idx=current;
	if( idx==SIZE ) {
	  /* This should be invalid acording to the rules  :)  */
	  fprintf(stderr, "Invalid number of DVD titles.  Please contact Scott Bucholtz at:\n");
	  fprintf(stderr, "\tlinuxman@linuxmanland.com\n");
	  exit(-3);
	}
	title_size[idx]+=entry_info.st_size;
      }
    }
  }
  free(entry_name);  /* Clearing memory */
  free(dvd_dir);  /* Clearing memory */
  closedir(directory);
  
  /* Now we find the largest */
  /* This whole thing works because the smallest value of current MUST be 1 (see sscanf() above) */
  for( current=1 ; current<=idx ; current++ ) {
    if( title_size[0]<=title_size[current] ) {
      largest=current;
      title_size[0]=title_size[current];
    }
  }
  
  /* Only print the largest if a valid title was found */
  if(largest) printf("%d\n", largest); else exit(-2);
  exit(0);
}
