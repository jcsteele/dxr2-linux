#include "overlay.h"
#include "XOverlay.h"
#include "dxr2-api.h"
#include <math.h>

dxr2_status_info_t* dxr2_info;

static float x_scale = 1;
static int y_scale = 1;
static int cur_mode;

void init_overlay(int argc, char** argv, dxr2_status_info_t* _dxr2_info)
{
  geom_t geom;

  dxr2_info = _dxr2_info;

  geom.x = dxr2_info->overlay_pos.arg1;
  geom.y = dxr2_info->overlay_pos.arg2;
  geom.width = dxr2_info->overlay_dim.arg1;
  geom.height = dxr2_info->overlay_dim.arg2;
  
  cur_mode = dxr2_info->overlay_mode.arg;

  init_win(&argc, argv, geom);
  set_geom_fn( &resize_overlay );
  set_switch_fn( &overlay_switch );
}

void dest_overlay()
{
  destwin();
}

int resize_overlay(geom_t geom)
{
  if(cur_mode == DXR2_OVERLAY_DISABLED && geom.y > 0)
    overlay_switch();

  dxr2_info->overlay_pos.arg1 = geom.x;
  dxr2_info->overlay_pos.arg2 = geom.y;
  dxr2_info->overlay_dim.arg1 = geom.width;
  dxr2_info->overlay_dim.arg2 = geom.height;

  dxr2_set_overlay_geom(geom);
}

int overlay_switch()
{
#ifdef __X_DVD_DEBUG
  print_info("Overlay cur_mode: %d\n", dxr2_info->overlay_mode.arg);
#endif
  if(dxr2_info->overlay_mode.arg == cur_mode)
    cur_mode = DXR2_OVERLAY_DISABLED;
  else
    cur_mode = dxr2_info->overlay_mode.arg;

  dxr2_set_overlay_mode(cur_mode);
}

