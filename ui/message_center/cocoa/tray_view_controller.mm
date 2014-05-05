// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ui/message_center/cocoa/tray_view_controller.h"

#include <cmath>

#include "base/time.h"
#include "grit/ui_resources.h"
#include "grit/ui_strings.h"
#include "skia/ext/skia_utils_mac.h"
#import "ui/base/cocoa/hover_image_button.h"
#include "ui/base/l10n/l10n_util_mac.h"
#include "ui/base/resource/resource_bundle.h"
#import "ui/message_center/cocoa/notification_controller.h"
#include "ui/message_center/message_center.h"
#include "ui/message_center/message_center_style.h"

@interface MCTrayViewController (Private)
// Creates all the views for the control area of the tray.
- (void)layoutControlArea;
@end

namespace {

// The height of the bar at the top of the tray that contains buttons.
const CGFloat kControlAreaHeight = 50;

// Amount of spacing between control buttons. There is kMarginBetweenItems
// between a button and the edge of the tray, though.
const CGFloat kButtonXMargin = 20;

// Amount of padding to leave between the bottom of the screen and the bottom
// of the message center tray.
const CGFloat kTrayBottomMargin = 75;

}  // namespace

@implementation MCTrayViewController

- (id)initWithMessageCenter:(message_center::MessageCenter*)messageCenter {
  if ((self = [super initWithNibName:nil bundle:nil])) {
    messageCenter_ = messageCenter;
    notifications_.reset([[NSMutableArray alloc] init]);
  }
  return self;
}

- (void)loadView {
  // Configure the root view as a background-colored box.
  scoped_nsobject<NSBox> view([[NSBox alloc] initWithFrame:
      NSMakeRect(0, 0,
                 message_center::kNotificationWidth +
                     2 * message_center::kMarginBetweenItems,
                 kControlAreaHeight)]);
  [view setBorderType:NSNoBorder];
  [view setBoxType:NSBoxCustom];
  [view setContentViewMargins:NSZeroSize];
  [view setFillColor:gfx::SkColorToCalibratedNSColor(
      message_center::kMessageCenterBackgroundColor)];
  [view setTitlePosition:NSNoTitle];
  [view setWantsLayer:YES];  // Needed for notification view shadows.
  [self setView:view];

  [self layoutControlArea];

  // Configure the scroll view in which all the notifications go.
  scoped_nsobject<NSView> documentView(
      [[NSView alloc] initWithFrame:NSZeroRect]);
  scrollView_.reset([[NSScrollView alloc] initWithFrame:[view frame]]);
  [scrollView_ setAutohidesScrollers:YES];
  [scrollView_ setAutoresizingMask:NSViewHeightSizable | NSViewMaxYMargin];
  [scrollView_ setDocumentView:documentView];
  [scrollView_ setDrawsBackground:NO];
  [scrollView_ setHasHorizontalScroller:NO];
  [scrollView_ setHasVerticalScroller:YES];
  [view addSubview:scrollView_];

  [self onMessageCenterTrayChanged];
}

