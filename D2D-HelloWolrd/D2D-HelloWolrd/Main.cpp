#include <tchar.h>
#include <Windows.h>
#include <d2d1.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlcomcli.h>

HWND g_Hwnd = nullptr;

CComPtr<ID2D1Factory> spD2DFactory; // Direct2D factory
CComPtr<ID2D1HwndRenderTarget> g_spRenderTarget; // Render target
CComPtr<ID2D1SolidColorBrush> g_spYellowGreenBrush; // A black brush, reflect the line color

CComPtr<ID2D1PathGeometry> g_pathGeometry;

RECT rc; // Render area

void CreatePathGeometry()
{
	auto hr = spD2DFactory->CreatePathGeometry(&g_pathGeometry);

	if (SUCCEEDED(hr))
	{
		CComPtr<ID2D1GeometrySink> pSink;
		hr = g_pathGeometry->Open(&pSink);

		if (SUCCEEDED(hr))
		{
			pSink->BeginFigure(D2D1::Point2F(100, 100), D2D1_FIGURE_BEGIN_FILLED);

			D2D1_POINT_2F points[5] = {
				D2D1::Point2F(267, 177),
				D2D1::Point2F(236, 192),
				D2D1::Point2F(212, 160),
				D2D1::Point2F(156, 255),
				D2D1::Point2F(346, 255),
			};

			pSink->AddLines(points, ARRAYSIZE(points));
			pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		}


		pSink->Close();
	}
}

void CreateD2DResource(HWND hWnd)
{
	if (!g_spRenderTarget)
	{
		HRESULT hr;

		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &spD2DFactory);
		if (FAILED(hr))
		{
			MessageBox(hWnd, "Create D2D factory failed!", "Error", 0);
			return;
		}

		// Obtain the size of the drawing area
		GetClientRect(hWnd, &rc);

		// Create a Direct2D render target
		hr = spD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
				hWnd,
				D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
			),
			&g_spRenderTarget
		);
		if (FAILED(hr))
		{
			MessageBox(hWnd, "Create render target failed!", "Error", 0);
			return;
		}

		// Create a brush
		hr = g_spRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::YellowGreen),
			&g_spYellowGreenBrush
		);
		if (FAILED(hr))
		{
			MessageBox(hWnd, "Create brush failed!", "Error", 0);
			return;
		}

		CreatePathGeometry();
	}
}

void DrawRectangle()
{
	g_spRenderTarget->BeginDraw();
	g_spRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::BlueViolet));

// 	D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
// 		D2D1::RectF(100.f, 100.f, 500.f, 500.f),
// 		30.0f,
// 		50.0f
// 	);
// 
// 	g_spRenderTarget->DrawRoundedRectangle(
// 		roundedRect,
// 		g_spYellowGreenBrush
// 	);

	g_spRenderTarget->DrawGeometry(g_pathGeometry, g_spYellowGreenBrush, 1.0f);

	HRESULT hr = g_spRenderTarget->EndDraw();

	if (FAILED(hr))
	{
		MessageBox(NULL, "Draw failed!", "Error", 0);
		return;
	}
}

void Cleanup()
{

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		DrawRectangle();
		ValidateRect(g_Hwnd, NULL);
		return 0;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		default:
			break;
		}
	}
	break;

	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int nCmdShow)
{
	WNDCLASSEX winClass;

	winClass.lpszClassName = "Direct2D";
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc = WndProc;
	winClass.hInstance = hInstance;
	winClass.hIcon = NULL;
	winClass.hIconSm = NULL;
	winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = NULL;
	winClass.lpszMenuName = NULL;
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;

	if (!RegisterClassEx(&winClass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"), "error", MB_ICONERROR);
		return 0;
	}

	g_Hwnd = CreateWindowEx(NULL,
		"Direct2D", // window class name
		"Draw Rectangle", // window caption
		WS_OVERLAPPEDWINDOW, // window style
		CW_USEDEFAULT, // initial x position
		CW_USEDEFAULT, // initial y position
		1200, // initial x size
		800, // initial y size
		NULL, // parent window handle
		NULL, // window menu handle
		hInstance, // program instance handle
		NULL); // creation parameters

	CreateD2DResource(g_Hwnd);

	ShowWindow(g_Hwnd, nCmdShow);
	UpdateWindow(g_Hwnd);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}