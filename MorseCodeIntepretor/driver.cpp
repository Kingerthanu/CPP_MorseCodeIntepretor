#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <chrono>
#include <thread>
#include <mmsystem.h>
#include "Vertex_Array.h"
#include "shader.h"
#include <mutex>
#include <signal.h>
#include <vector>
#include <cmath>
#include <atomic>
#include <string>

#define PI 3.141592653589793238
#define THRESHOLD 0.3
#define REFTIMES_PER_SEC  10000000

static const unsigned int dotWait = 100;
static const unsigned int dashWait = dotWait * 3;
static const unsigned int spaceWait = dashWait;
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

char morseToAlphabet(const std::string& morse) {
    switch (morse.length()) {
    case 1:
        if (morse == ".") return 'E';
        if (morse == "-") return 'T';
        break;
    case 2:
        if (morse == "..") return 'I';
        if (morse == ".-") return 'A';
        if (morse == "-.") return 'N';
        if (morse == "--") return 'M';
        break;
    case 3:
        if (morse == "...") return 'S';
        if (morse == "..-") return 'U';
        if (morse == ".-.") return 'R';
        if (morse == ".--") return 'W';
        if (morse == "-..") return 'D';
        if (morse == "-.-") return 'K';
        if (morse == "--.") return 'G';
        if (morse == "---") return 'O';
        break;
    case 4:
        if (morse == "....") return 'H';
        if (morse == "...-") return 'V';
        if (morse == "..-.") return 'F';
        if (morse == ".-..") return 'L';
        if (morse == ".--.") return 'P';
        if (morse == ".---") return 'J';
        if (morse == "-...") return 'B';
        if (morse == "-..-") return 'X';
        if (morse == "-.-.") return 'C';
        if (morse == "-.--") return 'Y';
        if (morse == "--..") return 'Z';
        if (morse == "--.-") return 'Q';
        break;
    case 5:
        if (morse == "-----") return '0';
        if (morse == ".----") return '1';
        if (morse == "..---") return '2';
        if (morse == "...--") return '3';
        if (morse == "....-") return '4';
        if (morse == ".....") return '5';
        if (morse == "-....") return '6';
        if (morse == "--...") return '7';
        if (morse == "---..") return '8';
        if (morse == "----.") return '9';
        break;
    default:
        return ' ';
    }
    return ' ';
}

void playSineWave(double frequency, double durationMs, int sampleRate) {
    const float amplitude = 0.1f;
    int samplesCount = static_cast<int>((durationMs / 1000.0) * sampleRate);
    float* buffer = new float[samplesCount];

    for (int i = 0; i < samplesCount; ++i) {
        buffer[i] = 0.7f + (amplitude * sin(2.0 * PI * frequency * i / sampleRate));
    }

    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    wfx.nChannels = 1; // Mono sound
    wfx.nSamplesPerSec = sampleRate;
    wfx.nAvgBytesPerSec = sampleRate * sizeof(float);
    wfx.nBlockAlign = sizeof(float);
    wfx.wBitsPerSample = 32; // 16-bit PCM sound

    HWAVEOUT hWaveOut;
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        std::cerr << "Error opening waveform output device." << std::endl;
        delete[] buffer;
        return;
    }

    WAVEHDR waveHeader = {};
    waveHeader.lpData = reinterpret_cast<LPSTR>(buffer);
    waveHeader.dwBufferLength = samplesCount * sizeof(float);
    waveHeader.dwFlags = 0;
    waveHeader.dwLoops = 0;

    if (waveOutPrepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
        std::cerr << "Error preparing waveform header." << std::endl;
        waveOutClose(hWaveOut);
        delete[] buffer;
        return;
    }

    if (waveOutWrite(hWaveOut, &waveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
        std::cerr << "Error writing waveform data." << std::endl;
        waveOutUnprepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR));
        waveOutClose(hWaveOut);
        delete[] buffer;
        return;
    }

    // Wait until the sound has finished playing
    while (waveOutUnprepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING) {
       std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    waveOutClose(hWaveOut);
    delete[] buffer;
}

void playMorseSound(const char* morseCode) {
    while (*morseCode != '\0' && !stopThreads) {
        switch (*morseCode++) {
        case '.':
            playSineWave(1000, dotWait, 48000);
            break;
        case '-':
            playSineWave(1000, dashWait, 48000);
            break;
        case ' ':
            std::this_thread::sleep_for(std::chrono::milliseconds(spaceWait));
            break;
        }

    }
}