- (void)onMessageCenterTrayChanged {
  std::map<std::string, MCNotificationController*> newMap;

  scoped_nsobject<NSShadow> shadow([[NSShadow alloc] init]);
  [shadow setShadowColor:
      gfx::SkColorToCalibratedNSColor(message_center::kShadowColor)];
  [shadow setShadowOffset:NSMakeSize(0, -1)];
  [shadow setShadowBlurRadius:4.0];

  CGFloat minY = message_center::kMarginBetweenItems;

  // Iterate over the notifications in reverse, since the Cocoa coordinate
  // origin is in the lower-left. Remove from |notificationsMap_| all the
  // ones still in the updated model, so that those that should be removed
  // will remain in the map.
  const auto& modelNotifications = messageCenter_->GetNotifications();
  for (auto it = modelNotifications.rbegin();
       it != modelNotifications.rend();
       ++it) {
    // Check if this notification is already in the tray.
    const auto& existing = notificationsMap_.find((*it)->id());
    MCNotificationController* notification = nil;
    if (existing == notificationsMap_.end()) {
      scoped_nsobject<MCNotificationController> controller(
          [[MCNotificationController alloc]
              initWithNotification:*it
                     messageCenter:messageCenter_]);
      [[controller view] setShadow:shadow];
      [[scrollView_ documentView] addSubview:[controller view]];

      [notifications_ addObject:controller];  // Transfer ownership.
      notification = controller.get();
    } else {
      notification = existing->second;
      [notification updateNotification:*it];
      notificationsMap_.erase(existing);
    }

    DCHECK(notification);

    NSRect frame = [[notification view] frame];
    frame.origin.x = message_center::kMarginBetweenItems;
    frame.origin.y = minY;
    [[notification view] setFrame:frame];

    newMap.insert(std::make_pair((*it)->id(), notification));

    minY = NSMaxY(frame) + message_center::kMarginBetweenItems;
  }

  // Remove any notifications that are no longer in the model.
  for (const auto& pair : notificationsMap_) {
    [[pair.second view] removeFromSuperview];
    [notifications_ removeObject:pair.second];
  }

  // Don't add extra padding if the center is empty.
  if (minY == message_center::kMarginBetweenItems)
    minY = 0;

  // Resize the scroll view's content.
  NSRect scrollViewFrame = [scrollView_ frame];
  NSRect documentFrame = [[scrollView_ documentView] frame];
  documentFrame.size.width = NSWidth(scrollViewFrame);
  documentFrame.size.height = minY;
  [[scrollView_ documentView] setFrame:documentFrame];

  // Copy the new map of notifications to replace the old.
  notificationsMap_ = newMap;

  // Resize the container view.
  NSRect screenFrame = [[[NSScreen screens] objectAtIndex:0] visibleFrame];
  NSRect frame = [[self view] frame];
  frame.size.height = std::min(NSHeight(screenFrame) - kTrayBottomMargin,
                               minY + kControlAreaHeight);
  [[self view] setFrame:frame];

  // Resize the scroll view.
  scrollViewFrame.size.height = NSHeight(frame) - kControlAreaHeight;
  [scrollView_ setFrame:scrollViewFrame];

  // Hide the clear-all button if there are no notifications. Simply swap the
  // X position of it and the pause button in that case.
  BOOL hidden = modelNotifications.size() == 0;
  if ([clearAllButton_ isHidden] != hidden) {
    [clearAllButton_ setHidden:hidden];

    NSRect pauseButtonFrame = [pauseButton_ frame];
    NSRect clearAllButtonFrame = [clearAllButton_ frame];
    std::swap(clearAllButtonFrame.origin.x, pauseButtonFrame.origin.x);
    [pauseButton_ setFrame:pauseButtonFrame];
    [clearAllButton_ setFrame:clearAllButtonFrame];
  }
}

- (void)toggleQuietMode:(id)sender {
  ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
  if (messageCenter_->IsQuietMode()) {
    messageCenter_->SetQuietMode(false);
    [pauseButton_ setTrackingEnabled:YES];
    [pauseButton_ setDefaultImage:
        rb.GetNativeImageNamed(IDR_NOTIFICATION_PAUSE).ToNSImage()];
  } else {
    messageCenter_->EnterQuietModeWithExpire(base::TimeDelta::FromDays(1));
    [pauseButton_ setTrackingEnabled:NO];
    [pauseButton_ setDefaultImage:
        rb.GetNativeImageNamed(IDR_NOTIFICATION_PAUSE_PRESSED).ToNSImage()];
  }
}

- (void)clearAllNotifications:(id)sender {
  messageCenter_->RemoveAllNotifications(true);
}

- (void)showSettings:(id)sender {
  // TODO(rsesek): Implement settings.
}

// Testing API /////////////////////////////////////////////////////////////////

- (NSScrollView*)scrollView {
  return scrollView_.get();
}

- (HoverImageButton*)pauseButton {
  return pauseButton_.get();
}

- (HoverImageButton*)clearAllButton {
  return clearAllButton_.get();
}

// Private /////////////////////////////////////////////////////////////////////

