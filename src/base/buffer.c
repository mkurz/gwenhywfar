/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri Sep 12 2003
    copyright   : (C) 2003 by Martin Preuss
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

#include "buffer_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#define DISABLE_DEBUGLOG


#ifdef GWEN_MEMTRACE
static GWEN_TYPE_UINT32 GWEN_Buffer_Buffers=0;

#endif

GWEN_BUFFER *GWEN_Buffer_new(char *buffer,
                             GWEN_TYPE_UINT32 size,
                             GWEN_TYPE_UINT32 used,
                             int take){
  GWEN_BUFFER *bf;

  GWEN_NEW_OBJECT(GWEN_BUFFER, bf);
#ifdef GWEN_MEMTRACE
  GWEN_Buffer_Buffers++;
  DBG_DEBUG(0, "New buffer (now %d)", GWEN_Buffer_Buffers);
#endif
  if (!buffer) {
    /* allocate buffer */
    if (size) {
      bf->realPtr=(char*)malloc(size);
      assert(bf->realPtr);
      bf->ptr=bf->realPtr;
      bf->realBufferSize=size;
      bf->bufferSize=size;
      bf->flags=GWEN_BUFFER_FLAGS_OWNED;
      bf->bytesUsed=used;
      bf->ptr[0]=0;
    }
  }
  else {
    /* use existing buffer */
    bf->realPtr=buffer;
    bf->ptr=buffer;
    bf->realBufferSize=size;
    bf->bufferSize=size;
    bf->bytesUsed=used;
    if (take)
      bf->flags=GWEN_BUFFER_FLAGS_OWNED;
  }

  bf->mode=GWEN_BUFFER_MODE_DEFAULT;
  bf->hardLimit=GWEN_BUFFER_DEFAULT_HARDLIMIT;
  bf->step=GWEN_BUFFER_DYNAMIC_STEP;
  return bf;
}



void GWEN_Buffer_free(GWEN_BUFFER *bf){
  if (bf) {
#ifdef GWEN_MEMTRACE
    assert(GWEN_Buffer_Buffers);
    GWEN_Buffer_Buffers--;
    DBG_DEBUG(0, "Free buffer (now %d)", GWEN_Buffer_Buffers);
#endif
    if (bf->flags & GWEN_BUFFER_FLAGS_OWNED)
      free(bf->realPtr);
    if (bf->bio) {
      if (bf->flags & GWEN_BUFFER_FLAGS_OWN_BIO) {
        GWEN_BufferedIO_free(bf->bio);
      }
    }
    GWEN_FREE_OBJECT(bf);
  }
}



GWEN_BUFFER *GWEN_Buffer_dup(GWEN_BUFFER *bf) {
  GWEN_BUFFER *newbf;
  GWEN_TYPE_UINT32 i;

  GWEN_NEW_OBJECT(GWEN_BUFFER, newbf);
#ifdef GWEN_MEMTRACE
  GWEN_Buffer_Buffers++;
  DBG_DEBUG(0, "New buffer (now %d)", GWEN_Buffer_Buffers);
#endif
  if (bf->realPtr && bf->realBufferSize) {
    newbf->realPtr=(char*)malloc(bf->realBufferSize);
    newbf->ptr=newbf->realPtr+(bf->ptr-bf->realPtr);
    newbf->realBufferSize=bf->realBufferSize;
    newbf->bufferSize=bf->bufferSize;
    newbf->bytesUsed=bf->bytesUsed;
    if (newbf->bytesUsed)
      memmove(newbf->ptr, bf->ptr, bf->bytesUsed);
    newbf->pos=bf->pos;
  }
  newbf->flags=bf->flags | GWEN_BUFFER_FLAGS_OWNED;
  newbf->mode=bf->mode&GWEN_BUFFER_MODE_COPYMASK;
  newbf->hardLimit=bf->hardLimit;
  newbf->step=bf->step;
  for (i=0; i<GWEN_BUFFER_MAX_BOOKMARKS; i++)
    newbf->bookmarks[i]=bf->bookmarks[i];

  return newbf;
}



