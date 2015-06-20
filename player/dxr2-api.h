#ifndef __DXR2_API
#define __DXR2_API

#include <dxr2ioctl.h>
#include "types.h"

typedef struct {
  dxr2_oneArg_t    tv_format;
  dxr2_threeArg_t  video_freq;
  dxr2_oneArg_t    output_aspect_ratio;
  dxr2_oneArg_t    source_aspect_ratio;
  dxr2_oneArg_t    scaling_mode;
  dxr2_oneArg_t    bitstream_type;
  dxr2_oneArg_t    macro_vision;
  dxr2_oneArg_t    interlaced_mode;
  dxr2_oneArg_t    pixel_mode;
  dxr2_oneArg_t    x75ire_mode;
  dxr2_oneArg_t    volume;
  dxr2_oneArg_t    mute;
  dxr2_oneArg_t    audio_width;
  dxr2_oneArg_t    audio_freq;
  dxr2_oneArg_t    iec_output_mode;  // not sure what this is
  dxr2_oneArg_t    subpicture;
  dxr2_twoArg_t    subpicture_stream_id;
  dxr2_twoArg_t    audio_stream;
  dxr2_twoArg_t    video_stream;
  dxr2_fourArg_t   overlay_crop;
  dxr2_twoArg_t    overlay_pos;
  dxr2_twoArg_t    overlay_dim;
  dxr2_oneArg_t    overlay_mode;
  dxr2_oneArg_t    in_delay;
  dxr2_fourArg_t   picture;
  dxr2_fourArg_t   gain;
  dxr2_vgaParams_t vgaBuf;
  dxr2_sixArg_t    color_key;
} dxr2_status_info_t;


extern int dxr2_init(int);
extern int dxr2_set_params(dxr2_status_info_t*);
extern int dxr2_reset();

extern int dxr2_install_firmware(int, dxr2_uCode_t*);

extern int dxr2_set_playmode(int);
extern int dxr2_fast_fowards(int);
extern int dxr2_slow_forwards(int);

extern int dxr2_set_tv_format(int);
extern int dxr2_set_output_ratio(int);
extern int dxr2_set_source_ratio(int);
extern int dxr2_set_scaling_mode(int);
extern int dxr2_set_macro_vision(int);
extern int dxr2_set_pixel_mode(int);
extern int dxr2_set_interlace_mode(int);

extern int dxr2_subpicture(int);
extern int dxr2_select_subpicture(int);

extern int dxr2_select_audio(int, int);
extern int dxr2_set_audio_width(int);
extern int dxr2_set_audio_freq(int);
extern int dxr2_set_volume(int);
extern int dxr2_mute(int);

extern int dxr2_select_video(int);
extern int dxr2_set_video_freq(int);
extern int dxr2_set_video_type(int);

extern int dxr2_overlay_mode(int);
extern int dxr2_set_overlay_ratio(int);
extern int dxr2_set_overlay_geom(geom_t);
extern int dxr2_set_overlay_position(int, int);
extern int dxr2_set_overlay_size(int, int);
extern int dxr2_set_overlay_crop(int, int, int, int);
extern int dxr2_set_in_delay(int);
extern int dxr2_set_overlay_gain(int, int, int, int);
extern int dxr2_set_overlay_picture_controls(int, int, int, int);
extern int dxr2_set_overlay_color_key(int, int, int, int, int, int);

#endif


