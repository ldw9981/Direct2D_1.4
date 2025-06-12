#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <comdef.h>
#include <d2d1_1helper.h>
#include <string>
#include <dwrite.h>
#include <assert.h>

#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"dwrite.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"windowscodecs.lib")


#include "Transform.h"

using namespace D2D1;
using namespace std;

// 전역 변수:
HINSTANCE g_hInst;                                // 현재 인스턴스입니다.
HWND g_hWnd;


// 윌드행렬(복합변환)을 위한 행렬 
D2D_SIZE_U g_ScreenSize = { 1024,768 };

//              Direct2D                                        Unity 
// (0,0)   +----------------+  (1024,0)         (-512,384) +----------------+  (+512,+384)
//         |                |                              |                |
//         |                |                              |       +(0,0)   |
//         |                |                              |                |
// (0,768) +----------------+  (1024,768)       (-512,-384)+----------------+  (+512,-384)
Matrix3x2F g_matUnity = Matrix3x2F::Identity();

Matrix3x2F g_rootWorld = Matrix3x2F::Identity(); // 월드 좌표계 변환을 위한 행렬
Matrix3x2F g_childWorld = Matrix3x2F::Identity(); // 월드 좌표계 변환을 위한 행렬
Matrix3x2F g_matCameraInv = Matrix3x2F::Identity();
Matrix3x2F g_matCamera = Matrix3x2F::Identity(); // 카메라 좌표계 변환을 위한 행렬


//  D2D 개체 인터페이스 포인터 변수
ID2D1Factory* g_pD2DFactory;
ID2D1HwndRenderTarget* g_pRenderTarget;

// D2D1비트맵 생성을 위한 이미지 처리 인터페이스
IWICImagingFactory* g_pWICFactory;
ID2D1Bitmap* g_bitmapMan = nullptr;
ID2D1Bitmap* g_bitmapSign = nullptr;
ID2D1Bitmap* g_bitmapStone = nullptr;

// DWrite
IDWriteFactory* g_pDWriteFactory;
IDWriteTextFormat* g_pDWriteTextFormat;

ID2D1SolidColorBrush* g_pBrush; // 브러시 개체 인터페이스 포인터 변수

Transform g_signTransform;
Transform g_rootTransform;

Transform g_manTransform;
Transform g_stoneTransform;
Transform g_cameraTransform;

bool g_bRenderMatrix = true;
bool g_bUnityCoords = true;
bool g_bMirror = false;
bool g_bAttachCamraToStone = false;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);



HRESULT CreateD2DBitmapFromFile(const WCHAR* szFilePath, ID2D1Bitmap** ppID2D1Bitmap)
{
    HRESULT hr;
    // Create a decoder
    IWICBitmapDecoder* pDecoder = NULL;
    IWICFormatConverter* pConverter = NULL;

    hr = g_pWICFactory->CreateDecoderFromFilename(
        szFilePath,                      // Image to be decoded
        NULL,                            // Do not prefer a particular vendor
        GENERIC_READ,                    // Desired read access to the file
        WICDecodeMetadataCacheOnDemand,  // Cache metadata when needed
        &pDecoder                        // Pointer to the decoder
    );

    // Retrieve the first frame of the image from the decoder
    IWICBitmapFrameDecode* pFrame = NULL;
    if (SUCCEEDED(hr))
    {
        hr = pDecoder->GetFrame(0, &pFrame);
    }

    //Step 3: Format convert the frame to 32bppPBGRA
    if (SUCCEEDED(hr))
    {
        hr = g_pWICFactory->CreateFormatConverter(&pConverter);
    }

    if (SUCCEEDED(hr))
    {
        hr = pConverter->Initialize(
            pFrame,                          // Input bitmap to convert
            GUID_WICPixelFormat32bppPBGRA,   // Destination pixel format
            WICBitmapDitherTypeNone,         // Specified dither pattern
            NULL,                            // Specify a particular palette 
            0.f,                             // Alpha threshold
            WICBitmapPaletteTypeCustom       // Palette translation type
        );
    }

    if (SUCCEEDED(hr))
    {
        hr = g_pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, ppID2D1Bitmap);
    }


    // 파일을 사용할때마다 다시 만든다.
    if (pConverter)
        pConverter->Release();

    if (pDecoder)
        pDecoder->Release();

    if (pFrame)
        pFrame->Release();

    return hr;
}


