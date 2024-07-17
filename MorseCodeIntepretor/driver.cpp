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

// Tell During Compile-Time For Compiler To Swap Defs Out With Literal
#define PI 3.141592653589793238

/*
   How Sensitive We Want Our Morse To Start To Be Detected At (Needs To Be Adjusted Based Upon Volume)
   Lower Value -> Audio Of Lower Frequencies Considered [Drop When Lower Background Noise] |  Higher Value -> Audio Of Higher Frequencies Considered [Up When Higher Background Noise]
*/
#define THRESHOLD 0.4
#define REFTIMES_PER_SEC  10000000

// Morse Timing Constants
static const unsigned int dotWait = 70;                 // 70ms Wait Dot Single Unit/Dot Wait Time
static const unsigned int dashWait = dotWait * 3;       // 3 * Single Unit Wait Time
static const unsigned int spaceWait = dashWait;         // 3 * Single Unit Wait Time (Two Spaces Between Words, 6 Units Of Wait Time)
std::atomic<bool> stopThreads(false);


// Preconditions:
//   1.) chunkSize Is The Amount Of Characters In toInsert's Buffer
//   2.) oldSize Is The Amount Of Characters In toEnlargen's Buffer
// Postconditions:
//   1.) Will Return A New List In toEnlargen's Ptr Holding All toEnlargen's Characters + toInsert Appended At The End
//   2.) oldSize Will Be Updated To The New Size Of toEnlargen
void enlargeList(char*& toEnlargen, unsigned int& oldSize, const char* toInsert, const unsigned int& chunkSize) 
{

    // Pre-Load The New Final Size Of Our List
    unsigned int newSize = oldSize + chunkSize;

    // Create A Handler For A Shallow-Copy Of The Data In toEnlargen
    char* tmpHandler = toEnlargen;
    toEnlargen = new char[newSize];

    // Character Step Is For Shallow Copy; insertStep Is For The New Entries Being Added
    unsigned int characterStep = 0, insertStep = 0;
    while (characterStep < oldSize) 
    {
        toEnlargen[characterStep] = tmpHandler[characterStep++];
    }
    delete[] tmpHandler;
    while (insertStep < chunkSize) 
    {
        toEnlargen[characterStep + insertStep] = toInsert[insertStep++];
    }

    // Our Old Size Variable-Reference Is Now The New Size
    oldSize = newSize;

}

// Preconditions:
//   1.) Morse Code Language: '.' -> short beep   |   '-' -> long beep
//   2.) Expects Input To Be Purely Morse Code
//   3.) Spaces And Other Non-Morse Chars Will Be Set To ' '
// Postconditions:
//   1.) Returns New Char Buffer Holding Morse Conversion Of morse
//   2.) '\0' Is Added At End Of Char Buffer (C-Style String)
char morseToAlphabet(const std::string& morse) 
{

    // Check The Morse Code Message's Length
    switch (morse.length()) 
    {
        case 1:
            if (morse == ".") return 'E';
            else if (morse == "-") return 'T';
            break;
        case 2:
            if (morse == "..") return 'I';
            else if (morse == ".-") return 'A';
            else if (morse == "-.") return 'N';
            else if (morse == "--") return 'M';
            break;
        case 3:
            if (morse == "...") return 'S';
            else if (morse == "..-") return 'U';
            else if (morse == ".-.") return 'R';
            else if (morse == ".--") return 'W';
            else if (morse == "-..") return 'D';
            else if (morse == "-.-") return 'K';
            else if (morse == "--.") return 'G';
            else if (morse == "---") return 'O';
            break;
        case 4:
            if (morse == "....") return 'H';
            else if (morse == "...-") return 'V';
            else if (morse == "..-.") return 'F';
            else if (morse == ".-..") return 'L';
            else if (morse == ".--.") return 'P';
            else if (morse == ".---") return 'J';
            else if (morse == "-...") return 'B';
            else if (morse == "-..-") return 'X';
            else if (morse == "-.-.") return 'C';
            else if (morse == "-.--") return 'Y';
            else if (morse == "--..") return 'Z';
            else if (morse == "--.-") return 'Q';
            break;
        case 5: // Handling Numbers
            if (morse == "-----") return '0';
            else if (morse == ".----") return '1';
            else if (morse == "..---") return '2';
            else if (morse == "...--") return '3';
            else if (morse == "....-") return '4';
            else if (morse == ".....") return '5';
            else if (morse == "-....") return '6';
            else if (morse == "--...") return '7';
            else if (morse == "---..") return '8';
            else if (morse == "----.") return '9';
            break;
        default:
            // If A Invalid Morse Code Message Add A Null Space
            return ' ';
    }

    // If Empty, Return Nothing.
    return '';

}


