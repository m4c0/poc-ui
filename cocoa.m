#import <AppKit/AppKit.h>
#import <CoreFoundation/CoreFoundation.h>

#include "microui.h"

static mu_Context ctx = {0};

@interface POCView : NSView
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
- (void) drawRect:(NSRect)dirtyRect {
  mu_begin(&ctx);
  int opt = MU_OPT_NOCLOSE;
  if (mu_begin_window_ex(&ctx, "Window", mu_rect(10, 10, 300, 400), opt)) {
    if (mu_button(&ctx, "Le button")) {
      mu_open_popup(&ctx, "popup");
    }
    if (mu_begin_popup(&ctx, "popup")) {
      mu_label(&ctx, "Le popup");
      mu_end_popup(&ctx);
    }

    mu_end_window(&ctx);
  }
  mu_end(&ctx);

  mu_Command * cmd = NULL;
  while (mu_next_command(&ctx, &cmd)) {
    switch (cmd->type) {
      case MU_COMMAND_TEXT: {
        break;
      }
      case MU_COMMAND_CLIP: {
        break;
      }
      case MU_COMMAND_RECT: {
        break;
      }
      case MU_COMMAND_ICON: break;
    }
  }
}
@end

@interface POCAppDelegate : NSObject<NSApplicationDelegate>
@end
@implementation POCAppDelegate
- (void)applicationWillTerminate:(NSApplication *)app {
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app {
  return YES;
}
@end

static void run() {
  NSView * v = [POCView new];

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
        initWithTitle:@"Quit Cocoa Test"
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

static int font_width(mu_Font f, const char * txt, int len) {
  return 0;
}
static int font_height(mu_Font f) {
  return 0;
}

int main() {
  mu_init(&ctx);
  ctx.text_width  = &font_width;
  ctx.text_height = &font_height;

  @autoreleasepool {
    run();
  }
}
