#include "app.h"
#include "vlk.h"
#include "microui.h"

static mu_Context ctx = {0};

static int font_width(mu_Font f, const char * txt, int len) {
  return 8 * len;
}
static int font_height(mu_Font f) {
  return 8;
}

void app_init() {
  vlk_create();

  mu_init(&ctx);
  ctx.text_width  = &font_width;
  ctx.text_height = &font_height;
}

void app_deinit() {
  vkDeviceWaitIdle(vlk_dev);

  vlk_destroy();
}

void app_frame() {
  vlk_frame();

  mu_begin(&ctx);
  if (mu_begin_window(&ctx, "Window", mu_rect(10, 10, 300, 400))) {
    if (mu_button(&ctx, "Le button")) {
      puts("button");
    }
    mu_end_window(&ctx);
  }
  mu_end(&ctx);

  puts(">>>>>>>>> frame");
  mu_Command * cmd = NULL;
  while (mu_next_command(&ctx, &cmd)) {
    switch (cmd->type) {
      case MU_COMMAND_TEXT:
        puts("text");
        break;
      case MU_COMMAND_CLIP:
        puts("clip");
        break;
      case MU_COMMAND_RECT:
        puts("rect");
        break;
      case MU_COMMAND_ICON:
        puts("icon");
        break;
    }

  }
}

void vlk_record(VkCommandBuffer cb) {
}
