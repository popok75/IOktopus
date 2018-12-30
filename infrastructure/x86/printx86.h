#ifndef PRINTx86_H
#define PRINTx86_H

#include<iostream>

#include<math.h>

template <typename T>
inline void print(T str) {	std::cout << str ;}
template <typename T>
inline void println(T str){std::cout << str << std::endl;}
template <typename T>
inline void println(T str, int i){println(str);}

inline void println() {std::cout << std::endl;}


#define to_string std::to_string


#define DEC 1
static inline void initPrint(){}

#endif
