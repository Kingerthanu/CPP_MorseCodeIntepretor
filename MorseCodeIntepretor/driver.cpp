#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <chrono>
#include <thread>
#include "Vertex_Array.h"
#include "shader.h"
#include <mutex>
#include "glm/glm.hpp"


// How Sensitive We Want Our Morse To Start To Be Detected At (Will Detect Any Sound Aswell Simply Above Threshold)
const float THRESHOLD = 0.15f;

// Morse Timing Constants
static const unsigned int dotWait = 80;                 // 70ms Wait Dot Single Unit/Dot Wait Time
static const unsigned int dashWait = dotWait * 3;       // 3 * Single Unit Wait Time
static const unsigned int spaceWait = dashWait;         // 3 * Single Unit Wait Time (Two Spaces Between Words, 6 Units Of Wait Time)
std::atomic<bool> stopThreads(false);





class WINDOW_AUDIOWAVES
{
    private:
        GLuint VAO;
        GLuint VBO;
        GLFWwindow* _WINDOW;
        Shader contextShader;
        std::mutex contextWand;

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

            glfwSetFramebufferSizeCallback(this->_WINDOW, resize_callback);
            this->contextShader = Shader("default.vert", "default.frag");
            this->contextShader.Activate();

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(this->_WINDOW);
        
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

        void RenderDiscrete(const float* audioData, const unsigned int& length)
        {

            // Lock the mutex to synchronize access to OpenGL context
            std::lock_guard<std::mutex> lock(contextWand);

            // Make the window's OpenGL context current
            glfwMakeContextCurrent(this->_WINDOW);

            // Clear the color buffer
            glClear(GL_COLOR_BUFFER_BIT);

            // Normalize the data
            std::vector<Vertex> normalizedBuffer;
            normalizedBuffer.reserve(length);

            float stepIncrement = 2.0f / length;
            float step = -1.0f;

            for (unsigned int i = 0; i < length; ++i, step += stepIncrement) 
            {
                normalizedBuffer.push_back(Vertex{ glm::vec2(step, audioData[i] * 0.75f), glm::vec3(0.76f, 0.2f, 0.0f) });
            }

            // Bind VAO and VBO
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            // Update buffer data using glBufferData with GL_DYNAMIC_DRAW
            glBufferData(GL_ARRAY_BUFFER, normalizedBuffer.size() * sizeof(Vertex), normalizedBuffer.data(), GL_DYNAMIC_DRAW);

            // Draw all lines
            glDrawArrays(GL_LINE_STRIP, 0, normalizedBuffer.size());

            // Swap the front and back buffers
            glfwSwapBuffers(this->_WINDOW);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            glfwMakeContextCurrent(nullptr);
        }

        static void resize_callback(GLFWwindow* window, int width, int height)
        {
            glViewport(0, 0, width, height);
        }

        // Destructor
        ~WINDOW_AUDIOWAVES()
        {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glfwDestroyWindow(this->_WINDOW);
        }
};





// Preconditions:
//   1.) chunkSize Is The Amount Of Characters In toInsert's Buffer
//   2.) oldSize Is The Amount Of Characters In toEnlargen's Buffer
// Postconditions:
//   1.) Will Return A New List In toEnlargen's Ptr Holding All toEnlargen's Characters + toInsert Appended At The End
//   2.) oldSize Will Be Updated To The New Size Of toEnlargen
void enlargeList(char*& toEnlargen, unsigned int& oldSize, const char* toInsert, const unsigned int& chunkSize)
{

    unsigned int newSize = oldSize + chunkSize;
    char* tmpHandler = toEnlargen;
    toEnlargen = new char[newSize];

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

    oldSize = newSize;

}

