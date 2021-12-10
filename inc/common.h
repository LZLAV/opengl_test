#ifndef COMMON_H
#define COMMON_H

/**
 * @file common.h
 * @author lzl
 * @brief 
 * @version 0.1
 * @date 2021-12-06
 * 
 * @copyright Copyright (c) 2021
 * @description 公共头文件
 * 
 */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <memory>

#define LOG_STD     1
#define LOG_FILE    0

#define SAFE_FREE(x) if(x){free(x);}

#if LOG_STD
#define LogOut(str) std::cout<<str<<std::endl;
#elif LOG_FILE

#endif

#endif