int GWEN_Buffer_ReserveBytes(GWEN_BUFFER *bf, GWEN_TYPE_UINT32 res){
  assert(bf);
  if (!res)
    return 0;

  if (bf->bytesUsed) {
    /* we need to move data */
    if (GWEN_Buffer_AllocRoom(bf, res))
      return -1;

    memmove(bf->ptr+res, bf->ptr, bf->bytesUsed);
    bf->ptr+=res;
    bf->bufferSize-=res;
    return 0;
  }
  else {
    /* no data in buffer, so simply move ptrs */
    if (GWEN_Buffer_AllocRoom(bf, res))
      return -1;

    bf->ptr+=res;
    bf->bufferSize-=res;
    return 0;
  }
}



GWEN_TYPE_UINT32 GWEN_Buffer_GetMode(GWEN_BUFFER *bf){
  assert(bf);
  return bf->mode;
}



void GWEN_Buffer_SetMode(GWEN_BUFFER *bf, GWEN_TYPE_UINT32 mode){
  assert(bf);
  bf->mode=mode;
}


void GWEN_Buffer_AddMode(GWEN_BUFFER *bf, GWEN_TYPE_UINT32 mode){
  assert(bf);
  bf->mode|=mode;
}


void GWEN_Buffer_SubMode(GWEN_BUFFER *bf, GWEN_TYPE_UINT32 mode){
  assert(bf);
  bf->mode&=~mode;
}



GWEN_TYPE_UINT32 GWEN_Buffer_GetHardLimit(GWEN_BUFFER *bf){
  assert(bf);
  return bf->hardLimit;
}



void GWEN_Buffer_SetHardLimit(GWEN_BUFFER *bf, GWEN_TYPE_UINT32 l){
  assert(bf);
  assert(l);
  bf->hardLimit=l;
}



char *GWEN_Buffer_GetStart(GWEN_BUFFER *bf){
  assert(bf);
  return bf->ptr;
}



GWEN_TYPE_UINT32 GWEN_Buffer_GetSize(GWEN_BUFFER *bf){
  assert(bf);
  if (bf->mode & GWEN_BUFFER_MODE_DYNAMIC)
    return bf->hardLimit;
  return bf->bufferSize;
}



GWEN_TYPE_UINT32 GWEN_Buffer_GetPos(GWEN_BUFFER *bf){
  assert(bf);
  return bf->pos;
}



int GWEN_Buffer_SetPos(GWEN_BUFFER *bf, GWEN_TYPE_UINT32 i){
  assert(bf);

  if (i>=bf->bufferSize) {
    if (bf->mode & GWEN_BUFFER_MODE_USE_BIO) {
      bf->pos=i;
    }
    else {
      DBG_ERROR(0, "Position %d outside buffer boundaries (%d bytes)",
                i, bf->bufferSize);
      return -1;
    }
  }
  bf->pos=i;
  return 0;
}



GWEN_TYPE_UINT32 GWEN_Buffer_GetUsedBytes(GWEN_BUFFER *bf){
  assert(bf);
  return bf->bytesUsed;
}



int GWEN_Buffer_SetUsedBytes(GWEN_BUFFER *bf, GWEN_TYPE_UINT32 i){
  assert(bf);

  DBG_WARN(0,
           "GWEN_Buffer_SetUsedBytes: Deprecated, "
           "please use GWEN_Buffer_Crop instead.");
  if (i>bf->bufferSize) {
    DBG_ERROR(0, "Bytes used>buffer size (%d>%d bytes)",
              i, bf->bufferSize);
    return 1;
  }
  bf->bytesUsed=i;
  return 0;
}



