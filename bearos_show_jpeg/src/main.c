/*=========================================================================

  bearos_show_jpeg
 
  main.c

  Copyright (c)2023 Kevin Boone, GPL 3.0

=========================================================================*/
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#ifdef BEAROS
#include <bearos/devctl.h>
#endif
#include "picojpeg.h"

#define TRUE 1
#define FALSE 0
typedef int BOOL;

#define MIN ((x) < (y) ? (x) : (y))

/*=========================================================================
  pjpeg_need_bytes_callback 
  This function is called by the JPEG decoder to read data from the file.
  It always succeeds, until end of file. This means that, in principle, 
  a defective file (of the wrong length) might get partially displayed.
=========================================================================*/
unsigned char pjpeg_need_bytes_callback (unsigned char* pBuf, 
    unsigned char buf_size, unsigned char *pBytes_actually_read, 
    void *pCallback_data)
  {
  FILE *f = (FILE *)pCallback_data; 
  int n = fread(pBuf, 1, buf_size, f);
  *pBytes_actually_read = (unsigned char)n;
  return 0;
  }

/*=========================================================================
  rgb888_to_rgb565
=========================================================================*/
static uint16_t rgb888_to_rgb565 (uint8_t r, uint8_t g, uint8_t b)
  {
  return (((uint16_t)r & 0xF8) << 8) | (((uint16_t)g & 0xFC) << 3) | (b >> 3);
  }

/*=========================================================================
  transfer
  Transfer a data block from the JPEG decode to the display 
=========================================================================*/
static void transfer (int fd, int x, int y, int cx, int cy, uint16_t *block)
  {
#ifdef BEAROS
  DevCtlGfxRegion region;
  region.x = x;
  region.y = y;
  region.cx = cx;
  region.cy = cy;
  devctl (fd, DC_GFX_SET_REGION, (int32_t)&region);
#endif 

  write (fd, block, 2 * cx * cy);
  //printf ("target_x = %d target_y = %d cx=%d cy=%d\n", x, y, cx, cy);
  }

/*=========================================================================
  show_jpeg
=========================================================================*/
int show_jpeg (const char *argv0, const char *filename, const char *gfx_dev, 
      BOOL verbose)
  {
  int ret = 0;
  pjpeg_image_info_t image_info;
  int fd = open (gfx_dev, O_RDWR);
  if (fd >= 0)
    {
    BOOL isgfx = FALSE;
#ifdef BEAROS 
    DevCtlGfxProps props;
    devctl (fd, DC_GFX_GET_PROPS, (int32_t)&props);
    int display_width = props.width;
    int display_height = props.height;
    int32_t flags;
    devctl (fd, DC_GET_GEN_FLAGS, (int32_t)&flags);
    if (flags & DC_FLAG_ISGFX)
      isgfx = TRUE;
#else
    int display_width = 100; // TODO
    int display_height = 100; // TODO
    isgfx = TRUE; // Well, how else can we test?
#endif
    if (isgfx)
      {
      FILE *fin = fopen (filename, "r");
      if (fin)
	{
	unsigned char r = pjpeg_decode_init (&image_info, 
		pjpeg_need_bytes_callback, fin, 0);

	if (r == 0)
	  {
	  int decoded_width = image_info.m_width;
	  int decoded_height = image_info.m_height;
	  int block_width = image_info.m_MCUWidth;
	  int block_height = image_info.m_MCUHeight;
	  if (verbose)
	    {
	    printf ("width %d, height %d, block size %dx%d\n", 
	      decoded_width, decoded_height, block_width, block_height);
	    }
	  uint16_t *block = 
	    malloc (block_width * block_height * sizeof (uint16_t));

	  int mcu_x = 0, mcu_y = 0;
	  for (;;)
	    {
	    unsigned char r = pjpeg_decode_mcu();
	    if (r)
	      {
	      if (r != PJPG_NO_MORE_BLOCKS)
		{
	  	fprintf (stderr, 
	           "%s: defective JPEG file: %s\n", argv0, filename);
		}
	      break;
	      }

	    int target_x = mcu_x * block_width;
	    int target_y = mcu_y * block_height;

	    for (int y = 0; y < block_height; y += 8)
	      {
	      int by_limit = decoded_height - (mcu_y * block_height + y);
	      if (by_limit > 8) by_limit = 8;

	      for (int x = 0; x < block_width; x += 8)
		{
		int bx_limit = decoded_width - (mcu_x * block_width + x);
		if (bx_limit > 8) bx_limit = 8;

		int src_ofs = (x * 8U) + (y * 16U);
		uint8_t *pSrcR = image_info.m_pMCUBufR + src_ofs;
		uint8_t *pSrcG = image_info.m_pMCUBufG + src_ofs;
		uint8_t *pSrcB = image_info.m_pMCUBufB + src_ofs;

		int bc = 0;
		for (int by = 0; by < by_limit; by++)
		  {
		  for (int bx = 0; bx < bx_limit; bx++)
		     {
		     uint8_t r = *pSrcR++;
		     uint8_t g = *pSrcG++;
		     uint8_t b = *pSrcB++;
		     block [bc] = rgb888_to_rgb565 (r, g, b); 
		     bc++; 
		     }

		  pSrcR += (8 - bx_limit);
		  pSrcG += (8 - bx_limit);
		  pSrcB += (8 - bx_limit);
		  }

		if (target_x + bx_limit < display_width &&
		     target_y + by_limit < display_height)
		  {
		  transfer (fd, target_x, target_y, 
                     bx_limit, by_limit, block);
		  }
		}
	      }

	    mcu_x++;
	    if (mcu_x == image_info.m_MCUSPerRow)
	      {
	      mcu_x = 0;
	      mcu_y++;
	      }
	    }
	  free (block);
	  }
	else if (r == PJPG_UNSUPPORTED_MODE)
	  {
	  fprintf (stderr, 
	     "%s: %s: progressive encoding is not yet supported\n", 
	     argv0, filename);
	  ret = EINVAL;
	  }
	else
	  {
	  fprintf (stderr, 
	     "%s: %s: bad JPEG file (error %d)\n", 
	     argv0, filename, r);
	  ret = EINVAL;
	  }

	fclose (fin);
	}
      else
        {
        ret = errno;
        fprintf (stderr, "%s: Can't open %s\n", argv0, filename);
        }
      }
    else
      {
      ret = EINVAL;
      fprintf (stderr, "%s: Not a graphics device: %s\n", argv0, gfx_dev);
      }
    close (fd);
    }
  else
    {
    ret = errno;
    fprintf (stderr, "%s: Can't open graphics device %s\n", argv0, gfx_dev);
    }

  return ret;
  }