char* alphabetToMorse(char*& toConvert) {
    int messageLength = 0;
    while (toConvert[messageLength] != '\0') {
        messageLength++;
    }

    if (messageLength == 0) {
        return nullptr;
    }

    char* morseBuffer = new char[0];
    unsigned int listSize = 0;

    for (unsigned int cIndex = 0; cIndex < messageLength; cIndex++) {
        switch (toConvert[cIndex]) {
        case 'A': case 'a': enlargeList(morseBuffer, listSize, ".- ", 3); break;
        case 'B': case 'b': enlargeList(morseBuffer, listSize, "-... ", 5); break;
        case 'C': case 'c': enlargeList(morseBuffer, listSize, "-.-. ", 5); break;
        case 'D': case 'd': enlargeList(morseBuffer, listSize, "-.. ", 4); break;
        case 'E': case 'e': enlargeList(morseBuffer, listSize, ". ", 2); break;
        case 'F': case 'f': enlargeList(morseBuffer, listSize, "..-. ", 5); break;
        case 'G': case 'g': enlargeList(morseBuffer, listSize, "--. ", 4); break;
        case 'H': case 'h': enlargeList(morseBuffer, listSize, ".... ", 5); break;
        case 'I': case 'i': enlargeList(morseBuffer, listSize, ".. ", 3); break;
        case 'J': case 'j': enlargeList(morseBuffer, listSize, ".--- ", 5); break;
        case 'K': case 'k': enlargeList(morseBuffer, listSize, "-.- ", 4); break;
        case 'L': case 'l': enlargeList(morseBuffer, listSize, ".-.. ", 5); break;
        case 'M': case 'm': enlargeList(morseBuffer, listSize, "-- ", 3); break;
        case 'N': case 'n': enlargeList(morseBuffer, listSize, "-. ", 3); break;
        case 'O': case 'o': enlargeList(morseBuffer, listSize, "--- ", 4); break;
        case 'P': case 'p': enlargeList(morseBuffer, listSize, ".--. ", 5); break;
        case 'Q': case 'q': enlargeList(morseBuffer, listSize, "--.- ", 5); break;
        case 'R': case 'r': enlargeList(morseBuffer, listSize, ".-. ", 4); break;
        case 'S': case 's': enlargeList(morseBuffer, listSize, "... ", 4); break;
        case 'T': case 't': enlargeList(morseBuffer, listSize, "- ", 2); break;
        case 'U': case 'u': enlargeList(morseBuffer, listSize, "..- ", 4); break;
        case 'V': case 'v': enlargeList(morseBuffer, listSize, "...- ", 5); break;
        case 'W': case 'w': enlargeList(morseBuffer, listSize, ".-- ", 4); break;
        case 'X': case 'x': enlargeList(morseBuffer, listSize, "-..- ", 5); break;
        case 'Y': case 'y': enlargeList(morseBuffer, listSize, "-.-- ", 5); break;
        case 'Z': case 'z': enlargeList(morseBuffer, listSize, "--.. ", 5); break;
        case '1': enlargeList(morseBuffer, listSize, ".---- ", 6); break;
        case '2': enlargeList(morseBuffer, listSize, "..--- ", 6); break;
        case '3': enlargeList(morseBuffer, listSize, "...-- ", 6); break;
        case '4': enlargeList(morseBuffer, listSize, "....- ", 6); break;
        case '5': enlargeList(morseBuffer, listSize, "..... ", 6); break;
        case '6': enlargeList(morseBuffer, listSize, "-.... ", 6); break;
        case '7': enlargeList(morseBuffer, listSize, "--... ", 6); break;
        case '8': enlargeList(morseBuffer, listSize, "---.. ", 6); break;
        case '9': enlargeList(morseBuffer, listSize, "----. ", 6); break;
        case '0': enlargeList(morseBuffer, listSize, "----- ", 6); break;
        default: enlargeList(morseBuffer, listSize, " ", 1); break;
        }
    }

    enlargeList(morseBuffer, listSize, "\0", 1);
    return morseBuffer;
}

void signalShutdown(int) {
    std::cout << "Shutting Down...\n";
    stopThreads = true;
}

class WINDOW_AUDIOWAVES {
private:
    GLuint VAO, VBO;
    GLFWwindow* _WINDOW;
    Shader contextShader;
    std::mutex contextWand;
    const float _circleRadius = 0.65f;

