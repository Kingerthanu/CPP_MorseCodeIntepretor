#include <iostream>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <conio.h>
#include <comdef.h>
#include <functional>

using namespace std;

const double PI = 3.141592653589793238460;
const float THRESHOLD = 0.00001f; // Amplitude threshold for Morse code detection

// Morse timing constants
static const float dotWait = 100.0f;
static const float dashWait = dotWait * 3.0f;
static const float spaceWait = dashWait;

std::atomic<bool> stopThreads(false);

void enlargeList(char*& toEnlargen, unsigned int& oldSize, const char* toInsert, const unsigned int& chunkSize) {
    unsigned int newSize = oldSize + chunkSize;
    char* tmpHandler = toEnlargen;
    toEnlargen = new char[newSize];

    unsigned int characterStep = 0, insertStep = 0;

    while (characterStep < oldSize) {
        toEnlargen[characterStep] = tmpHandler[characterStep++];
    }

    delete[] tmpHandler;

    while (insertStep < chunkSize) {
        toEnlargen[characterStep + insertStep] = toInsert[insertStep++];
    }

    oldSize = newSize;
}

char* alphabetToMorse(char*& toConvert) {
    int messageLength = 0;

    while (toConvert[messageLength] != '\0') {
        messageLength++;
    }

    if (messageLength == 0) {
        return nullptr;
    }

    char* morseBuffer = new char[1];
    morseBuffer[0] = '\0';
    unsigned int listSize = 0;

    for (unsigned int cIndex = 0; cIndex < messageLength; cIndex++) {
        switch (toConvert[cIndex]) {
        case 'A': case 'a':
            enlargeList(morseBuffer, listSize, ".- ", 3);
            break;
        case 'B': case 'b':
            enlargeList(morseBuffer, listSize, "-... ", 5);
            break;
        case 'C': case 'c':
            enlargeList(morseBuffer, listSize, "-.-. ", 5);
            break;
        case 'D': case 'd':
            enlargeList(morseBuffer, listSize, "-.. ", 4);
            break;
        case 'E': case 'e':
            enlargeList(morseBuffer, listSize, ". ", 2);
            break;
        case 'F': case 'f':
            enlargeList(morseBuffer, listSize, "..-. ", 5);
            break;
        case 'G': case 'g':
            enlargeList(morseBuffer, listSize, "--. ", 4);
            break;
        case 'H': case 'h':
            enlargeList(morseBuffer, listSize, ".... ", 5);
            break;
        case 'I': case 'i':
            enlargeList(morseBuffer, listSize, ".. ", 3);
            break;
        case 'J': case 'j':
            enlargeList(morseBuffer, listSize, ".--- ", 5);
            break;
        case 'K': case 'k':
            enlargeList(morseBuffer, listSize, "-.- ", 4);
            break;
        case 'L': case 'l':
            enlargeList(morseBuffer, listSize, ".-.. ", 5);
            break;
        case 'M': case 'm':
            enlargeList(morseBuffer, listSize, "-- ", 3);
            break;
        case 'N': case 'n':
            enlargeList(morseBuffer, listSize, "-. ", 3);
            break;
        case 'O': case 'o':
            enlargeList(morseBuffer, listSize, "--- ", 4);
            break;
        case 'P': case 'p':
            enlargeList(morseBuffer, listSize, ".--. ", 5);
            break;
        case 'Q': case 'q':
            enlargeList(morseBuffer, listSize, "--.- ", 5);
            break;
        case 'R': case 'r':
            enlargeList(morseBuffer, listSize, ".-. ", 4);
            break;
        case 'S': case 's':
            enlargeList(morseBuffer, listSize, "... ", 4);
            break;
        case 'T': case 't':
            enlargeList(morseBuffer, listSize, "- ", 2);
            break;
        case 'U': case 'u':
            enlargeList(morseBuffer, listSize, "..- ", 4);
            break;
        case 'V': case 'v':
            enlargeList(morseBuffer, listSize, "...- ", 5);
            break;
        case 'W': case 'w':
            enlargeList(morseBuffer, listSize, ".-- ", 4);
            break;
        case 'X': case 'x':
            enlargeList(morseBuffer, listSize, "-..- ", 5);
            break;
        case 'Y': case 'y':
            enlargeList(morseBuffer, listSize, "-.-- ", 5);
            break;
        case 'Z': case 'z':
            enlargeList(morseBuffer, listSize, "--.. ", 5);
            break;
        default:
            enlargeList(morseBuffer, listSize, " ", 1);
            break;
        }
    }

    enlargeList(morseBuffer, listSize, "\0", 1);

    return morseBuffer;
}