int GWEN_Buffer_AllocRoom(GWEN_BUFFER *bf, GWEN_TYPE_UINT32 size) {
  assert(bf);
  /*DBG_VERBOUS(0, "Allocating %d bytes", size);*/
  /*if (bf->pos+size>bf->bufferSize) {*/
  if (bf->bytesUsed+size>bf->bufferSize) {
    /* need to realloc */
    GWEN_TYPE_UINT32 nsize;
    GWEN_TYPE_UINT32 noffs;
    GWEN_TYPE_UINT32 reserved;
    void *p;

    /* check for dynamic mode */
    if (!(bf->mode & GWEN_BUFFER_MODE_DYNAMIC)) {
      DBG_ERROR(0, "Not in dynamic mode");
      if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
        abort();
      }
      return 1;
    }

    /* calculate reserved bytes (to set ptr later) */
    reserved=bf->ptr-bf->realPtr;
    /* this is the raw number of bytes we need */
    /*nsize=bf->pos+size-bf->bufferSize;*/
    nsize=bf->bytesUsed+size-bf->bufferSize;
    /* round it up */
    nsize=(nsize+(bf->step-1));
    nsize&=~(bf->step-1);
    /* store number of additional bytes to allocate */
    noffs=nsize;
    /* add current size to it */
    nsize+=bf->realBufferSize;
    if (nsize>bf->hardLimit) {
      DBG_ERROR(0, "Size is beyond hard limit (%d>%d)", nsize, bf->hardLimit);
      if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
        abort();
      }
      return 1;
    }
    DBG_DEBUG(0, "Reallocating from %d to %d bytes", bf->bufferSize, nsize);
    p=realloc(bf->realPtr, nsize);
    if (!p) {
      DBG_ERROR(0, "Realloc failed.");
      if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL) {
        abort();
      }
      return 1;
    }
    /* store new size and pointer */
    bf->realPtr=p;
    bf->ptr=bf->realPtr+reserved;
    bf->realBufferSize=nsize;
    bf->bufferSize+=noffs;
  }

  return 0;
}



int GWEN_Buffer_AppendBytes(GWEN_BUFFER *bf,
                            const char *buffer,
                            GWEN_TYPE_UINT32 size){
  assert(bf);
  if (GWEN_Buffer_AllocRoom(bf, size+1)) {
    DBG_DEBUG(0, "called from here");
    return 1;
  }
  /* if (bf->pos+size>bf->bufferSize) { */
  if (bf->bytesUsed+size>bf->bufferSize) {
    DBG_ERROR(0, "Buffer full (%d [%d] of %d bytes)",
              /*bf->pos, size,*/
              bf->bytesUsed, size+1,
              bf->bufferSize);
    return 1;
  }
  /*memmove(bf->ptr+bf->pos, buffer, size);*/
  memmove(bf->ptr+bf->bytesUsed, buffer, size);
  /*bf->pos+=size;*/
  if (bf->pos==bf->bytesUsed)
    bf->pos+=size;
  bf->bytesUsed+=size;
  /* append a NULL to allow using the buffer as ASCIIZ string */
  bf->ptr[bf->bytesUsed]=0;
  return 0;
}



int GWEN_Buffer_AppendByte(GWEN_BUFFER *bf, char c){
  assert(bf);

  if (GWEN_Buffer_AllocRoom(bf, 1+1)) { /* +1 for the trailing 0 */
    DBG_DEBUG(0, "called from here");
    return 1;
  }
  /*if (bf->pos+1>bf->bufferSize) {*/
  if (bf->bytesUsed+1>bf->bufferSize) {
    /*DBG_ERROR(0, "Buffer full (%d of %d bytes)", bf->pos, bf->bufferSize);*/
    DBG_ERROR(0, "Buffer full (%d of %d bytes)",
              bf->bytesUsed, bf->bufferSize);
    return 1;
  }
  bf->ptr[bf->bytesUsed]=c;
  if (bf->pos==bf->bytesUsed)
    bf->pos++;
  bf->bytesUsed++;
  /* append a NULL to allow using the buffer as ASCIIZ string */
  bf->ptr[bf->bytesUsed]=0;
  return 0;
}



int GWEN_Buffer_PeekByte(GWEN_BUFFER *bf){
  assert(bf);

  if (bf->pos>=bf->bytesUsed) {
    if (bf->mode & GWEN_BUFFER_MODE_USE_BIO) {
      if (bf->bio) {
        unsigned int toread;
        GWEN_ERRORCODE gerr;

        if (GWEN_BufferedIO_CheckEOF(bf->bio)) {
          DBG_INFO(0, "End of data stream reached");
          return -1;
        }
        toread=bf->pos-bf->bytesUsed+1;
        if (GWEN_Buffer_AllocRoom(bf, toread+1)) {
          DBG_INFO(0, "Buffer too small");
          return -1;
        }
        gerr=GWEN_BufferedIO_ReadRawForced(bf->bio,
                                           bf->ptr+bf->bytesUsed,
                                           &toread);
        if (!GWEN_Error_IsOk(gerr)) {
          DBG_INFO_ERR(0, gerr);
          return -1;
        }
        bf->bytesUsed+=toread;
      }
      else {
        DBG_DEBUG(0, "End of used area reached and no BIO (%d bytes)",
                  bf->pos);
        return -1;
      }
    }
    else {
      DBG_DEBUG(0, "End of used area reached (%d bytes)", bf->pos);
      return -1;
    }
  }

  return (unsigned char) (bf->ptr[bf->pos]);
}