    std::vector<Vertex> generateSegmentedCircle(const float& centerX, const float& centerY, const float* audioData, const UINT32& segmentCount) {
        std::vector<Vertex> vertices;
        float angleStep = 2.0f * PI / segmentCount;
        for (UINT32 i = 0; i < segmentCount; ++i) {
            float normalizedSample = fabs(audioData[i]) * 0.75f;
            float angle = i * angleStep;
            vertices.push_back(Vertex{ glm::vec2(centerX + (_circleRadius + normalizedSample) * cos(angle), centerY + (_circleRadius + normalizedSample) * sin(angle)), glm::vec3(0.93f, 0.15f, 0.45f) });
        }
        vertices.push_back(Vertex{ glm::vec2(centerX + (_circleRadius + (fabs(audioData[0]) * 0.75f)), 0), glm::vec3(0.93f, 0.15f, 0.45f) });
        return vertices;
    }

public:
    WINDOW_AUDIOWAVES(const unsigned int& newWidth, const unsigned int& newHeight) {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        this->_WINDOW = glfwCreateWindow(newWidth, newHeight, "Audio Waves", NULL, NULL);
        if (!this->_WINDOW) {
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(this->_WINDOW);
        gladLoadGL();
        this->contextShader = Shader("default.vert", "default.frag");
        this->contextShader.Activate();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(this->_WINDOW);
        glfwSetFramebufferSizeCallback(this->_WINDOW, resize_callback);

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(1);
        glfwPollEvents();
        glfwMakeContextCurrent(nullptr);
    }

    void RenderDiscrete(const float* audioData, const UINT32 length) {
        std::lock_guard<std::mutex> lock(contextWand);
        glfwMakeContextCurrent(this->_WINDOW);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, (length + 1) * sizeof(Vertex), generateSegmentedCircle(0.0f, 0.0f, audioData, length).data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_STRIP, 0, (length + 1));
        glfwSwapBuffers(this->_WINDOW);
        glfwMakeContextCurrent(nullptr);
    }

    static void resize_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    ~WINDOW_AUDIOWAVES() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glfwDestroyWindow(this->_WINDOW);
    }
};

float getNormalizationFactor(IMMDevice* pDevice) {
    HRESULT hr;
    IAudioMeterInformation* pMeterInfo = NULL;
    float peakValue = 0.0f;

    hr = pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeterInfo);
    if (SUCCEEDED(hr))
    {
        hr = pMeterInfo->GetPeakValue(&peakValue);
        pMeterInfo->Release();
    }

    if (peakValue > 0.0f) {
        return 1.0f / peakValue;
    }

    return 1.0f; // Return default normalization factor if unable to calculate
}

float getMasterVolumeLevel(IMMDevice* pDevice) {
    IAudioEndpointVolume* pEndpointVolume = NULL;
    HRESULT hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pEndpointVolume);
    if (FAILED(hr)) {
        std::cerr << "Unable to activate endpoint volume: " << std::hex << hr << std::endl;
        return 1.0f; // Return default volume if unable to get the actual value
    }

    float volumeLevel = 0.0f;
    hr = pEndpointVolume->GetMasterVolumeLevelScalar(&volumeLevel);
    if (FAILED(hr)) {
        std::cerr << "Unable to get master volume level: " << std::hex << hr << std::endl;
        volumeLevel = 1.0f; // Return default volume if unable to get the actual value
    }

    pEndpointVolume->Release();
    return volumeLevel;
}

void processAudioData(const float* data, UINT32& length, bool& signalDetected, std::chrono::high_resolution_clock::time_point& signalStart, long long& duration, WINDOW_AUDIOWAVES& audioWindow, float normalizationFactor, float masterVolume) {
    float scaledThreshold = THRESHOLD * masterVolume;

    std::thread([&audioWindow, data, length]() {
        audioWindow.RenderDiscrete(data, length);
        }).detach();

        for (UINT32 i = 0; i < length; ++i) {

            //std::cout << fabs(data[i]) << '\n';

            if (scaledThreshold <= fabs(data[i])) {
                if (!signalDetected) {
                    //std::cout << fabs(data[i]) << '\n';
                    signalDetected = true;
                    signalStart = std::chrono::high_resolution_clock::now(); 
                }
            }
            else {
                if (signalDetected) {
                    auto now = std::chrono::high_resolution_clock::now();
                    duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - signalStart).count();
                    signalDetected = false;
                    //std::cout << "Duration: " << duration << '\n';
                    
                }
            }
        }
}