/*=========================================================================
  show_usage 
=========================================================================*/
static void show_usage (const char *argv0)
  {
  printf ("Usage: %s [options] {JPEG file}\n", argv0); 
  printf ("   -b:   blank display first\n");
#ifdef BEAROS
  printf ("   -d:   graphics device (default %s)\n", BEAROS_DEFAULT_GFX);
#else
  printf ("   -d:   graphics device\n");
#endif
  printf ("   -i:   show file information\n");
  printf ("   -v:   show version\n");
  printf ("Displays a JPEG image\n");
  }

/*=========================================================================
  show_version
=========================================================================*/
static void show_version (const char *argv0)
  {
  printf ("%s for BearOS version " VERSION "\n", argv0); 
  printf ("Copyright (c)2022 Kevin Boone, GPL3\n");
  }

/*=========================================================================
  clear 
=========================================================================*/
static int clear (const char *argv0, const char *gfx_dev)
  {
  int ret = 0;
  int fd = open (gfx_dev, O_RDWR);
  if (fd >= 0)
    {
#ifdef BEAROS 
    int32_t flags;
    devctl (fd, DC_GET_GEN_FLAGS, (int32_t)&flags);
    if (flags & DC_FLAG_ISGFX)
      { 
      DevCtlGfxProps props;
      devctl (fd, DC_GFX_GET_PROPS, (int32_t)&props);
      int display_width = props.width;
      int display_height = props.height;

      DevCtlGfxRegion region;
      region.x = 0;
      region.y = 0;
      region.cx = display_width;
      region.cy = display_height;
      devctl (fd, DC_GFX_SET_REGION, (int32_t)&region);

      DevCtlGfxColour colour;
      // Setting the RGB888 field will set any smaller fields as well.
      colour.rgb888 = 0;
      devctl (fd, DC_GFX_FILL, (int32_t)&colour);
      }
    else
      {
      fprintf (stderr, "%s: not a graphics device %s\n", argv0, gfx_dev);
      ret = EINVAL;
      }
#else
#endif
    close (fd);
    }
  else
    {
    ret = errno;
    fprintf (stderr, "%s: Can't open graphics device %s\n", argv0, gfx_dev);
    }

  return ret;
  }

/*=========================================================================
  main 
=========================================================================*/
int main (int argc, char **argv)
  {
  char gfx_dev[128]; // TODO
#ifdef BEAROS
  strcpy (gfx_dev, BEAROS_DEFAULT_GFX);
#else
  strcpy (gfx_dev, "/dev/null");
#endif

  int opt;
  int ret = 0;
  optind = 0;
  BOOL usage = FALSE;
  BOOL version = FALSE;
  BOOL info = FALSE;
  BOOL blank = FALSE;

  while (((opt = getopt (argc, argv, "bd:hvi")) != -1) && (ret == 0))
    {
    switch (opt)
      {
      case 'b':
        blank = TRUE;
        break;
      case 'v':
        version = TRUE;
        break;
      case 'h':
        usage = TRUE;
        break;
      case 'd':
        strcpy (gfx_dev, optarg);
        break;
      case 'i':
        info = TRUE;
        break;
      default:
        ret = -1; 
      }
    }

  if (usage)
    {
    ret = -1;
    show_usage (argv[0]);
    }  

  if (version)
    {
    ret = -1;
    show_version (argv[0]);
    }  

  if (ret == 0)
    {
    if (argc - optind == 1)
      {
      if (blank)
        clear (argv[0], gfx_dev);
      show_jpeg (argv[0], argv[optind], gfx_dev, info);
      }
    else
      show_usage (argv[0]);
    }

  //free (gfx_dev);
  if (usage) ret = 0;
  return ret;
  }
  

