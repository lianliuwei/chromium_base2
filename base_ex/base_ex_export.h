// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_EX_BASE_EX_EXPORT_H_
#define BASE_EX_BASE_EX_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(BASE_EX_IMPLEMENTATION)
#define BASE_EX_EXPORT __declspec(dllexport)
#else
#define BASE_EX_EXPORT __declspec(dllimport)
#endif  // defined(BASE_EX_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(BASE_EX_IMPLEMENTATION)
#define BASE_EX_EXPORT __attribute__((visibility("default")))
#else
#define BASE_EX_EXPORT
#endif  // defined(BASE_EX_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define BASE_EX_EXPORT
#endif

#endif  // BASE_EX_BASE_EX_EXPORT_H_
