#ifndef TYPES_H
#define TYPES_H

// cpu
typedef unsigned short int b16;
typedef unsigned int       b32;
typedef          char      int8;
typedef unsigned char      nat8;
typedef          short     int16;
typedef unsigned short     nat16;
typedef          int       int32;
typedef unsigned int       nat32;
typedef          int       word;
typedef          int       bool;

typedef unsigned char byte;
#define true 1
#define false 0
#define NULL 0

typedef int STATUS;
#define SUCCESS 0
#define ENODEV 19  /* No such device */
#define EACCERT 20 /* Assertion failure */

#endif