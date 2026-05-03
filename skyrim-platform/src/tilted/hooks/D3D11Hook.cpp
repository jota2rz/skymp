#include <D3D11Hook.hpp>

#include <d3d11.h>

#include <FunctionHook.hpp>

using TDXGISwapChainPresent = HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain* This,
                                                          UINT SyncInterval,
                                                          UINT Flags);

TDXGISwapChainPresent RealDXGISwapChainPresent = nullptr;

namespace CEFUtils {

static bool s_initialized = false;

static void DoPresent(IDXGISwapChain* This)
{
  auto& d3d11 = D3D11Hook::Get();

  if (!s_initialized) {
    s_initialized = true;
    d3d11.OnCreate(This);
  }

  d3d11.OnPresent(This);
}

HRESULT __stdcall HookDXGISwapChainPresent(IDXGISwapChain* This,
                                           UINT SyncInterval, UINT Flags)
{
  try {
    DoPresent(This);
  } catch (...) {
  }

  return RealDXGISwapChainPresent(This, SyncInterval, Flags);
}

D3D11Hook::D3D11Hook() noexcept
{
}

void D3D11Hook::Install() noexcept
{
  // No-op. Use InstallOnSwapChain() after D3D11 initialization instead.
  // IAT hooking D3D11CreateDeviceAndSwapChain conflicts with Community Shaders.
}

void D3D11Hook::InstallOnSwapChain(IDXGISwapChain* pSwapChain) noexcept
{
  if (!pSwapChain || RealDXGISwapChainPresent != nullptr)
    return;

  RealDXGISwapChainPresent =
    HookVTable(pSwapChain, 8, &HookDXGISwapChainPresent);
}

D3D11Hook& D3D11Hook::Get() noexcept
{
  static D3D11Hook s_instance;
  return s_instance;
}
}
