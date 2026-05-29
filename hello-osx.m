#import <AppKit/AppKit.h>
#import <CoreFoundation/CoreFoundation.h>
#import <MetalKit/MetalKit.h>

#include "app.h"

@interface POCViewDelegate : NSObject<MTKViewDelegate>
@property (nonatomic) BOOL ready;
@end
@implementation POCViewDelegate
- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
}
- (void)drawInMTKView:(MTKView *)view {
  if (!self.ready) {
    app_init();
    self.ready = YES;
  }
  app_frame();
}
@end

@interface POCView : MTKView
@end
@implementation POCView
- (BOOL)acceptsFirstResponder {
  return YES;
}
- (void)keyDown:(NSEvent *)event {
  NSString * chrs = event.charactersIgnoringModifiers;
  if (chrs.length != 1) return;

  unichar c = [chrs characterAtIndex:0];
  switch (c) {
    //case NSLeftArrowFunctionKey:  return skb_api->move(-1,  0);
    //case NSRightArrowFunctionKey: return skb_api->move( 1,  0);
    //case NSUpArrowFunctionKey:    return skb_api->move( 0, -1);
    //case NSDownArrowFunctionKey:  return skb_api->move( 0,  1);

    //case 27: return skb_api->escape();
    //case 32: return skb_api->space();
  }
}
@end

@interface POCAppDelegate : NSObject<NSApplicationDelegate>
@end
@implementation POCAppDelegate
- (void)applicationWillTerminate:(NSApplication *)app {
  app_deinit();
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app {
  return YES;
}
@end

CAMetalLayer * vlk_metal_layer() {
  return (CAMetalLayer *)[NSApplication sharedApplication].windows[0].contentView.layer;
}

FILE * app_open(const char * name, const char * ext) {
  NSString * n = [NSString stringWithFormat:@"%s", name];
  NSString * e = [NSString stringWithFormat:@"%s", ext];
  NSString * path = [[NSBundle mainBundle] pathForResource:n ofType:e];
  return fopen(path.UTF8String, "rb");
}

void vlk_fail(int r, const char * msg) {
  NSLog(@"Vulkan call failed (code=%d): %s\n", r, msg);
  exit(1);
}

static void run() {
  MTKView * v = [POCView new];
  v.delegate = [POCViewDelegate new];

  NSWindow * w = [NSWindow new];
  w.contentView = v;
  w.styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

  NSRect crect = NSMakeRect(0, 0, 800, 600);
  NSRect frect = [w frameRectForContentRect:crect];
  [w setFrame:frect display:YES];
  [w center];
  [w makeKeyAndOrderFront:w];

  // Apple menu
  NSMenu * menu = [NSMenu new];
  [menu       addItem:[[NSMenuItem alloc]
        initWithTitle:@"Quit Sokoban"
               action:@selector(terminate:)
        keyEquivalent:@"q"]];

  NSMenuItem * item = [NSMenuItem new];
  item.submenu = menu;

  NSMenu * bar = [NSMenu new];
  [bar addItem:item];

  NSApplication * a = [NSApplication sharedApplication];
  a.delegate = [POCAppDelegate new];
  a.mainMenu = bar;
  [a activateIgnoringOtherApps:YES];
  [a run];
}

int main() {
  @autoreleasepool {
    run();
  }
}
