#include <windows.h>
#include <wrl.h>
#include <assert.h>
#include <d3d11.h>
#include <d2d1_3.h>
#include <dxgi1_6.h>
#include <wincodec.h>
#include <comdef.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowscodecs.lib")

using namespace Microsoft::WRL;

// 전역 변수
HWND                        g_hwnd = nullptr;
UINT                        g_width = 1024;
UINT                        g_height = 768;
bool                        g_resizePending = false;

// D3D/D2D 관련
ComPtr<ID3D11Device>        g_d3dDevice;
ComPtr<IDXGISwapChain1>     g_swapChain;
ComPtr<ID2D1DeviceContext7> g_d2dContext;
ComPtr<ID2D1Bitmap1>        g_d2dTarget;

// 이미지 로드용
ComPtr<IWICImagingFactory>  g_wicFactory;
ComPtr<ID2D1Bitmap1>        g_bitmap;

// SpriteBatch
ComPtr<ID2D1SpriteBatch>    g_spriteBatch;

// 에러 출력 함수
void OutputError(HRESULT hr) {
	_com_error err(hr);
	OutputDebugString(err.ErrorMessage());
}

// WIC를 통해 PNG 등을 로드하여 ID2D1Bitmap1**으로 반환
HRESULT CreateBitmapFromFile(const wchar_t* path, ID2D1Bitmap1** outBitmap) {
	ComPtr<IWICBitmapDecoder>     decoder;
	ComPtr<IWICBitmapFrameDecode> frame;
	ComPtr<IWICFormatConverter>   converter;

	// ① 디코더 생성
	HRESULT hr = g_wicFactory->CreateDecoderFromFilename(
		path, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	if (FAILED(hr)) return hr;

	// ② 첫 프레임 얻기
	hr = decoder->GetFrame(0, &frame);
	if (FAILED(hr)) return hr;

	// ③ 포맷 변환기 생성
	hr = g_wicFactory->CreateFormatConverter(&converter);
	if (FAILED(hr)) return hr;

	// ④ GUID_WICPixelFormat32bppPBGRA로 변환
	hr = converter->Initialize(
		frame.Get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.0f,
		WICBitmapPaletteTypeCustom
	);
	if (FAILED(hr)) return hr;

	// ⑤ Direct2D 비트맵 속성 (premultiplied alpha, B8G8R8A8_UNORM)
	D2D1_BITMAP_PROPERTIES1 bmpProps = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_NONE,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	// ⑥ DeviceContext에서 WIC 비트맵으로부터 D2D1Bitmap1 생성
	hr = g_d2dContext->CreateBitmapFromWicBitmap(converter.Get(), &bmpProps, outBitmap);
	return hr;
}

// SwapChain과 D2D 타겟 비트맵 생성 (또는 재생성)
void CreateSwapChainAndD2DTarget(HWND hwnd) {
	// 1. 현재 타겟 해제
	if (g_d2dContext) {
		g_d2dContext->SetTarget(nullptr);
	}
	g_d2dTarget.Reset();

	// 2. SwapChain 재설정
	if (g_swapChain) {
		// ResizeBuffers: 넓이/높이가 0이면 현재 설정 유지, 여기선 실제 크기로 넘깁니다.
		g_swapChain->ResizeBuffers(
			0,
			g_width,
			g_height,
			DXGI_FORMAT_UNKNOWN,
			0
		);
	}

	// 3. 백버퍼 IDXGISurface 얻기
	ComPtr<IDXGISurface> backBuffer;
	HRESULT hr = g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	if (FAILED(hr)) {
		OutputError(hr);
		return;
	}

	// 4. D2D1BitmapProperties1 생성 
	D2D1_BITMAP_PROPERTIES1 bmpProps = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	// 5. DeviceContext에 바인딩할 ID2D1Bitmap1 생성
	hr = g_d2dContext->CreateBitmapFromDxgiSurface(backBuffer.Get(), &bmpProps, &g_d2dTarget);
	if (FAILED(hr)) {
		OutputError(hr);
		return;
	}

	// 6. 컨텍스트에 SetTarget
	g_d2dContext->SetTarget(g_d2dTarget.Get());
}

// 윈도우 프로시저: Resize 처리
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_SIZE: {
		if (wParam == SIZE_MINIMIZED) {
			break;
		}
		g_width = LOWORD(lParam);
		g_height = HIWORD(lParam);
		g_resizePending = true;  // 메시지 루프에서 처리하도록 플래그 설정
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

// 초기화 (D3D11 디바이스, SwapChain, D2D 디바이스/컨텍스트, WIC, Bitmap, SpriteBatch)
void Init(HWND hwnd) {
	// 1. D3D11 디바이스 생성 (BGRA_SUPPORT 필수)
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL obtainedLevel;
	HRESULT hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		featureLevels,
		_countof(featureLevels),
		D3D11_SDK_VERSION,
		g_d3dDevice.GetAddressOf(),
		&obtainedLevel,
		nullptr
	);
	assert(SUCCEEDED(hr));

	// 2. Direct2D 팩토리 및 디바이스, 컨텍스트 생성
	ComPtr<ID2D1Factory8> d2dFactory;
	D2D1_FACTORY_OPTIONS options = { D2D1_DEBUG_LEVEL_INFORMATION };
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, d2dFactory.GetAddressOf());

	ComPtr<IDXGIDevice> dxgiDevice;
	g_d3dDevice.As(&dxgiDevice);

	ComPtr<ID2D1Device7> d2dDevice;
	hr = d2dFactory->CreateDevice(dxgiDevice.Get(), d2dDevice.GetAddressOf());
	assert(SUCCEEDED(hr));

	hr = d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, g_d2dContext.GetAddressOf());
	assert(SUCCEEDED(hr));

	// 3. DXGI Factory 를 이용해 SwapChain 생성
	ComPtr<IDXGIFactory7> dxgiFactory;
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = g_width;
	scDesc.Height = g_height;
	scDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scDesc.SampleDesc.Count = 1;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 2;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	hr = dxgiFactory->CreateSwapChainForHwnd(
		g_d3dDevice.Get(),
		hwnd,
		&scDesc,
		nullptr,
		nullptr,
		g_swapChain.GetAddressOf()
	);
	assert(SUCCEEDED(hr));

	// 4. D2D 타겟 비트맵 및 SetTarget
	CreateSwapChainAndD2DTarget(hwnd);

	// 5. WIC Imaging Factory 생성
	hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&g_wicFactory));
	assert(SUCCEEDED(hr));

	// 6. Resource/mushroom.png 로드
	hr = CreateBitmapFromFile(L"../Resource/mushroom.png", &g_bitmap);
	if (FAILED(hr)) {
		OutputError(hr);
	}

	// 7. SpriteBatch 생성
	hr = g_d2dContext->CreateSpriteBatch(g_spriteBatch.GetAddressOf());
	assert(SUCCEEDED(hr));
}