void PrintText(const wchar_t* str, float left, float top)
{
    g_pRenderTarget->SetTransform(Matrix3x2F::Identity());
    g_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
    g_pRenderTarget->DrawTextW(str, (UINT32)wcslen(str), g_pDWriteTextFormat, RectF(left, top, left + 300, top + 250), g_pBrush);
}

void PrintMatrix(const wchar_t* str, D2D_MATRIX_3X2_F& mat, float left, float top)
{
    WCHAR buffer[256] = { 0 };
    swprintf_s(buffer, L"%.1f, %.1f\n%.1f, %.1f\n%.1f, %.1f\n",
        mat._11, mat._12, mat._21, mat._22, mat._31, mat._32);
    g_pRenderTarget->SetTransform(Matrix3x2F::Identity());
    g_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
    g_pRenderTarget->DrawTextW(str, (UINT32)wcslen(str), g_pDWriteTextFormat, RectF(left, top, left + 300, top + 300), g_pBrush);
    g_pRenderTarget->DrawTextW(buffer, (UINT32)wcslen(buffer), g_pDWriteTextFormat, RectF(left, top + 60, left + 300, top + 60 + 300), g_pBrush);
}

void PrintPoint(const wchar_t* str, D2D_POINT_2F point, float left, float top)
{
    WCHAR buffer[256] = { 0 };
    swprintf_s(buffer, L"%.1f, %.1f\n", point.x, point.y);
    g_pRenderTarget->SetTransform(Matrix3x2F::Identity());
    g_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
    g_pRenderTarget->DrawTextW(str, (UINT32)wcslen(str), g_pDWriteTextFormat, RectF(left, top, left + 300, top + 250), g_pBrush);
    g_pRenderTarget->DrawTextW(buffer, (UINT32)wcslen(buffer), g_pDWriteTextFormat, RectF(left, top + 25, left + 300, top + 25 + 300), g_pBrush);
}

void PrintFloat(const wchar_t* str, float value, float left, float top)
{
    WCHAR buffer[256] = { 0 };
    swprintf_s(buffer, L"%.1f\n", value);
    g_pRenderTarget->SetTransform(Matrix3x2F::Identity());
    g_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
    g_pRenderTarget->DrawTextW(str, (UINT32)wcslen(str), g_pDWriteTextFormat, RectF(left, top, left + 300, top + 250), g_pBrush);
    g_pRenderTarget->DrawTextW(buffer, (UINT32)wcslen(buffer), g_pDWriteTextFormat, RectF(left, top + 25, left + 300, top + 25 + 300), g_pBrush);
}

// WorldTransform 보다 더 앞에 최초 적용하여 렌더링 방법을 변경한다.
D2D1::Matrix3x2F MakeRenderMatrix(bool bMirror, bool bUnityCoords, float offsetX, float offsetY)
{
    if (!g_bRenderMatrix)
        return D2D1::Matrix3x2F::Identity();

    float scaleX = bMirror ? -1.0f : 1.0f;  // 이미지 좌우반전 이면 x축 반전 
    float scaleY = bUnityCoords ? -1.0f : 1.0f; // 유니티 좌표계면 y축 상하 반전
    offsetX = bMirror ? offsetX : -offsetX; //  좌우반전일때 이미지 이동 값 처리
    offsetY = bUnityCoords ? offsetY : -offsetY; //  유니티 좌표계일때 이미지 이동 값 처리
    return D2D1::Matrix3x2F::Scale(scaleX, scaleY) * D2D1::Matrix3x2F::Translation(offsetX, offsetY);
}

void InitDirect2D()
{
    WCHAR buffer[MAX_PATH];
    // Get the current working directory
    DWORD result = GetCurrentDirectory(MAX_PATH, buffer);
    OutputDebugString(buffer);

    HRESULT hr = S_OK;

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
    assert(SUCCEEDED(hr));
    // Create a Direct2D render target.
    hr = g_pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(g_hWnd, g_ScreenSize),
        &g_pRenderTarget);
    assert(SUCCEEDED(hr));

    // Create WIC factory
    hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pWICFactory));
    assert(SUCCEEDED(hr));
    hr = CreateD2DBitmapFromFile(L"../Resource/man.png", &g_bitmapMan);
    assert(SUCCEEDED(hr));
    hr = CreateD2DBitmapFromFile(L"../Resource/sign.png", &g_bitmapSign);
    assert(SUCCEEDED(hr));
    hr = CreateD2DBitmapFromFile(L"../Resource/Stone.png", &g_bitmapStone);
    assert(SUCCEEDED(hr));
    // DirectWrite 팩터리를 만듭니다.
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(g_pDWriteFactory), reinterpret_cast<IUnknown**>(&g_pDWriteFactory));
    assert(SUCCEEDED(hr));

    // DirectWrite 텍스트 형식 개체를 만듭니다.
    hr = g_pDWriteFactory->CreateTextFormat(
        L"", // FontName    제어판-모든제어판-항목-글꼴-클릭 으로 글꼴이름 확인가능
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        15.0f,   // Font Size
        L"", //locale
        &g_pDWriteTextFormat
    );
    assert(SUCCEEDED(hr));

    hr = g_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::GreenYellow), &g_pBrush);
    assert(SUCCEEDED(hr));
}

