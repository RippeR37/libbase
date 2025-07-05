#ifndef UNICODE
#define UNICODE
#endif

#include <thread>

#include "base/message_loop/win/win_message_loop_attachment.h"
#include "base/platform/windows.h"
#include "base/threading/thread_pool.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void SetWindowTitle(HWND hWnd, const wchar_t* title) {
  SetWindowText(hWnd, title);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
  // Setup libbase attachment to main Window thread's message loop
  auto loopAttachment = base::win::WinMessageLoopAttachment::TryCreate();
  if (!loopAttachment) {
    return 0;
  }

  // Register the window class.
  const wchar_t CLASS_NAME[] = L"Window Class";
  WNDCLASS wc = {};
  wc.lpfnWndProc = &WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  RegisterClass(&wc);

  // Adjust window size to account for borders and title bar
  RECT rect = {0, 0, 800, 600};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

  HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Default title",
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                             rect.right - rect.left, rect.bottom - rect.top,
                             nullptr, nullptr, hInstance, nullptr);
  if (hwnd == nullptr) {
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);

  // Some example testing
  loopAttachment->TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&SetWindowTitle, hwnd,
                                L"Changed window title from post-task"));
  loopAttachment->TaskRunner()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&SetWindowTitle, hwnd, L"Delayed task executed as well!"),
      base::Seconds(3));

  base::ThreadPool threadPool{4};
  threadPool.Start();
  threadPool.GetTaskRunner()->PostTaskAndReplyWithResult(
      FROM_HERE, base::BindOnce([]() -> const wchar_t* {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return L"Greetings from thread pool!";
      }),
      base::BindOnce(&SetWindowTitle, hwnd));

  // Run the message loop.
  MSG msg = {};
  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd,
                            UINT uMsg,
                            WPARAM wParam,
                            LPARAM lParam) {
  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    case WM_CLOSE:
      DestroyWindow(hwnd);
      return 0;

    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
      EndPaint(hwnd, &ps);
      return 0;
    }
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
