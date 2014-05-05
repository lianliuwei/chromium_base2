// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ui/message_center/cocoa/notification_controller.h"

#include "base/mac/foundation_util.h"
#include "base/strings/sys_string_conversions.h"
#include "grit/ui_resources.h"
#include "skia/ext/skia_utils_mac.h"
#import "third_party/GTM/AppKit/GTMUILocalizerAndLayoutTweaker.h"
#import "ui/base/cocoa/hover_image_button.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/message_center/message_center.h"
#include "ui/message_center/message_center_style.h"
#include "ui/message_center/notification.h"

@interface MCNotificationButtonCell : NSButtonCell {
  BOOL hovered_;
}
@end

@implementation MCNotificationButtonCell
- (void)drawBezelWithFrame:(NSRect)frame inView:(NSView*)controlView {
  // Else mouseEntered: and mouseExited: won't be called and hovered_ won't be
  // valid.
  DCHECK([self showsBorderOnlyWhileMouseInside]);

  if (!hovered_)
    return;
  [gfx::SkColorToCalibratedNSColor(
      message_center::kHoveredButtonBackgroundColor) set];
  NSRectFill(frame);
}

- (void)drawImage:(NSImage*)image
        withFrame:(NSRect)frame
           inView:(NSView*)controlView {
  if (!image)
    return;
  NSRect rect = NSMakeRect(message_center::kButtonHorizontalPadding,
                           message_center::kButtonIconTopPadding,
                           message_center::kNotificationButtonIconSize,
                           message_center::kNotificationButtonIconSize);
  [image drawInRect:rect
            fromRect:NSZeroRect
           operation:NSCompositeSourceOver
            fraction:1.0
      respectFlipped:YES
               hints:nil];
}

- (NSRect)drawTitle:(NSAttributedString*)title
          withFrame:(NSRect)frame
             inView:(NSView*)controlView {
  CGFloat offsetX = message_center::kButtonHorizontalPadding;
  if ([base::mac::ObjCCastStrict<NSButton>(controlView) image]) {
    offsetX += message_center::kNotificationButtonIconSize +
               message_center::kButtonIconToTitlePadding;
  }
  frame.origin.x = offsetX;
  frame.size.width -= offsetX;

  NSDictionary* attributes = @{
    NSFontAttributeName : [title attribute:NSFontAttributeName
                                   atIndex:0
                            effectiveRange:NULL],
  };
  [[title string] drawWithRect:frame
                       options:(NSStringDrawingUsesLineFragmentOrigin |
                                NSStringDrawingTruncatesLastVisibleLine)
                    attributes:attributes];
  return frame;
}

- (void)mouseEntered:(NSEvent*)event {
  hovered_ = YES;

  // Else the cell won't be repainted on hover.
  [super mouseEntered:event];
}

- (void)mouseExited:(NSEvent*)event {
  hovered_ = NO;
  [super mouseExited:event];
}
@end

@interface MCNotificationController (Private)
// Configures a NSBox to be borderless, titleless, and otherwise appearance-
// free.
- (void)configureCustomBox:(NSBox*)box;

// Initializes the icon_ ivar and returns the view to insert into the hierarchy.
- (NSView*)createImageView;

// Initializes the closeButton_ ivar with the configured button.
- (void)configureCloseButtonInFrame:(NSRect)rootFrame;

// Initializes title_ in the given frame.
- (void)configureTitleInFrame:(NSRect)rootFrame;

// Initializes message_ in the given frame.
- (void)configureBodyInFrame:(NSRect)rootFrame;

// Creates a NSTextField that the caller owns configured as a label in a
// notification.
- (NSTextField*)newLabelWithFrame:(NSRect)frame;

// Gets the rectangle in which notification content should be placed. This
// rectangle is to the right of the icon and left of the control buttons.
// This depends on the icon_ and closeButton_ being initialized.
- (NSRect)currentContentRect;
@end

@implementation MCNotificationController

- (id)initWithNotification:(const message_center::Notification*)notification
    messageCenter:(message_center::MessageCenter*)messageCenter {
  if ((self = [super initWithNibName:nil bundle:nil])) {
    notification_ = notification;
    notificationID_ = notification_->id();
    messageCenter_ = messageCenter;
  }
  return self;
}

- (void)loadView {
  // Create the root view of the notification.
  NSRect rootFrame = NSMakeRect(0, 0,
      message_center::kNotificationPreferredImageSize,
      message_center::kNotificationIconSize);
  scoped_nsobject<NSBox> rootView([[NSBox alloc] initWithFrame:rootFrame]);
  [self configureCustomBox:rootView];
  [rootView setFillColor:gfx::SkColorToCalibratedNSColor(
      message_center::kNotificationBackgroundColor)];
  [self setView:rootView];

  [rootView addSubview:[self createImageView]];

  // Create the close button.
  [self configureCloseButtonInFrame:rootFrame];
  [rootView addSubview:closeButton_];

  // Create the title.
  [self configureTitleInFrame:rootFrame];
  [rootView addSubview:title_];

  // Create the message body.
  [self configureBodyInFrame:rootFrame];
  [rootView addSubview:message_];

  // Populate the data.
  [self updateNotification:notification_];
}

