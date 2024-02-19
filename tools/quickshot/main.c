#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>
#include <uxtheme.h>

static const char *className = "quickshotClass";
static const char *windowName = "quickshot";

static int screenWidth;
static int screenHeight;
static int startX;
static int startY;
static int endX;
static int endY;
static int lowerX;
static int lowerY;
static int width;
static int height;
static int selecting;

static HBITMAP ScreenBitmap;
static HDC BitmapMemory;
static HBITMAP DarkenScreenBitmap;
static HDC DarkenBitmapMemory;

static LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);

void CaptureScreenshot() {
	if ((startX - endX) * (startY - endY) == 0) {
		return;
	}

	HDC Screen = GetDC(NULL);
	HDC Mem = CreateCompatibleDC(Screen);
	HBITMAP Bitmap = CreateCompatibleBitmap(Screen, width, height);
	SelectObject(Mem, Bitmap);
	ReleaseDC(NULL, Screen);

	BitBlt(Mem, 0, 0, width, height, BitmapMemory, lowerX, lowerY, SRCCOPY);

	if (OpenClipboard(NULL)) {
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, Bitmap);
		CloseClipboard();
	}

	DeleteObject(Bitmap);
	DeleteDC(Mem);
}


int CALLBACK WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CmdLine,
	int ShowCmd
) {

	HWND Existing = FindWindow(className, NULL);
	if (Existing) {
		MessageBoxA(NULL, "You're already trying to take a screenshot.", "Error", MB_OK | MB_ICONWARNING);
		ExitProcess(0);
	}

	screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	HDC Screen = GetDC(NULL);
	BitmapMemory = CreateCompatibleDC(Screen);
	DarkenBitmapMemory = CreateCompatibleDC(Screen);
	ScreenBitmap = CreateCompatibleBitmap(Screen, screenWidth, screenHeight);
	DarkenScreenBitmap = CreateCompatibleBitmap(Screen, screenWidth, screenHeight);
	SelectObject(BitmapMemory, ScreenBitmap);
	SelectObject(DarkenBitmapMemory, DarkenScreenBitmap);

	BitBlt(BitmapMemory, 0, 0, screenWidth, screenHeight, Screen, 0, 0, SRCCOPY);
	BLENDFUNCTION Blend = {
		.BlendOp = AC_SRC_OVER,
		.SourceConstantAlpha = 0x40
	};
	AlphaBlend(DarkenBitmapMemory, 0, 0, screenWidth, screenHeight, BitmapMemory, 0, 0, screenWidth, screenHeight, Blend);

	ReleaseDC(NULL, Screen);

	WNDCLASSEXA quickshotClass = {
		.cbSize = sizeof(quickshotClass),
		.lpfnWndProc = WindowProc,
		.hInstance = Instance,
		.lpszClassName = className
	};
	RegisterClassExA(&quickshotClass);

#ifdef SLEEP
	Sleep(100);
#endif

	HWND Window = CreateWindowEx(
		0,
		className,
		windowName,
		WS_POPUP|WS_VISIBLE,
		0,
		0,
		screenWidth,
		screenHeight,
		NULL,
		NULL,
		Instance,
		NULL
	);

	if (!Window) {
		ExitProcess(0);
	}

	for (;;) {
		MSG Message;
		BOOL Result = GetMessageA(&Message, NULL, 0, 0);
		if (Result > 0) {
			TranslateMessage(&Message);
			DispatchMessageA(&Message);
		} else {
			break;
		}
	}

	DeleteObject(ScreenBitmap);
	DeleteObject(DarkenScreenBitmap);
	DeleteDC(BitmapMemory);
	DeleteDC(DarkenBitmapMemory);

	return 0;
}


static LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
	LRESULT Result = 0;
	switch (Message) {
	case WM_PAINT: {
#ifdef _DEBUG
		__builtin_printf("paint called\n");
#endif
		PAINTSTRUCT Paint;
#ifndef UNBUFFERED_PAINT
		HDC UnbufferedContext = BeginPaint(Window, &Paint);
		HDC Context;
		HPAINTBUFFER BufferedPaint = BeginBufferedPaint(UnbufferedContext, &Paint.rcPaint, BPBF_COMPATIBLEBITMAP, NULL, &Context);

		if (!BufferedPaint) goto giveup;
#else
		HDC Context = BeginPaint(Window, &Paint);
#endif
		BitBlt(Context, 0, 0, screenWidth, screenHeight, DarkenBitmapMemory, 0, 0, SRCCOPY);

		if (selecting) {
			if ((startX - endX) * (startY - endY) == 0) {
				goto end;
			}
#ifdef _DEBUG
			__builtin_printf("drawing non darkened w/ size %dx%d @ %dx%d\n", width, height, lowerX, lowerY);
#endif
			BitBlt(Context, lowerX, lowerY, width, height, BitmapMemory, lowerX, lowerY, SRCCOPY);
		} 
end:
#ifndef UNBUFFERED_PAINT
		EndBufferedPaint(BufferedPaint, TRUE);
giveup:
#endif
		EndPaint(Window, &Paint);
		break;
	}

	case WM_KEYDOWN: {
		if (WParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}
		break;
	}

	case WM_LBUTTONDOWN: {
		selecting = TRUE;
		startX = GET_X_LPARAM(LParam);
		startY = GET_Y_LPARAM(LParam);
		break;
	}

	case WM_LBUTTONUP: {
		endX = GET_X_LPARAM(LParam);
		endY = GET_Y_LPARAM(LParam);
		CaptureScreenshot();
		PostQuitMessage(0);
		break;
	}

	case WM_MOUSEMOVE: {
		if (!selecting)
			break;

		endX = GET_X_LPARAM(LParam);
		endY = GET_Y_LPARAM(LParam);

		if (startX > endX) {
			lowerX = endX;
			width = startX - endX;
		} else {
			lowerX = startX;
			width = endX - startX;
		}

		if (startY > endY) {
			lowerY = endY;
			height = startY - endY;
		} else {
			lowerY = startY;
			height = endY - startY;
		}

		InvalidateRect(Window, NULL, FALSE);
		break;
	}

	case WM_SETCURSOR: {
		HCURSOR Cursor = LoadCursorA(NULL, IDC_CROSS);
		SetCursor(Cursor);
		Result = TRUE;
		break;
	}

	default:
#ifdef _DEBUG
		__builtin_printf("Default Proc @ Message :%d\n", Message);
#endif
		Result = DefWindowProcA(Window, Message, WParam, LParam); 
	}

	return Result;
}
