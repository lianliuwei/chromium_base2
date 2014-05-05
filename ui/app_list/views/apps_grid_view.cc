// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/app_list/views/apps_grid_view.h"

#include <algorithm>

#include "ui/app_list/app_list_item_model.h"
#include "ui/app_list/apps_grid_view_delegate.h"
#include "ui/app_list/pagination_model.h"
#include "ui/app_list/views/app_list_drag_and_drop_host.h"
#include "ui/app_list/views/app_list_item_view.h"
#include "ui/app_list/views/page_switcher.h"
#include "ui/app_list/views/pulsing_block_view.h"
#include "ui/base/animation/animation.h"
#include "ui/base/events/event.h"
#include "ui/views/border.h"
#include "ui/views/view_model_utils.h"
#include "ui/views/widget/widget.h"

#if defined(USE_AURA)
#include "ui/aura/root_window.h"
#endif

namespace {

// Padding space in pixels for fixed layout.
const int kLeftRightPadding = 20;
const int kTopPadding = 1;

// Padding space in pixels between pages.
const int kPagePadding = 40;

// Preferred tile size when showing in fixed layout.
const int kPreferredTileWidth = 88;
const int kPreferredTileHeight = 98;

// Width in pixels of the area on the sides that triggers a page flip.
const int kPageFlipZoneSize = 40;

// Delay in milliseconds to do the page flip.
const int kPageFlipDelayInMs = 1000;

// How many pages on either side of the selected one we prerender.
const int kPrerenderPages = 1;

// The drag and drop proxy should get scaled by this factor.
const float kDragAndDropProxyScale = 1.5f;

// For testing we remember the last created grid view.
app_list::AppsGridView* last_created_grid_view_for_test = NULL;

// RowMoveAnimationDelegate is used when moving an item into a different row.
// Before running the animation, the item's layer is re-created and kept in
// the original position, then the item is moved to just before its target
// position and opacity set to 0. When the animation runs, this delegate moves
// the layer and fades it out while fading in the item at the same time.
class RowMoveAnimationDelegate
    : public views::BoundsAnimator::OwnedAnimationDelegate {
 public:
  RowMoveAnimationDelegate(views::View* view,
                            ui::Layer* layer,
                            const gfx::Rect& layer_target)
      : view_(view),
        layer_(layer),
        layer_start_(layer ? layer->bounds() : gfx::Rect()),
        layer_target_(layer_target) {
  }
  virtual ~RowMoveAnimationDelegate() {}

  // ui::AnimationDelegate overrides:
  virtual void AnimationProgressed(const ui::Animation* animation) OVERRIDE {
    view_->layer()->SetOpacity(animation->GetCurrentValue());
    view_->layer()->ScheduleDraw();

    if (layer_) {
      layer_->SetOpacity(1 - animation->GetCurrentValue());
      layer_->SetBounds(animation->CurrentValueBetween(layer_start_,
                                                       layer_target_));
      layer_->ScheduleDraw();
    }
  }
  virtual void AnimationEnded(const ui::Animation* animation) OVERRIDE {
    view_->layer()->SetOpacity(1.0f);
    view_->layer()->ScheduleDraw();
  }
  virtual void AnimationCanceled(const ui::Animation* animation) OVERRIDE {
    view_->layer()->SetOpacity(1.0f);
    view_->layer()->ScheduleDraw();
  }

 private:
  // The view that needs to be wrapped. Owned by views hierarchy.
  views::View* view_;

  scoped_ptr<ui::Layer> layer_;
  const gfx::Rect layer_start_;
  const gfx::Rect layer_target_;

  DISALLOW_COPY_AND_ASSIGN(RowMoveAnimationDelegate);
};

}  // namespace