- (NSRect)updateNotification:(const message_center::Notification*)notification {
  DCHECK_EQ(notification->id(), notificationID_);
  notification_ = notification;

  NSRect rootFrame = NSMakeRect(0, 0,
      message_center::kNotificationPreferredImageSize,
      message_center::kNotificationIconSize);

  // Update the icon.
  [icon_ setImage:notification_->icon().AsNSImage()];

  // The message_center:: constants are relative to capHeight at the top and
  // relative to the baseline at the bottom, but NSTextField uses the full line
  // height for its height.
  CGFloat titleTopGap = [[title_ font] ascender] - [[title_ font] capHeight];
  CGFloat titleBottomGap = fabs([[title_ font] descender]);
  CGFloat titlePadding = message_center::kTextTopPadding - titleTopGap;

  CGFloat messageTopGap =
      [[message_ font] ascender] - [[message_ font] capHeight];
  CGFloat messagePadding =
      message_center::kTextTopPadding - titleBottomGap - messageTopGap;

  // Set the title and recalculate the frame.
  [title_ setStringValue:base::SysUTF16ToNSString(notification_->title())];
  [GTMUILocalizerAndLayoutTweaker sizeToFitFixedWidthTextField:title_];
  NSRect titleFrame = [title_ frame];
  titleFrame.origin.y = NSMaxY(rootFrame) - titlePadding - NSHeight(titleFrame);

  // Set the message and recalculate the frame.
  [message_ setStringValue:base::SysUTF16ToNSString(notification_->message())];
  [GTMUILocalizerAndLayoutTweaker sizeToFitFixedWidthTextField:message_];
  NSRect messageFrame = [message_ frame];
  messageFrame.origin.y =
      NSMinY(titleFrame) - messagePadding - NSHeight(messageFrame);
  messageFrame.size.height = NSHeight([message_ frame]);

  // In this basic notification UI, the message body is the bottom-most
  // vertical element. If it is out of the rootView's bounds, resize the view.
  if (NSMinY(messageFrame) < messagePadding) {
    CGFloat delta = messagePadding - NSMinY(messageFrame);
    rootFrame.size.height += delta;
    titleFrame.origin.y += delta;
    messageFrame.origin.y += delta;
  }

  // Add the bottom container view.
  NSRect frame = rootFrame;
  frame.size.height = 0;
  [bottomView_ removeFromSuperview];
  bottomView_.reset([[NSView alloc] initWithFrame:frame]);
  CGFloat y = 0;

  // Create action buttons if appropriate, bottom-up.
  std::vector<message_center::ButtonInfo> buttons = notification->buttons();
  for (int i = buttons.size() - 1; i >= 0; --i) {
    message_center::ButtonInfo buttonInfo = buttons[i];
    NSRect buttonFrame = frame;
    buttonFrame.origin = NSMakePoint(0, y);
    buttonFrame.size.height = message_center::kButtonHeight;
    scoped_nsobject<NSButton> button(
        [[NSButton alloc] initWithFrame:buttonFrame]);
    scoped_nsobject<MCNotificationButtonCell> cell(
        [[MCNotificationButtonCell alloc]
            initTextCell:base::SysUTF16ToNSString(buttonInfo.title)]);
    [cell setShowsBorderOnlyWhileMouseInside:YES];
    [button setCell:cell];
    [button setImage:buttonInfo.icon.AsNSImage()];
    [button setBezelStyle:NSSmallSquareBezelStyle];
    [button setImagePosition:NSImageLeft];
    [button setTag:i];
    [button setTarget:self];
    [button setAction:@selector(buttonClicked:)];
    y += NSHeight(buttonFrame);
    frame.size.height += NSHeight(buttonFrame);
    [bottomView_ addSubview:button];

    NSRect separatorFrame = frame;
    separatorFrame.origin = NSMakePoint(0, y);
    separatorFrame.size.height = 1;
    scoped_nsobject<NSBox> separator(
        [[NSBox alloc] initWithFrame:separatorFrame]);
    [self configureCustomBox:separator];
    [separator setFillColor:gfx::SkColorToCalibratedNSColor(
        message_center::kButtonSeparatorColor)];
    y += NSHeight(separatorFrame);
    frame.size.height += NSHeight(separatorFrame);
    [bottomView_ addSubview:separator];
  }

  // Create the image view if appropriate.
  if (!notification->image().IsEmpty()) {
    NSImage* image = notification->image().AsNSImage();
    NSRect imageFrame = frame;
    imageFrame.origin = NSMakePoint(0, y);
    imageFrame.size = NSSizeFromCGSize(message_center::GetImageSizeForWidth(
        NSWidth(frame), notification->image().Size()).ToCGSize());
    scoped_nsobject<NSImageView> imageView(
        [[NSImageView alloc] initWithFrame:imageFrame]);
    [imageView setImage:image];
    [imageView setImageScaling:NSImageScaleProportionallyUpOrDown];
    y += NSHeight(imageFrame);
    frame.size.height += NSHeight(imageFrame);
    [bottomView_ addSubview:imageView];
  }

  [bottomView_ setFrame:frame];
  [[self view] addSubview:bottomView_];

  rootFrame.size.height += NSHeight(frame);
  titleFrame.origin.y += NSHeight(frame);
  messageFrame.origin.y += NSHeight(frame);

  [[self view] setFrame:rootFrame];
  [title_ setFrame:titleFrame];
  [message_ setFrame:messageFrame];

  return rootFrame;
}

