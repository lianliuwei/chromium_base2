// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_BASE_NET_EXPORT_H_
#define NET_BASE_NET_EXPORT_H_

// Defines NET_EXPORT so that functionality implemented by the net module can
// be exported to consumers, and NET_EXPORT_PRIVATE that allows unit tests to
// access features not intended to be used directly by real consumers.


#define NET_EXPORT
#define NET_EXPORT_PRIVATE


#endif  // NET_BASE_NET_EXPORT_H_