- (void)layoutControlArea {
  NSView* view = [self view];

  // Create the "Notifications" label at the top of the tray.
  NSFont* font = [NSFont labelFontOfSize:message_center::kTitleFontSize];
  scoped_nsobject<NSTextField> title(
      [[NSTextField alloc] initWithFrame:NSZeroRect]);
  [title setAutoresizingMask:NSViewMinYMargin];
  [title setBezeled:NO];
  [title setBordered:NO];
  [title setDrawsBackground:NO];
  [title setEditable:NO];
  [title setFont:font];
  [title setSelectable:NO];
  [title setStringValue:
      l10n_util::GetNSString(IDS_MESSAGE_CENTER_FOOTER_TITLE)];
  [title setTextColor:gfx::SkColorToCalibratedNSColor(
      message_center::kFooterTextColor)];
  [title sizeToFit];

  NSRect titleFrame = [title frame];
  titleFrame.origin.x = message_center::kMarginBetweenItems;
  titleFrame.origin.y = kControlAreaHeight/2 - NSMidY(titleFrame);
  [title setFrame:titleFrame];
  [view addSubview:title];

  // Create the divider line between the control area and the notifications.
  scoped_nsobject<NSBox> divider(
      [[NSBox alloc] initWithFrame:NSMakeRect(0, 0, NSWidth([view frame]), 1)]);
  [divider setAutoresizingMask:NSViewMinYMargin];
  [divider setBorderType:NSNoBorder];
  [divider setBoxType:NSBoxCustom];
  [divider setContentViewMargins:NSZeroSize];
  [divider setFillColor:gfx::SkColorToCalibratedNSColor(
      message_center::kFooterDelimiterColor)];
  [divider setTitlePosition:NSNoTitle];
  [view addSubview:divider];

  auto getButtonFrame = ^NSRect(CGFloat maxX, NSImage* image) {
      NSSize size = [image size];
      return NSMakeRect(
          maxX - size.width,
          kControlAreaHeight/2 - size.height/2,
          size.width,
          size.height);
  };

  auto configureButton = ^(HoverImageButton* button) {
      [[button cell] setHighlightsBy:NSOnState];
      [button setTrackingEnabled:YES];
      [button setBordered:NO];
      [button setAutoresizingMask:NSViewMinYMargin];
      [button setTarget:self];
  };

  // Create the settings button at the far-right.
  ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
  NSImage* defaultImage =
      rb.GetNativeImageNamed(IDR_NOTIFICATION_SETTINGS).ToNSImage();
  NSRect settingsButtonFrame = getButtonFrame(
      NSWidth([view frame]) - message_center::kMarginBetweenItems,
      defaultImage);
  scoped_nsobject<HoverImageButton> settingsButton(
      [[HoverImageButton alloc] initWithFrame:settingsButtonFrame]);
  [settingsButton setDefaultImage:defaultImage];
  [settingsButton setHoverImage:
      rb.GetNativeImageNamed(IDR_NOTIFICATION_SETTINGS_HOVER).ToNSImage()];
  [settingsButton setPressedImage:
      rb.GetNativeImageNamed(IDR_NOTIFICATION_SETTINGS_PRESSED).ToNSImage()];
  [settingsButton setToolTip:
      l10n_util::GetNSString(IDS_MESSAGE_CENTER_SETTINGS_BUTTON_LABEL)];
  [settingsButton setAction:@selector(showSettings:)];
  configureButton(settingsButton);
  [view addSubview:settingsButton];

  // Create the clear all button.
  defaultImage = rb.GetNativeImageNamed(IDR_NOTIFICATION_CLEAR_ALL).ToNSImage();
  NSRect clearAllButtonFrame = getButtonFrame(
      NSMinX(settingsButtonFrame) - kButtonXMargin,
      defaultImage);
  clearAllButton_.reset(
      [[HoverImageButton alloc] initWithFrame:clearAllButtonFrame]);
  [clearAllButton_ setDefaultImage:defaultImage];
  [clearAllButton_ setHoverImage:
      rb.GetNativeImageNamed(IDR_NOTIFICATION_CLEAR_ALL_HOVER).ToNSImage()];
  [clearAllButton_ setPressedImage:
      rb.GetNativeImageNamed(IDR_NOTIFICATION_CLEAR_ALL_PRESSED).ToNSImage()];
  [clearAllButton_ setToolTip:
      l10n_util::GetNSString(IDS_MESSAGE_CENTER_CLEAR_ALL)];
  [clearAllButton_ setAction:@selector(clearAllNotifications:)];
  configureButton(clearAllButton_);
  [view addSubview:clearAllButton_];

  // Create the pause button.
  defaultImage = rb.GetNativeImageNamed(IDR_NOTIFICATION_PAUSE).ToNSImage();
  NSRect pauseButtonFrame = getButtonFrame(
      NSMinX(clearAllButtonFrame) - kButtonXMargin,
      defaultImage);
  pauseButton_.reset([[HoverImageButton alloc] initWithFrame:pauseButtonFrame]);
  [pauseButton_ setDefaultImage:defaultImage];
  [pauseButton_ setHoverImage:
      rb.GetNativeImageNamed(IDR_NOTIFICATION_PAUSE_HOVER).ToNSImage()];
  [pauseButton_ setPressedImage:
      rb.GetNativeImageNamed(IDR_NOTIFICATION_PAUSE_PRESSED).ToNSImage()];
  [pauseButton_ setToolTip:
      l10n_util::GetNSString(IDS_MESSAGE_CENTER_QUIET_MODE_BUTTON_TOOLTIP)];
  [pauseButton_ setAction:@selector(toggleQuietMode:)];
  configureButton(pauseButton_);
  [view addSubview:pauseButton_];
}

@end
