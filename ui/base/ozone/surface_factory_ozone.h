// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_OZONE_SURFACE_LNUX_FACTORY_OZONE_H_
#define UI_BASE_OZONE_SURFACE_LNUX_FACTORY_OZONE_H_

#include  "ui/gfx/native_widget_types.h"

namespace gfx {
class VSyncProvider;
} //  namespace gfx

namespace ui {

class SurfaceFactoryOzone {
 public:
  SurfaceFactoryOzone();
  virtual ~SurfaceFactoryOzone();

  // Returns the instance
  static SurfaceFactoryOzone* GetInstance();

  // Returns a display spec as in |CreateDisplayFromSpec| for the default
  // native surface.
  virtual const char* DefaultDisplaySpec();

  // Sets the implementation delegate.
  static void SetInstance(SurfaceFactoryOzone* impl);

  // TODO(rjkroege): Add a status code if necessary.
  // Configures the display hardware. Must be called from within the GPU
  // process before the sandbox has been activated.
  virtual void InitializeHardware() = 0;

  // Cleans up display hardware state. Call this from within the GPU process.
  // This method must be safe to run inside of the sandbox.
  virtual void ShutdownHardware() = 0;

  // Obtains an AcceleratedWidget backed by a native Linux framebuffer.
  // The  returned AcceleratedWidget is an opaque token that must realized
  // before it can be used to create a GL surface.
  virtual gfx::AcceleratedWidget GetAcceleratedWidget() = 0;

  // Realizes an AcceleratedWidget so that the returned AcceleratedWidget
  // can be used to to create a GLSurface. This method may only be called in
  // a process that has a valid GL context.
  virtual gfx::AcceleratedWidget RealizeAcceleratedWidget(
      gfx::AcceleratedWidget w) = 0;

  // Sets up GL bindings for the native surface.
  virtual bool LoadEGLGLES2Bindings() = 0;

  // Tests if the given AcceleratedWidget instance can be resized. (Native
  // hardware may only support a single fixed size.)
  // Perhaps, this should be "attempt to resize the accelerated widget"?
  virtual bool AcceleratedWidgetCanBeResized(gfx::AcceleratedWidget w) = 0;

  // Returns a gfx::VsyncProvider for the provided AcceleratedWidget. Note
  // that this may be called after we have entered the sandbox so if there are
  // operations (e.g. opening a file descriptor providing vsync events) that
  // must be done outside of the sandbox, they must have been completed
  // in InitializeHardware. Returns NULL on error.
  virtual gfx::VSyncProvider* GetVSyncProvider(gfx::AcceleratedWidget w) = 0;
};

}  // namespace ui


#endif  // UI_BASE_OZONE_SURFACE_LNUX_FACTORY_OZONE_H_
