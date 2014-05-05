// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ui/message_center/cocoa/popup_controller.h"

#include <cmath>

#import "base/mac/foundation_util.h"
#import "ui/base/cocoa/window_size_constants.h"
#import "ui/message_center/cocoa/notification_controller.h"
#import "ui/message_center/cocoa/popup_collection.h"
#include "ui/message_center/message_center.h"

#if !defined(MAC_OS_X_VERSION_10_7) || \
    MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_7
enum {
  NSWindowCollectionBehaviorFullScreenAuxiliary = 1 << 8
};

enum {
  NSEventPhaseNone        = 0, // event not associated with a phase.
  NSEventPhaseBegan       = 0x1 << 0,
  NSEventPhaseStationary  = 0x1 << 1,
  NSEventPhaseChanged     = 0x1 << 2,
  NSEventPhaseEnded       = 0x1 << 3,
  NSEventPhaseCancelled   = 0x1 << 4,
};
typedef NSUInteger NSEventPhase;

enum {
  NSEventSwipeTrackingLockDirection = 0x1 << 0,
  NSEventSwipeTrackingClampGestureAmount = 0x1 << 1
};
typedef NSUInteger NSEventSwipeTrackingOptions;

@interface NSEvent (LionAPI)
- (NSEventPhase)phase;
- (CGFloat)scrollingDeltaX;
- (CGFloat)scrollingDeltaY;
- (void)trackSwipeEventWithOptions:(NSEventSwipeTrackingOptions)options
          dampenAmountThresholdMin:(CGFloat)minDampenThreshold
                               max:(CGFloat)maxDampenThreshold
                      usingHandler:(void (^)(CGFloat gestureAmount,
                                             NSEventPhase phase,
                                             BOOL isComplete,
                                             BOOL* stop))trackingHandler;
@end
#endif  // MAC_OS_X_VERSION_10_7

////////////////////////////////////////////////////////////////////////////////

@interface MCPopupController (Private)
- (void)notificationSwipeStarted;
- (void)notificationSwipeMoved:(CGFloat)amount;
- (void)notificationSwipeEnded:(BOOL)ended complete:(BOOL)isComplete;
@end

// Window Subclass /////////////////////////////////////////////////////////////

@interface MCPopupWindow : NSWindow {
  // The cumulative X and Y scrollingDeltas since the -scrollWheel: event began.
  NSPoint totalScrollDelta_;
}
@end

@implementation MCPopupWindow

- (void)scrollWheel:(NSEvent*)event {
  // Gesture swiping only exists on 10.7+.
  if (![event respondsToSelector:@selector(phase)])
    return;

  NSEventPhase phase = [event phase];
  BOOL shouldTrackSwipe = NO;

  if (phase == NSEventPhaseBegan) {
    totalScrollDelta_ = NSZeroPoint;
  } else if (phase == NSEventPhaseChanged) {
    shouldTrackSwipe = YES;
    totalScrollDelta_.x += [event scrollingDeltaX];
    totalScrollDelta_.y += [event scrollingDeltaY];
  }

  // Only allow horizontal scrolling.
  if (std::abs(totalScrollDelta_.x) < std::abs(totalScrollDelta_.y))
    return;

  if (shouldTrackSwipe) {
    MCPopupController* controller =
        base::mac::ObjCCastStrict<MCPopupController>([self windowController]);

    auto handler = ^(CGFloat gestureAmount, NSEventPhase phase,
                     BOOL isComplete, BOOL* stop) {
        if (phase == NSEventPhaseBegan) {
          [controller notificationSwipeStarted];
          return;
        }

        [controller notificationSwipeMoved:gestureAmount];

        BOOL ended = phase == NSEventPhaseEnded;
        if (ended || isComplete)
          [controller notificationSwipeEnded:ended complete:isComplete];
    };
    [event trackSwipeEventWithOptions:0
             dampenAmountThresholdMin:-1
                                  max:1
                         usingHandler:handler];
  }
}

@end

////////////////////////////////////////////////////////////////////////////////

@implementation MCPopupController