void Uninitialize()
{
	//CoUninitialize(); 호출전에 Release가 호출되어야 크래시가 나지 않음.
	g_d3dDevice = nullptr;
	g_swapChain = nullptr;
	g_d2dContext = nullptr;
	g_d2dTarget = nullptr;
	g_wicFactory = nullptr;
	g_bitmap = nullptr;
	g_spriteBatch = nullptr;
}

// 렌더 루프: BeginDraw → DrawSpriteBatch → EndDraw → Present
void Render() {
	// Resize 플래그가 세트되었으면 SwapChain과 타겟 비트맵 재생성
	if (g_resizePending) {
		CreateSwapChainAndD2DTarget(g_hwnd);
		g_resizePending = false;
	}

	// ① 매 프레임마다 SetTarget 호출
	g_d2dContext->SetTarget(g_d2dTarget.Get());

	g_d2dContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED); //DrawSpriteBatch requires

	// ② BeginDraw
	g_d2dContext->BeginDraw();
	g_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::DarkSlateBlue));

	// ③ SpriteBatch로 이미지 세 번 나란히 그리기
	D2D1_SIZE_U bmpSize = g_bitmap->GetPixelSize();
	D2D1_RECT_F destRects[3] = {
		D2D1::RectF(50,  50,  50 + (FLOAT)bmpSize.width,  50 + (FLOAT)bmpSize.height),
		D2D1::RectF(200, 100, 200 + (FLOAT)bmpSize.width, 100 + (FLOAT)bmpSize.height),
		D2D1::RectF(400, 200, 400 + (FLOAT)bmpSize.width, 200 + (FLOAT)bmpSize.height)
	};
	//D2D1_RECT_U srcRect = { 0, 0, bmpSize.width, bmpSize.height };
	D2D1_RECT_U srcRects[3] = {
	{ 0, 0, bmpSize.width, bmpSize.height },
	{ 0, 0, bmpSize.width, bmpSize.height },
	{ 0, 0, bmpSize.width, bmpSize.height }
	};

	g_spriteBatch->Clear();
	HRESULT hr = g_spriteBatch->AddSprites(3, destRects, srcRects, nullptr, nullptr);
	assert(SUCCEEDED(hr));

	g_d2dContext->DrawSpriteBatch(
		g_spriteBatch.Get(),
		0,
		3,
		g_bitmap.Get(),
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		D2D1_SPRITE_OPTIONS_NONE
	);

	// ④ EndDraw & 에러 체크
	hr = g_d2dContext->EndDraw();
	if (FAILED(hr)) {
		OutputError(hr);
	}

	// ⑤ Present
	g_swapChain->Present(1, 0);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
	// 1. 윈도우 클래스 등록
	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = L"D2D1SpriteBatchSample";
	RegisterClass(&wc);

	// 2. 창 생성
	RECT rc = { 0, 0, (LONG)g_width, (LONG)g_height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hwnd = CreateWindow(
		wc.lpszClassName,
		L"Direct2D 1.3 SpriteBatch 예제",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top,
		nullptr, nullptr, hInst, nullptr
	);
	ShowWindow(g_hwnd, nCmdShow);

	// 3. COM 및 D3D/D2D 초기화
	CoInitialize(nullptr);
	Init(g_hwnd);

	// 4. 메시지 루프 + 렌더 콜
	MSG msg = {};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			Render();
		}
	}

	Uninitialize();

	// 5. 정리
	CoUninitialize();
	return 0;
}
