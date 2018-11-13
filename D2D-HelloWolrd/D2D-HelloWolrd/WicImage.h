#pragma once

#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <wincodec.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlcomcli.h>
#include <wincodecsdk.h>

HRESULT LoadBitmapFromFile(
	ID2D1RenderTarget *pRenderTarget,
	IWICImagingFactory *pIWICFactory,
	PCWSTR uri,
	UINT destinationWidth,
	UINT destinationHeight,
	ID2D1Bitmap **ppBitmap
);

HRESULT GetWICFactory(IWICImagingFactory** factory);