void playMorseSound(const char* morseCode) {
    while (*morseCode != '\0' && !stopThreads) {
        switch (*morseCode) {
        case '.':
            Beep(1000, dotWait);
            break;
        case '-':
            Beep(998, dashWait);
            break;
        case ' ':
            Sleep(spaceWait);
            break;
        }
        morseCode++;
    }
}

// Morse code detection function
void processAudioData(const float* data, size_t length, bool& signalDetected, std::chrono::high_resolution_clock::time_point& signalStart, long long& duration) {
    for (size_t i = 0; i < length; ++i) {
        if (fabs(data[i]) > THRESHOLD) {
            if (!signalDetected) {
                signalDetected = true;
            }
        }
        else {
            if (signalDetected) {
                auto now = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - signalStart).count();
                signalDetected = false;
            }
        }
    }
}

// Reference times in 100-nanosecond units
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

HRESULT CaptureAudio() {
    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
    UINT32 packetLength = 0;
    UINT32 numFramesAvailable;
    BYTE* pData;
    DWORD flags;

    // Initialize COM library
    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf("Unable to initialize COM library: %x\n", hr);
        return hr;
    }

    // Get the default audio device
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        printf("Unable to get default audio device: %x\n", hr);
        return hr;
    }

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) {
        printf("Unable to get default audio endpoint: %x\n", hr);
        return hr;
    }

    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr)) {
        printf("Unable to activate audio client: %x\n", hr);
        return hr;
    }

    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) {
        printf("Unable to get mix format: %x\n", hr);
        return hr;
    }

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, hnsRequestedDuration, 0, pwfx, NULL);
    if (FAILED(hr)) {
        printf("Unable to initialize audio client: %x\n", hr);
        return hr;
    }

    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    if (FAILED(hr)) {
        printf("Unable to get capture client: %x\n", hr);
        return hr;
    }

    hr = pAudioClient->Start();
    if (FAILED(hr)) {
        printf("Unable to start audio client: %x\n", hr);
        return hr;
    }

    bool signalDetected = false;
    auto signalStart = std::chrono::high_resolution_clock::now();
    long long duration = 0;
    

    while (!stopThreads) {
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) {
            printf("Unable to get next packet size: %x\n", hr);
            break;
        }

        if (packetLength > 0) 
        {
            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
            if (FAILED(hr)) {
                printf("Unable to get buffer: %x\n", hr);
                break;
            }

            // Process the audio data
            processAudioData((const float*)pData, numFramesAvailable, signalDetected, signalStart, duration);

            if (!signalDetected && duration > 0) 
            {
                // Output detected Morse code duration
                if (duration <= dotWait)
                {
                    std::cout << '.';
                }
                else if (duration > dotWait)
                {
                    std::cout << '-';
                }
                //std::cout << "Duration: " << duration << " ms" << std::endl;
                duration = 0; // Reset duration after printing
                signalStart = std::chrono::high_resolution_clock::now();
             
            }
            else if (duration <= 0)
            {   
                signalStart = std::chrono::high_resolution_clock::now();
            }

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            if (FAILED(hr)) {
                printf("Unable to release buffer: %x\n", hr);
                break;
            }
            
        }

        

    }

    hr = pAudioClient->Stop();
    if (FAILED(hr)) {
        printf("Unable to stop audio client: %x\n", hr);
        return hr;
    }

    CoTaskMemFree(pwfx);
    pCaptureClient->Release();
    pAudioClient->Release();
    pDevice->Release();
    pEnumerator->Release();
    CoUninitialize();

    return hr;
}

int main() {
    char* userInput = new char[100];

    std::cout << "Enter Message In English To Convert Into Morse: \n";
    std::cin.getline(userInput, 100);

    char* morseUserInput = alphabetToMorse(userInput);
    if (!morseUserInput) {
        std::cerr << "Error converting input to Morse code." << std::endl;
        delete[] userInput;
        return 1;
    }

    std::cout << morseUserInput << std::endl;

    std::thread captureThread(CaptureAudio);
    std::this_thread::sleep_for(std::chrono::seconds(1));  // Ensure the capture thread starts first
    playMorseSound(morseUserInput);

    captureThread.join();

    delete[] userInput;
    delete[] morseUserInput;

    return 0;
}
