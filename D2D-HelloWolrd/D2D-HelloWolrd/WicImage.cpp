#include "WicImage.h"

HRESULT GetWICFactory(IWICImagingFactory** factory)
{
	static CComPtr<IWICImagingFactory> spWicImagingFactory;
	HRESULT hr = S_OK;

	if (nullptr == spWicImagingFactory)
	{
		hr = CoCreateInstance(
			CLSID_WICImagingFactory, nullptr,
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&spWicImagingFactory));
	}

	if (!factory)
	{
		return E_POINTER;
	}

	*factory = spWicImagingFactory;
	(*factory)->AddRef();

	return hr;
}

HRESULT LoadBitmapFromFile(
	ID2D1RenderTarget *pRenderTarget,
	IWICImagingFactory *pIWICFactory,
	PCWSTR uri,
	UINT destinationWidth,
	UINT destinationHeight,
	ID2D1Bitmap **ppBitmap
)
{
	HRESULT hr = S_OK;

	CComPtr<IWICBitmapDecoder> pDecoder;
	CComPtr<IWICBitmapFrameDecode> pSource;
	CComPtr<IWICFormatConverter> pConverter;
	CComPtr<IWICBitmapScaler> pScaler;


	hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);
	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}

	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}

	if (destinationWidth != 0 || destinationHeight != 0)
	{
		UINT originalWidth, originalHeight;
		hr = pSource->GetSize(&originalWidth, &originalHeight);
		if (SUCCEEDED(hr))
		{
			if (destinationWidth == 0)
			{
				FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
				destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
			}
			else if (destinationHeight == 0)
			{
				FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
				destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
			}
		}
	}

	hr = pIWICFactory->CreateBitmapScaler(&pScaler);
	if (SUCCEEDED(hr))
	{
		hr = pScaler->Initialize(
			pSource,
			destinationWidth,
			destinationHeight,
			WICBitmapInterpolationModeCubic
		);
	}

	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(
			pScaler,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}

	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}

	return hr;
}