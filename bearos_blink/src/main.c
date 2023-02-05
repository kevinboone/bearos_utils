#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <bearos/gpio.h> 
#include <bearos/printf.h> 
#include <bearos/compat.h> 

typedef int BOOL;
#define TRUE 1
#define FALSE 0

#define DEFAULT_PIN 25

/*=========================================================================
  show_usage 
=========================================================================*/
static void show_usage (const char *argv0)
  {
  printf_ ("Usage: %s [options]\n", argv0); 
  printf_ ("   -v:   show version\n");
  printf_ ("   -p N: set PGIO pin number\n");
  }

/*=========================================================================
  show_version
=========================================================================*/
static void show_version (const char *argv0)
  {
  printf_ ("%s for BearOS version 0.1\n", argv0); 
  printf_ ("Copyright (c)2022 Kevin Boone, GPL3\n");
  }

/*=========================================================================
  send_gpio_command 
=========================================================================*/
static void send_gpio_command (int fd, const char *cmd, int pin)
  {
  char buff[6]; // Command is 4 bytes, but _sprintf will write a final \0
  sprintf_ (buff, "%s%02d", cmd, pin);
  write (fd, buff, 4); 
  }

/*=========================================================================
  usage 
=========================================================================*/
int main (int argc, char **argv) 
  {
  int opt;
  int ret = 0;
  int pin = DEFAULT_PIN;
  optind = 0;
  BOOL usage = FALSE;
  BOOL version = FALSE;

  while (((opt = getopt (argc, argv, "hvp:")) != -1) && (ret == 0))
    {
    switch (opt)
      {
      case 'v':
        version = TRUE;
        break;
      case 'h':
        usage = TRUE;
        break;
      case 'p':
        pin= atoi (optarg); 
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

  if (pin < 0 || pin > 30)
    {
    printf_ ("%s: ping number out of range: %d\n", argv[0], pin);
    ret = ERANGE;
    }

  if (ret == 0)
    {
    int fd = open (BEAROS_GPIO_DEVICE, O_WRONLY);
    if (fd)
      {
      send_gpio_command (fd, GPIOCMD_INIT, pin);
      send_gpio_command (fd, GPIOCMD_SET_OUTPUT, pin);
      BOOL stop = FALSE;
      printf("Press any key to stop...\n");
      while (!stop)
        {
        send_gpio_command (fd, GPIOCMD_SET_HIGH, pin);
        usleep (300000);
        send_gpio_command (fd, GPIOCMD_SET_LOW, pin);
        usleep (300000);
        stop = kbhit(); 
        }
      close (fd);
      }
    else
      {
      printf_ ("%s: %s\n", argv[0], strerror (errno));
      ret = errno;
      }
    }
 
  if (usage) ret = 0;
  return ret;
  }


