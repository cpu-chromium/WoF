

#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>

#include <Windows.h>

#include <wofapi.h>

#pragma comment(lib, "wofutil.lib")

const wchar_t vol_name[] = L"\\\\.\\C:";
const wchar_t wim_path[] = L"C:\\Test\\test.wim";

const unsigned char pic_sha1_hash[] = {
    0xe0, 0x45, 0xb9, 0xf7, 0x64,
    0xe1, 0x29, 0x4e, 0x0a, 0xda,
    0x57, 0xce, 0x80, 0x3e, 0x06,
    0xe4, 0xf6, 0xea, 0x01, 0xf3 };

int wmain(int argc, wchar_t* argv[]) {
  if (argc != 2) {
    wprintf(L"bad arguments. only argument is the path to overlay bmp file\n");
    return 1;
  }

  auto file_path = argv[1];
  LARGE_INTEGER wim_source_id;

  HRESULT hr = ::WofWimAddEntry(vol_name, wim_path, WIM_BOOT_NOT_OS_WIM, 1, &wim_source_id);
  if (hr != S_OK) {
    wprintf(L"error %x adding wim overlay [%s] for [%s]\n -are you running elevated?\n", hr, wim_path, vol_name);
    return 2;
  }

  wprintf(L"wim id is %x:%x \n", wim_source_id.HighPart, wim_source_id.LowPart);

  auto file = ::CreateFile(file_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE) {
    wprintf(L"error %x making [%s]\n", ::GetLastError(), file_path);
    return 3;
  }

  WIM_EXTERNAL_FILE_INFO wim_ex_fi;
  wim_ex_fi.DataSourceId = wim_source_id;
  memcpy(wim_ex_fi.ResourceHash, pic_sha1_hash, WIM_PROVIDER_HASH_SIZE);
  wim_ex_fi.Flags = 0;

  hr = ::WofSetFileDataLocation(file, WOF_PROVIDER_WIM, &wim_ex_fi, sizeof(wim_ex_fi));
  ::CloseHandle(file);

  if (hr != S_OK) {
    wprintf(L"error %x setting location for [%s]\n", hr, file_path);
    return 3;
  }

  wprintf(L"success overlaying [%s]\\pic.bmp for [%s]\n", wim_path, file_path);
  return 0;
}
