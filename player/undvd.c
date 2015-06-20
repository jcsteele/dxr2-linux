/******************************************************************************
 * Written by Scott Bucholtz
 * last modified 2/2/2002
 ******************************************************************************
 * This is the wrapper for unloading the modules and ejecting the DVD after watching a movie
 ******************************************************************************
 */

#include <stdio.h>
/* Begin include for str* functions */
#include <string.h>
/* End include for str* functions */
/* Begin include for *alloc functions */
#include <stdlib.h>
/* End include for *alloc functions */
/* Begin include for set*uid functions */
#include <unistd.h>
/* End include for set*uid functions */

const int SIZE=100;  /* I'm hard-coding this because it's just easier. */
const char *PROC_MTS="/proc/mounts";  /* This is the file to search for the DVD's mount point */
const char *PROC_MOD="/proc/sys/kernel/modprobe";  /* This is the file which stores the exact name of the modprobe program */
const char *MNTPT="/dvd";  /* This is the default mount point of the DVD */
const char *UMNTNAME="umount";  /* This is the default name of the umount program */
const char *MODNAME="/sbin/modprobe";  /* This is the default name of the modprobe program */
const char *MODOPTS=" -r ";  /* This is the option to modprobe which unloads the drivers */
const char *DRIVER="dxr2";  /* This is the name of the driver to be loaded by modprobe */
const char *EJECTNAME="eject";  /* This is the default name of the eject program */

int
main(int argc, char **argv)
{
  FILE *file;  /* This is a pointer to the files in /proc */
  char *mntPtr;  /* This is the pointer where I store the mount points as I'm searching through the file PROC */
  char *dvd_dir=NULL;  /* This will point to the mount point of the DVD */
  char *entry_name=NULL;  /* This will hold the mount strings from PROC */
  char *umount_cmd=NULL;  /* This is the true name of the mount program */
  char input[SIZE];  /* This is where I parse the CONFIG file */
  char *modprobe_cmd=NULL;  /* This is the true name of the modprobe program */
  char *eject_cmd=NULL;  /* This is the true name of the eject program and options */
  int blah=1;
  
  if( argc!=1 ) {
    printf("This program unmounts the DVD, unloads the Dxr2 drivers, and ejects the disk.\n");
    printf("NOTE:  you must have your DVD mounted with the UDF filesystem, be using the\n");
    printf("proc filesystem, and have the eject program.\n");
    exit(1);
  }

  /* This will make sure that the program is running setuid root...don't like it but that's the way it has to be. */
  setuid(0);
  seteuid(0);
  
  /* Here is where I find the mount point of the DVD */
  if( (file=fopen(PROC_MTS, "r"))==NULL ) {
    fprintf(stderr, "Cannot open file %s.  Using default mount point %s...\n", PROC_MTS, MNTPT);
    fclose(file);
  } else {
    entry_name=realloc(entry_name,SIZE);
    while( fgets(entry_name,SIZE-1,file)!=NULL ) {
      strtok(entry_name," ");
      mntPtr=strtok(NULL," ");
      if( strcmp(strtok(NULL," "),"udf")==0 ) {
        blah=0;
        break;  /* We've found the mount point so now we can stop */
      }
      entry_name=realloc(entry_name,SIZE);
      entry_name[0]='\0';
    }
    fclose(file);
    if(blah) fprintf(stderr, "Cannot determine mount point from file %s.  Using default mount point %s...\n", PROC_MTS, MNTPT);
  }
  if(blah) {  /* If we didn't find the mount point then revert to default (will probably terminate shortly) */
    dvd_dir=realloc(dvd_dir,strlen(MNTPT)+1);
    strcpy(dvd_dir,MNTPT);
  } else {
    dvd_dir=realloc(dvd_dir,strlen(mntPtr)+1);
    strcpy(dvd_dir,mntPtr);
  }    
  free(mntPtr);  /* Clearing memory */

  /* Here is where I unmount the disk */
  // There is no place in /proc to search for the actual name of the umount program so I'll cheat hear and assume it's just in the default $PATH
  umount_cmd=realloc(umount_cmd,strlen(UMNTNAME)+strlen(dvd_dir)+2);  /* Leave room for a space between strings */
  strcpy(umount_cmd,UMNTNAME);
  strcat(umount_cmd," ");
  strcat(umount_cmd,dvd_dir);
  system(umount_cmd);
  free(umount_cmd);  /* Clearing memory */

  /* Here is where I find the exact name of the modprobe program */
  if( (file=fopen(PROC_MOD, "r"))==NULL ) {
    fprintf(stderr, "Cannot open file %s.  Using default modprobe name:  %s...\n", PROC_MOD, MODNAME);
    fclose(file);
    modprobe_cmd=realloc(modprobe_cmd,strlen(MODNAME)+1);
    strcpy(modprobe_cmd,MODNAME);
  }  else {
    fgets(input,SIZE-1,file);
    fclose(file);
    modprobe_cmd=realloc(modprobe_cmd,strlen(input)+1);
    strcpy(modprobe_cmd,input);
    modprobe_cmd[strlen(modprobe_cmd)-1]='\0';  /* Get rid of the newline character */
  }
  
  /* Here is where I unload the Dxr2 drivers */
  modprobe_cmd=realloc(modprobe_cmd,strlen(modprobe_cmd)+strlen(MODOPTS)+strlen(DRIVER)+1);
  strcat(modprobe_cmd,MODOPTS);
  strcat(modprobe_cmd,DRIVER);
  system(modprobe_cmd);
  free(modprobe_cmd);  /* Clearing memory */
  input[0]='\0';  /* Resetting input */

  /* Here is where I eject the disk */
  // There is no place in /proc to search for the actual name of the umount program so I'll cheat hear and assume it's just in the default $PATH
  eject_cmd=realloc(eject_cmd,strlen(EJECTNAME)+strlen(dvd_dir)+2);  /* Leave room for a space between strings */
  strcpy(eject_cmd,EJECTNAME);
  strcat(eject_cmd," ");
  strcat(eject_cmd,dvd_dir);
  system(eject_cmd);
  free(eject_cmd);
  free(dvd_dir);
  
  exit(0);
}
