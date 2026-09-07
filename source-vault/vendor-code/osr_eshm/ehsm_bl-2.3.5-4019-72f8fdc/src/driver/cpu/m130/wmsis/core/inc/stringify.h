/**
 * Copyright (c) 2022 - 2023, WingSemi Technology LTD.
 *
 * All rights reserved.
 */
#ifndef __STRINGIFY_H__
#define __STRINGIFY_H__

/// @bug Reserved identifiers
///@{
#define _TOSTR(s)  __TOSTR(s)
#define __TOSTR(s) #s

#define _CONCAT(s1, s2) s1##s2
///@}

#endif //__STRINGIFY_H__
