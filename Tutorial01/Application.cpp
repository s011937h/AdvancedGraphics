#define _XM_NO_INTRINSICS_

#include "Application.h"

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int g_mousePositionX = 0;
int g_mousePositionY = 0;

int g_mouseDeltaX = 0;
int g_mouseDeltaY = 0;

Application * g_Application = nullptr;

Application::Application()
{
    hInst = nullptr;
    hWnd = nullptr;
    driverType = D3D_DRIVER_TYPE_NULL;
    featureLevel = D3D_FEATURE_LEVEL_11_0;
    m_pd3dDevice = nullptr;
    m_pd3dDevice1 = nullptr;
    m_ImmediateContext = nullptr;
    m_ImmediateContext1 = nullptr;
    m_SwapChain = nullptr;
    m_SwapChain1 = nullptr;
    m_RenderTargetView = nullptr;
    m_DepthStencil = nullptr;
    m_DepthStencilView = nullptr;
   
    m_ConstantBuffer = nullptr;
    m_MaterialConstantBuffer = nullptr;
    m_ParallaxMaterialConstantBuffer = nullptr;
    m_LightConstantBuffer = nullptr;
    m_PostProcessBuffer = nullptr;

    m_SamplerLinear = nullptr;
    m_SamplerNormal = nullptr;

    currentCamera = nullptr;
    eyePosition = XMFLOAT4(0.0f, 0.0f, -3.0f, 1.0f);

    g_Application = this;
}

