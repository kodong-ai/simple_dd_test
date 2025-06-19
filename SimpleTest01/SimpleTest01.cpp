#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>

static const char* CDI_NODE_NAME   = R"(\\.\MyDeviceNode)";   // 드라이버 노드 이름
static const DWORD DMA_BUFFER_SIZE = 1 * 1024 * 1024;        // 1 MB
static const DWORD CHUNK_SIZE      = 256 * 1024;             // 256 KB

// IOCTL 코드 정의 (파일 디바이스 채널 unknown, 범위 0x800–0x801)
#define IOCTL_ALLOC_DMA_BUFFER CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_TRIGGER_DMA      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

void usage()
{
    std::cerr << "Usage: SimpleTest01.exe -input <filename>\n";
}

int main(int argc, char* argv[])
{
    if (argc != 3 || strcmp(argv[1], "-input") != 0) {
        usage();
        return 1;
    }
    const char* filename = argv[2];

    // 1) 드라이버 노드 열기 (관리자 권한 필요)
    HANDLE hDev = CreateFileA(
        CDI_NODE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,              // 공유 없음
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (hDev == INVALID_HANDLE_VALUE) {
        std::cerr << "ERROR: CreateFile failed (" << GetLastError() << ")\n";
        return 1;
    }

    // 2) DMA 버퍼 할당 요청
    void* dmaBuffer = nullptr;
    DWORD returned = 0;
    if (!DeviceIoControl(
            hDev,
            IOCTL_ALLOC_DMA_BUFFER,
            nullptr, 0,
            &dmaBuffer, sizeof(dmaBuffer),
            &returned,
            nullptr))
    {
        std::cerr << "ERROR: IOCTL_ALLOC_DMA_BUFFER failed (" << GetLastError() << ")\n";
        CloseHandle(hDev);
        return 1;
    }
    std::cout << "Allocated DMA buffer at userptr: " << dmaBuffer << "\n";

    // 3) 파일 열기
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "ERROR: Cannot open input file: " << filename << "\n";
        CloseHandle(hDev);
        return 1;
    }

    // 4) 파일을 256KB씩 읽어 DMA 버퍼에 쓰고, 1MB마다 TRIGGER
    char* bufPtr = static_cast<char*>(dmaBuffer);
    size_t used = 0;
    std::vector<char> temp(CHUNK_SIZE);

    while (inFile) {
        inFile.read(temp.data(), temp.size());
        std::streamsize n = inFile.gcount();
        if (n <= 0) break;

        // 남은 공간이 충분하지 않으면 먼저 TRIGGER
        if (used + n > DMA_BUFFER_SIZE) {
            // TRIGGER ioctl
            if (!DeviceIoControl(hDev, IOCTL_TRIGGER_DMA, &used, sizeof(used), nullptr, 0, &returned, nullptr)) {
                std::cerr << "ERROR: IOCTL_TRIGGER_DMA failed at full (" << GetLastError() << ")\n";
                break;
            }
            used = 0;
        }

        // DMA 버퍼에 복사
        memcpy(bufPtr + used, temp.data(), n);
        used += static_cast<size_t>(n);
    }

    // 남은 데이터가 있으면 마지막 TRIGGER
    if (used > 0) {
        if (!DeviceIoControl(hDev, IOCTL_TRIGGER_DMA, &used, sizeof(used), nullptr, 0, &returned, nullptr)) {
            std::cerr << "ERROR: IOCTL_TRIGGER_DMA failed at final (" << GetLastError() << ")\n";
        }
    }

    // 정리
    inFile.close();
    CloseHandle(hDev);
    std::cout << "Done.\n";
    return 0;
}
