#include <windows.h>
#include <d3d11.h>
#include <d2d1_3.h> //ID2D1Factory8,ID2D1DeviceContext7
#include <dxgi1_6.h>  // IDXGIFactory7
#include <wrl.h>  // ComPtr 사용을 위한 헤더
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;

// 전역 변수
HWND g_hwnd = nullptr;
ComPtr<ID3D11Device> g_d3dDevice;
ComPtr<IDXGISwapChain1> g_dxgiSwapChain;
ComPtr<ID2D1DeviceContext7> g_d2dDeviceContext;
ComPtr<ID2D1Bitmap1> g_d2dBitmapTarget;

ComPtr<ID2D1SolidColorBrush> g_pBlackBrush;		// 렌더타겟이 생성하는 리소스 역시 장치의존
ComPtr<ID2D1SolidColorBrush> g_pGrayBrush;

UINT g_width = 800;
UINT g_height = 600;
bool g_resized = false;

void Initialize(HWND hwnd);
void Uninitialize();

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SIZE:
	{
		if (wParam == SIZE_MINIMIZED)
			break; // 최소화는 무시

		UINT width = LOWORD(lParam); // 새 너비
		UINT height = HIWORD(lParam); // 새 높이			
		if (g_width != width || g_height != height)
		{
			g_width = width;
			g_height = height;
			g_resized = true;
		}
	}
	break;
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// 초기화
void Initialize(HWND hwnd)
{
	// D3D11 디바이스 생성
	D3D_FEATURE_LEVEL featureLevel;
	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT, levels, 1,
		D3D11_SDK_VERSION, g_d3dDevice.GetAddressOf(), &featureLevel, nullptr);

	// D2D 팩토리 및 디바이스
	ComPtr<ID2D1Factory8> d2dFactory;
	D2D1_FACTORY_OPTIONS options = {};
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, d2dFactory.GetAddressOf());

	ComPtr<IDXGIDevice> dxgiDevice;
	g_d3dDevice.As(&dxgiDevice);
	ComPtr<ID2D1Device7> d2dDevice;
	d2dFactory->CreateDevice((dxgiDevice.Get()), d2dDevice.GetAddressOf());
	d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, g_d2dDeviceContext.GetAddressOf());

	ComPtr<IDXGIFactory7> dxgiFactory;
	CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	
	
	// SwapChain 생성
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = g_width;
	scDesc.Height = g_height;
	scDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scDesc.SampleDesc.Count = 1;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 2;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiFactory->CreateSwapChainForHwnd(g_d3dDevice.Get(), hwnd, &scDesc, nullptr, nullptr, g_dxgiSwapChain.GetAddressOf());

	// 백버퍼를 타겟으로 설정
	ComPtr<IDXGISurface> backBuffer;
	g_dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	D2D1_BITMAP_PROPERTIES1 bmpProps = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(scDesc.Format, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);
	g_d2dDeviceContext->CreateBitmapFromDxgiSurface(backBuffer.Get(), &bmpProps, g_d2dBitmapTarget.GetAddressOf());
	g_d2dDeviceContext->SetTarget(g_d2dBitmapTarget.Get());



	g_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), g_pBlackBrush.GetAddressOf());	

	g_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), g_pGrayBrush.GetAddressOf());
	
}

void Uninitialize()
{
	//CoUninitialize(); 호출전에 Release가 호출되어야 크래시가 나지 않음.
	g_pBlackBrush = nullptr;	// 렌더타겟이 생성하는 리소스 역시 장치의존
	g_pGrayBrush = nullptr;

	g_d3dDevice = nullptr;
	g_dxgiSwapChain = nullptr;
	g_d2dDeviceContext = nullptr;
	g_d2dBitmapTarget = nullptr;
}

void Render()
{
	g_d2dDeviceContext->BeginDraw();
	g_d2dDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::DarkSlateBlue));

	D2D1_SIZE_F size = g_d2dDeviceContext->GetSize();
	for (float y = 0; y < size.height; y += 10)
	{
		g_d2dDeviceContext->DrawLine(
			D2D1::Point2F(0.0f, y),
			D2D1::Point2F(size.width, y),
			g_pBlackBrush.Get(), 0.5f

		);
	}

	g_d2dDeviceContext->FillRectangle(
		D2D1::RectF(size.width / 2 - 150.0f, size.height / 2 - 150.0f,
			size.width / 2 + 150.0f, size.height / 2 + 150.0f), g_pGrayBrush.Get());

	g_d2dDeviceContext->DrawRectangle(
		D2D1::RectF(size.width / 2 - 50.0f, size.height / 2 - 50.0f,
			size.width / 2 + 50.0f, size.height / 2 + 50.0f), g_pBlackBrush.Get());



	g_d2dDeviceContext->EndDraw();
	g_dxgiSwapChain->Present(1, 0);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"MyD2DWindowClass";
	RegisterClass(&wc);

	SIZE clientSize = { (LONG)g_width,(LONG)g_height };
	RECT clientRect = { 0, 0, clientSize.cx, clientSize.cy };
	AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);

	g_hwnd = CreateWindowEx(0, L"MyD2DWindowClass", L"D2D1 Clear Example",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		clientRect.right - clientRect.left, clientRect.bottom - clientRect.top,
		nullptr, nullptr, hInstance, nullptr);
	ShowWindow(g_hwnd, nCmdShow);

	CoInitialize(nullptr);
	Initialize(g_hwnd);
	// 메시지 루프
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
	CoUninitialize();
	return 0;
}