void playSineWave(double frequency, double durationMs, int sampleRate) 
{

    // Max Offset From Baseline Of 0.8f (I.E. Could Be As Low As 0.7f, And As High As 0.9f)
    const float amplitude = 0.1f;

    // Our Sample Count Will Be Based Upon The Amount Of Seconds Elapse During Our Millisecond Noise-Interval And The Amount Of Samples For This Device
    int samplesCount = static_cast<int>((durationMs / 1000.0) * sampleRate);
    float* buffer = new float[samplesCount];

    // Duration Of The Fade-Out In Milliseconds
    double fadeOutDurationMs = 10;
    int fadeOutSamplesCount = static_cast<int>((fadeOutDurationMs / 1000.0) * sampleRate);

    // Create Each Audio-Data Frame (Entry In Array)
    for (int i = 0; i < samplesCount; ++i) 
    {

        //         Baseline + Current Sine Periodicity At This Position
        buffer[i] = 0.8f + (amplitude * sin(2.0 * PI * frequency * i / sampleRate));

        // Apply More Aggressive Exponential Fade-Out Effect (If Current Entry Is The Last fadeOutDurationMs)
        if (i >= samplesCount - fadeOutSamplesCount) 
        {
            // Our Fade Out Factor Is Based Upon How Long Into The Fade-Out We Are ( End i < Start i )
            float fadeOutFactor = static_cast<float>(samplesCount - i) / fadeOutSamplesCount;
            buffer[i] *= exp(-13.0f * (1.0f - fadeOutFactor)); // More aggressive exponential falloff
        }
    }

    // Determine The Format To Transfer Our Audio Data As
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_IEEE_FLOAT; // We Are Giving A Float
    wfx.nChannels = 1; // Mono-Sound
    wfx.nSamplesPerSec = sampleRate; // At A Rate Of sampleRate
    wfx.nAvgBytesPerSec = sampleRate * sizeof(float); // Divide Amount Of Entries/Frames Per-Second
    wfx.nBlockAlign = sizeof(float); // Offset Between Entries Is A Float-Size
    wfx.wBitsPerSample = 32; // 32-Bit Float Sound

    // Using Our Format, Open Up A Session Of Audio Input
    HWAVEOUT hWaveOut;
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        std::cerr << "Error opening waveform output device." << std::endl;
        delete[] buffer;
        return;
    }

    // Set The Transfer Buffer's Traits (Buffer To Use, And It's Buffer Size Count)
    WAVEHDR waveHeader = {};
    waveHeader.lpData = reinterpret_cast<LPSTR>(buffer);
    waveHeader.dwBufferLength = samplesCount * sizeof(float);
    waveHeader.dwFlags = 0;
    waveHeader.dwLoops = 0;

    // Send Into API Our Buffer
    if (waveOutPrepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
        std::cerr << "Error preparing waveform header." << std::endl;
        waveOutClose(hWaveOut);
        delete[] buffer;
        return;
    }

    // Send Over Our Audio Buffer
    if (waveOutWrite(hWaveOut, &waveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
        std::cerr << "Error writing waveform data." << std::endl;
        waveOutUnprepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR));
        waveOutClose(hWaveOut);
        delete[] buffer;
        return;
    }

    // Wait Until Our Sound Has Concluded
    while (waveOutUnprepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Close Our Audio Session
    waveOutClose(hWaveOut);

    // Clear Helper Buffer
    delete[] buffer;

}


