#ifndef CHARS_H
#define CHARS_H

#include "stdint.h"

#define CHAR_RASTER 80U
#define CHAR_HIGHT 14U
#define CHAR_WIDTH 10U
#define COLUMS_BETWEEN_CHARS 3U
#define BORDER_BETWEEN_CHARS 24U


// Empty colums before/after the characters, depending on the amount of characters
#define EMPTY_COLUMNS_START_END_1CHARS   ((128U - (1*CHAR_WIDTH + COLUMS_BETWEEN_CHARS * 0U))/2)
#define EMPTY_COLUMNS_START_END_2CHARS   ((128U - (2*CHAR_WIDTH + COLUMS_BETWEEN_CHARS * 1U))/2)
#define EMPTY_COLUMNS_START_END_3CHARS   ((128U - (3*CHAR_WIDTH + COLUMS_BETWEEN_CHARS * 2U))/2)
#define EMPTY_COLUMNS_START_END_4CHARS   ((128U - (4*CHAR_WIDTH + COLUMS_BETWEEN_CHARS * 3U))/2)
#define EMPTY_COLUMNS_START_END_5CHARS   ((128U - (5*CHAR_WIDTH + COLUMS_BETWEEN_CHARS * 4U))/2)
#define EMPTY_COLUMNS_START_END_6CHARS   ((128U - (6*CHAR_WIDTH + COLUMS_BETWEEN_CHARS * 5U))/2)
#define EMPTY_COLUMNS_START_END_7CHARS   ((128U - (7*CHAR_WIDTH + COLUMS_BETWEEN_CHARS * 6U))/2)
#define EMPTY_COLUMNS_START_END_8CHARS   ((128U - (8*CHAR_WIDTH + COLUMS_BETWEEN_CHARS * 7U))/2)
#define EMPTY_COLUMNS_START_END_9CHARS   ((128U - (9*CHAR_WIDTH + COLUMS_BETWEEN_CHARS * 8U))/2)

// all empty bytes that come before and after the characters (8 bytes per column)
#define EMPTY_BYTES_START_END_1CHARS  (EMPTY_COLUMNS_START_END_1CHARS * 8U) 
#define EMPTY_BYTES_START_END_2CHARS  (EMPTY_COLUMNS_START_END_2CHARS * 8U)
#define EMPTY_BYTES_START_END_3CHARS  (EMPTY_COLUMNS_START_END_3CHARS * 8U)
#define EMPTY_BYTES_START_END_4CHARS  (EMPTY_COLUMNS_START_END_4CHARS * 8U)
#define EMPTY_BYTES_START_END_5CHARS  (EMPTY_COLUMNS_START_END_5CHARS * 8U)
#define EMPTY_BYTES_START_END_6CHARS  (EMPTY_COLUMNS_START_END_6CHARS * 8U)
#define EMPTY_BYTES_START_END_7CHARS  (EMPTY_COLUMNS_START_END_7CHARS * 8U)
#define EMPTY_BYTES_START_END_8CHARS  (EMPTY_COLUMNS_START_END_8CHARS * 8U)
#define EMPTY_BYTES_START_END_9CHARS  (EMPTY_COLUMNS_START_END_9CHARS * 8U)


extern const uint8_t BorderBetweenChars[BORDER_BETWEEN_CHARS];
extern const uint8_t CharEmpty[CHAR_RASTER];
extern const uint8_t CharSlash[CHAR_RASTER];
extern const uint8_t CharColon[CHAR_RASTER];
extern const uint8_t CharA[CHAR_RASTER];
extern const uint8_t CharB[CHAR_RASTER];
extern const uint8_t CharC[CHAR_RASTER];
extern const uint8_t CharD[CHAR_RASTER];
extern const uint8_t CharE[CHAR_RASTER];
extern const uint8_t CharF[CHAR_RASTER];
extern const uint8_t CharG[CHAR_RASTER];
extern const uint8_t CharH[CHAR_RASTER];
extern const uint8_t CharI[CHAR_RASTER];
extern const uint8_t CharJ[CHAR_RASTER];
extern const uint8_t CharK[CHAR_RASTER];
extern const uint8_t CharL[CHAR_RASTER];
extern const uint8_t CharM[CHAR_RASTER];
extern const uint8_t CharN[CHAR_RASTER];
extern const uint8_t CharO[CHAR_RASTER];
extern const uint8_t CharP[CHAR_RASTER];
extern const uint8_t CharQ[CHAR_RASTER];
extern const uint8_t CharR[CHAR_RASTER];
extern const uint8_t CharS[CHAR_RASTER];
extern const uint8_t CharT[CHAR_RASTER];
extern const uint8_t CharU[CHAR_RASTER];
extern const uint8_t CharV[CHAR_RASTER];
extern const uint8_t CharW[CHAR_RASTER];
extern const uint8_t CharX[CHAR_RASTER];
extern const uint8_t CharY[CHAR_RASTER];
extern const uint8_t CharZ[CHAR_RASTER];
extern const uint8_t Char0[CHAR_RASTER];
extern const uint8_t Char1[CHAR_RASTER];
extern const uint8_t Char2[CHAR_RASTER];
extern const uint8_t Char3[CHAR_RASTER];
extern const uint8_t Char4[CHAR_RASTER];
extern const uint8_t Char5[CHAR_RASTER];
extern const uint8_t Char6[CHAR_RASTER];
extern const uint8_t Char7[CHAR_RASTER];
extern const uint8_t Char8[CHAR_RASTER];
extern const uint8_t Char9[CHAR_RASTER];

#endif