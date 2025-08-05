#include "read_cpu_crypt.h"
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Crypt32.lib")
#include <openssl/sha.h>

#if 1

// main.cpp  – 读取主板序列号（Windows: WMI；Linux: /sys/...）

#if defined(_WIN32) || defined(_WIN64)      // ---- Windows 实现 ----
#   include <comdef.h>
#   include <Wbemidl.h>
#   pragma comment(lib, "wbemuuid.lib")
#   pragma comment(lib, "Ole32.lib")

class read_cpu_crypt::Ipmi
{
public:
    std::string sha256_openssl(const std::string& input);

    std::string GetBoardSerial();

    std::string GetCpuSerial();
};

std::string read_cpu_crypt::Ipmi::GetCpuSerial()
{
    int cpuInfo[4] = { 0 };
    char vendor[13] = { 0 };

    __cpuid(cpuInfo, 0); // 获取厂商名
    memcpy(vendor, &cpuInfo[1], 4); // EBX
    memcpy(vendor + 4, &cpuInfo[3], 4); // EDX
    memcpy(vendor + 8, &cpuInfo[2], 4); // ECX

    // 再尝试获取 CPU ID 信息
    __cpuid(cpuInfo, 1);
    int id1 = cpuInfo[0]; // EAX
    int id2 = cpuInfo[3]; // EDX

    char result[32];
    sprintf_s(result, "%08X%08X", id2, id1);
    return std::string(result);
}

std::string read_cpu_crypt::Ipmi::GetBoardSerial()
{
    HRESULT hr;

    // 1) 初始化 COM
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) return "";

    // 2) 设置 COM 安全（允许我们调用 WMI）
    hr = CoInitializeSecurity(
        nullptr, -1, nullptr, nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr, EOAC_NONE, nullptr);
    if (FAILED(hr) && hr != RPC_E_TOO_LATE) { CoUninitialize(); return ""; }

    // 3) 创建 WMI 入口
    IWbemLocator* pLoc = nullptr;
    hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hr)) { CoUninitialize(); return ""; }

    // 4) 连接 ROOT\CIMV2
    IWbemServices* pSvc = nullptr;
    hr = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0,
        0, nullptr, nullptr, &pSvc);
    if (FAILED(hr)) { pLoc->Release(); CoUninitialize(); return ""; }

    // 5) 设置代理级别
    hr = CoSetProxyBlanket(
        pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr, EOAC_NONE);
    if (FAILED(hr)) {
        pSvc->Release(); pLoc->Release(); CoUninitialize(); return "";
    }

    // 6) 查询 Win32_BaseBoard
    IEnumWbemClassObject* pEnumerator = nullptr;
    hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT SerialNumber FROM Win32_BaseBoard"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr, &pEnumerator);
    if (FAILED(hr)) {
        pSvc->Release(); pLoc->Release(); CoUninitialize(); return "";
    }

    // 7) 取第一条记录
    IWbemClassObject* pObj = nullptr;
    ULONG ret = 0;
    std::string serial;
    if (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &ret) == S_OK) {
        VARIANT vtProp;
        if (SUCCEEDED(pObj->Get(L"SerialNumber", 0, &vtProp, nullptr, nullptr))
            && vtProp.vt == VT_BSTR)
            serial = _bstr_t(vtProp.bstrVal);
        VariantClear(&vtProp);
        pObj->Release();
    }

    // 8) 清理
    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    return serial;
}




#elif defined(__linux__)                     // ---- Linux 实现 ----




class read_cpu_crypt::Ipmi
{
public:
    std::string sha256_openssl(const std::string& input);

    std::string GetBoardSerial();

    std::string GetCpuSerial();
};

std::string read_cpu_crypt::Ipmi::GetCpuSerial()
{
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;

    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            std::cout << line << std::endl;
            break; // 找到第一个CPU型号就退出
        }
    }

    return line.substr(line.find(":") + 2); // 提取型号信息
}

std::string read_cpu_crypt::Ipmi::GetBoardSerial()
{
    std::ifstream fin("/sys/class/dmi/id/board_serial");
    std::string serial;
    if (fin) std::getline(fin, serial);
    return serial;
}

#else                                        // ---- 其他平台 ----
std::string GetBoardSerial() { return ""; }
#endif

// ----------------------- DEMO -----------------------
#endif

std::string read_cpu_crypt::Ipmi::sha256_openssl(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)input.c_str(), input.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}

std::string read_cpu_crypt::hash(const std::string& input)
{
    return p->sha256_openssl(input);
}

std::string read_cpu_crypt::GetBoardSerial()
{
    return p->GetBoardSerial();
}

std::string read_cpu_crypt::GetCpuSerial()
{
    return p->GetCpuSerial();
}

#if 0
int main()
{
    std::string serial = GetCpuSerial();
    std::cout << "CPU ID: " << serial << std::endl;

    std::string sn = GetBoardSerial();
    if (sn.empty())
        std::cout << "无法读取主板序列号\n";
    else
        std::cout << "主板序列号: " << sn << '\n';

    std::string t = sha256_openssl(serial + sn);
    std::string macine_code = t.substr(0, 8);

    std::cout << "hash: " << t << std::endl
        << "macine_code: " << macine_code;
}
#endif