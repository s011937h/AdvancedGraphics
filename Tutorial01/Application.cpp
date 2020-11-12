#define _XM_NO_INTRINSICS_

#include "Application.h"

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int g_mousePositionX = 0;
int g_mousePositionY = 0;

int g_mouseDeltaX = 0;
int g_mouseDeltaY = 0;

Application::Application()
{
    hInst = nullptr;
    hWnd = nullptr;
    driverType = D3D_DRIVER_TYPE_NULL;
    featureLevel = D3D_FEATURE_LEVEL_11_0;
    pd3dDevice = nullptr;
    pd3dDevice1 = nullptr;
    pImmediateContext = nullptr;
    pImmediateContext1 = nullptr;
    pSwapChain = nullptr;
    pSwapChain1 = nullptr;
    pRenderTargetView = nullptr;
    pDepthStencil = nullptr;
    pDepthStencilView = nullptr;
    pVertexShader = nullptr;
    pParallaxVertexShader = nullptr;

    pPixelShader = nullptr;
    pPixelShaderSolid = nullptr;
    pParallaxPixelShader = nullptr;

    pVertexLayout = nullptr;
    pVertexBuffer = nullptr;
    pIndexBuffer = nullptr;

    pConstantBuffer = nullptr;
    pMaterialConstantBuffer = nullptr;
    pLightConstantBuffer = nullptr;

    pTextureRV = nullptr;
    pNormalTextureRV = nullptr;
    pParallaxTextureRV = nullptr;

    pSamplerLinear = nullptr;
    pSamplerNormal = nullptr;

    currentCamera = nullptr;
    eyePosition = XMFLOAT4(0.0f, 0.0f, -3.0f, 1.0f);
}

Application::~Application()
{
    CleanupDevice();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
    {
        return E_FAIL;
    }

    RECT rc;
    GetClientRect(hWnd, &rc);
    windowWidth = rc.right - rc.left;
    windowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        CleanupDevice();

        return E_FAIL;
    }

    _XValue = 0.0f;
    _YValue = 0.0f;
    _ZValue = -3.0f;
    currentCamera = new Camera(windowWidth, windowHeight, XMVectorSet(_XValue, _YValue, _ZValue, 1.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), false);
    eyePosition = currentCamera->GetPosition();
    //g_LightPosition = g_EyePosition;

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    hInst = hInstance;
    RECT rc = { 0, 0, 640, 480 };

    viewWidth = 640;
    viewHeight = 480;

    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    hWnd = CreateWindow(L"TutorialWindowClass", L"Direct3D 11 Tutorial 5",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
        nullptr);
    if (!hWnd)
        return E_FAIL;

    ShowWindow(hWnd, nCmdShow);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
            pErrorBlob->Release();
        }
        return hr;
    }
    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &pd3dDevice, &featureLevel, &pImmediateContext);

        if (hr == E_INVALIDARG)
        {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                D3D11_SDK_VERSION, &pd3dDevice, &featureLevel, &pImmediateContext);
        }

        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return hr;

    // Create swap chain
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
    if (dxgiFactory2)
    {
        // DirectX 11.1 or later
        hr = pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&pd3dDevice1));
        if (SUCCEEDED(hr))
        {
            (void)pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&pImmediateContext1));
        }

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;//  DXGI_FORMAT_R16G16B16A16_FLOAT;////DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd(pd3dDevice, hWnd, &sd, nullptr, nullptr, &pSwapChain1);
        if (SUCCEEDED(hr))
        {
            hr = pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&pSwapChain));
        }

        dxgiFactory2->Release();
    }
    else
    {
        // DirectX 11.0 systems
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain(pd3dDevice, &sd, &pSwapChain);
    }

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

    dxgiFactory->Release();

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
        return hr;

    hr = pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = pd3dDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);
    if (FAILED(hr))
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = pd3dDevice->CreateDepthStencilView(pDepthStencil, &descDSV, &pDepthStencilView);
    if (FAILED(hr))
        return hr;

    pImmediateContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    pImmediateContext->RSSetViewports(1, &vp);

    hr = InitMesh();
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to initialise mesh.", L"Error", MB_OK);
        return hr;
    }

    hr = InitWorld(width, height);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to initialise world.", L"Error", MB_OK);
        return hr;
    }

    hr = gameObject.initMesh(pd3dDevice, pImmediateContext);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ***************************************************************************************
