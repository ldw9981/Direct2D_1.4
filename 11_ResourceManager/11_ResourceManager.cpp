#include <windows.h>
#include <assert.h>
#include <wrl.h>  // ComPtr 사용을 위한 헤더

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <d2d1_3.h> //ID2D1Factory8,ID2D1DeviceContext7
#pragma comment(lib, "d2d1.lib")

#include <dxgi1_6.h> // IDXGIFactory7
#pragma comment(lib, "dxgi.lib")

#include <wincodec.h>
#pragma comment(lib,"windowscodecs.lib")

#include <psapi.h>                // GetProcessMemoryInfo, PROCESS_MEMORY_COUNTERS_EX
#pragma comment(lib, "psapi.lib")


#include <iostream>
#include <conio.h> // _getch() 사용을 위한 헤더
#include <queue>


	

using namespace Microsoft::WRL;

// 전역 변수
HWND g_hwnd = nullptr;
ComPtr<ID3D11Device> g_d3dDevice;
ComPtr<IDXGISwapChain1> g_dxgiSwapChain;
ComPtr<ID2D1DeviceContext7> g_d2dDeviceContext;
ComPtr<ID2D1Bitmap1> g_d2dBitmapTarget;

// For ImageDraw
ComPtr<IWICImagingFactory> g_wicImagingFactory;


ComPtr<ID2D1Bitmap1> g_d2dBitmapFromFile;
ComPtr<IDXGIAdapter3> g_dxgiAdapter;

UINT g_width = 1024;
UINT g_height = 768;
bool g_resized = false;

void Initialize(HWND hwnd);
void Uninitialize();

// WIC를 통해 PNG 등을 로드하여 ID2D1Bitmap1**으로 반환
HRESULT CreateBitmapFromFile(const wchar_t* path, ID2D1Bitmap1** outBitmap)
{
	ComPtr<IWICBitmapDecoder>     decoder;
	ComPtr<IWICBitmapFrameDecode> frame;
	ComPtr<IWICFormatConverter>   converter;

	// ① 디코더 생성
	HRESULT hr = g_wicImagingFactory->CreateDecoderFromFilename(
		path, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	if (FAILED(hr)) 
		return hr;

	// ② 첫 프레임 얻기
	hr = decoder->GetFrame(0, &frame);
	if (FAILED(hr)) 
		return hr;

	// ③ 포맷 변환기 생성
	hr = g_wicImagingFactory->CreateFormatConverter(&converter);
	if (FAILED(hr)) 
		return hr;

	// ④ GUID_WICPixelFormat32bppPBGRA로 변환
	hr = converter->Initialize(
		frame.Get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.0f,
		WICBitmapPaletteTypeCustom
	);
	if (FAILED(hr)) 
		return hr;

	// ⑤ Direct2D 비트맵 속성 (premultiplied alpha, B8G8R8A8_UNORM)
	D2D1_BITMAP_PROPERTIES1 bmpProps = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_NONE,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	// ⑥ DeviceContext에서 WIC 비트맵으로부터 D2D1Bitmap1 생성
	hr = g_d2dDeviceContext->CreateBitmapFromWicBitmap(converter.Get(), &bmpProps, outBitmap);
	assert(SUCCEEDED(hr));

	return hr;
}

void PrintMemoryUsage() {
	DXGI_QUERY_VIDEO_MEMORY_INFO memInfo = {};
	g_dxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo);

	std::cout << "VRAM: " << (memInfo.CurrentUsage / 1024.0 / 1024.0) << " MB" << std::endl;


	HANDLE hProcess = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS_EX pmc;
	pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX);

	// 현재 프로세스의 메모리 사용 정보 조회
	GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));


	std::cout << "DRAM: " << (pmc.WorkingSetSize / 1024 / 1024) << " MB" << std::endl;

	std::cout << "PAGE: " << (pmc.PagefileUsage / 1024 / 1024) << " MB" << std::endl;
}


// 초기화
void Initialize(HWND hwnd)
{
	HRESULT hr;
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

	ComPtr<IDXGIDevice2> dxgiDevice;
	ComPtr<IDXGIAdapter> dxgiAdapter;
	g_d3dDevice.As(&dxgiDevice);
	dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
	dxgiAdapter.As(&g_dxgiAdapter);
	
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


	// Create WIC factory
	hr = CoCreateInstance(CLSID_WICImagingFactory,
		NULL, CLSCTX_INPROC_SERVER,
		__uuidof(g_wicImagingFactory),
		(void**)g_wicImagingFactory.GetAddressOf());

	hr = CreateBitmapFromFile(L"../Resource/mushroom.png", g_d2dBitmapFromFile.GetAddressOf());
	assert(SUCCEEDED(hr));
}

void Uninitialize()
{
	//CoUninitialize(); 호출전에 Release가 호출되어야 크래시가 나지 않음.
	g_wicImagingFactory = nullptr;
	g_d2dBitmapFromFile = nullptr;

	g_d3dDevice = nullptr;
	g_dxgiSwapChain = nullptr;
	g_d2dDeviceContext = nullptr;
	g_d2dBitmapTarget = nullptr;
	g_dxgiAdapter = nullptr;

}


int main()
{
	// COM 사용 시작
	CoInitialize(NULL);
	Initialize(GetConsoleWindow());
   
	std::queue<ComPtr<ID2D1Bitmap1>> bitmaps;

	// 메인 루프
	char input;
	while (input=_getch())
	{
		if (input == 'q' || input == 'Q') {
			break; // 'q' 입력 시 종료
		}
		if (input == 'i' || input == 'I') 
		{
			
			ComPtr<ID2D1Bitmap1> bitmap;
			CreateBitmapFromFile(L"../Resource/50mb.bmp", bitmap.GetAddressOf());
			if (bitmap) {
				bitmaps.push(bitmap);
				std::cout << "Image loaded. Total images: " << bitmaps.size() << std::endl;
			}
			else {
				std::cout << "Failed to load image." << std::endl;
			}
			PrintMemoryUsage();
		}
		if (input == 'r' || input == 'R')
		{

			if(!bitmaps.empty()) {
				
				bitmaps.pop();
				std::cout << "Image removed. Total images: " << bitmaps.size() << std::endl;
			}			
			else {
				std::cout << "No images to remove." << std::endl;
			}
		
			ComPtr<IDXGIDevice3> dxgiDevice;
			g_d3dDevice.As(&dxgiDevice);
			dxgiDevice->Trim();  // 드라이버에 메모리 해제 요청
			
			PrintMemoryUsage();
		}
	}

	Uninitialize();
	CoUninitialize();
}