HRESULT CaptureAudio(WAVEFORMATEX* pwfx, WINDOW_AUDIOWAVES* audioWindow) {
    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient3* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;
    UINT32 packetLength = 0;
    UINT32 numFramesAvailable;
    BYTE* pData;
    DWORD flags;

    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf("Unable to initialize COM library: %x\n", hr);
        return hr;
    }

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

    float normalizationFactor = getNormalizationFactor(pDevice);
    float masterVolume = getMasterVolumeLevel(pDevice);

    hr = pDevice->Activate(__uuidof(IAudioClient3), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr)) {
        printf("Unable to activate audio client: %x\n", hr);
        return hr;
    }

    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) {
        printf("Unable to get mix format: %x\n", hr);
        return hr;
    }

    printf("Sample rate: %d Hz\n", pwfx->nSamplesPerSec);

    AudioClientProperties props = {};
    props.cbSize = sizeof(props);
    props.eCategory = AudioCategory_Other;
    props.Options = AUDCLNT_STREAMOPTIONS_RAW;
    hr = pAudioClient->SetClientProperties(&props);
    if (FAILED(hr)) {
        printf("Unable to set client properties: %x\n", hr);
        return hr;
    }

    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        hnsRequestedDuration,
        0,
        pwfx,
        NULL
    );

    if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
        UINT32 nFrames;
        hr = pAudioClient->GetBufferSize(&nFrames);
        if (FAILED(hr)) {
            printf("Unable to get buffer size: %x\n", hr);
            return hr;
        }

        hnsRequestedDuration = (REFERENCE_TIME)((10000.0 * 1000 / pwfx->nSamplesPerSec * nFrames) + 0.5);

        pAudioClient->Release();
        CoTaskMemFree(pwfx);

        hr = pDevice->Activate(__uuidof(IAudioClient3), CLSCTX_ALL, NULL, (void**)&pAudioClient);
        if (FAILED(hr)) {
            printf("Unable to re-activate audio client: %x\n", hr);
            return hr;
        }

        hr = pAudioClient->GetMixFormat(&pwfx);
        if (FAILED(hr)) {
            printf("Unable to get mix format after re-activation: %x\n", hr);
            return hr;
        }

        hr = pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
            hnsRequestedDuration,
            0,
            pwfx,
            NULL
        );
        if (FAILED(hr)) {
            printf("Unable to initialize audio client with aligned buffer size: %x\n", hr);
            return hr;
        }
    }

    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    if (FAILED(hr)) {
        printf("Unable to get capture client: %x\n", hr);
        return hr;
    }

    HANDLE hCaptureEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hCaptureEvent == NULL) {
        printf("Unable to create capture event handle\n");
        return E_FAIL;
    }

    hr = pAudioClient->SetEventHandle(hCaptureEvent);
    if (FAILED(hr)) {
        printf("Unable to set event handle: %x\n", hr);
        return hr;
    }

    hr = pAudioClient->Start();
    if (FAILED(hr)) {
        printf("Unable to start audio client: %x\n", hr);
        return hr;
    }

    bool signalDetected = false;
    auto signalStart = std::chrono::high_resolution_clock::now();
    auto lastSignalEnd = std::chrono::high_resolution_clock::now();
    long long duration = 0;
    std::string currentWord;

    while (!stopThreads) {
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) {
            printf("Unable to get next packet size: %x\n", hr);
            break;
        }

        if (packetLength > 0) {
            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
            if (FAILED(hr)) {
                printf("Unable to get buffer: %x\n", hr);
                break;
            }

            processAudioData((const float*)pData, numFramesAvailable, signalDetected, signalStart, duration, *audioWindow, normalizationFactor, masterVolume);

            if (!signalDetected && duration > 0.0f) {
                if (15 <= duration && duration <= dotWait) {
                    currentWord += '.';
                }
                else if (dotWait < duration && duration <= dashWait) {
                    currentWord += '-';
                }

                duration = 0.0f;
                signalStart = std::chrono::high_resolution_clock::now();
                lastSignalEnd = std::chrono::high_resolution_clock::now();
            }
            else if (!signalDetected) {
                if (std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::high_resolution_clock::now()) - lastSignalEnd).count() >= (spaceWait)) {
                    if (!currentWord.empty()) {
                        char letter = morseToAlphabet(currentWord);
                        std::cout << letter;
                        currentWord.clear();
                    }
                    std::cout << ' ';
                    lastSignalEnd = std::chrono::high_resolution_clock::now();
                }

                signalStart = std::chrono::high_resolution_clock::now();
            }

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            if (FAILED(hr)) {
                printf("Unable to release buffer: %x\n", hr);
                break;
            }

            std::this_thread::sleep_for(std::chrono::nanoseconds(500));
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

int main()
{
    signal(SIGINT, signalShutdown);

    char* userInput = new char[200];
    std::cout << "Enter Message In English To Convert Into Morse: \n";
    std::cin.getline(userInput, 200);

    char* morseUserInput = alphabetToMorse(userInput);
    if (!morseUserInput) {
        std::cerr << "Error converting input to Morse code." << std::endl;
        delete[] userInput;
        return 1;
    }

    std::cout << morseUserInput << std::endl;

    WINDOW_AUDIOWAVES audioWindow(800, 800);

    WAVEFORMATEX wfx;
    std::thread captureThread(CaptureAudio, &wfx, &audioWindow);

    std::this_thread::sleep_for(std::chrono::milliseconds(7500));

    playMorseSound(morseUserInput);

    captureThread.join();

    delete[] userInput;
    delete[] morseUserInput;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return 0;
}
