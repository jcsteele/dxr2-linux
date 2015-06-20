#include "overlay.h"
#include <dxr2ioctl.h>
#include <stdio.h>
#include <errno.h>

int init_from_disk(int, char*, dxr2_status_info_t*);
int readParams(FILE*, dxr2_vgaParams_t*);
int save_to_disk(dxr2_vgaParams_t*, char*);
int writeParams(FILE*, dxr2_vgaParams_t*);

int setup_overlay_params(int dxr2FD, dxr2_status_info_t* dxr2_info, char* cacheName)
{
  return init_from_disk(dxr2FD, cacheName, dxr2_info) ||
    calculate_overlay_params(dxr2FD, dxr2_info, cacheName);
}

int init_from_disk(int dxr2FD, char* cacheName, dxr2_status_info_t* dxr2_info)
{
  int xRes, yRes;
  FILE* cachedRes; 
  dxr2_vgaParams_t vgaBuf;

  errno = 0;
  cachedRes = fopen(cacheName, "r");

  if(!errno) {
    getScreenRes(&xRes, &yRes);
  
    while(readParams(cachedRes, &vgaBuf))
      if(vgaBuf.xScreen==xRes && vgaBuf.yScreen==yRes) {
	fclose(cachedRes);
	dxr2_info->vgaBuf = vgaBuf;
	return !ioctl(dxr2FD, DXR2_IOC_SET_VGA_PARAMETERS, &vgaBuf) &&
	  !dxr2_setup_overlay(&vgaBuf, dxr2_info);
      }

    fclose(cachedRes);
  }

  return 0;
}

int readParams(FILE* cache, dxr2_vgaParams_t* vgaBuf)
{
  return fscanf(cache, "%d %d %d %d %d %d %d %d %d %d %d\n",
		&vgaBuf->hOffWinKey,
		&vgaBuf->vOffWinKey,
		&vgaBuf->xScreen,
		&vgaBuf->yScreen,
		&vgaBuf->hsyncPol,
		&vgaBuf->vsyncPol,
		&vgaBuf->blankStart,
		&vgaBuf->blankWidth,
		&vgaBuf->hOffset,
		&vgaBuf->vOffset,
		&vgaBuf->ratio) == 11;
}

int save_to_disk(dxr2_vgaParams_t* vgaBuf, char* cacheName)
{
  FILE* cache = fopen(cacheName, "a");

  if(!errno) {
    writeParams(cache, vgaBuf);
    fclose(cache);
    return 1;
  }

  return 0;
}

int writeParams(FILE* cache, dxr2_vgaParams_t* vgaBuf)
{
  return fprintf(cache, "%d %d %d %d %d %d %d %d %d %d %d\n",
		 vgaBuf->hOffWinKey,
		 vgaBuf->vOffWinKey,
		 vgaBuf->xScreen,
		 vgaBuf->yScreen,
		 vgaBuf->hsyncPol,
		 vgaBuf->vsyncPol,
		 vgaBuf->blankStart,
		 vgaBuf->blankWidth,
		 vgaBuf->hOffset,
		 vgaBuf->vOffset,
		 vgaBuf->ratio) == 11;

}

int calculate_overlay_params(int dxr2FD,
			     dxr2_status_info_t* dxr2_info, char* cacheName)
{
  int pid;
  int xRes;
  int yRes;
  dxr2_sixArg_t buf6;
  dxr2_oneArg_t buf1;
  dxr2_twoArg_t buf2;

  buf6.arg1 = 0;
  buf6.arg2 = 0; 
  buf6.arg3 = 55;
  buf6.arg4 = 300;
  ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_CROPPING, &buf6);                    

  // get screen res
  getScreenRes(&xRes, &yRes);

  // OK, fork off another process to do the whitescreen thang.
  pid = fork();
  if (pid == 0) { // i.e. the child process
    
    whitescreen();
    exit(0);
  }

  // set colour key
  buf6.arg1 = 0x40; // Red min
  buf6.arg2 = 0xff; // Red Max
  buf6.arg3 = 0x40; // Green
  buf6.arg4 = 0xff;
  buf6.arg5 = 0x40; // Blue
  buf6.arg6 = 0xff;
  print_info("set overlay colour status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_COLOUR, &buf6));

  // setup window key
  buf6.arg1 = 1000;
  print_info("set overlay ratio status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_RATIO, &buf6));

  buf6.arg1 = 100;
  buf6.arg2 = 3;
  print_info("set overlay position status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_POSITION, &buf6));

  buf6.arg1 = xRes;
  buf6.arg2 = yRes;
  print_info("set overlay dimension status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_DIMENSION, &buf6));

  // back to window keying
  buf1.arg = 3;
  print_info("set overlay mode status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_IN_DELAY, &buf1));


  // turn window AND colour keying on
  buf1.arg = DXR2_OVERLAY_WINDOW_COLOUR_KEY;
  print_info("set overlay mode status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_MODE, &buf1));

  // calculate VGA parameters
  dxr2_info->vgaBuf.xScreen = xRes;
  dxr2_info->vgaBuf.yScreen = yRes;
  dxr2_info->vgaBuf.hOffWinKey = 100;
  dxr2_info->vgaBuf.vOffWinKey = 3;
  print_info("calc = %i\n", ioctl(dxr2FD, DXR2_IOC_CALCULATE_VGA_PARAMETERS, &dxr2_info->vgaBuf));

  // set VGA parameters
  print_info("set = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_VGA_PARAMETERS, &dxr2_info->vgaBuf));

  kill( pid, 9 ) ;

  // OK, setup the overlay properly
  dxr2_setup_overlay(&dxr2_info->vgaBuf, dxr2_info);
  return save_to_disk(&dxr2_info->vgaBuf, cacheName);
}


