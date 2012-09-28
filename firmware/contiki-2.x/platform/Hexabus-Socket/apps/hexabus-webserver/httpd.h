/*
 * Copyright (c) 2001-2005, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: httpd.h,v 1.4 2009/07/24 15:41:52 dak664 Exp $
 *
 */

#ifndef __HTTPD_H__
#define __HTTPD_H__


#include "contiki-net.h"
#include "httpd-fs.h" 
#include "lib/petsciiconv.h"
#include "eeprom_variables.h"

struct httpd_state {
  unsigned char timer;
  struct psock sin, sout;
  struct pt outputpt, scriptpt;
  char inputbuf[50];
  char filename[20];
  char state;
  uint16_t error_number; // HTTP error code (if not 404 or 200)
  struct httpd_fs_file file;  
  int len;
  char *scriptptr;
  int scriptlen;
  union {
    unsigned short count;
    void *ptr;
  } u;
};
/* httpd string storage is in RAM by default. Other storage can be defined here */
#define HTTPD_STRING_TYPE PROGMEM_TYPE
#define PROGMEM_TYPE 1
#define EEPROM_TYPE 2

#if HTTPD_STRING_TYPE==PROGMEM_TYPE
#define HTTPD_STRING_ATTR PROGMEM
/* These will fail if the server strings are above 64K in program flash */
#define httpd_memcpy       memcpy_P
#define httpd_strcpy       strcpy_P
#define httpd_strcmp       strcmp_P
#define httpd_strncmp      strncmp_P
#define httpd_strlen       strlen_P
#define httpd_snprintf     snprintf_P
#elif HTTPD_STRING_TYPE==EEPROM_TYPE
#define HTTPD_STRING_ATTR EEPROM
/* These are not implemented as yet */
#define httpd_memcpy       memcpy_E
#define httpd_strcpy       strcpy_E
#define httpd_strcmp       strcmp_E
#define httpd_strncmp      strncmp_E
#define httpd_strlen       strlen_E
#define httpd_snprintf     snprintf_E
#else
#define httpd_memcpy       memcpy
#define httpd_strcpy       strcpy
#define httpd_strcmp       strcmp
#define httpd_strncmp      strncmp
#define httpd_strlen       strlen
#define httpd_snprintf     snprintf
#endif

void httpd_init(void);
void httpd_appcall(void *state);

#define SM_COND_LENGTH (EE_STATEMACHINE_N_CONDITIONS_SIZE+EE_STATEMACHINE_CONDITIONS_SIZE)
#define SM_TRANS_LENGTH (EE_STATEMACHINE_N_TRANSITIONS_SIZE+EE_STATEMACHINE_TRANSITIONS_SIZE)
#define SM_DTTRANS_LENGHT (EE_STATEMACHINE_N_DT_TRANSITIONS_SIZE+EE_STATEMACHINE_DATETIME_TRANSITIONS_SIZE)

#define COND 1
#define TRANS 2
#define DTTRANS 3

static uint8_t sm_success;

#define SM_UPLOAD_SUCCESS 0
#define SM_UPLOAD_IMGTOOLARGE 1
#define SM_UPLOAD_PARSINGERROR 2

#endif /* __HTTPD_H__ */
