/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Jan 05 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "padd_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/crypt.h>
#include <string.h>
#include <stdlib.h>




/*
 * This code has been taken from OpenHBCI (rsakey.cpp, written by Fabian
 * Kaiser)
 */
unsigned char GWEN_Padd_permutate(unsigned char input) {
  unsigned char leftNibble;
  unsigned char rightNibble;
  static const unsigned char lookUp[2][16] =
    {{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
    {14,3,5,8,9,4,2,15,0,13,11,6,7,10,12,1}};

  rightNibble = input & 15;
  leftNibble = input & 240;
  leftNibble = leftNibble / 16;
  rightNibble = lookUp[1][rightNibble];
  leftNibble = lookUp[1][leftNibble];
  leftNibble = leftNibble * 16;

  return leftNibble + rightNibble;
}



/*
 * The original code (in C++) has been written by Fabian Kaiser for OpenHBCI
 * (file rsakey.cpp). Moved to C by me (Martin Preuss)
 */
int GWEN_Padd_PaddWithISO9796(GWEN_BUFFER *src) {
  unsigned char *p;
  unsigned int l;
  unsigned int i;
  unsigned char buffer[GWEN_PADD_ISO9796_KEYSIZE];
  unsigned char hash[20];
  unsigned char c;

  p=(unsigned char*)GWEN_Buffer_GetStart(src);
  l=GWEN_Buffer_GetUsedBytes(src);
  memmove(hash, p, l);

  /* src+src+src */
  if (GWEN_Buffer_AppendBytes(src, (const char*)hash, l)) {
    DBG_INFO(0, "here");
    return -1;
  }

  if (GWEN_Buffer_AppendBytes(src, (const char*)hash, l)) {
    DBG_INFO(0, "here");
    return -1;
  }

  /* src=src(20,40) */
  if (GWEN_Buffer_Crop(src, 20, 40)) {
    DBG_INFO(0, "here");
    return -1;
  }

  memset(buffer, 0, sizeof(buffer));

  /* append redundancy */
  p=(unsigned char*)GWEN_Buffer_GetStart(src);
  for (i=0; i<=47; i++) {
    int j1, j2, j3;

    j1=1 + sizeof(buffer) - (2*i);
    j2=40-i;
    j3=sizeof(buffer) - (2*i);

    if (j1>=0 && j1<(int)sizeof(buffer) && j2>=0) {
      buffer[j1]=p[j2];
    }
    if (j3>=0 && j3<(int)sizeof(buffer) && j2>=0) {
      buffer[j3]=GWEN_Padd_permutate(p[j2]);
    }
  } /* for */

  /* copy last 16 bytes to the beginning */
  memmove(buffer, buffer+(sizeof(buffer)-16), 16);

  p=buffer;
  /* finish */
  c=p[sizeof(buffer)-1];
  c = (c & 15) * 16;
  c += 6;
  p[sizeof(buffer)-1]=c;
  p[0] = p[0] & 127;
  p[0] = p[0] | 64;
  p[sizeof(buffer) - 40] = p[sizeof(buffer) - 40] ^ 1;

  GWEN_Buffer_Reset(src);
  if (GWEN_Buffer_AppendBytes(src, (const char*)buffer, sizeof(buffer))) {
    DBG_INFO(0, "here");
    return -1;
  }

  return 0;
}




int GWEN_Padd_PaddWithANSIX9_23(GWEN_BUFFER *src) {
  unsigned char paddLength;
  unsigned int i;

  paddLength=8-(GWEN_Buffer_GetUsedBytes(src) % 8);
  for (i=0; i<paddLength; i++)
    GWEN_Buffer_AppendByte(src, paddLength);
  return 0;
}



int GWEN_Padd_UnpaddWithANSIX9_23(GWEN_BUFFER *src) {
  const char *p;
  unsigned int lastpos;
  unsigned char paddLength;

  lastpos=GWEN_Buffer_GetUsedBytes(src);
  if (lastpos<8) {
    DBG_ERROR(0, "Buffer too small");
    return -1;
  }
  lastpos--;

  p=GWEN_Buffer_GetStart(src)+lastpos;
  paddLength=*p;
  if (paddLength<1 || paddLength>8) {
    DBG_ERROR(0, "Invalid padding (%d bytes ?)", paddLength);
    return -1;
  }
  GWEN_Buffer_SetUsedBytes(src, GWEN_Buffer_GetUsedBytes(src)-paddLength);
  GWEN_Buffer_SetPos(src, lastpos-paddLength);
  return 0;
}