int GWEN_Buffer_ReadByte(GWEN_BUFFER *bf){
  int c;

  c=GWEN_Buffer_PeekByte(bf);
  if (c!=-1)
    bf->pos++;
  return c;
}



int GWEN_Buffer_IncrementPos(GWEN_BUFFER *bf, GWEN_TYPE_UINT32 i){
  assert(bf);

  if (i+bf->pos>=bf->bufferSize) {
    if (!(bf->mode & GWEN_BUFFER_MODE_USE_BIO)) {
      DBG_DEBUG(0,
                "Position %d outside buffer boundaries (%d bytes)\n"
                "Incrementing anyway",
                i+bf->pos, bf->bufferSize);
    }
  }
  bf->pos+=i;
  return 0;
}



int GWEN_Buffer_AdjustUsedBytes(GWEN_BUFFER *bf){
  assert(bf);
  if (bf->pos<=bf->bufferSize) {
    if (bf->pos>bf->bytesUsed) {
      DBG_DEBUG(0, "Adjusted buffer (uses now %d bytes)",
               bf->pos);
      bf->bytesUsed=bf->pos;
    }
    return 0;
  }
  else {
    DBG_ERROR(0, "Pointer outside buffer size (%d bytes)",
              bf->bufferSize);
    return 1;
  }
}



int GWEN_Buffer_DecrementPos(GWEN_BUFFER *bf, GWEN_TYPE_UINT32 i){
  assert(bf);

  if (bf->pos-i<0) {
    DBG_ERROR(0, "Position %d outside buffer boundaries (%d bytes)",
              bf->pos-i, bf->bufferSize);
    return 1;
  }
  bf->pos-=i;
  return 0;
}



int GWEN_Buffer_AppendBuffer(GWEN_BUFFER *bf,
                             GWEN_BUFFER *sf){
  assert(bf);
  assert(sf);
  if (sf->bytesUsed)
    return GWEN_Buffer_AppendBytes(bf, sf->ptr, sf->bytesUsed);
  return 0;
}



GWEN_TYPE_UINT32 GWEN_Buffer_RoomLeft(GWEN_BUFFER *bf){
  assert(bf);

  DBG_WARN(0, "You should better use \"GWEN_Buffer_AllocRoom\"");
  if (bf->mode & GWEN_BUFFER_MODE_DYNAMIC)
    return bf->hardLimit-bf->bytesUsed;
  return (bf->bufferSize-bf->bytesUsed);
}



GWEN_TYPE_UINT32 GWEN_Buffer_BytesLeft(GWEN_BUFFER *bf){
  DBG_WARN(0, "You should better use \"GWEN_Buffer_GetBytesLeft\"");
  return GWEN_Buffer_GetBytesLeft(bf);
}


GWEN_TYPE_UINT32 GWEN_Buffer_GetBytesLeft(GWEN_BUFFER *bf){
  assert(bf);

  if (bf->pos<bf->bytesUsed)
    return bf->bytesUsed-bf->pos;
  else
    return 0;
}



char *GWEN_Buffer_GetPosPointer(GWEN_BUFFER *bf){
  assert(bf);
  return bf->ptr+bf->pos;
}



GWEN_TYPE_UINT32 GWEN_Buffer_GetBookmark(GWEN_BUFFER *bf, unsigned int idx){
  assert(bf);
  assert(idx<GWEN_BUFFER_MAX_BOOKMARKS);
  return bf->bookmarks[idx];
}