- (id)initWithNotification:(const message_center::Notification*)notification
             messageCenter:(message_center::MessageCenter*)messageCenter
           popupCollection:(MCPopupCollection*)popupCollection {
  scoped_nsobject<MCPopupWindow> window(
      [[MCPopupWindow alloc] initWithContentRect:ui::kWindowSizeDeterminedLater
                                  styleMask:NSBorderlessWindowMask
                                    backing:NSBackingStoreBuffered
                                      defer:YES]);
  if ((self = [super initWithWindow:window])) {
    messageCenter_ = messageCenter;
    popupCollection_ = popupCollection;
    notificationController_.reset(
        [[MCNotificationController alloc] initWithNotification:notification
                                                 messageCenter:messageCenter_]);
    isClosing_ = NO;
    bounds_ = [[notificationController_ view] frame];

    [window setReleasedWhenClosed:NO];

    [window setLevel:NSFloatingWindowLevel];
    [window setExcludedFromWindowsMenu:YES];
    [window setCollectionBehavior:
        NSWindowCollectionBehaviorIgnoresCycle |
        NSWindowCollectionBehaviorFullScreenAuxiliary];

    [window setHasShadow:YES];
    [window setContentView:[notificationController_ view]];
  }
  return self;
}

- (void)close {
  if (boundsAnimation_) {
    [boundsAnimation_ stopAnimation];
    boundsAnimation_.reset();
  }
  [super close];
  [self release];
}

- (MCNotificationController*)notificationController {
  return notificationController_.get();
}

- (const message_center::Notification*)notification {
  return [notificationController_ notification];
}

- (const std::string&)notificationID {
  return [notificationController_ notificationID];
}

// Private /////////////////////////////////////////////////////////////////////

- (void)notificationSwipeStarted {
  originalFrame_ = [[self window] frame];
  swipeGestureEnded_ = NO;
}

- (void)notificationSwipeMoved:(CGFloat)amount {
  NSWindow* window = [self window];

  [window setAlphaValue:1.0 - std::abs(amount)];
  NSRect frame = [window frame];
  CGFloat originalMin = NSMinX(originalFrame_);
  frame.origin.x = originalMin + (NSMidX(originalFrame_) - originalMin) *
                   -amount;
  [window setFrame:frame display:YES];
}

- (void)notificationSwipeEnded:(BOOL)ended complete:(BOOL)isComplete {
  swipeGestureEnded_ |= ended;
  if (swipeGestureEnded_ && isComplete)
    messageCenter_->RemoveNotification([self notificationID], /*by_user=*/true);
}

- (void)animationDidEnd:(NSAnimation*)animation {
  if (animation != boundsAnimation_.get())
    return;
  boundsAnimation_.reset();

  [popupCollection_ onPopupAnimationEnded:[self notificationID]];

  if (isClosing_)
    [self close];
}

- (void)showWithAnimation:(NSRect)newBounds {
  NSRect startBounds = newBounds;
  startBounds.origin.x += startBounds.size.width;
  startBounds.size.width = 0;
  bounds_ = startBounds;
  [[self window] setFrame:startBounds display:NO];
  [self showWindow:nil];
  [self setBounds:newBounds];
}

- (void)closeWithAnimation {
  if (isClosing_)
    return;
  isClosing_ = YES;

  NSDictionary* animationDict = @{
    NSViewAnimationTargetKey:   [self window],
    NSViewAnimationEffectKey:   NSViewAnimationFadeOutEffect
  };
  boundsAnimation_.reset([[NSViewAnimation alloc]
      initWithViewAnimations:[NSArray arrayWithObject:animationDict]]);
  [boundsAnimation_ setDuration:[popupCollection_ popupAnimationDuration]];
  [boundsAnimation_ setDelegate:self];
  [boundsAnimation_ startAnimation];
}

- (void)markPopupCollectionGone {
  popupCollection_ = nil;
}

- (NSRect)bounds {
  return bounds_;
}

- (void)setBounds:(NSRect)newBounds {
  if (isClosing_ || NSEqualRects(bounds_ , newBounds))
    return;
  bounds_ = newBounds;

  NSDictionary* animationDict = @{
    NSViewAnimationTargetKey:   [self window],
    NSViewAnimationEndFrameKey: [NSValue valueWithRect:newBounds]
  };
  boundsAnimation_.reset([[NSViewAnimation alloc]
      initWithViewAnimations:[NSArray arrayWithObject:animationDict]]);
  [boundsAnimation_ setDuration:[popupCollection_ popupAnimationDuration]];
  [boundsAnimation_ setDelegate:self];
  [boundsAnimation_ startAnimation];
}

@end