Application::~Application()
{
    CleanupDevice();

    if (g_Application == this)
    {
        g_Application = nullptr;
    }
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
    currentCamera = std::make_unique<Camera>(windowWidth, windowHeight, XMVectorSet(_XValue, _YValue, _ZValue, 1.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), false);
    eyePosition = currentCamera->GetPosition();

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
    hWnd = CreateWindow(L"TutorialWindowClass", L"Direct3D 11",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
        nullptr);
    if (!hWnd)
        return E_FAIL;

    ShowWindow(hWnd, nCmdShow);

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
            D3D11_SDK_VERSION, m_pd3dDevice.ReleaseAndGetAddressOf(), &featureLevel, m_ImmediateContext.ReleaseAndGetAddressOf());

        if (hr == E_INVALIDARG)
        {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                D3D11_SDK_VERSION, m_pd3dDevice.ReleaseAndGetAddressOf(), &featureLevel, m_ImmediateContext.ReleaseAndGetAddressOf());
        }

        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    m_d3dDebug = m_pd3dDevice.QueryInterfaceCast<ID3D11Debug>();

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = m_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
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
        hr = m_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&m_pd3dDevice1));
        if (SUCCEEDED(hr))
        {
            (void)m_ImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&m_ImmediateContext1));
        }

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;//  DXGI_FORMAT_R16G16B16A16_FLOAT;////DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_SwapChain1.ReleaseAndGetAddressOf());
        if (SUCCEEDED(hr))
        {
            hr = m_SwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&m_SwapChain));
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

        hr = dxgiFactory->CreateSwapChain(m_pd3dDevice.Get(), &sd, m_SwapChain.ReleaseAndGetAddressOf());
    }

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

    dxgiFactory->Release();

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
        return hr;

    hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, m_RenderTargetView.ReleaseAndGetAddressOf());
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
    hr = m_pd3dDevice->CreateTexture2D(&descDepth, nullptr, m_DepthStencil.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;

    // Create the depth stencil view - 
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = m_pd3dDevice->CreateDepthStencilView(m_DepthStencil.Get(), &descDSV, m_DepthStencilView.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;

    //create light accumulation
    D3D11_TEXTURE2D_DESC lightAccumulationDesc = {};
    lightAccumulationDesc.Width = width;
    lightAccumulationDesc.Height = height;
    lightAccumulationDesc.MipLevels = 1;
    lightAccumulationDesc.ArraySize = 1;
    lightAccumulationDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    lightAccumulationDesc.SampleDesc.Count = 1;
    lightAccumulationDesc.SampleDesc.Quality = 0;
    lightAccumulationDesc.Usage = D3D11_USAGE_DEFAULT;
    lightAccumulationDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    lightAccumulationDesc.CPUAccessFlags = 0;
    lightAccumulationDesc.MiscFlags = 0;
    hr = m_pd3dDevice->CreateTexture2D(&lightAccumulationDesc, nullptr, m_LightAccumulation.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = m_pd3dDevice->CreateRenderTargetView(m_LightAccumulation.Get(), nullptr, m_LightAccumulationRTV.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC lightAccumulationSRVDesc = {};
    lightAccumulationSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    lightAccumulationSRVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    lightAccumulationSRVDesc.Texture2D.MostDetailedMip = 0;
    lightAccumulationSRVDesc.Texture2D.MipLevels = -1;
    hr = m_pd3dDevice->CreateShaderResourceView(m_LightAccumulation.Get(), &lightAccumulationSRVDesc, m_LightAccumulationSRV.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;

    for (int i = 0; i < kGBufferCount; i++)
    {
        //create g buffers
        D3D11_TEXTURE2D_DESC gBufferDesc = {};
        gBufferDesc.Width = width;
        gBufferDesc.Height = height;
        gBufferDesc.MipLevels = 1;
        gBufferDesc.ArraySize = 1;
        gBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        gBufferDesc.SampleDesc.Count = 1;
        gBufferDesc.SampleDesc.Quality = 0;
        gBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        gBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        gBufferDesc.CPUAccessFlags = 0;
        gBufferDesc.MiscFlags = 0;
        hr = m_pd3dDevice->CreateTexture2D(&gBufferDesc, nullptr, m_GBufferTexture[i].ReleaseAndGetAddressOf());
        if (FAILED(hr))
            return hr;

        hr = m_pd3dDevice->CreateRenderTargetView(m_GBufferTexture[i].Get(), nullptr, m_GBufferRTV[i].ReleaseAndGetAddressOf());
        if (FAILED(hr))
            return hr;

        D3D11_SHADER_RESOURCE_VIEW_DESC gBufferSRVDesc = {};
        gBufferSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        gBufferSRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        gBufferSRVDesc.Texture2D.MostDetailedMip = 0;
        gBufferSRVDesc.Texture2D.MipLevels = -1;
        hr = m_pd3dDevice->CreateShaderResourceView(m_GBufferTexture[i].Get(), &gBufferSRVDesc, m_GBufferSRV[i].ReleaseAndGetAddressOf());
        if (FAILED(hr))
            return hr;
    }

    //TODO : make textures for Gbuffer
    // TODO : make textures for post-process

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_ImmediateContext->RSSetViewports(1, &vp);

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

    hr = gameObject.InitGameObjectMesh(m_pd3dDevice.Get(), m_ImmediateContext.Get());
    if (FAILED(hr))
        return hr;

    // Compile the post process vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = ShaderManager::Get().CompileShaderFromFile(L"postProcessShader.fx", "VS", "vs_4_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the PP vertex shader
    hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_PostProcessVertexShader.ReleaseAndGetAddressOf());
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    hr = ShaderManager::Get().CompileShaderFromFile(L"postProcessShader.fx", "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_PostProcessPixelShader.ReleaseAndGetAddressOf());
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Compile the deferred vertex shader
    pVSBlob = nullptr;
    hr = ShaderManager::Get().CompileShaderFromFile(L"deferredLightingShader.fx", "VS", "vs_4_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the PP vertex shader
    hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_DeferredLightingVertexShader.ReleaseAndGetAddressOf());
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    // Compile the pixel shader
    pPSBlob = nullptr;
    hr = ShaderManager::Get().CompileShaderFromFile(L"deferredLightingShader.fx", "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_DeferredLightingPixelShader.ReleaseAndGetAddressOf());
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ***************************************************************************************
// InitMesh
// ***************************************************************************************

HRESULT	Application::InitMesh()
{

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
    HRESULT hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, m_ConstantBuffer.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;
    SetDebugName(m_ConstantBuffer.Get(), "ConstantBuffer");

    // Create the material constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(MaterialPropertiesConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, m_MaterialConstantBuffer.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;
    SetDebugName(m_MaterialConstantBuffer, "pMaterialConstantBuffer");

    // Create the parallax material constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(MaterialPropertiesConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, m_ParallaxMaterialConstantBuffer.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;
    SetDebugName(m_ParallaxMaterialConstantBuffer, "m_ParallaxMaterialConstantBuffer");

    // Create the light constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(LightPropertiesConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, m_LightConstantBuffer.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;
    SetDebugName(m_LightConstantBuffer, "pLightConstantBuffer");

    // Create the post process constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(PostProcessBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, m_PostProcessBuffer.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;
    SetDebugName(m_PostProcessBuffer, "m_PostProcessBuffer");

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_pd3dDevice->CreateSamplerState(&sampDesc, m_SamplerLinear.ReleaseAndGetAddressOf());

	return hr;
}

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
    if (m_ImmediateContext)
    {
        m_ImmediateContext->Flush();
        m_ImmediateContext->Flush();
    }

    if (m_ImmediateContext) m_ImmediateContext->ClearState();

    gameObject.CleaupGameObject();
    if (m_ConstantBuffer) m_ConstantBuffer.Reset();
    if (m_MaterialConstantBuffer) m_MaterialConstantBuffer.Reset();
    if (m_ParallaxMaterialConstantBuffer) m_ParallaxMaterialConstantBuffer.Reset();
    if (m_LightConstantBuffer) m_LightConstantBuffer.Reset();
    if (m_PostProcessBuffer) m_PostProcessBuffer.Reset();
    if (m_DepthStencil) m_DepthStencil.Reset();
    if (m_DepthStencilView) m_DepthStencilView.Reset();
    if (m_RenderTargetView) m_RenderTargetView.Reset();
    if (m_SwapChain1) m_SwapChain1.Reset();
    if (m_SwapChain) m_SwapChain.Reset();
    if (m_ImmediateContext1) m_ImmediateContext1.Reset();
    if (m_ImmediateContext) m_ImmediateContext.Reset();
    if (m_pd3dDevice1) m_pd3dDevice1.Reset();
    if (m_pd3dDevice) m_pd3dDevice.Reset();
    if (m_SamplerLinear) m_SamplerLinear.Reset();
    if (m_LightAccumulation) m_LightAccumulation.Reset();
    if (m_LightAccumulationRTV) m_LightAccumulationRTV.Reset();
    if (m_LightAccumulationSRV) m_LightAccumulationSRV.Reset();
    if (m_PostProcessVertexShader) m_PostProcessVertexShader.Reset();
    if (m_PostProcessPixelShader) m_PostProcessPixelShader.Reset();

#if _DEBUG
    if (m_d3dDebug)
    {
        m_d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
    }
#endif
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
    case WM_CHAR:
    {
        if (g_Application)
        {
            g_Application->CharTyped(wParam);
        }
        break;
    }

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


/***********************************************

MARKING SCHEME: Special effects pipeline

DESCRIPTION: Render to texture implemented

***********************************************/
//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Application::Render()
{
    //  bind g-buffer rendertargets
    //  render to gbuffer
    ID3D11RenderTargetView* gBufferRenderTargetViews[kGBufferCount] =
    {
        m_GBufferRTV[0].Get(),
        m_GBufferRTV[1].Get(),
        m_GBufferRTV[2].Get(),
        m_GBufferRTV[3].Get()
    };

    m_ImmediateContext->OMSetRenderTargets(kGBufferCount, gBufferRenderTargetViews, m_DepthStencilView.Get());
    
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

    m_ImmediateContext->ClearRenderTargetView(m_GBufferRTV[0].Get(), Colors::LightPink);
    m_ImmediateContext->ClearRenderTargetView(m_GBufferRTV[1].Get(), Colors::Black);
    m_ImmediateContext->ClearRenderTargetView(m_GBufferRTV[2].Get(), Colors::Black);
    m_ImmediateContext->ClearRenderTargetView(m_GBufferRTV[3].Get(), Colors::Black); //might want to change the colour to normalised normal colour

    // Clear the depth buffer to 1.0 (max depth)
    m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);


    // Update variables for a cube
    gameObject.Update(t);

    // Update variables for the cube
    XMMATRIX mGO = XMLoadFloat4x4(gameObject.GetTransform());

    XMFLOAT4X4 view = currentCamera->GetView();
    ConstantBuffer cb1;
    cb1.mWorld = XMMatrixTranspose(mGO);
    XMMATRIX viewMatrix = XMLoadFloat4x4(&view);
    cb1.mView = XMMatrixTranspose(viewMatrix);
    cb1.mProjection = XMMatrixTranspose(projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    m_ImmediateContext->UpdateSubresource(m_ConstantBuffer.Get(), 0, nullptr, &cb1, 0, 0);

    MaterialPropertiesConstantBuffer redPlasticMaterial;
    redPlasticMaterial.Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    redPlasticMaterial.Material.Specular = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
    redPlasticMaterial.Material.SpecularPower = 32.0f;
    redPlasticMaterial.Material.UseTexture = true;
    m_ImmediateContext->UpdateSubresource(m_MaterialConstantBuffer.Get(), 0, nullptr, &redPlasticMaterial, 0, 0);

    ParallaxMaterialPropertiesConstantBuffer parallaxMaterial;
    parallaxMaterial.ParallaxMaterial.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    parallaxMaterial.ParallaxMaterial.Specular = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
    parallaxMaterial.ParallaxMaterial.SpecularPower = 32.0f;
    parallaxMaterial.ParallaxMaterial.UseTexture = true;
    parallaxMaterial.ParallaxMaterial.Bias = -0.01f;
    parallaxMaterial.ParallaxMaterial.ScaleFactor = 1.0f;
    m_ImmediateContext->UpdateSubresource(m_ParallaxMaterialConstantBuffer.Get(), 0, nullptr, &parallaxMaterial, 0, 0);

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
    m_ImmediateContext->UpdateSubresource(m_LightConstantBuffer.Get(), 0, nullptr, &lightProperties, 0, 0);

    //update values for post processing
    PostProcessBuffer postProcessBuffer = {};
    postProcessBuffer.enableColourInversion = colourInversion;
    m_ImmediateContext->UpdateSubresource(m_PostProcessBuffer.Get(), 0, nullptr, &postProcessBuffer, sizeof(PostProcessBuffer), 0);

    // Begin g-buffer pass


    // Draw objects to g-buffer
    ID3D11Buffer* vsBuffer[1] = {
      m_ConstantBuffer.Get()
    };
    m_ImmediateContext->VSSetConstantBuffers(0, 1, vsBuffer);

    if (gameObject.isParallax)
    {
        ID3D11Buffer* psBuffers[3] = {
        m_ConstantBuffer.Get(),
        m_ParallaxMaterialConstantBuffer.Get(),
        m_LightConstantBuffer.Get()
        };
        m_ImmediateContext->PSSetConstantBuffers(0, 3, psBuffers);
    }
    else
    {
        ID3D11Buffer* psBuffers[3] = {
        m_ConstantBuffer.Get(),
        m_MaterialConstantBuffer.Get(),
        m_LightConstantBuffer.Get()
        };
        m_ImmediateContext->PSSetConstantBuffers(0, 3, psBuffers);
    }

    gameObject.Draw(m_ImmediateContext.Get());

    // Deferred Lighting
    ID3D11RenderTargetView* lightAccumulationRenderTargetViews[4] =
    {
        m_LightAccumulationRTV.Get(),
        nullptr,
        nullptr,
        nullptr
    };
    m_ImmediateContext->OMSetRenderTargets(4, lightAccumulationRenderTargetViews, m_DepthStencilView.Get());
    // Clear the back buffer
    m_ImmediateContext->ClearRenderTargetView(m_LightAccumulationRTV.Get(), Colors::LightPink); //render to texture

    m_ImmediateContext->VSSetShader(m_DeferredLightingVertexShader.Get(), nullptr, 0);
    m_ImmediateContext->PSSetShader(m_DeferredLightingPixelShader.Get(), nullptr, 0);
    ID3D11ShaderResourceView* gBufferShaderResources[kGBufferCount] =
    {
        m_GBufferSRV[0].Get(),
        m_GBufferSRV[1].Get(),
        m_GBufferSRV[2].Get(),
        m_GBufferSRV[3].Get(),
    };
    m_ImmediateContext->PSSetShaderResources(0, kGBufferCount, gBufferShaderResources);

    ID3D11Buffer* deferredLightingBuffers[2] = {
        m_ConstantBuffer.Get(),
        m_LightConstantBuffer.Get(),
    };
    m_ImmediateContext->PSSetConstantBuffers(0, 2, deferredLightingBuffers);
    m_ImmediateContext->Draw(3, 0);

    // unbind gbuffers
    ID3D11ShaderResourceView* nullResources[kGBufferCount] =
    {
        nullptr,
        nullptr,
        nullptr,
        nullptr,
    };
    m_ImmediateContext->PSSetShaderResources(0, kGBufferCount, nullResources);

    // Begin lighting pass
    //  bind light accumlation texture rendertarget

    // draw light(s)

    // begin post-process pass
    //  bind swapchain rendertarget
    ID3D11RenderTargetView* renderTargetViews[] =
    {
        m_RenderTargetView.Get()
    };

    m_ImmediateContext->OMSetRenderTargets(1, renderTargetViews, m_DepthStencilView.Get());
   
    // Clear the back buffer
    m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), Colors::LightCyan); 


    /***********************************************

    MARKING SCHEME: Special Effects Pipeline

    DESCRIPTION: Render of texture to full screen triangle

    ***********************************************/
    // draw post-process fullscreen
    m_ImmediateContext->VSSetShader(m_PostProcessVertexShader.Get(), nullptr, 0);
    m_ImmediateContext->PSSetShader(m_PostProcessPixelShader.Get(), nullptr, 0);
    ID3D11ShaderResourceView* psShaderResources[1] =
    {
        m_LightAccumulationSRV.Get()
    };
    m_ImmediateContext->PSSetShaderResources(0, 1, psShaderResources);

    ID3D11Buffer* psPostProcessBuffers[2] = {
        m_ConstantBuffer.Get(),
        m_PostProcessBuffer.Get(),
    };
    m_ImmediateContext->PSSetConstantBuffers(0, 2, psPostProcessBuffers);
    m_ImmediateContext->Draw(3, 0); //render texture to full screen triangle


    ID3D11ShaderResourceView* nullresource[1] = { nullptr };
    m_ImmediateContext->PSSetShaderResources(0, 1, nullresource);

    // Present our back buffer to our front buffer
    m_SwapChain->Present(0, 0);
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


    /***********************************************

    MARKING SCHEME: Normal mapping

    DESCRIPTION: Evidence the effect works correctly by moving object, camera and light source

    ***********************************************/

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

        XMStoreFloat4x4(&cameraPosM, XMMatrixTranslation(_XValue, _YValue, _ZValue));
        XMMATRIX cameraPositionM = XMLoadFloat4x4(&cameraPosM);

        cameraPosV = XMVector3TransformCoord(cameraPosV, cameraPositionM);
        XMStoreFloat3(&newCameraPos, cameraPosV);

        currentCamera->Update(XMVectorSet(newCameraPos.x, newCameraPos.y, newCameraPos.z, 1.0f), g_mouseDeltaX, g_mouseDeltaY);
        eyePosition = currentCamera->GetPosition();
    }
}

void Application::CharTyped(char charTyped)
{
    if (charTyped == 'p')
    {
        gameObject.isParallax = true;
        CleanupDevice();
        InitDevice();
    }
    if (charTyped == 'n')
    {
        gameObject.isParallax = false;
        CleanupDevice();
        InitDevice();
    }
    if (charTyped == 'i')
    {
        colourInversion = !colourInversion;
    }
}

void SetDebugName(const ComPtr<ID3D11DeviceChild>& object, const std::string& name)
{
    SetDebugName(object.Get(), name);
}

void SetDebugName(ID3D11DeviceChild* object, const std::string& name)
{
    object->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str());
}