void GWEN_Buffer_SetBookmark(GWEN_BUFFER *bf, unsigned int idx,
                             GWEN_TYPE_UINT32 v){
  assert(bf);
  assert(idx<GWEN_BUFFER_MAX_BOOKMARKS);
  bf->bookmarks[idx]=v;
}



void GWEN_Buffer_Dump(GWEN_BUFFER *bf, FILE *f, unsigned int insert) {
  GWEN_TYPE_UINT32 k;

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Buffer:\n");

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Pos            : %d (%04x)\n", bf->pos, bf->pos);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Buffer Size    : %d\n", bf->bufferSize);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Hard limit     : %d\n", bf->hardLimit);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Bytes Used     : %d\n", bf->bytesUsed);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Bytes Reserved : %d\n", bf->ptr-bf->realPtr);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Flags          : %08x ( ", bf->flags);
  if (bf->flags & GWEN_BUFFER_FLAGS_OWNED)
    fprintf(f, "OWNED ");
  fprintf(f, ")\n");

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Mode           : %08x ( ", bf->mode);
  if (bf->mode & GWEN_BUFFER_MODE_DYNAMIC)
    fprintf(f, "DYNAMIC ");
  if (bf->mode & GWEN_BUFFER_MODE_ABORT_ON_MEMFULL)
    fprintf(f, "ABORT_ON_MEMFULL ");
  fprintf(f, ")\n");

  if (bf->ptr && bf->bytesUsed) {
    for (k=0; k<insert; k++)
      fprintf(f, " ");
    fprintf(f, "Data:\n");
    GWEN_Text_DumpString(bf->ptr, bf->bytesUsed, f, insert+1);
  }
}



void GWEN_Buffer_Reset(GWEN_BUFFER *bf){
  assert(bf);
  bf->pos=0;
  bf->bytesUsed=0;
  bf->ptr[0]=0;
}



void GWEN_Buffer_Rewind(GWEN_BUFFER *bf){
  assert(bf);
  bf->pos=0;
}



int GWEN_Buffer_ReadBytes(GWEN_BUFFER *bf,
                          char *buffer,
                          GWEN_TYPE_UINT32 *size){
  GWEN_TYPE_UINT32 i;
  int c;

  i=0;

  while(i<*size) {
    c=GWEN_Buffer_ReadByte(bf);
    if (c==-1)
      break;
    buffer[i]=c;
    i++;
  } /* while */

  *size=i;
  return 0;
}



GWEN_TYPE_UINT32 GWEN_Buffer_GetStep(GWEN_BUFFER *bf){
  assert(bf);
  return bf->step;
}



void GWEN_Buffer_SetStep(GWEN_BUFFER *bf, GWEN_TYPE_UINT32 step){
  assert(bf);
  bf->step=step;
}



void GWEN_Buffer_AdjustBookmarks(GWEN_BUFFER *bf,
                                 GWEN_TYPE_UINT32 pos,
                                 int offset) {
  GWEN_TYPE_UINT32 i;

  assert(bf);
  for (i=0; i<GWEN_BUFFER_MAX_BOOKMARKS; i++) {
    if (bf->bookmarks[i]>=pos)
      bf->bookmarks[i]+=offset;
  } /* for */
}



int GWEN_Buffer_InsertBytes(GWEN_BUFFER *bf,
                            const char *buffer,
                            GWEN_TYPE_UINT32 size){
  char *p;
  int i;

  assert(bf);

  if (bf->pos==0) {
    if (bf->bytesUsed==0) {
      int rv;
      GWEN_TYPE_UINT32 pos;

      /* no bytes used, simply append data */
      pos=bf->pos;
      rv=GWEN_Buffer_AppendBytes(bf, buffer, size);
      if (rv) {
        DBG_DEBUG(0, "here");
        return rv;
      }
      bf->pos=pos;
      return 0;
    }
    else {
      if ( (bf->ptr - bf->realPtr) >= (int)size ) {
        /* we can simply insert it by occupying the reserved space */
        bf->ptr-=size;
        memmove(bf->ptr, buffer, size);
        bf->bytesUsed+=size;
        bf->bufferSize+=size;
        GWEN_Buffer_AdjustBookmarks(bf, bf->pos, size);
        return 0;
      }
    }
  }

  if (GWEN_Buffer_AllocRoom(bf, size)) {
    DBG_DEBUG(0, "called from here");
    return 1;
  }
  if (bf->pos+size>bf->bufferSize) {
    DBG_ERROR(0, "Buffer full (%d [%d] of %d bytes)",
	      bf->pos, size,
	      bf->bufferSize);
    return -1;
  }
  p=bf->ptr+bf->pos;
  i=bf->bytesUsed-bf->pos;
  if (i>0)
    /* move current data at pos out of the way */
    memmove(p+size, p, i);
  /* copy in new data */
  memmove(bf->ptr+bf->pos, buffer, size);
  bf->bytesUsed+=size;
  GWEN_Buffer_AdjustBookmarks(bf, bf->pos, size);
  return 0;
}



