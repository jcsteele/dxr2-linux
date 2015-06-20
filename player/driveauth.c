#include <stdio.h>
#include <stdlib.h>
#include <dxr2ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h> 
#include <signal.h>
#include <fcntl.h>
#include "css.h"
#include "player.h"
#include "dxr2-api.h"
#include "config-files.h"

void install_firmware(player_info_t* player_info);

int main(int argc, char** argv)
{
  player_info_t player_info;
  dxr2_status_info_t dxr2_info;
  dxr2_css_info *info;
  int fd;

  memset(&player_info, 0, sizeof(player_info_t));
  memset(&dxr2_info, 0, sizeof(dxr2_status_info_t));

  read_config(&player_info, &dxr2_info);

  if ((player_info.dxr2FD = open(player_info.dxr2_device, O_WRONLY)) < 0) {
    fprintf(stderr, "ERROR: Cannot open DVD device (%s): %s\n", "/dev/dxr2", strerror(errno));
  }  
  else
  {
    if ((fd=open(player_info.dvd_device, O_RDONLY)) < 0)
    {
      fprintf(stderr, "ERROR: Cannot open DVD drive (%s)\n", player_info.dvd_device);
    }
    else
    {
      install_firmware(&player_info);
      dxr2_init(player_info.dxr2FD);
      dxr2_css_set_dxr2_fd(player_info.dxr2FD);
      if (!(info=dxr2_css_open(fd)))
      {
	fprintf(stderr, "ERROR: Cannot authenticate with DVD drive (%s)\n", player_info.dvd_device);
      }
      dxr2_css_close(info);
      close(fd);
    }
  }
}

void install_firmware(player_info_t* player_info)
{
  int uCodeFD;
  int uCodeSize;
  dxr2_uCode_t* uCode;

  if ((uCodeFD = open(player_info->uCode_file, O_RDONLY)) < 0) {
    
    print_error("ERROR: Could not open uCode (%s): %s\n", player_info->uCode_file, strerror(errno));
    exit(0);
  }
  uCodeSize = lseek(uCodeFD, 0, SEEK_END);
  if ((uCode = malloc(uCodeSize + 4)) == NULL) {
    
    print_error("ERROR: Could not allocate memory for uCode: %s\n", strerror(errno));
    exit(0);
  }
  lseek(uCodeFD, 0, SEEK_SET);
  if (read(uCodeFD, uCode+4, uCodeSize) != uCodeSize) {
    
    print_error("ERROR: Could not read uCode uCode: %s\n", strerror(errno));
    exit(0);
  }
  close(uCodeFD);
  uCode->uCodeLength = uCodeSize;

  // upload ucode
  if (dxr2_install_firmware(player_info->dxr2FD, uCode)) {
    
    print_error("uCode upload failed!\n");
    exit(0);
  }
}
