/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Nov 06 2003
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


#include "crypt_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>

#include <string.h>
#include <assert.h>
#include <stdlib.h>


static int gwen_crypt_is_initialized=0;
static GWEN_ERRORTYPEREGISTRATIONFORM *gwen_crypt_errorform=0;


static GWEN_CRYPT_PROVIDER *gwen_crypt_providers=0;



const char *GWEN_Crypt_ErrorString(int c){
  const char *s;

  switch(c) {
  case GWEN_CRYPT_ERROR_ALREADY_REGISTERED:
    s="Crypto provider already registered";
    break;
  case GWEN_CRYPT_ERROR_NOT_REGISTERED:
    s="Crypto provider not registered";
    break;
  default:
    s=0;
  } /* switch */

  return s;
}



GWEN_ERRORCODE GWEN_Crypt_ModuleInit(){
  if (!gwen_crypt_is_initialized) {
    GWEN_ERRORCODE err;

    gwen_crypt_errorform=GWEN_ErrorType_new();
    GWEN_ErrorType_SetName(gwen_crypt_errorform,
                           GWEN_CRYPT_ERROR_TYPE);
    GWEN_ErrorType_SetMsgPtr(gwen_crypt_errorform,
                             GWEN_Crypt_ErrorString);
    err=GWEN_Error_RegisterType(gwen_crypt_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    err=GWEN_CryptImpl_Init();
    if (!GWEN_Error_IsOk(err)) {
      GWEN_Error_UnregisterType(gwen_crypt_errorform);
      return err;
    }
    gwen_crypt_is_initialized=1;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_Crypt_ModuleFini(){
  if (gwen_crypt_is_initialized) {
    GWEN_ERRORCODE err;

    err=GWEN_CryptImpl_Fini();
    if (!GWEN_Error_IsOk(err))
      return err;

    err=GWEN_Error_UnregisterType(gwen_crypt_errorform);
    GWEN_ErrorType_free(gwen_crypt_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    gwen_crypt_is_initialized=0;
  }
  return 0;
}





GWEN_CRYPTKEY *GWEN_CryptKey_new(){
  GWEN_CRYPTKEY *ck;

  GWEN_NEW_OBJECT(GWEN_CRYPTKEY, ck);
  return ck;
}


void GWEN_CryptKey_free(GWEN_CRYPTKEY *key){
  if (key) {
    if (key->freeKeyDataFn)
      key->freeKeyDataFn(key);
    free(key->keyType);
    free(key->owner);
    free(key);
  }
}



GWEN_ERRORCODE GWEN_CryptKey_Encrypt(GWEN_CRYPTKEY *key,
                                     GWEN_BUFFER *src,
                                     GWEN_BUFFER *dst){
  assert(key);
  assert(src);
  assert(dst);
  assert(key->encryptFn);
  return key->encryptFn(key, src, dst);
}



GWEN_ERRORCODE GWEN_CryptKey_Decrypt(GWEN_CRYPTKEY *key,
                                     GWEN_BUFFER *src,
                                     GWEN_BUFFER *dst){
  assert(key);
  assert(src);
  assert(dst);
  assert(key->decryptFn);
  return key->decryptFn(key, src, dst);
}



GWEN_ERRORCODE GWEN_CryptKey_Sign(GWEN_CRYPTKEY *key,
                                  GWEN_BUFFER *src,
                                  GWEN_BUFFER *dst){
  assert(key);
  assert(src);
  assert(dst);
  assert(key->signFn);
  return key->signFn(key, src, dst);
}



GWEN_ERRORCODE GWEN_CryptKey_Verify(GWEN_CRYPTKEY *key,
                                    GWEN_BUFFER *src,
                                    GWEN_BUFFER *dst){
  assert(key);
  assert(src);
  assert(dst);
  assert(key->verifyFn);
  return key->verifyFn(key, src, dst);
}



unsigned int GWEN_CryptKey_GetChunkSize(GWEN_CRYPTKEY *key){
  assert(key);
  assert(key->getChunkSizeFn);
  return key->getChunkSizeFn(key);
}



GWEN_ERRORCODE GWEN_CryptKey_FromDB(GWEN_CRYPTKEY *key,
                                    GWEN_DB_NODE *db){
  assert(key);
  assert(key->fromDbFn);
  return key->fromDbFn(key, db);
}



GWEN_ERRORCODE GWEN_CryptKey_ToDB(GWEN_CRYPTKEY *key,
                                  GWEN_DB_NODE *db,
                                  int pub){
  assert(key);
  assert(key->toDbFn);
  return key->toDbFn(key, db, pub);
}



GWEN_ERRORCODE GWEN_CryptKey_Generate(GWEN_CRYPTKEY *key,
                                      unsigned keylength){
  assert(key);
  assert(key->generateKeyFn);
  return key->generateKeyFn(key, keylength);
}



GWEN_ERRORCODE GWEN_CryptKey_Open(GWEN_CRYPTKEY *key){
  assert(key);
  assert(key->openFn);
  return key->openFn(key);
}



GWEN_ERRORCODE GWEN_CryptKey_Close(GWEN_CRYPTKEY *key){
  assert(key);
  assert(key->closeFn);
  return key->closeFn(key);
}



const char *GWEN_CryptKey_GetKeyType(GWEN_CRYPTKEY *key){
  assert(key);
  return key->keyType;
}



void GWEN_CryptKey_SetKeyType(GWEN_CRYPTKEY *key,
                              const char *s){
  assert(key);
  assert(s);
  free(key->keyType);
  key->keyType=strdup(s);
}



const char *GWEN_CryptKey_GetOwner(GWEN_CRYPTKEY *key){
  assert(key);
  return key->owner;
}



void GWEN_CryptKey_SetOwner(GWEN_CRYPTKEY *key,
                            const char *s){
  assert(key);
  assert(s);
  free(key->owner);
  key->owner=strdup(s);
}



unsigned int GWEN_CryptKey_GetNumber(GWEN_CRYPTKEY *key){
  assert(key);
  return key->number;
}



void GWEN_CryptKey_SetNumber(GWEN_CRYPTKEY *key,
                             unsigned int i){
  assert(key);
  key->number=i;
}



unsigned int GWEN_CryptKey_GetVersion(GWEN_CRYPTKEY *key){
  assert(key);
  return key->version;
}



void GWEN_CryptKey_SetVersion(GWEN_CRYPTKEY *key,
                              unsigned int i){
  assert(key);
  key->version=i;
}



void *GWEN_CryptKey_GetKeyData(GWEN_CRYPTKEY *key){
  assert(key);
  return key->keyData;
}



void GWEN_CryptKey_SetKeyData(GWEN_CRYPTKEY *key,
                              void *kd){
  assert(key);
  assert(kd);

  if (key->keyData && key->freeKeyDataFn)
    key->freeKeyDataFn(key);
  key->keyData=kd;
}



int GWEN_CryptKey_GetOpenCount(GWEN_CRYPTKEY *key){
  assert(key);
  return key->openCount;
}



void GWEN_CryptKey_IncrementOpenCount(GWEN_CRYPTKEY *key){
  assert(key);
  key->openCount++;
}



void GWEN_CryptKey_DecrementOpenCount(GWEN_CRYPTKEY *key){
  assert(key);
  if (key->openCount>0)
    key->openCount--;
  else {
    DBG_WARN(0, "OpenCount already 0");
  }
}


void GWEN_CryptKey_SetEncryptFn(GWEN_CRYPTKEY *key,
                                GWEN_CRYPTKEY_ENCRYPT_FN encryptFn){
  assert(key);
  key->encryptFn=encryptFn;
}



void GWEN_CryptKey_SetDecryptFn(GWEN_CRYPTKEY *key,
                                GWEN_CRYPTKEY_DECRYPT_FN decryptFn){
  assert(key);
  key->decryptFn=decryptFn;
}



void GWEN_CryptKey_SetSignFn(GWEN_CRYPTKEY *key,
                             GWEN_CRYPTKEY_SIGN_FN signFn){
  assert(key);
  key->signFn=signFn;
}



void GWEN_CryptKey_SetVerifyFn(GWEN_CRYPTKEY *key,
                               GWEN_CRYPTKEY_VERIFY_FN verifyFn){
  assert(key);
  key->verifyFn=verifyFn;
}



void GWEN_CryptKey_SetGetChunkSize(GWEN_CRYPTKEY *key,
                                   GWEN_CRYPTKEY_GETCHUNKSIZE_FN getChunkSizeFn){
  assert(key);
  key->getChunkSizeFn=getChunkSizeFn;
}



void GWEN_CryptKey_SetFromDbFn(GWEN_CRYPTKEY *key,
                               GWEN_CRYPTKEY_FROMDB_FN fromDbFn){
  assert(key);
  key->fromDbFn=fromDbFn;
}



void GWEN_CryptKey_SetToDbFn(GWEN_CRYPTKEY *key,
                             GWEN_CRYPTKEY_TODB_FN toDbFn){
  assert(key);
  key->toDbFn=toDbFn;
}



void GWEN_CryptKey_SetGenerateKeyFn(GWEN_CRYPTKEY *key,
                                    GWEN_CRYPTKEY_GENERATEKEY_FN generateKeyFn){
  assert(key);
  key->generateKeyFn=generateKeyFn;
}



void GWEN_CryptKey_SetFreeKeyDataFn(GWEN_CRYPTKEY *key,
                                    GWEN_CRYPTKEY_FREEKEYDATA_FN freeKeyDataFn){
  assert(key);
  key->freeKeyDataFn=freeKeyDataFn;
}



void GWEN_CryptKey_SetOpenKeyFn(GWEN_CRYPTKEY *key,
                                GWEN_CRYPTKEY_OPEN_FN openFn){
  assert(key);
  key->openFn=openFn;
}



void GWEN_CryptKey_SetCloseFn(GWEN_CRYPTKEY *key,
                              GWEN_CRYPTKEY_CLOSE_FN closeKeyFn){
  assert(key);
  key->closeFn=closeKeyFn;
}




GWEN_CRYPT_PROVIDER *GWEN_CryptProvider_new(){
  GWEN_CRYPT_PROVIDER *pr;

  GWEN_NEW_OBJECT(GWEN_CRYPT_PROVIDER, pr);
  return pr;
}



void GWEN_CryptProvider_SetNewKeyFn(GWEN_CRYPT_PROVIDER *pr,
                                    GWEN_CRYPTPROVIDER_NEWKEY_FN newKeyFn){
  assert(pr);
  pr->newKeyFn=newKeyFn;
}



GWEN_CRYPT_PROVIDER *GWEN_Crypt_FindProvider(const char *name){
  GWEN_CRYPT_PROVIDER *pr;

  pr=gwen_crypt_providers;
  while(pr) {
    if (strcasecmp(pr->name, name)==0)
      return pr;
    pr=pr->next;
  } /* while */

  return 0;
}



GWEN_ERRORCODE GWEN_Crypt_RegisterProvider(GWEN_CRYPT_PROVIDER *pr){
  assert(pr);

  if (GWEN_Crypt_FindProvider(pr->name)){
    DBG_INFO(0, "Service \"%s\" already registered", pr->name);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_ALREADY_REGISTERED);
  }

  GWEN_LIST_ADD(GWEN_CRYPT_PROVIDER, pr, &gwen_crypt_providers);
  return 0;
}



GWEN_ERRORCODE GWEN_Crypt_UnregisterProvider(GWEN_CRYPT_PROVIDER *pr){
  assert(pr);

  if (!GWEN_Crypt_FindProvider(pr->name)){
    DBG_INFO(0, "Service \"%s\" not registered", pr->name);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_NOT_REGISTERED);
  }

  GWEN_LIST_DEL(GWEN_CRYPT_PROVIDER, pr, &gwen_crypt_providers);
  return 0;
}