- (void)close:(id)sender {
  messageCenter_->RemoveNotification(notification_->id(), /*by_user=*/true);
}

- (void)buttonClicked:(id)button {
  messageCenter_->ClickOnNotificationButton(notification_->id(), [button tag]);
}

- (const message_center::Notification*)notification {
  return notification_;
}

- (const std::string&)notificationID {
  return notificationID_;
}

// Private /////////////////////////////////////////////////////////////////////

- (void)configureCustomBox:(NSBox*)box {
  [box setBoxType:NSBoxCustom];
  [box setBorderType:NSNoBorder];
  [box setTitlePosition:NSNoTitle];
  [box setContentViewMargins:NSZeroSize];
}

- (NSView*)createImageView {
  // Create another box that shows a background color when the icon is not
  // big enough to fill the space.
  NSRect imageFrame = NSMakeRect(0, 0,
       message_center::kNotificationIconSize,
       message_center::kNotificationIconSize);
  scoped_nsobject<NSBox> imageBox([[NSBox alloc] initWithFrame:imageFrame]);
  [self configureCustomBox:imageBox];
  [imageBox setFillColor:gfx::SkColorToCalibratedNSColor(
      message_center::kLegacyIconBackgroundColor)];
  [imageBox setAutoresizingMask:NSViewMinYMargin];

  // Inside the image box put the actual icon view.
  icon_.reset([[NSImageView alloc] initWithFrame:imageFrame]);
  [imageBox setContentView:icon_];

  return imageBox.autorelease();
}

- (void)configureCloseButtonInFrame:(NSRect)rootFrame {
  closeButton_.reset([[HoverImageButton alloc] initWithFrame:NSMakeRect(
      NSMaxX(rootFrame) - message_center::kControlButtonSize,
      NSMaxY(rootFrame) - message_center::kControlButtonSize,
      message_center::kControlButtonSize,
      message_center::kControlButtonSize)]);
  ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
  [closeButton_ setDefaultImage:
      rb.GetNativeImageNamed(IDR_NOTIFICATION_CLOSE).ToNSImage()];
  [closeButton_ setHoverImage:
      rb.GetNativeImageNamed(IDR_NOTIFICATION_CLOSE_HOVER).ToNSImage()];
  [closeButton_ setPressedImage:
      rb.GetNativeImageNamed(IDR_NOTIFICATION_CLOSE_PRESSED).ToNSImage()];
  [[closeButton_ cell] setHighlightsBy:NSOnState];
  [closeButton_ setTrackingEnabled:YES];
  [closeButton_ setBordered:NO];
  [closeButton_ setAutoresizingMask:NSViewMinYMargin];
  [closeButton_ setTarget:self];
  [closeButton_ setAction:@selector(close:)];
}

- (void)configureTitleInFrame:(NSRect)rootFrame {
  NSRect frame = [self currentContentRect];
  frame.size.height = 0;
  title_.reset([self newLabelWithFrame:frame]);
  [title_ setAutoresizingMask:NSViewMinYMargin];
  [title_ setFont:[NSFont messageFontOfSize:message_center::kTitleFontSize]];
}

- (void)configureBodyInFrame:(NSRect)rootFrame {
  NSRect frame = [self currentContentRect];
  frame.size.height = 0;
  message_.reset([self newLabelWithFrame:frame]);
  [message_ setAutoresizingMask:NSViewMinYMargin];
  [message_ setFont:
      [NSFont messageFontOfSize:message_center::kMessageFontSize]];
}

- (NSTextField*)newLabelWithFrame:(NSRect)frame {
  NSTextField* label = [[NSTextField alloc] initWithFrame:frame];
  [label setDrawsBackground:NO];
  [label setBezeled:NO];
  [label setEditable:NO];
  [label setSelectable:NO];
  [label setTextColor:gfx::SkColorToCalibratedNSColor(
      message_center::kRegularTextColor)];
  return label;
}

- (NSRect)currentContentRect {
  DCHECK(icon_);
  DCHECK(closeButton_);

  NSRect iconFrame, contentFrame;
  NSDivideRect([[self view] bounds], &iconFrame, &contentFrame,
      NSWidth([icon_ frame]) + message_center::kIconToTextPadding,
      NSMinXEdge);
  contentFrame.size.width -= NSWidth([closeButton_ frame]);
  return contentFrame;
}

@end