// Preconditions:
//   1.) Morse Code Language: '.' -> short beep   |   '-' -> long beep | ' ' -> long wait 
//   2.) Will Ignore Any Non-Morse Characters In morseCode
// Postconditions:
//   1.) Will Call Windows Beep Function In Which Will Sound Each Character Concurrently For Their Duration
//   2.) Will Stop If Reached End Of Morse Code
//   3.) Will Wait 1/4 The Given Single Unit Time Between Beeps To Synchronize
void playMorseSound(const char* morseCode) 
{

    // Go Through Each Entry In Our Morse Code And Do It's Associated Beep Time
    while (*morseCode != '\0' && !stopThreads) 
    {
        switch (*morseCode++) 
        {
            case '.':
                // 1000hz For dotWait-ms At S.R. 48000
                playSineWave(1000, dotWait, 48000);
                break;
            case '-':
                // 1000hz For dashWait-ms At S.R. 48000
                playSineWave(1000, dashWait, 48000);
                break;
            case ' ':
                // No Noise For spaceWait-ms
                std::this_thread::sleep_for(std::chrono::milliseconds(spaceWait));
                break;
        }
    }

}

// Preconditions:
//   1.) Morse Code Language: '.' -> short beep   |   '-' -> long beep
//   2.) Expects Input To Be Purely AlphaNumeric
//   3.) Spaces And Other Non-AlphaNumeric Chars Will Be Set To ' '
//   4.) Each Letter Appends ' ' At End For End-Of-Char In Morse
// Postconditions:
//   1.) Returns New Char Buffer Holding Morse Code Conversion Of toConvert
//   2.) '\0' Is Added At End Of Char Buffer (C-Style String)
char* alphabetToMorse(char*& toConvert) 
{

    // Get The Length Of Our AlphaNumeric String
    int messageLength = 0;
    while (toConvert[messageLength] != '\0') 
    {
        messageLength++;
    }

    // If Empty Input, Return A Empty Ptr
    if (messageLength == 0) 
    {
        return nullptr;
    }

    // Create Our Empty Buffer Of Size, listSize
    char* morseBuffer = new char[0];
    unsigned int listSize = 0;

    // For Each AlphaNumeric Character..
    for (unsigned int cIndex = 0; cIndex < messageLength; cIndex++) 
    {
        // Convert Into Morse-Code Standard Translation Of Character With Letter Gap Included
        switch (toConvert[cIndex]) 
        {
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

    // End Off By Adding A C-Style Termination Character
    enlargeList(morseBuffer, listSize, "\0", 1);
    return morseBuffer;

}


void signalShutdown(int) 
{
    // Shutdown By Telling All Their Mainloops To Stop
    std::cout << "Shutting Down...\n";
    stopThreads = true;
}

class WINDOW_AUDIOWAVES {
private:
    // Low-Level ID's Of Vertex Array And Vertex Buffer For Window Rendering
    GLuint VAO, VBO;
    
    // Current GLFW Window Being Draw On
    GLFWwindow* _WINDOW;
    
    // Shader Used To Render Buffer Data
    Shader contextShader;

    // Rendering Synchronization Mutex
    std::mutex contextWand;

    // Base-Line Radius Of Audio-Wave Circle
    const float _circleRadius = 0.65f;


    // Function To Generate Vertices For A Segmented Circle
    std::vector<Vertex> generateSegmentedCircle(const float& centerX, const float& centerY, const float* audioData, const UINT32& segmentCount) 
    {

        // Hold Vertices Of Current Circle
        std::vector<Vertex> vertices;

        // How Much Each Angle Is Offsetted From Eachother
        float angleStep = 2.0f * PI / segmentCount;

        // Render Each Segment
        for (UINT32 i = 0; i < segmentCount; ++i) 
        {
            // Normalize Our Audio Sample To A Variable-Const Used As Offset From Origin
            float normalizedSample = fabs(audioData[i]) * 0.75f;

            // Current Angle Of 0 Of Segment Vertex
            float angle = i * angleStep;

            // Push Back Our Vertex Based Upon Offset From Center And Baseline Offset
            vertices.push_back(Vertex{ glm::vec2(centerX + (_circleRadius + normalizedSample) * cos(angle), centerY + (_circleRadius + normalizedSample) * sin(angle)), glm::vec3(0.93f, 0.15f, 0.45f) });
        }

        // Add First Position Again To Stitch Together Difference
        vertices.push_back(Vertex{ glm::vec2(centerX + (_circleRadius + (fabs(audioData[0]) * 0.75f)), 0), glm::vec3(0.93f, 0.15f, 0.45f) });
        return vertices;

    }

public:
    WINDOW_AUDIOWAVES(const unsigned int& newWidth, const unsigned int& newHeight) 
    {

        // Initialize GLFW and create the main window
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

        // Generate and bind the VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Generate and bind the VBO
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        
        // Link vertex attributes
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(1);

        // Poll Initial Events To Avoid Blue-Circle Hover
        glfwPollEvents();
        glfwMakeContextCurrent(nullptr);

    }

    void RenderDiscrete(const float* audioData, const UINT32 length) 
    {

        // Lock the mutex to synchronize access to OpenGL context
        std::lock_guard<std::mutex> lock(contextWand);

        // Make the window's OpenGL context current
        glfwMakeContextCurrent(this->_WINDOW);

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind VAO And VBO
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // Update Buffer Data Using glBufferData With GL_DYNAMIC_DRAW
        glBufferData(GL_ARRAY_BUFFER, (length + 1) * sizeof(Vertex), generateSegmentedCircle(0.0f, 0.0f, audioData, length).data(), GL_DYNAMIC_DRAW);
        
        // Draw All Lines
        glDrawArrays(GL_LINE_STRIP, 0, (length + 1));

        // Swap the front and back buffers
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

float getNormalizationFactor(IMMDevice* pDevice) 
{

    // Set Our Fail Flag, And Information On Current Endpoints Trait's
    HRESULT hr;
    IAudioMeterInformation* pMeterInfo = NULL;
    float peakValue = 0.0f;

    // Query For The IAudioMeterInformation Of Our Current Endpoint
    hr = pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeterInfo);
    if (SUCCEEDED(hr))
    {
        hr = pMeterInfo->GetPeakValue(&peakValue);
        pMeterInfo->Release();
    }

    if (peakValue > 0.0f) 
    {
        return 1.0f / peakValue;
    }

    return 1.0f; // Return Default Normalization Factor If Unable To Calculate

}

float getMasterVolumeLevel(IMMDevice* pDevice) 
{

    // Set Our Fail Flag, And Information On Current Endpoint's Volume
    IAudioEndpointVolume* pEndpointVolume = NULL;
    HRESULT hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pEndpointVolume);
    if (FAILED(hr)) 
    {
        std::cerr << "Unable to activate endpoint volume: " << std::hex << hr << std::endl;
        return 1.0f; // Return default volume if unable to get the actual value
    }

    
    // Grab Our Audio Level [0.0f, 1.0f]
    float volumeLevel = 0.0f;
    hr = pEndpointVolume->GetMasterVolumeLevelScalar(&volumeLevel);
    if (FAILED(hr)) 
    {
        std::cerr << "Unable to get master volume level: " << std::hex << hr << std::endl;
        volumeLevel = 1.0f; // Return default volume if unable to get the actual value
    }

    // Clean-Up Our Session
    pEndpointVolume->Release();
    return volumeLevel;

}


// Preconditions:
//   1.) Will Grab Float-Sound Input In data With Amount Of Samples In length
//   2.) Will Use duration To Help Callee Disabiguate Type Of Morse Code From Duration Of Sound
//   3.) signalStart Will Be Made From Callee After First Detection In This Function Using signalDetected Callback
// Postconditions:
//   1.) Sets duration Of Continuous Audio Output
//   2.) Sets signalDetected Showing If We Are Still In A Multi-Packet Signal
void processAudioData(const float* data, UINT32& length, bool& signalDetected, std::chrono::high_resolution_clock::time_point& signalStart, long long& duration, WINDOW_AUDIOWAVES& audioWindow, float normalizationFactor, float masterVolume) 
{

    // Scale Our Threshold Based On The Current Max Volume; Fast Sensitivity-Decay If Lower Volume
    float scaledThreshold =( (THRESHOLD * masterVolume) / (4.575f - (masterVolume * 3.575f)));
    //std::cout << scaledThreshold << '\n';
    
    // Launch Off Worker Thread To Render Current Buffer Data
    std::thread([&audioWindow, data, length]() 
    {
        
        audioWindow.RenderDiscrete(data, length);

    }
    ).detach();

        // Process Each Audio Data Frame
        for (UINT32 i = 0; i < length; ++i) 
        {

            //std::cout << fabs(data[i]) << '\n';
            
            // If We Exceed The Threshold Start Considering If Morse
            if (scaledThreshold <= fabs(data[i])) 
            {
                if (!signalDetected) {
                    //std::cout << fabs(data[i]) << '\n';
                    signalDetected = true;
                    signalStart = std::chrono::high_resolution_clock::now(); 
                }
            }
            // If We Do Not Exceed The Threshold, Don't Consider..
            else 
            {
                // Unless, This Is The End Of A Current Morse Message
                if (signalDetected) 
                {
                    // Get Duration Of Audio-Data Being Above Threshold In Milliseconds
                    auto now = std::chrono::high_resolution_clock::now();
                    duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - signalStart).count();

                `   // Also State We Are Done Detecting This Current Entry
                    signalDetected = false;
                    //std::cout << "Duration: " << duration << '\n';
                    
                }
            }
        }

}


// Preconditions:
//   1.) Listens To Audio Output For Sound Samples Above A Given Threshold, Interpolating Length Of Message For Morse Type
//   2.) Ignores Any Morse Previously Said In Buffer Before Opening
// Postconditions:
//   1.) Prints To Console The Interpreted Morse Code Character Translation Of Audio Output On System
HRESULT CaptureAudio(WAVEFORMATEX* pwfx, WINDOW_AUDIOWAVES* audioWindow) 
{
    // Pre-Load All Variables For Audio Listening Session
    HRESULT hr; // Hold Return Code
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC; // Hold Amount Of Nano-Seconds Per Second We Will Be Listening At
    IMMDeviceEnumerator* pEnumerator = NULL; // Handler Of Endpoint
    IMMDevice* pDevice = NULL; // Current Playback Device
    IAudioClient3* pAudioClient = NULL; // Get Current O.S. Sessions Client To Talk To
    IAudioCaptureClient* pCaptureClient = NULL; // Get Client's Listening Session Port
    UINT32 packetLength = 0; // Overall Buffer-Size
    UINT32 numFramesAvailable; // Amount Of Frames (Indexes) In packetLength
    BYTE* pData; // Audio-Packet's Bytes
    DWORD flags; // Session-Type Flags

    // Create A COM Instance
    hr = CoInitialize(NULL);
    if (FAILED(hr)) 
    {
        printf("Unable to initialize COM library: %x\n", hr);
        return hr;
    }

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) 
    {
        printf("Unable to get default audio device: %x\n", hr);
        return hr;
    }

    // Ask For Our Endpoint/Playback Device
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) 
    {
        printf("Unable to get default audio endpoint: %x\n", hr);
        return hr;
    }

    // Grab Helper Variables To Determine Max Float Value (I.E. Is It [0.0f, 1.0f] Or [0.0f, 0.10f])
    float normalizationFactor = getNormalizationFactor(pDevice);

    // Grab [0.0f, 1.0f] Float To Determine Master Volume To Help Scale With Lower/Higher Volumes
    float masterVolume = getMasterVolumeLevel(pDevice);

    // Create A Session With Our Client On Our Device
    hr = pDevice->Activate(__uuidof(IAudioClient3), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr)) 
    {
        printf("Unable to activate audio client: %x\n", hr);
        return hr;
    }

    // Ask For Mix Of Our Audio Client (Like Sample-Rate)
    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) {
        printf("Unable to get mix format: %x\n", hr);
        return hr;
    }

    printf("Sample rate: %d Hz\n", pwfx->nSamplesPerSec);

    // Tell Client Some Properties To Interpret Our Data Through
    AudioClientProperties props = {};
    props.cbSize = sizeof(props);
    props.eCategory = AudioCategory_Other;
    props.Options = AUDCLNT_STREAMOPTIONS_RAW;
    hr = pAudioClient->SetClientProperties(&props);
    if (FAILED(hr)) {
        printf("Unable to set client properties: %x\n", hr);
        return hr;
    }

    // Initilize A Listening Port On Our Client's Audio Session With Our Endpoint; Listening On The Endpoint For Its Inputted Audio Requests
    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        hnsRequestedDuration,
        0,
        pwfx,
        NULL
    );

    // If We Messed Up Initilization With Improper Request Duration
    if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) 
    {
        // Grab The Buffer Size
        UINT32 nFrames;
        hr = pAudioClient->GetBufferSize(&nFrames);
        if (FAILED(hr)) {
            printf("Unable to get buffer size: %x\n", hr);
            return hr;
        }

        // Based Upon Our Sample Rate And Frame Count Create A More Direct Requested Duration
        hnsRequestedDuration = (REFERENCE_TIME)((10000.0 * 1000 / pwfx->nSamplesPerSec * nFrames) + 0.5);

        pAudioClient->Release();
        CoTaskMemFree(pwfx);

        // Create A Session With Our Client On Our Device
        hr = pDevice->Activate(__uuidof(IAudioClient3), CLSCTX_ALL, NULL, (void**)&pAudioClient);
        if (FAILED(hr)) {
            printf("Unable to re-activate audio client: %x\n", hr);
            return hr;
        }

        // Ask For Mix Of Our Audio Client (Like Sample-Rate)
        hr = pAudioClient->GetMixFormat(&pwfx);
        if (FAILED(hr)) {
            printf("Unable to get mix format after re-activation: %x\n", hr);
            return hr;
        }

        // Initilize A Listening Port On Our Client's Audio Session With Our Endpoint; Listening On The Endpoint For Its Inputted Audio Requests
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

    // Grab The Listener On Our Endpoint
    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    if (FAILED(hr)) {
        printf("Unable to get capture client: %x\n", hr);
        return hr;
    }

    // Start Listening..
    hr = pAudioClient->Start();
    if (FAILED(hr)) {
        printf("Unable to start audio client: %x\n", hr);
        return hr;
    }

    // Variables For Morse-Code Detection
    bool signalDetected = false; // Running Boolean To Determine If We Are Scanning A Multi-Packet Morse-Code Signal

    // Start Of Our Morse-Code Signal
    auto signalStart = std::chrono::high_resolution_clock::now();

    // End Of Our Morse-Code Signal
    auto lastSignalEnd = std::chrono::high_resolution_clock::now();

    // Duration Of Our Morse-Code Signal
    long long duration = 0;

    // Current Word/Character From Morse Being Translated Back Into Its AlphaNumeric Character
    std::string currentWord;


    // While The User Wants To Scan
    while (!stopThreads) 
    {
        // Grab Our Current Packet Size
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) {
            printf("Unable to get next packet size: %x\n", hr);
            break;
        }

        
        if (packetLength > 0) 
        {
            // Grab Everything From Last .GetBuffer() Till Now, Adding Into pData And It's Frame-Count In numFramesAvailable
            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
            if (FAILED(hr)) {
                printf("Unable to get buffer: %x\n", hr);
                break;
            }

            // Pass Our Audio-Data Packet To Our Morse Code Algorithm
            processAudioData((const float*)pData, numFramesAvailable, signalDetected, signalStart, duration, *audioWindow, normalizationFactor, masterVolume);

            // After processAudioData(...) If Our Signal Is Done, And The Signal Was Above 0.0f Seconds
            if (!signalDetected && duration > 0.0f) 
            {
                if (15 <= duration && duration <= dotWait) 
                {
                    currentWord += '.';
                }
                else if (dotWait < duration && duration <= dashWait) 
                {
                    currentWord += '-';
                }

                duration = 0.0f;
                signalStart = std::chrono::high_resolution_clock::now();
                lastSignalEnd = std::chrono::high_resolution_clock::now();
            }
            // Else If The Duration Is Zero But Our Signal Is Done
            else if (!signalDetected) 
            {
                // If The Time During This Gap Of Time With No Noise Is spaceWait, Add A ' '
                if (std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::high_resolution_clock::now()) - lastSignalEnd).count() >= (spaceWait)) 
                {
                    // If Our Current Word Is Not Empty, Go And Reinterpret It Back Into A AlphaNumeric Medium
                    if (!currentWord.empty()) 
                    {
                        char letter = morseToAlphabet(currentWord);
                        std::cout << letter;
                        currentWord.clear();
                    }
                    // Even If No Word, Add A ' ' After
                    std::cout << ' ';

                    // Set Time Since Last Signal Printed Entry
                    lastSignalEnd = std::chrono::high_resolution_clock::now();
                }

                // If Duration Is Zero, Set signalStart To Current Time 
                signalStart = std::chrono::high_resolution_clock::now();
            }

            // Release The Audio-Buffer
            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            if (FAILED(hr)) {
                printf("Unable to release buffer: %x\n", hr);
                break;
            }

            // Sleep For 500ns To Not Over-Accelerate
            std::this_thread::sleep_for(std::chrono::nanoseconds(500));
        }
    }

    // End The Listening Session
    hr = pAudioClient->Stop();
    if (FAILED(hr)) {
        printf("Unable to stop audio client: %x\n", hr);
        return hr;
    }

    // Clean-Up COM Session
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
    // Register Signal Handler
    signal(SIGINT, signalShutdown);

    // Grab User-Message
    char* userInput = new char[200];
    std::cout << "Enter Message In English To Convert Into Morse: \n";
    std::cin.getline(userInput, 200);

    // Convert User-Message -> User-Morse
    char* morseUserInput = alphabetToMorse(userInput);
    if (!morseUserInput) {
        std::cerr << "Error converting input to Morse code." << std::endl;
        delete[] userInput;
        return 1;
    }

    // Print The Morse Code Interpretation Of User's Message
    std::cout << morseUserInput << std::endl;

    WINDOW_AUDIOWAVES audioWindow(800, 800);

    // Launch Off A Thread To Listen To The Current Audio Output Of The System
    WAVEFORMATEX wfx;
    std::thread captureThread(CaptureAudio, &wfx, &audioWindow);

    // Ensure The Capture Thread Starts First
    std::this_thread::sleep_for(std::chrono::milliseconds(7500));

    // Now Play Our Noise After Listener Is Ready
    playMorseSound(morseUserInput);

    // After Noise, Wait To Join Our Listening Thread Before Closing
    captureThread.join();

    delete[] userInput;
    delete[] morseUserInput;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return 0;
}