void UninitDirect2D()
{
    if (g_bitmapStone)   g_bitmapStone->Release();
    if (g_bitmapSign)   g_bitmapSign->Release();
    if (g_bitmapMan)   g_bitmapMan->Release();
    if (g_pWICFactory) g_pWICFactory->Release();
    if (g_pRenderTarget) g_pRenderTarget->Release();
    if (g_pD2DFactory) g_pD2DFactory->Release();


}

void ProcessKey()
{
    // Camera
    if (GetAsyncKeyState(VK_LEFT) & 0x8000)
        g_cameraTransform.SetTranslation( g_cameraTransform.Translation.x -1,
            g_cameraTransform.Translation.y);

    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
        g_cameraTransform.SetTranslation(g_cameraTransform.Translation.x + 1,
            g_cameraTransform.Translation.y);

    if (GetAsyncKeyState(VK_UP) & 0x8000)
        g_cameraTransform.SetTranslation(g_cameraTransform.Translation.x,
            g_cameraTransform.Translation.y+1);

    if (GetAsyncKeyState(VK_DOWN) & 0x8000)
        g_cameraTransform.SetTranslation(g_cameraTransform.Translation.x,
            g_cameraTransform.Translation.y - 1);


    //Stone
    if (GetAsyncKeyState('A') & 0x8000)        
        g_stoneTransform.SetTranslation(g_stoneTransform.Translation.x - 1,g_stoneTransform.Translation.y);    
    if (GetAsyncKeyState('D') & 0x8000)
        g_stoneTransform.SetTranslation(g_stoneTransform.Translation.x + 1,g_stoneTransform.Translation.y);
    if (GetAsyncKeyState('W') & 0x8000)
        g_stoneTransform.SetTranslation(g_stoneTransform.Translation.x,g_stoneTransform.Translation.y+1);
    if (GetAsyncKeyState('S') & 0x8000)
        g_stoneTransform.SetTranslation(g_stoneTransform.Translation.x,g_stoneTransform.Translation.y - 1);
    if (GetAsyncKeyState('Q') & 0x8000)
        g_stoneTransform.SetScale(g_stoneTransform.Scale.x - 0.01f, g_stoneTransform.Scale.y);
    if (GetAsyncKeyState('E') & 0x8000)
        g_stoneTransform.SetScale(g_stoneTransform.Scale.x + 0.01f, g_stoneTransform.Scale.y);
    if (GetAsyncKeyState('Z') & 0x8000)
        g_stoneTransform.SetRotation(g_stoneTransform.Rotation + 1);
    if (GetAsyncKeyState('C') & 0x8000)
        g_stoneTransform.SetRotation(g_stoneTransform.Rotation -1);

    //Man
    if (GetAsyncKeyState('G') & 0x8000)
    {
        g_bMirror = true;
        g_manTransform.SetTranslation(g_manTransform.Translation.x - 1, g_manTransform.Translation.y);
    }
    if (GetAsyncKeyState('J') & 0x8000)
    {
        g_bMirror = false;
        g_manTransform.SetTranslation(g_manTransform.Translation.x + 1, g_manTransform.Translation.y);
    }
    if (GetAsyncKeyState('Y') & 0x8000)
        g_manTransform.SetTranslation(g_manTransform.Translation.x, g_manTransform.Translation.y + 1);
    if (GetAsyncKeyState('H') & 0x8000)
        g_manTransform.SetTranslation(g_manTransform.Translation.x, g_manTransform.Translation.y - 1);
    if (GetAsyncKeyState('T') & 0x8000)
        g_manTransform.SetScale(g_manTransform.Scale.x - 0.01f, g_manTransform.Scale.y);
    if (GetAsyncKeyState('U') & 0x8000)
        g_manTransform.SetScale(g_manTransform.Scale.x + 0.01f, g_manTransform.Scale.y);
    if (GetAsyncKeyState('B') & 0x8000)
        g_manTransform.SetRotation(g_manTransform.Rotation + 1);
    if (GetAsyncKeyState('M') & 0x8000)
        g_manTransform.SetRotation(g_manTransform.Rotation - 1);

    if (GetAsyncKeyState('R') & 0x8000)
    {
        g_cameraTransform.Reset();
        g_rootTransform.Reset();
        g_stoneTransform.Reset();
        g_manTransform.Reset();
        g_bUnityCoords = false;
        g_bMirror = false;
    }
}