namespace app_list {

AppsGridView::AppsGridView(AppsGridViewDelegate* delegate,
                           PaginationModel* pagination_model)
    : model_(NULL),
      delegate_(delegate),
      pagination_model_(pagination_model),
      page_switcher_view_(new PageSwitcher(pagination_model)),
      cols_(0),
      rows_per_page_(0),
      selected_view_(NULL),
      drag_view_(NULL),
      drag_pointer_(NONE),
      drag_and_drop_host_(NULL),
      forward_events_to_drag_and_drop_host_(false),
      page_flip_target_(-1),
      page_flip_delay_in_ms_(kPageFlipDelayInMs),
      bounds_animator_(this) {
  last_created_grid_view_for_test = this;
  pagination_model_->AddObserver(this);
  AddChildView(page_switcher_view_);
}

AppsGridView::~AppsGridView() {
  if (model_) {
    model_->RemoveObserver(this);
    model_->apps()->RemoveObserver(this);
  }
  pagination_model_->RemoveObserver(this);
}

void AppsGridView::SetLayout(int icon_size, int cols, int rows_per_page) {
  icon_size_.SetSize(icon_size, icon_size);
  cols_ = cols;
  rows_per_page_ = rows_per_page;

  set_border(views::Border::CreateEmptyBorder(kTopPadding,
                                              kLeftRightPadding,
                                              0,
                                              kLeftRightPadding));
}

void AppsGridView::SetModel(AppListModel* model) {
  if (model_) {
    model_->RemoveObserver(this);
    model_->apps()->RemoveObserver(this);
  }

  model_ = model;
  if (model_) {
    model_->AddObserver(this);
    model_->apps()->AddObserver(this);
  }
  Update();
}

void AppsGridView::SetSelectedView(views::View* view) {
  if (IsSelectedView(view) || IsDraggedView(view))
    return;

  Index index = GetIndexOfView(view);
  if (IsValidIndex(index))
    SetSelectedItemByIndex(index);
}

void AppsGridView::ClearSelectedView(views::View* view) {
  if (view && IsSelectedView(view)) {
    selected_view_->SchedulePaint();
    selected_view_ = NULL;
  }
}

bool AppsGridView::IsSelectedView(const views::View* view) const {
  return selected_view_ == view;
}

void AppsGridView::EnsureViewVisible(const views::View* view) {
  if (pagination_model_->has_transition())
    return;

  Index index = GetIndexOfView(view);
  if (IsValidIndex(index))
    pagination_model_->SelectPage(index.page, false);
}

void AppsGridView::InitiateDrag(AppListItemView* view,
                                Pointer pointer,
                                const ui::LocatedEvent& event) {
  if (drag_view_ || pulsing_blocks_model_.view_size())
    return;

  drag_view_ = view;

  // When a drag and drop host is given, the item can be dragged out of the app
  // list window. In that case a proxy widget needs to be used.
  // Note: This code has very likely to be changed for Windows (non metro mode)
  // when a |drag_and_drop_host_| gets implemented.
  if (drag_and_drop_host_) {
    // We have to hide the original item since the drag and drop host will do
    // the OS dependent code to "lift off the dragged item".
    // Note that we cannot use SetVisible since it would remove the mouse input.
    drag_view_->SetSize(gfx::Size());
    drag_and_drop_host_->CreateDragIconProxy(event.root_location(),
                                             view->model()->icon(),
                                             drag_view_,
                                             kDragAndDropProxyScale);
  }
  drag_start_ = event.location();
}

void AppsGridView::UpdateDrag(AppListItemView* view,
                              Pointer pointer,
                              const ui::LocatedEvent& event) {
  if (!dragging() && drag_view_ &&
      ExceededDragThreshold(event.location() - drag_start_)) {
    drag_pointer_ = pointer;
    // Move the view to the front so that it appears on top of other views.
    ReorderChildView(drag_view_, -1);
    bounds_animator_.StopAnimatingView(drag_view_);
  }
  if (drag_pointer_ != pointer)
    return;

  ExtractDragLocation(event, &last_drag_point_);
  const Index last_drop_target = drop_target_;
  CalculateDropTarget(last_drag_point_, false);

  // If a drag and drop host is provided, see if the drag operation needs to be
  // forwarded.
  DispatchDragEventToDragAndDropHost(event);

  MaybeStartPageFlipTimer(last_drag_point_);

  gfx::Point page_switcher_point(last_drag_point_);
  views::View::ConvertPointToTarget(this, page_switcher_view_,
                                    &page_switcher_point);
  page_switcher_view_->UpdateUIForDragPoint(page_switcher_point);

  if (last_drop_target != drop_target_)
    AnimateToIdealBounds();

  if (drag_and_drop_host_) {
    drag_and_drop_host_->UpdateDragIconProxy(event.root_location());
    return;
  }

  drag_view_->SetPosition(
      gfx::PointAtOffsetFromOrigin(last_drag_point_ - drag_start_));
}

void AppsGridView::EndDrag(bool cancel) {
  if (forward_events_to_drag_and_drop_host_) {
    forward_events_to_drag_and_drop_host_ = false;
    drag_and_drop_host_->EndDrag(cancel);
  } else if (!cancel && dragging() && drag_view_) {
    CalculateDropTarget(last_drag_point_, true);
    if (IsValidIndex(drop_target_))
      MoveItemInModel(drag_view_, drop_target_);
  }

  // In case we had a drag and drop proxy icon, we delete it and make the real
  // item visible again.
  if (drag_and_drop_host_) {
    drag_and_drop_host_->DestroyDragIconProxy();
    // To avoid an incorrect animation on re-group, |drag_view_| gets positioned
    // at its last known drag location.
    drag_view_->SetPosition(
        gfx::PointAtOffsetFromOrigin(last_drag_point_ - drag_start_));
  }

  drag_pointer_ = NONE;
  drop_target_ = Index();
  if (drag_view_) {
    drag_view_ = NULL;
    AnimateToIdealBounds();
  }

  page_flip_timer_.Stop();
  page_flip_target_ = -1;
}

bool AppsGridView::IsDraggedView(const views::View* view) const {
  return drag_view_ == view;
}

void AppsGridView::SetDragAndDropHostOfCurrentAppList(
    ApplicationDragAndDropHost* drag_and_drop_host) {
  drag_and_drop_host_ = drag_and_drop_host;
}

void AppsGridView::Prerender(int page_index) {
  Layout();
  int start = std::max(0, (page_index - kPrerenderPages) * tiles_per_page());
  int end = std::min(view_model_.view_size(),
                     (page_index + 1 + kPrerenderPages) * tiles_per_page());
  for (int i = start; i < end; i++) {
    AppListItemView* v = static_cast<AppListItemView*>(view_model_.view_at(i));
    v->Prerender();
  }
}

gfx::Size AppsGridView::GetPreferredSize() {
  const gfx::Insets insets(GetInsets());
  const gfx::Size tile_size = gfx::Size(kPreferredTileWidth,
                                        kPreferredTileHeight);
  const int page_switcher_height =
      page_switcher_view_->GetPreferredSize().height();
  return gfx::Size(
      tile_size.width() * cols_ + insets.width(),
      tile_size.height() * rows_per_page_ +
          page_switcher_height + insets.height());
}

void AppsGridView::Layout() {
  if (bounds_animator_.IsAnimating())
    bounds_animator_.Cancel();

  CalculateIdealBounds();
  for (int i = 0; i < view_model_.view_size(); ++i) {
    views::View* view = view_model_.view_at(i);
    if (view != drag_view_)
      view->SetBoundsRect(view_model_.ideal_bounds(i));
  }
  views::ViewModelUtils::SetViewBoundsToIdealBounds(pulsing_blocks_model_);

  const int page_switcher_height =
      page_switcher_view_->GetPreferredSize().height();
  gfx::Rect rect(GetContentsBounds());
  rect.set_y(rect.bottom() - page_switcher_height);
  rect.set_height(page_switcher_height);
  page_switcher_view_->SetBoundsRect(rect);
}

bool AppsGridView::OnKeyPressed(const ui::KeyEvent& event) {
  bool handled = false;
  if (selected_view_)
    handled = selected_view_->OnKeyPressed(event);

  if (!handled) {
    const int forward_dir = base::i18n::IsRTL() ? -1 : 1;
    switch (event.key_code()) {
      case ui::VKEY_LEFT:
        MoveSelected(0, -forward_dir, 0);
        return true;
      case ui::VKEY_RIGHT:
        MoveSelected(0, forward_dir, 0);
        return true;
      case ui::VKEY_UP:
        MoveSelected(0, 0, -1);
        return true;
      case ui::VKEY_DOWN:
        MoveSelected(0, 0, 1);
        return true;
      case ui::VKEY_PRIOR: {
        MoveSelected(-1, 0, 0);
        return true;
      }
      case ui::VKEY_NEXT: {
        MoveSelected(1, 0, 0);
        return true;
      }
      default:
        break;
    }
  }

  return handled;
}

bool AppsGridView::OnKeyReleased(const ui::KeyEvent& event) {
  bool handled = false;
  if (selected_view_)
    handled = selected_view_->OnKeyReleased(event);

  return handled;
}

void AppsGridView::ViewHierarchyChanged(
    const ViewHierarchyChangedDetails& details) {
  if (!details.is_add && details.parent == this) {
    if (selected_view_ == details.child)
      selected_view_ = NULL;

    if (drag_view_ == details.child)
      EndDrag(true);

    bounds_animator_.StopAnimatingView(details.child);
  }
}

// static
AppsGridView* AppsGridView::GetLastGridViewForTest() {
  return last_created_grid_view_for_test;
}

void AppsGridView::Update() {
  DCHECK(!selected_view_ && !drag_view_);

  view_model_.Clear();
  if (model_ && model_->apps()->item_count())
    ListItemsAdded(0, model_->apps()->item_count());
}

void AppsGridView::UpdatePaging() {
  if (!view_model_.view_size() || !tiles_per_page()) {
    pagination_model_->SetTotalPages(0);
    return;
  }

  pagination_model_->SetTotalPages(
      (view_model_.view_size() - 1) / tiles_per_page() + 1);
}

void AppsGridView::UpdatePulsingBlockViews() {
  const int available_slots =
      tiles_per_page() - model_->apps()->item_count() % tiles_per_page();
  const int desired = model_->status() == AppListModel::STATUS_SYNCING ?
      available_slots : 0;

  if (pulsing_blocks_model_.view_size() == desired)
    return;

  while (pulsing_blocks_model_.view_size() > desired) {
    views::View* view = pulsing_blocks_model_.view_at(0);
    pulsing_blocks_model_.Remove(0);
    delete view;
  }

  while (pulsing_blocks_model_.view_size() < desired) {
    views::View* view = new PulsingBlockView(
        gfx::Size(kPreferredTileWidth, kPreferredTileHeight), true);
    pulsing_blocks_model_.Add(view, 0);
    AddChildView(view);
  }
}

views::View* AppsGridView::CreateViewForItemAtIndex(size_t index) {
  DCHECK_LT(index, model_->apps()->item_count());
  AppListItemView* view = new AppListItemView(this,
                                              model_->apps()->GetItemAt(index));
  view->SetIconSize(icon_size_);
#if defined(USE_AURA)
  view->SetPaintToLayer(true);
  view->SetFillsBoundsOpaquely(false);
#endif
  return view;
}

void AppsGridView::SetSelectedItemByIndex(const Index& index) {
  if (GetIndexOfView(selected_view_) == index)
    return;

  views::View* new_selection = GetViewAtIndex(index);
  if (!new_selection)
    return;  // Keep current selection.

  if (selected_view_)
    selected_view_->SchedulePaint();

  EnsureViewVisible(new_selection);
  selected_view_ = new_selection;
  selected_view_->SchedulePaint();
  selected_view_->NotifyAccessibilityEvent(
      ui::AccessibilityTypes::EVENT_FOCUS, true);
}

bool AppsGridView::IsValidIndex(const Index& index) const {
  return index.page >= 0 && index.page < pagination_model_->total_pages() &&
      index.slot >= 0 && index.slot < tiles_per_page() &&
      index.page * tiles_per_page() + index.slot < view_model_.view_size();
}

AppsGridView::Index AppsGridView::GetIndexOfView(
    const views::View* view) const {
  const int model_index = view_model_.GetIndexOfView(view);
  if (model_index == -1)
    return Index();

  return Index(model_index / tiles_per_page(), model_index % tiles_per_page());
}

views::View* AppsGridView::GetViewAtIndex(const Index& index) const {
  if (!IsValidIndex(index))
    return NULL;

  const int model_index = index.page * tiles_per_page() + index.slot;
  return view_model_.view_at(model_index);
}

void AppsGridView::MoveSelected(int page_delta,
                                int slot_x_delta,
                                int slot_y_delta) {
  if (!selected_view_)
    return SetSelectedItemByIndex(Index(pagination_model_->selected_page(), 0));

  const Index& selected = GetIndexOfView(selected_view_);
  int target_slot = selected.slot + slot_x_delta + slot_y_delta * cols_;

  if (selected.slot % cols_ == 0 && slot_x_delta == -1) {
    if (selected.page > 0) {
      page_delta = -1;
      target_slot = selected.slot + cols_ - 1;
    } else {
      target_slot = selected.slot;
    }
  }

  if (selected.slot % cols_ == cols_ - 1 && slot_x_delta == 1) {
    if (selected.page < pagination_model_->total_pages() - 1) {
      page_delta = 1;
      target_slot = selected.slot - cols_ + 1;
    } else {
      target_slot = selected.slot;
    }
  }

  // Clamp the target slot to the last item if we are moving to the last page
  // but our target slot is past the end of the item list.
  if (page_delta &&
      selected.page + page_delta == pagination_model_->total_pages() - 1) {
    int last_item_slot = (view_model_.view_size() - 1) % tiles_per_page();
    if (last_item_slot < target_slot) {
      target_slot = last_item_slot;
    }
  }

  int target_page = std::min(pagination_model_->total_pages() - 1,
                             std::max(selected.page + page_delta, 0));
  SetSelectedItemByIndex(Index(target_page, target_slot));
}

void AppsGridView::CalculateIdealBounds() {
  gfx::Rect rect(GetContentsBounds());
  if (rect.IsEmpty())
    return;

  gfx::Size tile_size(kPreferredTileWidth, kPreferredTileHeight);

  gfx::Rect grid_rect(gfx::Size(tile_size.width() * cols_,
                                tile_size.height() * rows_per_page_));
  grid_rect.Intersect(rect);

  // Page width including padding pixels. A tile.x + page_width means the same
  // tile slot in the next page.
  const int page_width = grid_rect.width() + kPagePadding;

  // If there is a transition, calculates offset for current and target page.
  const int current_page = pagination_model_->selected_page();
  const PaginationModel::Transition& transition =
      pagination_model_->transition();
  const bool is_valid =
      pagination_model_->is_valid_page(transition.target_page);

  // Transition to right means negative offset.
  const int dir = transition.target_page > current_page ? -1 : 1;
  const int transition_offset = is_valid ?
      transition.progress * page_width * dir : 0;

  const int total_views =
      view_model_.view_size() + pulsing_blocks_model_.view_size();
  int slot_index = 0;
  for (int i = 0; i < total_views; ++i) {
    if (i < view_model_.view_size() && view_model_.view_at(i) == drag_view_)
      continue;

    int page = slot_index / tiles_per_page();
    int slot = slot_index % tiles_per_page();

    if (drop_target_.page == page && drop_target_.slot == slot) {
      ++slot_index;
      page = slot_index / tiles_per_page();
      slot = slot_index % tiles_per_page();
    }

    // Decides an x_offset for current item.
    int x_offset = 0;
    if (page < current_page)
      x_offset = -page_width;
    else if (page > current_page)
      x_offset = page_width;

    if (is_valid) {
      if (page == current_page || page == transition.target_page)
        x_offset += transition_offset;
    }

    const int row = slot / cols_;
    const int col = slot % cols_;
    gfx::Rect tile_slot(
        gfx::Point(grid_rect.x() + col * tile_size.width() + x_offset,
                   grid_rect.y() + row * tile_size.height()),
        tile_size);
    if (i < view_model_.view_size()) {
      view_model_.set_ideal_bounds(i, tile_slot);
    } else {
      pulsing_blocks_model_.set_ideal_bounds(i - view_model_.view_size(),
                                             tile_slot);
    }

    ++slot_index;
  }
}

void AppsGridView::AnimateToIdealBounds() {
  const gfx::Rect visible_bounds(GetVisibleBounds());

  CalculateIdealBounds();
  for (int i = 0; i < view_model_.view_size(); ++i) {
    views::View* view = view_model_.view_at(i);
    if (view == drag_view_)
      continue;

    const gfx::Rect& target = view_model_.ideal_bounds(i);
    if (bounds_animator_.GetTargetBounds(view) == target)
      continue;

    const gfx::Rect& current = view->bounds();
    const bool current_visible = visible_bounds.Intersects(current);
    const bool target_visible = visible_bounds.Intersects(target);
    const bool visible = current_visible || target_visible;

    const int y_diff = target.y() - current.y();
    if (visible && y_diff && y_diff % kPreferredTileHeight == 0) {
      AnimationBetweenRows(view,
                           current_visible,
                           current,
                           target_visible,
                           target);
    } else {
      bounds_animator_.AnimateViewTo(view, target);
    }
  }
}

void AppsGridView::AnimationBetweenRows(views::View* view,
                                        bool animate_current,
                                        const gfx::Rect& current,
                                        bool animate_target,
                                        const gfx::Rect& target) {
  // Determine page of |current| and |target|. -1 means in the left invisible
  // page, 0 is the center visible page and 1 means in the right invisible page.
  const int current_page = current.x() < 0 ? -1 :
      current.x() >= width() ? 1 : 0;
  const int target_page = target.x() < 0 ? -1 :
      target.x() >= width() ? 1 : 0;

  const int dir = current_page < target_page ||
      (current_page == target_page && current.y() < target.y()) ? 1 : -1;

#if defined(USE_AURA)
  scoped_ptr<ui::Layer> layer;
  if (animate_current) {
    layer.reset(view->RecreateLayer());
    layer->SuppressPaint();

    view->SetFillsBoundsOpaquely(false);
    view->layer()->SetOpacity(0.f);
  }

  gfx::Rect current_out(current);
  current_out.Offset(dir * kPreferredTileWidth, 0);
#endif

  gfx::Rect target_in(target);
  if (animate_target)
    target_in.Offset(-dir * kPreferredTileWidth, 0);
  view->SetBoundsRect(target_in);
  bounds_animator_.AnimateViewTo(view, target);

#if defined(USE_AURA)
  bounds_animator_.SetAnimationDelegate(
      view,
      new RowMoveAnimationDelegate(view, layer.release(), current_out),
      true);
#endif
}

void AppsGridView::ExtractDragLocation(const ui::LocatedEvent& event,
                                       gfx::Point* drag_point) {
#if defined(USE_AURA)
  // Use root location of |event| instead of location in |drag_view_|'s
  // coordinates because |drag_view_| has a scale transform and location
  // could have integer round error and causes jitter.
  *drag_point = event.root_location();

  // GetWidget() could be NULL for tests.
  if (GetWidget()) {
    aura::Window::ConvertPointToTarget(
        GetWidget()->GetNativeWindow()->GetRootWindow(),
        GetWidget()->GetNativeWindow(),
        drag_point);
  }

  views::View::ConvertPointFromWidget(this, drag_point);
#else
  // For non-aura, root location is not clearly defined but |drag_view_| does
  // not have the scale transform. So no round error would be introduced and
  // it's okay to use View::ConvertPointToTarget.
  *drag_point = event.location();
  views::View::ConvertPointToTarget(drag_view_, this, drag_point);
#endif
}

void AppsGridView::CalculateDropTarget(const gfx::Point& drag_point,
                                       bool use_page_button_hovering) {
  const int current_page = pagination_model_->selected_page();

  if (use_page_button_hovering &&
      page_switcher_view_->bounds().Contains(drag_point)) {
    gfx::Point page_switcher_point(drag_point);
    views::View::ConvertPointToTarget(this, page_switcher_view_,
                                      &page_switcher_point);
    int page = page_switcher_view_->GetPageForPoint(page_switcher_point);
    if (pagination_model_->is_valid_page(page)) {
      drop_target_.page = page;
      drop_target_.slot = tiles_per_page() - 1;
    }
  } else {
    const int drop_row = drag_point.y() / kPreferredTileHeight;
    const int drop_col = std::min(cols_ - 1,
                                  drag_point.x() / kPreferredTileWidth);

    drop_target_.page = current_page;
    drop_target_.slot = std::max(0, std::min(
        tiles_per_page() - 1,
        drop_row * cols_ + drop_col));
  }

  // Limits to the last possible slot on last page.
  if (drop_target_.page == pagination_model_->total_pages() - 1) {
    drop_target_.slot = std::min(
        (view_model_.view_size() - 1) % tiles_per_page(),
        drop_target_.slot);
  }
}

void AppsGridView::DispatchDragEventToDragAndDropHost(
    const ui::LocatedEvent& event) {
  if (!drag_view_ || !drag_and_drop_host_)
    return;
  if (bounds().Contains(last_drag_point_)) {
    // The event was issued inside the app menu and we should get all events.
    if (forward_events_to_drag_and_drop_host_) {
      // The DnD host was previously called and needs to be informed that the
      // session returns to the owner.
      forward_events_to_drag_and_drop_host_ = false;
      drag_and_drop_host_->EndDrag(true);
    }
  } else {
    // The event happened outside our app menu and we might need to dispatch.
    if (forward_events_to_drag_and_drop_host_) {
      // Dispatch since we have already started.
      if (!drag_and_drop_host_->Drag(event.root_location())) {
        // The host is not active any longer and we cancel the operation.
        forward_events_to_drag_and_drop_host_ = false;
        drag_and_drop_host_->EndDrag(true);
      }
    } else {
      if (drag_and_drop_host_->StartDrag(drag_view_->model()->app_id(),
                                         event.root_location())) {
        // From now on we forward the drag events.
        forward_events_to_drag_and_drop_host_ = true;
        // Any flip operations are stopped.
        page_flip_timer_.Stop();
        page_flip_target_ = -1;
      }
    }
  }
}

void AppsGridView::MaybeStartPageFlipTimer(const gfx::Point& drag_point) {
  int new_page_flip_target = -1;

  if (page_switcher_view_->bounds().Contains(drag_point)) {
    gfx::Point page_switcher_point(drag_point);
    views::View::ConvertPointToTarget(this, page_switcher_view_,
                                      &page_switcher_point);
    new_page_flip_target =
        page_switcher_view_->GetPageForPoint(page_switcher_point);
  }

  // TODO(xiyuan): Fix this for RTL.
  if (new_page_flip_target == -1 && drag_point.x() < kPageFlipZoneSize)
    new_page_flip_target = pagination_model_->selected_page() - 1;

  if (new_page_flip_target == -1 &&
      drag_point.x() > width() - kPageFlipZoneSize) {
    new_page_flip_target = pagination_model_->selected_page() + 1;
  }

  if (new_page_flip_target == page_flip_target_)
    return;

  if (pagination_model_->is_valid_page(new_page_flip_target)) {
    page_flip_target_ = new_page_flip_target;
    page_flip_timer_.Stop();

    if (page_flip_target_ != pagination_model_->selected_page()) {
      page_flip_timer_.Start(FROM_HERE,
          base::TimeDelta::FromMilliseconds(page_flip_delay_in_ms_),
          this, &AppsGridView::OnPageFlipTimer);
    }
  } else {
    page_flip_target_ = -1;
    page_flip_timer_.Stop();
  }
}

void AppsGridView::OnPageFlipTimer() {
  DCHECK(pagination_model_->is_valid_page(page_flip_target_));
  pagination_model_->SelectPage(page_flip_target_, true);
}

void AppsGridView::MoveItemInModel(views::View* item_view,
                                   const Index& target) {
  int current_model_index = view_model_.GetIndexOfView(item_view);
  DCHECK_GE(current_model_index, 0);

  int target_model_index = target.page * tiles_per_page() + target.slot;
  if (target_model_index == current_model_index)
    return;

  model_->apps()->RemoveObserver(this);
  model_->apps()->Move(current_model_index, target_model_index);
  view_model_.Move(current_model_index, target_model_index);
  model_->apps()->AddObserver(this);

  if (pagination_model_->selected_page() != target.page)
    pagination_model_->SelectPage(target.page, false);
}

void AppsGridView::ButtonPressed(views::Button* sender,
                                 const ui::Event& event) {
  if (dragging())
    return;

  if (strcmp(sender->GetClassName(), AppListItemView::kViewClassName))
    return;

  if (delegate_) {
    delegate_->ActivateApp(static_cast<AppListItemView*>(sender)->model(),
                           event.flags());
  }
}

void AppsGridView::ListItemsAdded(size_t start, size_t count) {
  EndDrag(true);

  for (size_t i = start; i < start + count; ++i) {
    views::View* view = CreateViewForItemAtIndex(i);
    view_model_.Add(view, i);
    AddChildView(view);
  }

  UpdatePaging();
  UpdatePulsingBlockViews();
  Layout();
  SchedulePaint();
}

void AppsGridView::ListItemsRemoved(size_t start, size_t count) {
  EndDrag(true);

  for (size_t i = 0; i < count; ++i) {
    views::View* view = view_model_.view_at(start);
    view_model_.Remove(start);
    delete view;
  }

  UpdatePaging();
  UpdatePulsingBlockViews();
  Layout();
  SchedulePaint();
}

void AppsGridView::ListItemMoved(size_t index, size_t target_index) {
  EndDrag(true);
  view_model_.Move(index, target_index);

  UpdatePaging();
  AnimateToIdealBounds();
}

void AppsGridView::ListItemsChanged(size_t start, size_t count) {
  NOTREACHED();
}

void AppsGridView::TotalPagesChanged() {
}

void AppsGridView::SelectedPageChanged(int old_selected, int new_selected) {
  if (dragging()) {
    CalculateDropTarget(last_drag_point_, true);
    Layout();
    MaybeStartPageFlipTimer(last_drag_point_);
  } else {
    ClearSelectedView(selected_view_);
    Layout();
  }
}

void AppsGridView::TransitionChanged() {
  // Update layout for valid page transition only since over-scroll no longer
  // animates app icons.
  const PaginationModel::Transition& transition =
      pagination_model_->transition();
  if (pagination_model_->is_valid_page(transition.target_page))
    Layout();
}

void AppsGridView::OnAppListModelStatusChanged() {
  UpdatePulsingBlockViews();
  Layout();
  SchedulePaint();
}

}  // namespace app_list
