#pragma once

// #define DEBUG // toggle debug mode

#ifdef DEBUG

namespace midiate
{

struct Layer;
struct Time;

namespace debug
{

// forward declaration of Arduino Serial.print(...) methods

void print(const char[]);
void print(char);
void print(unsigned char);
void print(int);
void print(unsigned int);
void print(long);
void print(unsigned long);

void println(const char[]);
void println(char);
void println(unsigned char);
void println(int);
void println(unsigned int);
void println(long);
void println(unsigned long);
void println();

// midiate specific

void print(const Time &);
void print(const Layer &);

void prefix(const char file[], int line, const char function[]);

} // debug
} // midiate

#define PRINT(x)        debug::print(x)
#define PRINTLN(x)      debug::println(x)
#define TRACE_START()   debug::prefix(__FILE__, __LINE__, __FUNCTION__)
#define TRACE_END() PRINTLN()

#define TRACE_1(x1)                         TRACE_START(); PRINT(x1);                                                                               TRACE_END()
#define TRACE_2(x1,x2)                      TRACE_START(); PRINT(x1); PRINT(x2);                                                                    TRACE_END()
#define TRACE_3(x1,x2,x3)                   TRACE_START(); PRINT(x1); PRINT(x2); PRINT(x3);                                                         TRACE_END()
#define TRACE_4(x1,x2,x3,x4)                TRACE_START(); PRINT(x1); PRINT(x2); PRINT(x3); PRINT(x4);                                              TRACE_END()
#define TRACE_5(x1,x2,x3,x4,x5)             TRACE_START(); PRINT(x1); PRINT(x2); PRINT(x3); PRINT(x4); PRINT(x5);                                   TRACE_END()
#define TRACE_6(x1,x2,x3,x4,x5,x6)          TRACE_START(); PRINT(x1); PRINT(x2); PRINT(x3); PRINT(x4); PRINT(x5); PRINT(x6);                        TRACE_END()
#define TRACE_7(x1,x2,x3,x4,x5,x6,x7)       TRACE_START(); PRINT(x1); PRINT(x2); PRINT(x3); PRINT(x4); PRINT(x5); PRINT(x6); PRINT(x7);             TRACE_END()
#define TRACE_8(x1,x2,x3,x4,x5,x6,x7,x8)    TRACE_START(); PRINT(x1); PRINT(x2); PRINT(x3); PRINT(x4); PRINT(x5); PRINT(x6); PRINT(x7); PRINT(x8);  TRACE_END()

#else

#define PRINT(x)
#define PRINTLN(x)
#define TRACE_START()
#define TRACE_END()

#define TRACE_1(...)
#define TRACE_2(...)
#define TRACE_3(...)
#define TRACE_4(...)
#define TRACE_5(...)
#define TRACE_6(...)
#define TRACE_7(...)
#define TRACE_8(...)

#endif