void Render()
{
    g_pRenderTarget->BeginDraw();
    g_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
    
    if (g_bUnityCoords)
        g_matUnity = D2D1::Matrix3x2F::Scale(1.0f, -1.0f) *
            D2D1::Matrix3x2F::Translation(g_ScreenSize.width / 2, g_ScreenSize.height / 2);
    else
        g_matUnity = D2D1::Matrix3x2F::Identity();

    Matrix3x2F matParentWorld;
    Matrix3x2F matRender = MakeRenderMatrix(false, g_bUnityCoords, 0, 0);
    // 카메라 좌표계로 변환하는 행렬
    Matrix3x2F matCameraWorld = g_cameraTransform.GetWorldMatrix();
    Matrix3x2F matCameraInv = matCameraWorld;
    PrintText(L"[F4]Stone에 붙이기", 50, 260);
    PrintPoint(L"Camera 이동 <^>", D2D1::Point2(matCameraWorld.dx, matCameraWorld.dy), 50, 280);


    matCameraInv.Invert();

    // 0,0을 표시하는 표지판 그리기            
    Matrix3x2F matSignFinal;
    if (!g_bUnityCoords)
        matSignFinal = matRender * g_signTransform.GetWorldMatrix() * matCameraInv;
    else
        matSignFinal = matRender * g_signTransform.GetWorldMatrix() * matCameraInv * g_matUnity;

    g_pRenderTarget->SetTransform(matSignFinal);
    g_pRenderTarget->DrawBitmap(g_bitmapSign);

    // stone 그리기
    matRender = MakeRenderMatrix(false, g_bUnityCoords, 0, 0);
    Matrix3x2F matStoneWorld = g_stoneTransform.GetWorldMatrix();
    Matrix3x2F matStoneFinal;
    if (!g_bUnityCoords)
        matStoneFinal = matRender * matStoneWorld * matCameraInv;
    else
        matStoneFinal = matRender * matStoneWorld * matCameraInv * g_matUnity;

    g_pRenderTarget->SetTransform(matStoneFinal);
    g_pRenderTarget->DrawBitmap(g_bitmapStone);

    // 스톤 World Matrix 디버깅
    D2D1::Matrix3x2F matS, matR, matT;
    matS = D2D1::Matrix3x2F::Scale(g_stoneTransform.Scale.x, g_stoneTransform.Scale.y);
    matR = D2D1::Matrix3x2F::Rotation(g_stoneTransform.Rotation);
    matT = D2D1::Matrix3x2F::Translation(g_stoneTransform.Translation.x, g_stoneTransform.Translation.y);

   

    PrintText(L"^", matStoneFinal.dx, matStoneFinal.dy);
    PrintText(L"Stone", 75, 340);
    PrintPoint(L"[Q/E]", D2D1::Point2(g_stoneTransform.Scale.x, g_stoneTransform.Scale.y), 0, 360);
    PrintMatrix(L"\nScl   *", matS, 0, 400);
    PrintFloat(L"[Z/C]", g_stoneTransform.Rotation, 60, 360);
    PrintMatrix(L"\nRot  *", matR, 60, 400);
    PrintPoint(L"[WASD]", D2D1::Point2(g_stoneTransform.Translation.x, g_stoneTransform.Translation.y), 120, 360);
    PrintMatrix(L"\nPos", matT, 120, 400);

    PrintMatrix(L"[F2]끄기\n렌더행렬 *", matRender, 210, 400);
    PrintMatrix(L"Local\n월드행렬 *", matStoneWorld, 300, 400);
    PrintMatrix(L"\n카메라 역행렬 *", matCameraInv, 400, 400);
    PrintMatrix(L"[F3]끄기\nUnity행렬", g_matUnity, 550, 400);
    PrintMatrix(L"\n= 최종행렬", matStoneFinal, 650, 400);


    // Man그리기  
    matRender = MakeRenderMatrix(g_bMirror, g_bUnityCoords, g_bitmapMan->GetSize().width / 3, g_bitmapMan->GetSize().height);
    g_manTransform.Parent = &g_stoneTransform; //Stone을 부모로 연결
    Matrix3x2F matManWorld = g_manTransform.GetWorldMatrix();
    Matrix3x2F matManFinal;
    if (!g_bUnityCoords)
        matManFinal = matRender * matManWorld * matCameraInv;
    else
        matManFinal = matRender * matManWorld * matCameraInv * g_matUnity;

    g_pRenderTarget->SetTransform(matManFinal);
    g_pRenderTarget->DrawBitmap(g_bitmapMan);

    // Man World Matrix 디버깅
    matS = D2D1::Matrix3x2F::Scale(g_manTransform.Scale.x, g_manTransform.Scale.y);
    matR = D2D1::Matrix3x2F::Rotation(g_manTransform.Rotation);
    matT = D2D1::Matrix3x2F::Translation(g_manTransform.Translation.x, g_manTransform.Translation.y);


    PrintText(L"부모가 Stone인 Man", 75, 540);
    PrintPoint(L"[T/U]", D2D1::Point2(g_manTransform.Scale.x, g_manTransform.Scale.y), 0, 560);
    PrintMatrix(L"\nScl *", matS, 0, 600);
    PrintFloat(L"[B/M]", g_manTransform.Rotation, 60, 560);
    PrintMatrix(L"\nRot *", matR, 60, 600);
    PrintPoint(L"[YGHJ]", D2D1::Point2(g_manTransform.Translation.x, g_manTransform.Translation.y), 120, 560);
    PrintMatrix(L"\nPos", matT, 120, 600);

    PrintMatrix(L"[F2]끄기 \n렌더행렬 *", matRender, 210, 600);
    PrintMatrix(L"Local*Parent\n월드행렬 *", matManWorld, 300, 600);

    PrintMatrix(L"\n카메라 역행렬 *", matCameraInv, 400, 600);
    PrintMatrix(L"[F3]끄기\nUnity행렬", g_matUnity, 550, 600);
    PrintMatrix(L"\n= 최종행렬", matManFinal, 650, 600);
    g_pRenderTarget->EndDraw();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 윈도우 클래스 구조체 초기화
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;	// 윈도우 프로시저 함수
    wc.hInstance = hInstance;   // 인스턴스 핸들
    // NULL을 사용하면 현재 실행 중인 프로그램의 인스턴스 핸들을 사용하게 됩니다.두 번째 인자는 로드할 커서의 ID입니다.
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"BasicWindowClass";

    // 윈도우 클래스 등록
    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"윈도우 클래스 등록 실패", L"에러", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 원하는 크기가 조정되어 리턴
    SIZE clientSize = { (LONG)g_ScreenSize.width, (LONG)g_ScreenSize.height };
    RECT clientRect = { 0, 0, clientSize.cx, clientSize.cy };
    AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);

    // 윈도우 생성
    g_hWnd = CreateWindowEx(
        0,
        L"BasicWindowClass",
        L"간단한 윈도우",
        WS_OVERLAPPEDWINDOW,	// OR연산으로 조합된 윈도우창 스타일
        0, 0,	// 시작위치
        clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, // 너비, 높이
        NULL, NULL, hInstance, NULL
    );


    WCHAR Title[100] = { 0 };
    wsprintfW(Title, L"변환 테스트 %d*%d", g_ScreenSize.width, g_ScreenSize.height);
    SetWindowTextW(g_hWnd, Title);

    if (!g_hWnd)
    {
        MessageBox(NULL, L"윈도우 생성 실패", L"에러", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 윈도우 표시
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    // COM 사용 시작
    CoInitialize(NULL);

    InitDirect2D();

    MSG msg;
    // 기본 메시지 루프입니다:
    while (TRUE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            //윈도우 메시지 처리 
            TranslateMessage(&msg); // 키입력관련 메시지 변환  WM_KEYDOWN -> WM_CHAR
            DispatchMessage(&msg);
        }
        else
        {
            ProcessKey();
            Render();
        }
    }


    UninitDirect2D();
    // COM 사용 끝
    CoUninitialize();
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
        if (wParam == VK_F2)
            g_bRenderMatrix = !g_bRenderMatrix;

        if (wParam == VK_F3)
        {
            g_bUnityCoords = !g_bUnityCoords;
        }

        if (wParam == VK_F4)
        {
            g_bAttachCamraToStone = !g_bAttachCamraToStone;
            if (g_bAttachCamraToStone)
                g_cameraTransform.Parent = & g_stoneTransform;
            else
                g_cameraTransform.Parent = nullptr;
        }


        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

