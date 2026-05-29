#include "app.h"
#include "vlk.h"

void app_init() {
  vlk_create();
}

void app_deinit() {
  vkDeviceWaitIdle(vlk_dev);

  vlk_destroy();
}

void app_frame() {
  vlk_frame();
}

void vlk_record(VkCommandBuffer cb) {
}