int GWEN_Buffer_InsertByte(GWEN_BUFFER *bf, char c){
  char *p;
  int i;
  assert(bf);

  if (bf->pos==0) {
    if ((bf->ptr-bf->realPtr)>=1) {
      /* we can simply insert it by occupying the reserved space */
      DBG_DEBUG(0, "Using reserved space");
      bf->ptr--;
      bf->ptr[bf->pos]=c;
      bf->bytesUsed++;
      bf->bufferSize++;
      GWEN_Buffer_AdjustBookmarks(bf, bf->pos, 1);
      return 0;
    }
  }

  if (GWEN_Buffer_AllocRoom(bf, 1)) {
    DBG_DEBUG(0, "called from here");
    return -1;
  }
  if (bf->pos+1>bf->bufferSize) {
    DBG_ERROR(0, "Buffer full (%d [%d] of %d bytes)",
              bf->pos, 1,
              bf->bufferSize);
    return -1;
  }
  p=bf->ptr+bf->pos;
  i=bf->bytesUsed-bf->pos;
  if (i>0)
    /* move current data at pos out of the way */
    memmove(p+1, p, i);

  /* copy in new data */
  bf->ptr[bf->pos]=c;
  bf->bytesUsed++;
  GWEN_Buffer_AdjustBookmarks(bf, bf->pos, 1);
  return 0;
}



int GWEN_Buffer_InsertBuffer(GWEN_BUFFER *bf,
                             GWEN_BUFFER *sf){
  assert(bf);
  assert(sf);

  return GWEN_Buffer_InsertBytes(bf, sf->ptr, sf->bytesUsed);
}



int GWEN_Buffer_Crop(GWEN_BUFFER *bf,
                     GWEN_TYPE_UINT32 pos,
                     GWEN_TYPE_UINT32 l) {
  if (pos>=bf->bufferSize) {
    DBG_ERROR(0, "Position outside buffer");
    return -1;
  }
  bf->ptr+=pos;
  bf->bufferSize-=pos;
  bf->pos-=pos;
  if (bf->bytesUsed-pos<l) {
    DBG_INFO(0, "Invalid length");
    return -1;
  }
  bf->bytesUsed=l;
  GWEN_Buffer_AdjustBookmarks(bf, pos, -pos);
  bf->ptr[bf->bytesUsed]=0;

  return 0;
}



int GWEN_Buffer_AppendString(GWEN_BUFFER *bf,
                             const char *buffer){
  assert(bf);
  assert(buffer);
  return GWEN_Buffer_AppendBytes(bf, buffer, strlen(buffer));
}



int GWEN_Buffer_InsertString(GWEN_BUFFER *bf,
                             const char *buffer){
  assert(bf);
  assert(buffer);
  return GWEN_Buffer_InsertBytes(bf, buffer, strlen(buffer));
}



void GWEN_Buffer_SetSourceBIO(GWEN_BUFFER *bf,
                              GWEN_BUFFEREDIO *bio,
                              int take){
  assert(bf);
  if (bf->bio) {
    if (bf->flags & GWEN_BUFFER_FLAGS_OWN_BIO) {
      GWEN_BufferedIO_free(bf->bio);
    }
  }
  if (take)
    bf->flags|=GWEN_BUFFER_FLAGS_OWN_BIO;
  else
    bf->flags&=~GWEN_BUFFER_FLAGS_OWN_BIO;
  bf->bio=bio;
}







