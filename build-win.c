#define _CRT_SECURE_NO_WARNINGS
#include <sys/stat.h>
#include <assert.h>
#include <direct.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#define OPT "-gdwarf"
#define OPT "-O3"

static void usage() {
  fprintf(stderr, "just call 'build' without arguments\n");
}

static int run(char ** args) {
  assert(args && args[0]);

  if (0 == _spawnvp(_P_WAIT, args[0], (const char * const *)args)) {
    return 0;
  }

  fprintf(stderr, "failed to run child process: %s\n", args[0]);
  return 1;
}

static int shader(char * name) {
  char spv[1024]; snprintf(spv, 1024, "app/%s.spv", name);
  char src[1024]; snprintf(src, 1024, "../%s", name);

  char * args[] = { "glslang", "-V", src, "-o", spv, 0 };
  return run(args);
}

static int pch() {
  char * args[] = {
    "clang", "-Wall", OPT, "-x", "c-header",
    "-IVulkan-Headers/include",
    "-D", "VK_USE_PLATFORM_WIN32_KHR",
    "-D", "VLK_USE_VOLK",
    "-o", "pch.pch", "pch.h", 0 };
  return run(args);
}

static int cc_nopch(char * src, char * o) {
  char * args[] = {
    "clang", "-Wall", OPT, "-o", o, "-c", src, 0 };
  return run(args);
}

static int cc(char * src, char * o) {
  char * args[] = {
    "clang", "-Wall", OPT, "-include-pch", "pch.pch",
    "-o", o, "-c", src, 0 };
  return run(args);
}

static int hdr(char * src, char * o, char * d) {
  char * args[] = {
    "clang", "-Wall", "-x", "c", OPT, "-include-pch", "pch.pch",
    "-D", d, "-o", o, "-c", src, 0
  };
  return run(args);
}

static int link_hello() {
  char * args[] = {
    "clang", "-Wall", OPT,
    "microui.o", "hello.o", "hello-win.o", "volk.o",
    "-o", "app/hello.exe", "-luser32",
    0 };
  return run(args);
}

static int link_gdi() {
  char * args[] = {
    "clang", "-Wall", OPT,
    "microui.o", "gdi.o",
    "-o", "app/gdi.exe", "-lgdi32", "-luser32",
    0 };
  return run(args);
}

static int atlas() {
  int w, h, c;
  stbi_uc * img = stbi_load("monogram-bitmap.png", &w, &h, &c, 1);
  assert(w == 96);
  assert(h == 96);

  FILE * f = fopen("app/atlas.img", "wb");
  assert(f);
  assert(1 == fwrite(img, w * h, 1, f));
  fclose(f);

  return 0;
}

int main(int argc, char ** argv) {
  if (argc != 1) return (usage(), 1);

  _mkdir("app");

  if (pch()) return 1;
  if (atlas()) return 1;

  if (hdr("volk.h", "volk.o", "VOLK_IMPLEMENTATION")) return 1;

  if (cc_nopch("microui.c", "microui.o")) return 1;

  if (cc("hello.c", "hello.o")) return 1;
  if (cc("hello-win.c", "hello-win.o")) return 1;
  if (link_hello()) return 1;

  if (cc("gdi.c", "gdi.o")) return 1;
  if (link_gdi()) return 1;

  return 0;
}
