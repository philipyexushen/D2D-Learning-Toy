#include <tchar.h>
#include <Windows.h>
#include <d2d1.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlcomcli.h>
#include <vector>
#include <d2d1_1helper.h>
#include "WicImage.h"

using namespace std;

HWND g_Hwnd = nullptr;

CComPtr<ID2D1Factory> g_spD2DFactory; // Direct2D factory
CComPtr<ID2D1HwndRenderTarget> g_spRenderTarget; // Render target
CComPtr<ID2D1SolidColorBrush> g_spYellowGreenBrush; // A black brush, reflect the line color

CComPtr<ID2D1PathGeometry> g_pathGeometry;
CComPtr<ID2D1PathGeometry> g_sunGeometry;

vector<CComPtr<ID2D1EllipseGeometry>> vecSpEllipseGeometry1;
CComPtr<ID2D1GeometryGroup> g_pGeoGroup_AlternateFill;
CComPtr<IWICImagingFactory>	g_spWicImagingFactory;
CComPtr<ID2D1Bitmap>		g_spImage;

RECT rc; // Render area

void CreatePathGeometry()
{
	auto hr = g_spD2DFactory->CreatePathGeometry(&g_pathGeometry);

	if (SUCCEEDED(hr))
	{
		CComPtr<ID2D1GeometrySink> pSink;
		hr = g_pathGeometry->Open(&pSink);

		if (SUCCEEDED(hr))
		{
			pSink->BeginFigure(D2D1::Point2F(346, 255), D2D1_FIGURE_BEGIN_FILLED);

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

void CreateSunResource()
{
	auto hr = g_spD2DFactory->CreatePathGeometry(&g_sunGeometry);

	if (SUCCEEDED(hr))
	{
		CComPtr<ID2D1GeometrySink> pSink;
		hr = g_sunGeometry->Open(&pSink);

		pSink->BeginFigure(
			D2D1::Point2F(270, 255),
			D2D1_FIGURE_BEGIN_FILLED
		);

		// Ì«Ñô¶¥²¿Ô²»¡
		pSink->AddArc(
			D2D1::ArcSegment(
				D2D1::Point2F(440, 255), // end point
				D2D1::SizeF(85, 85),
				0.0f, // rotation angle
				D2D1_SWEEP_DIRECTION_CLOCKWISE,
				D2D1_ARC_SIZE_SMALL
			));
		pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

		// Ì«Ñô¹âÃ¢ÇúÏß
		pSink->BeginFigure(
			D2D1::Point2F(299, 182),
			D2D1_FIGURE_BEGIN_HOLLOW
		);
		pSink->AddBezier(
			D2D1::BezierSegment(
				D2D1::Point2F(299, 182),
				D2D1::Point2F(294, 176),
				D2D1::Point2F(285, 178)
			));
		pSink->AddBezier(
			D2D1::BezierSegment(
				D2D1::Point2F(276, 179),
				D2D1::Point2F(272, 173),
				D2D1::Point2F(272, 173)
			));

		pSink->EndFigure(D2D1_FIGURE_END_OPEN);

		pSink->Close();
	}
}

void CreateCircleGroup()
{
	vecSpEllipseGeometry1.resize(4);
	for (int i = 0; i < 4; i++)
	{
		const D2D1_ELLIPSE ellipse = D2D1::Ellipse(
			D2D1::Point2F(105.0f, 105.0f),
			25.0f + 5 * i,
			25.0f + 5 * i
		);

		auto hr = g_spD2DFactory->CreateEllipseGeometry(
			ellipse,
			&vecSpEllipseGeometry1.at(i)
		);
	}

	ID2D1Geometry *ppGeometries[] =
	{
		vecSpEllipseGeometry1.at(0),
		vecSpEllipseGeometry1.at(1),
		vecSpEllipseGeometry1.at(2),
		vecSpEllipseGeometry1.at(3),
	};
	auto hr = g_spD2DFactory->CreateGeometryGroup(
		D2D1_FILL_MODE_ALTERNATE,
		ppGeometries,
		ARRAYSIZE(ppGeometries),
		&g_pGeoGroup_AlternateFill
	);
}

void CreateD2DResource(HWND hWnd)
{
	CoInitialize(nullptr);
	if (!g_spRenderTarget)
	{
		HRESULT hr;

		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_spD2DFactory);
		if (FAILED(hr))
		{
			MessageBox(hWnd, "Create D2D factory failed!", "Error", 0);
			return;
		}

		// Obtain the size of the drawing area
		GetClientRect(hWnd, &rc);

		// Create a Direct2D render target
		hr = g_spD2DFactory->CreateHwndRenderTarget(
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

		D2D1_BRUSH_PROPERTIES brushProperties = { 1.0f, D2D1::Matrix3x2F::Rotation(0.8f) };

		// Create a brush
		hr = g_spRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::White),
			brushProperties,
			&g_spYellowGreenBrush
		);
		if (FAILED(hr))
		{
			MessageBox(hWnd, "Create brush failed!", "Error", 0);
			return;
		}

		GetWICFactory(&g_spWicImagingFactory);
		CreatePathGeometry();
		CreateSunResource();
		CreateCircleGroup();
		LoadBitmapFromFile(g_spRenderTarget, g_spWicImagingFactory,
			L"E:\\Users\\Administrator\\pictures\\Test\\user.jpg", 1200, 0, &g_spImage);
	}
}

void DrawBitmap()
{
	g_spRenderTarget->BeginDraw();

	// Clear background color to dark cyan
	g_spRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	D2D1_SIZE_F size = g_spImage->GetSize();
	D2D1_POINT_2F upperLeftCorner = D2D1::Point2F(0.f, 0.f);

	// Draw bitmap
	g_spRenderTarget->DrawBitmap(
		g_spImage,
		D2D1::RectF(
			upperLeftCorner.x,
			upperLeftCorner.y,
			upperLeftCorner.x + size.width,
			upperLeftCorner.y + size.height)
	);

	g_spRenderTarget->EndDraw();
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

// 	g_spRenderTarget->DrawGeometry(g_pathGeometry, g_spYellowGreenBrush, 1.0f);
// 	g_spRenderTarget->DrawGeometry(g_sunGeometry, g_spYellowGreenBrush, 1.0f);
// 	g_spRenderTarget->FillGeometry(g_pGeoGroup_AlternateFill, g_spYellowGreenBrush);
// 	g_spRenderTarget->DrawGeometry(g_pGeoGroup_AlternateFill, g_spYellowGreenBrush, 1.0f);

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
		DrawBitmap();
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
		"Draw Image", // window caption
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