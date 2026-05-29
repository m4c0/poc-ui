#ifndef APP_H
#define APP_H

void app_init();
void app_frame();
void app_deinit();

FILE * app_open(const char * name, const char * ext);

#endif

