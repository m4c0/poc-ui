#include "microui.h"

static mu_Context ctx = {0};
static HDC hdc;

static int font_width(mu_Font f, const char * txt, int len) {
  RECT rect = {0};
  DrawText(hdc, txt, len, &rect, DT_CALCRECT);
  return rect.right - rect.left;
}
static int font_height(mu_Font f) {
  RECT rect = {0};
  DrawText(hdc, "Lorem Ipsum", -1, &rect, DT_CALCRECT);
  return rect.bottom - rect.top;
}

static void repaint(HWND hwnd) {
  hdc = GetDC(hwnd);
  SetBkMode(hdc, TRANSPARENT);

  mu_begin(&ctx);
  if (mu_begin_window(&ctx, "Window", mu_rect(10, 10, 300, 400))) {
    if (mu_button(&ctx, "Le button")) {
      puts("button");
    }
    mu_end_window(&ctx);
  }
  mu_end(&ctx);

  mu_Command * cmd = NULL;
  while (mu_next_command(&ctx, &cmd)) {
    switch (cmd->type) {
      case MU_COMMAND_TEXT: {
        SetTextColor(hdc, RGB(
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
        DrawText(hdc, cmd->text.str, -1, &rect, fmt);
        break;
      }
      case MU_COMMAND_CLIP: {
        HRGN hrgn = CreateRectRgn(
            cmd->clip.rect.x,
            cmd->clip.rect.y,
            cmd->clip.rect.x + cmd->clip.rect.w,
            cmd->clip.rect.y + cmd->clip.rect.h);
        SelectClipRgn(hdc, hrgn);
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
        FillRect(hdc, &rect, bru);
        DeleteObject(bru);
        break;
      }
      case MU_COMMAND_ICON:
        puts("icon");
        break;
    }
  }

  ReleaseDC(hwnd, hdc);
  hdc = NULL;
}

static LRESULT window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
  switch (msg) {
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
      repaint(hwnd);
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