// InitMesh
// ***************************************************************************************

HRESULT	Application::InitMesh()
{
    /*HRESULT hr = CompileAndCreateVertexShader(L"shader.fx", "VS", pVertexShader);
    hr = CompileAndCreatePixelShader(L"shader.fx", "PS", pPixelShader);
    hr = CompileAndCreatePixelShader(L"shader.fx", "PS", pPixelShaderSolid);
    hr = CompileAndCreateVertexShader(L"ParallaxShader.fx", "VS", pParallaxVertexShader);
    hr = CompileAndCreatePixelShader(L"ParallaxShader.fx", "PS", pParallaxPixelShader);*/

    // Compile the vertex shader
// Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(L"shader.fx", "VS", "vs_4_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVertexShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &pVertexLayout);
    pVSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Set the input layout
    pImmediateContext->IASetInputLayout(pVertexLayout);

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"shader.fx", "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pPixelShader);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;


    // Compile the SOLID pixel shader
    pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"shader.fx", "PSSolid", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the SOLID pixel shader
    hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pPixelShaderSolid);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;


    // Create the constant buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;        // 36 vertices needed for 12 triangles in a triangle list
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = pd3dDevice->CreateBuffer(&bd, nullptr, &pConstantBuffer);
    if (FAILED(hr))
        return hr;

    // Create the material constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(MaterialPropertiesConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = pd3dDevice->CreateBuffer(&bd, nullptr, &pMaterialConstantBuffer);
    if (FAILED(hr))
        return hr;

    // Create the light constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(LightPropertiesConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = pd3dDevice->CreateBuffer(&bd, nullptr, &pLightConstantBuffer);
    if (FAILED(hr))
        return hr;

    // load and setup textures
    hr = CreateDDSTextureFromFile(pd3dDevice, L"Resources\\stone.dds", nullptr, &pTextureRV);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(pd3dDevice, L"Resources\\conenormal.dds", nullptr, &pNormalTextureRV);
    if (FAILED(hr))
        return hr;

    /*hr = CreateDDSTextureFromFile(pd3dDevice, L"Resources\\normals.dds", nullptr, &pParallaxTextureRV);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(pd3dDevice, L"Resources\\color.dds", nullptr, &pParallaxColorRV);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(pd3dDevice, L"Resources\\displacement.dds", nullptr, &pParallaxDisplacementMapRV);
    if (FAILED(hr))
        return hr;*/

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = pd3dDevice->CreateSamplerState(&sampDesc, &pSamplerLinear);

    return hr;
}
/*
HRESULT Application::CompileAndCreateVertexShader(const WCHAR *shaderFilename, const LPCSTR shaderName, ID3D11VertexShader* vertexShader)
{
    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(shaderFilename, shaderName, "vs_4_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &vertexShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &pVertexLayout);
    pVSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Set the input layout
    pImmediateContext->IASetInputLayout(pVertexLayout);
}

HRESULT Application::CompileAndCreatePixelShader(const WCHAR *shaderFilename, const LPCSTR shaderName, ID3D11PixelShader* pixelShader)
{
    ID3DBlob* pPSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(shaderFilename, "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pixelShader);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;
}*/

