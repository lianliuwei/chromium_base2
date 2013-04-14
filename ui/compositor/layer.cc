// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/compositor/layer.h"

#include <algorithm>

#include "base/command_line.h"
#include "base/debug/trace_event.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "ui/base/animation/animation.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/display.h"
#include "ui/gfx/interpolated_transform.h"
#include "ui/gfx/point3_f.h"
#include "ui/gfx/point_conversions.h"
#include "ui/gfx/size_conversions.h"

namespace {

const ui::Layer* GetRoot(const ui::Layer* layer) {
  while (layer->parent())
    layer = layer->parent();
  return layer;
}

}  // namespace

namespace ui {

Layer::Layer()
    : type_(LAYER_TEXTURED),
      compositor_(NULL),
      parent_(NULL),
      visible_(true),
      is_drawn_(true),
      force_render_surface_(false),
      fills_bounds_opaquely_(true),
      layer_updated_externally_(false),
      background_blur_radius_(0),
      layer_saturation_(0.0f),
      layer_brightness_(0.0f),
      layer_grayscale_(0.0f),
      layer_inverted_(false),
      layer_mask_(NULL),
      layer_mask_back_link_(NULL),
      zoom_(1),
      zoom_inset_(0),
      delegate_(NULL),
      scale_content_(true),
      device_scale_factor_(1.0f) {
}

Layer::Layer(LayerType type)
    : type_(type),
      compositor_(NULL),
      parent_(NULL),
      visible_(true),
      is_drawn_(true),
      force_render_surface_(false),
      fills_bounds_opaquely_(true),
      layer_updated_externally_(false),
      background_blur_radius_(0),
      layer_saturation_(0.0f),
      layer_brightness_(0.0f),
      layer_grayscale_(0.0f),
      layer_inverted_(false),
      layer_mask_(NULL),
      layer_mask_back_link_(NULL),
      zoom_(1),
      zoom_inset_(0),
      delegate_(NULL),
      scale_content_(true),
      device_scale_factor_(1.0f) {
}

Layer::~Layer() {
  if (parent_)
    parent_->Remove(this);
}

Compositor* Layer::GetCompositor() {
  NOTREACHED();
  return NULL;
}

float Layer::opacity() const {
  NOTREACHED();
  return 0.0;
}

void Layer::SetCompositor(Compositor* compositor) {
  NOTREACHED();
}

void Layer::Add(Layer* child) {
  DCHECK(!child->compositor_);
  if (child->parent_)
    child->parent_->Remove(child);
  child->parent_ = this;
  children_.push_back(child);
}

void Layer::Remove(Layer* child) {
  std::vector<Layer*>::iterator i =
    std::find(children_.begin(), children_.end(), child);
  DCHECK(i != children_.end());
  children_.erase(i);
  child->parent_ = NULL;
}

void Layer::StackAtTop(Layer* child) {
  if (children_.size() <= 1 || child == children_.back())
    return;  // Already in front.
  StackAbove(child, children_.back());
}

void Layer::StackAbove(Layer* child, Layer* other) {
  StackRelativeTo(child, other, true);
}

void Layer::StackAtBottom(Layer* child) {
  if (children_.size() <= 1 || child == children_.front())
    return;  // Already on bottom.
  StackBelow(child, children_.front());
}

void Layer::StackBelow(Layer* child, Layer* other) {
  StackRelativeTo(child, other, false);
}

bool Layer::Contains(const Layer* other) const {
  for (const Layer* parent = other; parent; parent = parent->parent()) {
    if (parent == this)
      return true;
  }
  return false;
}

void Layer::SetAnimator(LayerAnimator* animator) {
  NOTREACHED();
}

LayerAnimator* Layer::GetAnimator() {
  NOTREACHED();
  return NULL;
}

void Layer::SetTransform(const gfx::Transform& transform) {
  transform_ = transform;
}

gfx::Transform Layer::GetTargetTransform() const {
  NOTREACHED();
  return transform();
}

void Layer::SetBounds(const gfx::Rect& bounds) {
    bounds_ = bounds;
}

gfx::Rect Layer::GetTargetBounds() const {
  return bounds_;
}

void Layer::SetMasksToBounds(bool masks_to_bounds) {
  NOTREACHED();
}

bool Layer::GetMasksToBounds() const {
  NOTREACHED();
  return false;
}

void Layer::SetOpacity(float opacity) {
  NOTREACHED();
}

float Layer::GetCombinedOpacity() const {
  NOTREACHED();
  return 0.0;
}

void Layer::SetBackgroundBlur(int blur_radius) {
  NOTREACHED();
}

void Layer::SetLayerSaturation(float saturation) {
  NOTREACHED();
}

void Layer::SetLayerBrightness(float brightness) {
  NOTREACHED();
}

float Layer::GetTargetBrightness() const {
  NOTREACHED();
  return 0.0;
}

void Layer::SetLayerGrayscale(float grayscale) {
  NOTREACHED();
}

float Layer::GetTargetGrayscale() const {
  NOTREACHED();
  return 0.0;
}

void Layer::SetLayerInverted(bool inverted) {
  NOTREACHED();
}

void Layer::SetMaskLayer(Layer* layer_mask) {
  NOTREACHED();
}

void Layer::SetBackgroundZoom(float zoom, int inset) {
  NOTREACHED();
}

float Layer::GetTargetOpacity() const {
  NOTREACHED();
  return opacity();
}

void Layer::SetVisible(bool visible) {
  visible_ = visible;
}

bool Layer::GetTargetVisibility() const {
  NOTREACHED();
  return visible_;
}

bool Layer::IsDrawn() const {
    NOTREACHED();
  return is_drawn_;
}

bool Layer::ShouldDraw() const {
  NOTREACHED();
  return type_ != LAYER_NOT_DRAWN && GetCombinedOpacity() > 0.0f;
}

// static
void Layer::ConvertPointToLayer(const Layer* source,
                                const Layer* target,
                                gfx::Point* point) {
  NOTREACHED();
}

bool Layer::GetTargetTransformRelativeTo(const Layer* ancestor,
                                         gfx::Transform* transform) const {
  NOTREACHED();
  return false;
}

// static
gfx::Transform Layer::ConvertTransformToCCTransform(
    const gfx::Transform& transform,
    const gfx::Rect& bounds,
    float device_scale_factor) {
  NOTREACHED();
  gfx::Transform cc_transform;
  return cc_transform;
}

void Layer::SetFillsBoundsOpaquely(bool fills_bounds_opaquely) {
  NOTREACHED();
}

void Layer::SwitchCCLayerForTest() {
  NOTREACHED();
}

void Layer::SetExternalTexture(Texture* texture) {
  NOTREACHED();
}


void Layer::SetColor(SkColor color) {
  NOTREACHED();
}

bool Layer::SchedulePaint(const gfx::Rect& invalid_rect) {
  return true;
}

void Layer::ScheduleDraw() {
}

void Layer::SendDamagedRects() {
  NOTREACHED();
}

void Layer::SuppressPaint() {
  NOTREACHED();
}

void Layer::OnDeviceScaleFactorChanged(float device_scale_factor) {
  NOTREACHED();
}

void Layer::SetForceRenderSurface(bool force) {
  NOTREACHED();
}

void Layer::StackRelativeTo(Layer* child, Layer* other, bool above) {
  DCHECK_NE(child, other);
  DCHECK_EQ(this, child->parent());
  DCHECK_EQ(this, other->parent());

  const size_t child_i =
      std::find(children_.begin(), children_.end(), child) - children_.begin();
  const size_t other_i =
      std::find(children_.begin(), children_.end(), other) - children_.begin();
  if ((above && child_i == other_i + 1) || (!above && child_i + 1 == other_i))
    return;

  const size_t dest_i =
      above ?
      (child_i < other_i ? other_i : other_i + 1) :
      (child_i < other_i ? other_i - 1 : other_i);
  children_.erase(children_.begin() + child_i);
  children_.insert(children_.begin() + dest_i, child);
}



void Layer::SetBoundsFromAnimation(const gfx::Rect& bounds) {
  NOTREACHED();
}

void Layer::SetTransformFromAnimation(const gfx::Transform& transform) {
  NOTREACHED();
}

void Layer::SetOpacityFromAnimation(float opacity) {
  NOTREACHED();
}

void Layer::SetVisibilityFromAnimation(bool visibility) {
  NOTREACHED();
}

void Layer::SetBrightnessFromAnimation(float brightness) {
  NOTREACHED();
}

void Layer::SetGrayscaleFromAnimation(float grayscale) {
  NOTREACHED();
}

void Layer::SetColorFromAnimation(SkColor color) {
  NOTREACHED();
}

void Layer::ScheduleDrawForAnimation() {
  ScheduleDraw();
}

const gfx::Rect& Layer::GetBoundsForAnimation() const {
  return bounds();
}

gfx::Transform Layer::GetTransformForAnimation() const {
  return transform();
}

float Layer::GetOpacityForAnimation() const {
  return opacity();
}

bool Layer::GetVisibilityForAnimation() const {
  return visible();
}

float Layer::GetBrightnessForAnimation() const {
  return layer_brightness();
}

float Layer::GetGrayscaleForAnimation() const {
  return layer_grayscale();
}

SkColor Layer::GetColorForAnimation() const {
  NOTREACHED();
  return SK_ColorBLACK;
}

float Layer::GetDeviceScaleFactor() const {
  return device_scale_factor_;
}



void Layer::RemoveThreadedAnimation(int animation_id) {
  NOTREACHED();
}

gfx::Transform Layer::transform() const {
  return transform_;
}

}  // namespace ui
