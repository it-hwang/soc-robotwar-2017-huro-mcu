/******************************************************************************
 *
 * debug.h
 * 설명    : 사용하기 위해서 이 헤더 파일이 include 되기 전에
 *           "#define DEBUG"가 입력되어 있어야한다.
 * 주의사항: 헤더 파일에서 include하면 안된다.
 *
 *****************************************************************************/

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "log.h"

#endif //__DEBUG_H__


#ifdef DEBUG
    #define printDebug(fmt, ...) \
    do { printLog("%s:%d:%s(): " fmt, __FILE__, \
                  __LINE__, __func__, ##__VA_ARGS__); } while (0)
#else
    #define printDebug(fmt, ...) 
#endif

#undef DEBUG
