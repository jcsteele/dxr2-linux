/******************************************************************************
 * Written by Scott Bucholtz
 * last modified 3/1/2002
 ******************************************************************************
 * This is the new wrapper for watching DVD's
 ******************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "player.h"  /* This is so I have the player_info_t type.  This header file is found in dxr2-driver/player/ */

const int SIZE=100;  /* I'm hard-coding this because it's just easier. */
const char *PROC="/proc/sys/kernel/modprobe";  /* This is the file which stores the exact name of the modprobe program */
const char *MODNAME="/sbin/modprobe";  /* This is the default name of the modprobe program */
const char *DRIVER=" dxr2";  /* This is the name of the driver to be loaded by modprobe */
const char *MNTNAME="mount";  /* This is the default name of the mount program */
const char *MNTOPTS=" -t udf ";  /* These are the options given to MNTNAME */
const char *CONFIG="/etc/dxr2player.conf";  /* This is the location of the Dxr2 config file */
const char *PLAYER="dvdplay-curses";  /* This is the default name of the actual curses based player */
const char *PLAYEROPTS=" -cT ";  /* These are the options given to PLAYER */


int
main(int argc, char **argv) {
  FILE *file;  /* This is a pointer to the file I'm using (either PROC or CONFIG) */
  player_info_t player_info;  /* This is how the CONFIG file is split up */
  char *modprobe_name=NULL;  /* This is the true name of the modprobe program */
  char *mount_name=NULL;  /* This is the true name of the mount program */
  char *play_it=NULL;  /* This is the true name of the player */
  char input[SIZE];  /* This is where I parse the CONFIG file */
  
  if( argc>2 ) {
    printf("This program determines the location of modprobe (usually /sbin/modprobe),\n");
    printf("loads the Dxr2 modules, mounts the DVD via the UDF filesystem, and starts the\n");
    printf("dvdplay-curses program.\n\n");
    printf("If the IFO parser doesn't work, try running this program with a title option like:\n");
    printf("\t%s `dvd-sum`\n\n",argv[0]);
    printf("NOTE:  you must have your DVD mount info in /etc/dxr2player.conf and be using the\n");
    printf("proc filesystem for this to work.\n");
    exit(1);
  }
  
  /* This will make sure that the program is running setuid root...don't like it but that's the way it has to be. */
  setuid(0);
  seteuid(0);

  /* Here is where I find the exact name of the modprobe program */
  if( (file=fopen(PROC, "r"))==NULL ) {
    fprintf(stderr, "Cannot open file %s.  Using default modprobe name:  %s...\n", PROC, MODNAME);
    fclose(file);
    modprobe_name=realloc(modprobe_name,strlen(MODNAME)+1);
    strcpy(modprobe_name,MODNAME);
  }  else {
    fgets(input,SIZE-1,file);
    fclose(file);
    modprobe_name=realloc(modprobe_name,strlen(input)+1);
    strcpy(modprobe_name,input);
    modprobe_name[strlen(modprobe_name)-1]='\0';  /* Get rid of the newline character */
  }

  /* Here is where I load the Dxr2 drivers */
  modprobe_name=realloc(modprobe_name,strlen(modprobe_name)+strlen(DRIVER)+1);
  strcat(modprobe_name,DRIVER);
  system(modprobe_name);
  free(modprobe_name);  /* Clearing memory */
  input[0]='\0';  /* Resetting input */
  
  /* Here is where I mount the DVD...hopefully... */
  if( (file=fopen(CONFIG, "r"))==NULL ) {
    fprintf(stderr, "Cannot open the Dxr2 config file %s.  You must have this file in order to use the card.\n", CONFIG);
    fclose(file);
    exit(-3);
  } else {
    /* This "else" section was blatently stolen from player/config-files.c (and then chopped apart).  :-P  */
    memset(&player_info, 0, sizeof(player_info_t));
    while(!feof(file)) {
      fscanf(file, "%s", input);
      if(input[0]=='#')  /* skip to end of line */
        while(input[0]!='\n') fscanf(file, "%c", input);
      else {  /* Let's get the only two I need here... */
        if(strcmp(input, "drive:")==0) 
          fscanf(file, "%s", player_info.dvd_device);
        else if(strcmp(input, "mountpoint:")==0) 
          fscanf(file, "%s", player_info.dvd_mountpoint);
      }
    }
    fclose(file);
  }
  // There is no place in /proc to search for the actual name of the mount program so I'll cheat hear and assume it's just in the default $PATH
  mount_name=realloc(mount_name,strlen(MNTNAME)+strlen(MNTOPTS)+strlen(player_info.dvd_device)+strlen(player_info.dvd_mountpoint)+2);  /* Leave room for a space between options */
  strcpy(mount_name,MNTNAME);
  strcat(mount_name,MNTOPTS);
  strcat(mount_name,player_info.dvd_device);
  strcat(mount_name," ");
  strcat(mount_name,player_info.dvd_mountpoint);
  system(mount_name);
  free(mount_name);  /* Clearing memory */

  /* Here is where I start the player (and give it the correct title to play) */
  // Just like with "mount" above, I'm going to assume that dvdplay-curses and dvd-sum are in the default $PATH
  play_it=realloc(play_it,strlen(PLAYER)+strlen(PLAYEROPTS)+1);
  strcpy(play_it,PLAYER);
  strcat(play_it,PLAYEROPTS);
  if( argc==2 ) {
    play_it=realloc(play_it,strlen(play_it)+strlen(argv[1])+1);
    strcat(play_it,argv[1]);
  } else {
    play_it=realloc(play_it,strlen(play_it)+2);
    strcat(play_it,"1");  /*  In theory (and if the IFO parser works), the movie should always be on title #1.  */
  }
  system(play_it);
  free(play_it);  /* Clearing memory */
  
  exit(0);
}