// ***************************************************************************************
// InitWorld
// ***************************************************************************************
HRESULT Application::InitWorld(int width, int height)
{
    // Initialize the world matrix
    world1 = XMMatrixIdentity();

    // Initialize the projection matrix
    projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void Application::CleanupDevice()
{
    if (pImmediateContext) pImmediateContext->ClearState();

    if (pConstantBuffer) pConstantBuffer->Release();
    if (pVertexBuffer) pVertexBuffer->Release();
    if (pIndexBuffer) pIndexBuffer->Release();
    if (pVertexLayout) pVertexLayout->Release();
    if (pVertexShader) pVertexShader->Release();
    //if (pParallaxVertexShader) pParallaxVertexShader->Release();
    if (pPixelShader) pPixelShader->Release();
    //if (pParallaxPixelShader) pParallaxPixelShader->Release();
    if (pDepthStencil) pDepthStencil->Release();
    if (pDepthStencilView) pDepthStencilView->Release();
    if (pRenderTargetView) pRenderTargetView->Release();
    if (pSwapChain1) pSwapChain1->Release();
    if (pSwapChain) pSwapChain->Release();
    if (pImmediateContext1) pImmediateContext1->Release();
    if (pImmediateContext) pImmediateContext->Release();
    if (pd3dDevice1) pd3dDevice1->Release();
    if (pd3dDevice) pd3dDevice->Release();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_LBUTTONDOWN:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        break;
    }
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_MOUSEMOVE:
    {
        int currentXPos = GET_X_LPARAM(lParam);
        int currentYPos = GET_Y_LPARAM(lParam);

        int lastMouseXPos = g_mousePositionX;
        int lastMouseYPos = g_mousePositionY;

        g_mouseDeltaX = currentXPos - lastMouseXPos;
        g_mouseDeltaY = currentYPos - lastMouseYPos;

        g_mousePositionX = currentXPos;
        g_mousePositionY = currentYPos;
        break;
    }

    // Note that this tutorial does not handle resizing (WM_SIZE) requests,
    // so we created the window without the resize border.

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Application::Render()
{
    // Update our time
    static float t = 0.0f;
    if (driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float)XM_PI * 0.0125f;
    }
    else
    {
        static ULONGLONG timeStart = 0;
        ULONGLONG timeCur = GetTickCount64();
        if (timeStart == 0)
            timeStart = timeCur;
        t = (timeCur - timeStart) / 1000.0f;
    }

    // Clear the back buffer
    pImmediateContext->ClearRenderTargetView(pRenderTargetView, Colors::MidnightBlue);

    // Clear the depth buffer to 1.0 (max depth)
    pImmediateContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


    // Update variables for a cube
    gameObject.update(t);

    // Update variables for the cube
    XMMATRIX mGO = XMLoadFloat4x4(gameObject.getTransform());

    ConstantBuffer cb1;
    cb1.mWorld = XMMatrixTranspose(mGO);
    XMMATRIX viewMatrix = XMLoadFloat4x4(&currentCamera->GetView());
    cb1.mView = XMMatrixTranspose(viewMatrix);
    cb1.mProjection = XMMatrixTranspose(projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    pImmediateContext->UpdateSubresource(pConstantBuffer, 0, nullptr, &cb1, 0, 0);

    MaterialPropertiesConstantBuffer redPlasticMaterial;
    redPlasticMaterial.Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    redPlasticMaterial.Material.Specular = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
    redPlasticMaterial.Material.SpecularPower = 32.0f;
    redPlasticMaterial.Material.UseTexture = true;
    pImmediateContext->UpdateSubresource(pMaterialConstantBuffer, 0, nullptr, &redPlasticMaterial, 0, 0);

    Light light;
    light.Enabled = static_cast<int>(true);
    light.LightType = PointLight;
    light.Color = XMFLOAT4(Colors::White);
    light.SpotAngle = XMConvertToRadians(45.0f);
    light.ConstantAttenuation = 1.0f;
    light.LinearAttenuation = 1;
    light.QuadraticAttenuation = 1;


    // set up the light
    XMFLOAT4 LightPosition(eyePosition);
    light.Position = LightPosition;
    XMVECTOR LightDirection = XMVectorSet(-LightPosition.x, -LightPosition.y, -LightPosition.z, 0.0f);
    LightDirection = XMVector3Normalize(LightDirection);
    XMStoreFloat4(&light.Direction, LightDirection);

    LightPropertiesConstantBuffer lightProperties;
    lightProperties.EyePosition = LightPosition;
    lightProperties.Lights[0] = light;
    pImmediateContext->UpdateSubresource(pLightConstantBuffer, 0, nullptr, &lightProperties, 0, 0);

    // Render the cube - move all to drawable game object & then call drawable game object draw
    pImmediateContext->VSSetShader(pVertexShader, nullptr, 0); //if parallax set one shader, if not set the other
    //pImmediateContext->VSSetShader(pParallaxVertexShader, nullptr, 0); //if parallax set one shader, if not set the other
    pImmediateContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
    pImmediateContext->PSSetShader(pPixelShader, nullptr, 0);
    //pImmediateContext->PSSetShader(pParallaxPixelShader, nullptr, 0);

    pImmediateContext->PSSetConstantBuffers(1, 1, &pMaterialConstantBuffer); //two constant buffers needed possibly
    pImmediateContext->PSSetConstantBuffers(2, 1, &pLightConstantBuffer);

    //if statement for if it's parallax or not in update
    pImmediateContext->PSSetShaderResources(0, 1, &pTextureRV);
    //add brick color
    pImmediateContext->PSSetSamplers(0, 1, &pSamplerLinear);

    pImmediateContext->PSSetShaderResources(1, 1, &pNormalTextureRV);
    pImmediateContext->PSSetShaderResources(2, 1, &pParallaxTextureRV);
    //add displacement map

    pImmediateContext->DrawIndexed(36, 0, 0);

    // Present our back buffer to our front buffer
    pSwapChain->Present(0, 0);
}

void Application::Update()
{
    // Update our time
    static float t = 0.0f;

    if (driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float)XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

    bool isKeyDown = false;

    if (GetAsyncKeyState('W'))
    {
        uvf3 = XMFLOAT3(0.0f, 0.0001f, 0.0f);
        isKeyDown = true;
    }
    if (GetAsyncKeyState('S'))
    {
        uvf3 = XMFLOAT3(0.0f, -0.0001f, 0.0f);
        isKeyDown = true;
    }
    if (GetAsyncKeyState('D'))
    {
        uvf3 = XMFLOAT3(0.0001f, 0.0f, 0.0f);
        isKeyDown = true;
    }
    if (GetAsyncKeyState('A'))
    {
        uvf3 = XMFLOAT3(-0.0001f, 0.0f, 0.0f);
        isKeyDown = true;
    }
    if (isKeyDown)
    {
        XMVECTOR unitVector = XMLoadFloat3(&uvf3);

        cameraPos = XMFLOAT3(_XValue, _YValue, _ZValue);
        XMVECTOR cameraPosV = XMLoadFloat3(&cameraPos);

        XMVECTOR newCameraPosV;
        newCameraPosV = unitVector + cameraPosV;
        XMStoreFloat3(&newCameraPos, newCameraPosV);
        _XValue = newCameraPos.x;
        _YValue = newCameraPos.y;
        _ZValue = newCameraPos.z;

        XMStoreFloat4x4(&_cameraPosM, XMMatrixTranslation(_XValue, _YValue, _ZValue));
        XMMATRIX cameraPositionM = XMLoadFloat4x4(&_cameraPosM);

        cameraPosV = XMVector3TransformCoord(cameraPosV, cameraPositionM);
        XMStoreFloat3(&newCameraPos, cameraPosV);

        currentCamera->Update(XMVectorSet(newCameraPos.x, newCameraPos.y, newCameraPos.z, 1.0f), g_mouseDeltaX, g_mouseDeltaY);
        eyePosition = currentCamera->GetPosition();
    }
}