// Preconditions:
//   1.) Morse Code Language: '.' -> short beep   |   '-' -> long beep
//   2.) Expects Input To Be Purely Alphabetic
//   3.) Spaces And Other Non-Alphabetic Chars Will Be Set To ' '
//   4.) Each Letter Appends ' ' At End For End-Of-Char In Morse
//  Postconditions:
//   1.) Returns New Char Buffer Holding Morse Code Conversion Of toConvert
//   2.) '\0' Is Added At End Of Char Buffer (C-Style String)
char* alphabetToMorse(char*& toConvert)
{

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

    for (unsigned int cIndex = 0; cIndex < messageLength; cIndex++)
    {

        switch (toConvert[cIndex])
        {
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
        case '1':
            enlargeList(morseBuffer, listSize, ".---- ", 6);
            break;
        case '2':
            enlargeList(morseBuffer, listSize, "..--- ", 6);
            break;
        case '3':
            enlargeList(morseBuffer, listSize, "...-- ", 6);
            break;
        case '4':
            enlargeList(morseBuffer, listSize, "....- ", 6);
            break;
        case '5':
            enlargeList(morseBuffer, listSize, "..... ", 6);
            break;
        case '6':
            enlargeList(morseBuffer, listSize, "-.... ", 6);
            break;
        case '7':
            enlargeList(morseBuffer, listSize, "--... ", 6);
            break;
        case '8':
            enlargeList(morseBuffer, listSize, "---.. ", 6);
            break;
        case '9':
            enlargeList(morseBuffer, listSize, "----. ", 6);
            break;
        case '0':
            enlargeList(morseBuffer, listSize, "----- ", 6);
            break;
        default:
            enlargeList(morseBuffer, listSize, " ", 1);
            break;
        }

    }

    enlargeList(morseBuffer, listSize, "\0", 1);

    return morseBuffer;

}


