#include "app.h"

HWND vlk_hwnd;
void * vlk_gui_ptr;
void vlk_create_gui(unsigned w, unsigned h);
void vlk_copy_gui(unsigned w, unsigned h);

FILE * app_open(const char * name, const char * ext) {
  char exe[MAX_PATH];
  GetModuleFileName(NULL, exe, MAX_PATH);

  char * p = strrchr(exe, '\\');
  if (p) *p = 0;

  char buf[MAX_PATH]; snprintf(buf, MAX_PATH, "%s\\%s.%s", exe, name, ext);
  return fopen(buf, "rb");
}

FILE * vlk_open(const char * name, const char * ext) {
  char exe[MAX_PATH];
  GetModuleFileName(NULL, exe, MAX_PATH);

  char * p = strrchr(exe, '\\');
  if (p) *p = 0;

  char buf[MAX_PATH]; snprintf(buf, MAX_PATH, "%s\\%s.%s", exe, name, ext);
  return fopen(buf, "rb");
}

static char vlk_log_buf[1024];
void vlk_fail(int r, const char * msg) {
  snprintf(vlk_log_buf, 1024, "Vulkan call failed (code=%d): %s\n", r, msg);
  MessageBox(vlk_hwnd, vlk_log_buf, "Vulkan error", 0);

  // This is the only way to properly programatically exit an app from any
  // thread. Other attempts froze the app or kept it as a "background app".
  // i.e. We use WM_CLOSE instead of WM_QUIT (or PostQuitMessage etc) and we
  // need to use SendNotifyMessage instead of SendMessage.
  if (vlk_hwnd) SendNotifyMessage(vlk_hwnd, WM_CLOSE, 0, 0);
}

static HDC hdc;
static HBITMAP hbmp;
static void init_hdcs(HWND hwnd) {
  ReleaseDC(hwnd, hdc);
  DeleteDC(hdc);
  DeleteObject(hbmp);

  RECT rect; GetClientRect(hwnd, &rect);

  hdc = CreateCompatibleDC(NULL);
  hbmp = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
  SelectObject(hdc, hbmp);

  SetBkMode(hdc, TRANSPARENT);

  unsigned w = rect.right - rect.left;
  unsigned h = rect.bottom - rect.top;
  if (vlk_gui_ptr) vlk_create_gui(w, h);
}

static LRESULT window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
  switch (msg) {
    case WM_CREATE:
    case WM_SIZE:
      init_hdcs(hwnd);
      break;

    case WM_CLOSE:
      // Required to enable another thread sending "plz exit" messages
      DestroyWindow(hwnd);
      return 0;
    case WM_DESTROY:
      app_deinit();
      PostQuitMessage(0);
      return 0;
    case WM_ERASEBKGND:
      // i.e. "never erase background". Solves 99.999% of flicker issues
      return 1;
    case WM_KEYDOWN:
      if (HIWORD(l_param) & KF_REPEAT) return 0;

      switch (LOWORD(w_param)) {
        //case VK_LEFT:   skb_api->move(-1,  0); break;
        //case VK_RIGHT:  skb_api->move( 1,  0); break;
        //case VK_UP:     skb_api->move( 0, -1); break;
        //case VK_DOWN:   skb_api->move( 0,  1); break;
        //case VK_SPACE:  skb_api->space();      break;
        //case VK_ESCAPE: skb_api->escape();     break;
      }

      return 0;
    case WM_PAINT: {
      if (!vlk_hwnd) return 0;
      
      RECT rect; GetClientRect(hwnd, &rect);

      HBRUSH bru = CreateSolidBrush(RGB(200, 12, 14));
      RECT r = rect;
      r.top    += 100;
      r.left   += 100;
      r.bottom -= 100;
      r.right  -= 100;
      FillRect(hdc, &r, bru);
      DeleteObject(bru);

      unsigned w = rect.right - rect.left;
      unsigned h = rect.bottom - rect.top;
      BITMAPINFO bi = {
        .bmiHeader = {
          .biSize        = sizeof(BITMAPINFOHEADER),
          .biWidth       = w,
          .biHeight      = h,
          .biPlanes      = 1,
          .biBitCount    = 24,
          .biCompression = BI_RGB,
        },
      };
      GetDIBits(hdc, hbmp, 0, h, vlk_gui_ptr, &bi, DIB_RGB_COLORS);

      if (!vlk_gui_ptr) vlk_create_gui(w, h);

      vlk_copy_gui(w, h);
      app_frame();
      return 0;
    }
  }
  return DefWindowProc(hwnd, msg, w_param, l_param);
}

int WinMain(HINSTANCE h_instance, HINSTANCE h_prev, LPSTR cmd_line, int cmd_show) {
  HICON h_icon = LoadIcon(h_instance, "IDI_ICON");

  WNDCLASSEX wcex  = {
    .cbSize        = sizeof(WNDCLASSEX),
    .style         = CS_HREDRAW | CS_VREDRAW,
    .lpfnWndProc   = &window_proc,
    .hInstance     = h_instance,
    .hIcon         = h_icon,
    .hCursor       = LoadCursor(NULL, IDC_ARROW),
    .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
    .lpszClassName = "m4c0-window",
    .hIconSm       = h_icon,
  };
  if (!RegisterClassEx(&wcex)) {
    MessageBox(NULL, "Failed to register window class", "Unhandled error", 0);
    return 1;
  }

  DWORD style = WS_OVERLAPPEDWINDOW ^ WS_SIZEBOX ^ WS_MAXIMIZEBOX;

  HWND hwnd = CreateWindow(
      "m4c0-window",
      "Hello from poc-ui",
      style, CW_USEDEFAULT, CW_USEDEFAULT,
      800, 600, 
      NULL, NULL, h_instance, NULL);
  if (!hwnd) {
    MessageBox(NULL, "Failed to create window", "Unhandled error", 0);
    return 1;
  }

  ShowWindow(hwnd, cmd_show);
  UpdateWindow(hwnd);

  vlk_hwnd = hwnd;
  app_init();

  MSG msg;
  while (GetMessage(&msg, 0, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.wParam;
}
