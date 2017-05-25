#pragma once

#include "control_surface_interface.h"

static double charToVol(unsigned char val)
{
  double pos = ((double)val*1000.0) / 127.0;
  pos = SLIDER2DB(pos);
  return DB2VAL(pos);
}

static double int14ToVol(unsigned char msb, unsigned char lsb)
{
  int val = lsb | (msb << 7);
  double pos = ((double)val*1000.0) / 16383.0;
  pos = SLIDER2DB(pos);
  return DB2VAL(pos);
}

static double int14ToPan(unsigned char msb, unsigned char lsb)
{
  int val = lsb | (msb << 7);
  return 1.0 - (val / (16383.0*0.5));
}

static int volToInt14(double vol)
{
  double d = (DB2SLIDER(VAL2DB(vol))*16383.0 / 1000.0);
  if (d<0.0)
    d = 0.0;
  else if (d>16383.0)
    d = 16383.0;
  return (int)(d + 0.5);
}

static int panToInt14(double pan)
{
  double d = ((1.0 - pan)*16383.0*0.5);
  if (d<0.0)
    d = 0.0;
  else if (d>16383.0)
    d = 16383.0;
  return (int)(d + 0.5);
}

static unsigned char volToChar(double vol)
{
  double d = (DB2SLIDER(VAL2DB(vol))*127.0 / 1000.0);
  if (d<0.0)
    d = 0.0;
  else if (d>127.0)
    d = 127.0;
  return (unsigned char)(d + 0.5);
}

static double charToPan(unsigned char val)
{
  double pos = ((double)val*1000.0 + 0.5) / 127.0;
  pos = (pos - 500.0) / 500.0;
  if (fabs(pos) < 0.08)
    pos = 0.0;
  return pos;
}

static unsigned char panToChar(double pan)
{
  pan = (pan + 1.0)*63.5;
  if (pan<0.0)
    pan = 0.0;
  else if (pan>127.0)
    pan = 127.0;
  return (unsigned char)(pan + 0.5);
}
