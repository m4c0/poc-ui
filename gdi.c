#include "microui.h"

static mu_Context ctx = {0};
static HDC hdc_f, hdc_b;
static HBITMAP hbmp_b;

static void deinit_hdcs(HWND hwnd) {
  ReleaseDC(hwnd, hdc_f);
  DeleteDC(hdc_b);
  DeleteObject(hbmp_b);
}
static void init_hdcs(HWND hwnd) {
  deinit_hdcs(hwnd);

  hdc_f = GetDC(hwnd);
  hdc_b = CreateCompatibleDC(hdc_f);
  SetBkMode(hdc_b, TRANSPARENT);

  RECT rect;
  GetClientRect(hwnd, &rect);

  hbmp_b = CreateCompatibleBitmap(hdc_f, rect.right - rect.left, rect.bottom - rect.top);
  SelectObject(hdc_b, hbmp_b);
}

static int font_width(mu_Font f, const char * txt, int len) {
  RECT rect = {0};
  DrawText(hdc_f, txt, len, &rect, DT_CALCRECT);
  return rect.right - rect.left;
}
static int font_height(mu_Font f) {
  RECT rect = {0};
  DrawText(hdc_f, "Lorem Ipsum", -1, &rect, DT_CALCRECT);
  return rect.bottom - rect.top;
}

static void repaint(HWND hwnd) {
  RECT rect;
  GetClientRect(hwnd, &rect);

  HBRUSH bru = CreateSolidBrush(RGB(10, 12, 14));
  FillRect(hdc_b, &rect, bru);
  DeleteObject(bru);

  mu_begin(&ctx);
  if (mu_begin_window(&ctx, "Window", mu_rect(10, 10, 300, 400))) {
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
        SetTextColor(hdc_b, RGB(
              cmd->text.color.r,
              cmd->text.color.g,
              cmd->text.color.b));

        RECT rect = {
          .left  = cmd->text.pos.x,
          .top   = cmd->text.pos.y,
          .right  = cmd->text.pos.x + 500,
          .bottom = cmd->text.pos.y + 500,
        };
        UINT fmt = DT_TOP | DT_LEFT | DT_SINGLELINE;
        DrawText(hdc_b, cmd->text.str, -1, &rect, fmt);
        break;
      }
      case MU_COMMAND_CLIP: {
        HRGN hrgn = CreateRectRgn(
            cmd->clip.rect.x,
            cmd->clip.rect.y,
            cmd->clip.rect.x + cmd->clip.rect.w,
            cmd->clip.rect.y + cmd->clip.rect.h);
        SelectClipRgn(hdc_b, hrgn);
        DeleteObject(hrgn);
        break;
      }
      case MU_COMMAND_RECT: {
        RECT rect = {
          .left   = cmd->rect.rect.x,
          .top    = cmd->rect.rect.y,
          .right  = cmd->rect.rect.x + cmd->rect.rect.w,
          .bottom = cmd->rect.rect.y + cmd->rect.rect.h,
        };
        HBRUSH bru = CreateSolidBrush(RGB(
              cmd->rect.color.r,
              cmd->rect.color.g,
              cmd->rect.color.b));
        FillRect(hdc_b, &rect, bru);
        DeleteObject(bru);
        break;
      }
      case MU_COMMAND_ICON:
        puts("icon");
        break;
    }
  }

  BitBlt(hdc_f, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hdc_b, 0, 0, SRCCOPY);
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
    case WM_PAINT:
      repaint(hwnd);
      ValidateRect(hwnd, NULL);
      return 0;
    case WM_MOUSEMOVE:
      mu_input_mousemove(&ctx, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));
      InvalidateRect(hwnd, NULL, 0);
      return 0;
    case WM_LBUTTONDOWN:
      mu_input_mousedown(&ctx, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param), 1 << 0);
      InvalidateRect(hwnd, NULL, 0);
      return 0;
    case WM_LBUTTONUP:
      mu_input_mouseup(&ctx, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param), 1 << 0);
      InvalidateRect(hwnd, NULL, 0);
      return 0;
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

  mu_init(&ctx);
  ctx.text_width  = &font_width;
  ctx.text_height = &font_height;

  ShowWindow(hwnd, cmd_show);
  UpdateWindow(hwnd);

  MSG msg;
  while (GetMessage(&msg, 0, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.wParam;
}
