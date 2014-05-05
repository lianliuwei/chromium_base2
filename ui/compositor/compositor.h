// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_COMPOSITOR_COMPOSITOR_H_
#define UI_COMPOSITOR_COMPOSITOR_H_

#include <string>

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "ui/compositor/compositor_export.h"
#include "ui/gfx/rect.h"

namespace ui {



// Texture provide an abstraction over the external texture that can be passed
// to a layer.
class COMPOSITOR_EXPORT Texture : public base::RefCounted<Texture> {
 public:
  Texture() {
    NOTREACHED();
  }

 protected:
  virtual ~Texture() {}


 private:
  friend class base::RefCounted<Texture>;

  DISALLOW_COPY_AND_ASSIGN(Texture);
};



// Compositor object to take care of GPU painting.
// A Browser compositor object is responsible for generating the final
// displayable form of pixels comprising a single widget's contents. It draws an
// appropriately transformed texture for each transformed view in the widget's
// view hierarchy.
class COMPOSITOR_EXPORT Compositor {
 public:
  Compositor() {
    NOTREACHED();
  }
  ~Compositor() {}

  // Draws the scene created by the layer tree and any visual effects. If
  // |force_clear| is true, this will cause the compositor to clear before
  // compositing.
  void Draw(bool force_clear) {}

  void ScheduleRedrawRect(const gfx::Rect& damage_rect) {}


  DISALLOW_COPY_AND_ASSIGN(Compositor);
};

}  // namespace ui

#endif  // UI_COMPOSITOR_COMPOSITOR_H_