// Preconditions:
//   1.) Morse Code Language: '.' -> short beep   |   '-' -> long beep
//   2.) Expects Input To Be Purely Morse Code
//   3.) Spaces And Other Non-Morse Chars Will Be Set To ' '
//  Postconditions:
//   1.) Returns New Char Buffer Holding Morse Conversion Of morse
//   2.) '\0' Is Added At End Of Char Buffer (C-Style String)
char morseToAlphabet(const std::string& morse)
{

    switch (morse.length())
    {
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
    case 5: // Handling Numbers
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


// Preconditions:
//   1.) Morse Code Language: '.' -> short beep   |   '-' -> long beep | ' ' -> long wait 
//   2.) Will Ignore Any Non-Morse Characters In morseCode
// Postconditions:
//   1.) Will Call Windows Beep Function In Which Will Sound Each Character Concurrently For Their Duration
//   2.) Will Stop If Reached End Of Morse Code
//   3.) Will Wait 1/4 The Given Single Unit Time Between Beeps To Synchronize
void playMorseSound(const char* morseCode)
{

    while (*morseCode != '\0' && !stopThreads)
    {
        switch (*morseCode)
        {
        case '.':
            Beep(1000, dotWait);
            break;
        case '-':
            Beep(950, dashWait);
            break;
        case ' ':
            Beep(0, spaceWait);
            break;
        }
        morseCode++;
        std::this_thread::sleep_for(std::chrono::milliseconds(dotWait / 4));  // Only Execute Every 1/4th Of Lowest Beep Time (Under Communication For Either Type But Doesn't Allow Compound Beeps)

    }

}

// Preconditions:
//   1.) Will Grab Float-Sound Input In data With Amount Of Samples In length
//   2.) Will Use duration To Help Callee Disabiguate Type Of Morse Code From Duration Of Sound
//   3.) signalStart Will Be Made From Callee After First Detection In This Function Using signalDetected Callback
// Postconditions:
//   1.) Sets duration Of Continuous Audio Output
//   2.) Sets signalDetected Showing If We Are Still In A Multi-Packet Signal
void processAudioData(const float* data, size_t length, bool& signalDetected, std::chrono::high_resolution_clock::time_point& signalStart, long long& duration, WINDOW_AUDIOWAVES& audioWindow)
{

    std::thread([&audioWindow, data, length]() {
        audioWindow.RenderDiscrete(data, length);
        }).detach();
    for (size_t i = 0; i < length; ++i)
    {
        if (fabs(data[i]) > THRESHOLD)
        {
            if (!signalDetected) {
                signalDetected = true;
            }
        }
        else
        {
            if (signalDetected)
            {
                auto now = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - signalStart).count();
                signalDetected = false;
            }
        }
    }

}

// Reference Times In 100-Nanoseconds
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000


// Preconditions:
//   1.) Listens To Audio Output For Sound Samples Above A Given Threshold, Interpolating Length Of Message For Morse Type
//   2.) Ignores Any Morse Previously Said In Buffer Before Opening
// Postconditions:
//   1.) Prints To Console The Interpreted Morse Code Character Translation Of Audio Output On System
HRESULT CaptureAudio(WINDOW_AUDIOWAVES* audioWindow)
{

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
    if (FAILED(hr))
    {
        printf("Unable to initialize COM library: %x\n", hr);
        return hr;
    }

    // Get the default audio device
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr))
    {
        printf("Unable to get default audio device: %x\n", hr);
        return hr;
    }

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr))
    {
        printf("Unable to get default audio endpoint: %x\n", hr);
        return hr;
    }

    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr))
    {
        printf("Unable to activate audio client: %x\n", hr);
        return hr;
    }

    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr))
    {
        printf("Unable to get mix format: %x\n", hr);
        return hr;
    }

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, hnsRequestedDuration, 0, pwfx, NULL);
    if (FAILED(hr))
    {
        printf("Unable to initialize audio client: %x\n", hr);
        return hr;
    }

    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    if (FAILED(hr))
    {
        printf("Unable to get capture client: %x\n", hr);
        return hr;
    }

    // Clear the audio buffer
    while (pCaptureClient->GetNextPacketSize(&packetLength) == S_OK && packetLength > 0)
    {
        hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
        if (FAILED(hr))
        {
            printf("Unable to get buffer: %x\n", hr);
            return hr;
        }
        hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
        if (FAILED(hr))
        {
            printf("Unable to release buffer: %x\n", hr);
            return hr;
        }
    }

    hr = pAudioClient->Start();
    if (FAILED(hr))
    {
        printf("Unable to start audio client: %x\n", hr);
        return hr;
    }

    bool signalDetected = false;
    auto signalStart = std::chrono::high_resolution_clock::now();
    auto lastLetter = std::chrono::high_resolution_clock::now();
    long long duration = 0;
    std::string currentWord;

    while (!stopThreads)
    {
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr))
        {
            printf("Unable to get next packet size: %x\n", hr);
            break;
        }
        
        if (packetLength > 0)
        {
            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
            if (FAILED(hr))
            {
                printf("Unable to get buffer: %x\n", hr);
                break;
            }
    
            // Process the audio data
            processAudioData((const float*)pData, numFramesAvailable, signalDetected, signalStart, duration, *audioWindow);

            if (!signalDetected && duration > 0)
            {
                // Output detected Morse code duration
                if (5 <= duration && duration <= dotWait)
                {
                    currentWord += '.';
                }
                else if (duration > dotWait)
                {
                    currentWord += '-';
                }
                duration = 0; // Reset duration after printing
                signalStart = std::chrono::high_resolution_clock::now();
                lastLetter = std::chrono::high_resolution_clock::now();

            }
            else if (duration <= 0)
            {
                if (std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::high_resolution_clock::now()) - lastLetter).count() >= (spaceWait))
                {

                    if (!currentWord.empty())
                    {
                        char letter = morseToAlphabet(currentWord);
                        std::cout << letter;
                        currentWord.clear();
                    }

                    std::cout << ' ';
                    lastLetter = std::chrono::high_resolution_clock::now();
                }

                signalStart = std::chrono::high_resolution_clock::now();

            }

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            if (FAILED(hr))
            {
                printf("Unable to release buffer: %x\n", hr);
                break;
            }

        }

        std::this_thread::sleep_for(std::chrono::nanoseconds(500));

    }

    hr = pAudioClient->Stop();
    if (FAILED(hr))
    {
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

    // Grab User-Message
    char* userInput = new char[400];
    std::cout << "Enter Message In English To Convert Into Morse: \n";
    std::cin.getline(userInput, 400);

    // Convert User-Message -> User-Morse
    char* morseUserInput = alphabetToMorse(userInput);
    if (!morseUserInput) {
        std::cerr << "Error converting input to Morse code." << std::endl;
        delete[] userInput;
        return 1;
    }

    // Print The Morse Code Interpretation Of User's Message
    std::cout << morseUserInput << std::endl;

    WINDOW_AUDIOWAVES audioWindow(400, 400);
    

    // Launch Off A Thread To Listen To The Current Audio Output Of The System
    std::thread captureThread(CaptureAudio, &audioWindow);
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));  // Ensure The Capture Thread Starts First

    // Now Play Our Noise After Listener Is Ready
    playMorseSound(morseUserInput);

    // After Noise, Wait To Join Our Listening Thread Before Closing
    captureThread.join();
    
    delete[] userInput;
    delete[] morseUserInput;

    return 0;
}