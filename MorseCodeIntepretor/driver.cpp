#include <iostream>
#include <windows.h>


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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Tempo Settings ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Wait 200ms Between Individual Units
static const float dotWait = 100.0f;

// Wait 3 Unit-Long Beep For Dashes
static const float dashWait = (dotWait * 3.0f);

// Wait 7 Unit-Long Beep For Spaces
static const float spaceWait = dashWait;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

char* alphabetToMorseBinary(char*& toConvert)
{
    int messageLength = 0;

    // Manually count the characters until the null terminator
    while (toConvert[messageLength] != '\0')
    {
        messageLength++;
    }

    char* morseBuffer = new char[0];

    // If Empty Leave Early As Dont Need To Add Anything
    if (messageLength == 0)
    {
        return morseBuffer;
    }

    unsigned int listSize = 0;

    // Convert each character
    for (unsigned int cIndex = 0; cIndex < messageLength; cIndex++)
    {

        // . => 1   |   - => 000
        switch (toConvert[cIndex])
        {
            case 'A':
            case 'a':
                enlargeList(morseBuffer, listSize, "1000 ", 5);
                break;
            case 'B':
            case 'b':
                enlargeList(morseBuffer, listSize, "000111 ", 7);
                break;
            case 'C':
            case 'c':
                enlargeList(morseBuffer, listSize, "00010001 ", 9);
                break;
            case 'D':
            case 'd':
                enlargeList(morseBuffer, listSize, "00011 ", 6);
                break;
            case 'E':
            case 'e':
                enlargeList(morseBuffer, listSize, "1 ", 2);
                break;
            case 'F':
            case 'f':
                enlargeList(morseBuffer, listSize, "110001 ", 7);
                break;
            case 'G':
            case 'g':
                enlargeList(morseBuffer, listSize, "0000001 ", 8);
                break;
            case 'H':
            case 'h':
                enlargeList(morseBuffer, listSize, "1111 ", 5);
                break;
            case 'I':
            case 'i':
                enlargeList(morseBuffer, listSize, "11 ", 3);
                break;
            case 'J':
            case 'j':
                enlargeList(morseBuffer, listSize, "10000000 ", 9);
                break;
            case 'K':
            case 'k':
                enlargeList(morseBuffer, listSize, "000100 ", 7);
                break;
            case 'L':
            case 'l':
                enlargeList(morseBuffer, listSize, "100011 ", 7);
                break;
            case 'M':
            case 'm':
                enlargeList(morseBuffer, listSize, "0000 ", 5);
                break;
            case 'N':
            case 'n':
                enlargeList(morseBuffer, listSize, "0001 ", 5);
                break;
            case 'O':
            case 'o':
                enlargeList(morseBuffer, listSize, "0000000 ", 8);
                break;
            case 'P':
            case 'p':
                enlargeList(morseBuffer, listSize, "1000001 ", 8);
                break;
            case 'Q':
            case 'q':
                enlargeList(morseBuffer, listSize, "000001000 ", 10);
                break;
            case 'R':
            case 'r':
                enlargeList(morseBuffer, listSize, "10001 ", 6);
                break;
            case 'S':
            case 's':
                enlargeList(morseBuffer, listSize, "111 ", 4);
                break;
            case 'T':
            case 't':
                enlargeList(morseBuffer, listSize, "000 ", 5);
                break;
            case 'U':
            case 'u':
                enlargeList(morseBuffer, listSize, "1100 ", 5);
                break;
            case 'V':
            case 'v':
                enlargeList(morseBuffer, listSize, "111000 ", 7);
                break;
            case 'W':
            case 'w':
                enlargeList(morseBuffer, listSize, "100000 ", 7);
                break;
            case 'X':
            case 'x':
                enlargeList(morseBuffer, listSize, "00011000 ", 9);
                break;
            case 'Y':
            case 'y':
                enlargeList(morseBuffer, listSize, "00010000 ", 9);
                break;
            case 'Z':
            case 'z':
                enlargeList(morseBuffer, listSize, "00000011 ", 9);
                break;
            default:
                // Handle other characters or do nothing
                break;
        }
    }

    enlargeList(morseBuffer, listSize, "\0", 1);

    return morseBuffer;

};

char* alphabetToMorse(char*& toConvert)
{

    int messageLength = 0;

    // Manually count the characters until the null terminator
    while (toConvert[messageLength] != '\0')
    {
        messageLength++;
    }

    // If empty, leave early as there's nothing to convert
    if (messageLength == 0)
    {
        return nullptr;
    }

    char* morseBuffer = new char[0];
    unsigned int listSize = 0;

    // Convert each character
    for (unsigned int cIndex = 0; cIndex < messageLength; cIndex++)
    {
        // . => .   |   - => -
        switch (toConvert[cIndex])
        {
        case 'A':
        case 'a':
            enlargeList(morseBuffer, listSize, ".- ", 3);
            break;
        case 'B':
        case 'b':
            enlargeList(morseBuffer, listSize, "-... ", 5);
            break;
        case 'C':
        case 'c':
            enlargeList(morseBuffer, listSize, "-.-. ", 5);
            break;
        case 'D':
        case 'd':
            enlargeList(morseBuffer, listSize, "-.. ", 4);
            break;
        case 'E':
        case 'e':
            enlargeList(morseBuffer, listSize, ". ", 2);
            break;
        case 'F':
        case 'f':
            enlargeList(morseBuffer, listSize, "..-. ", 5);
            break;
        case 'G':
        case 'g':
            enlargeList(morseBuffer, listSize, "--. ", 4);
            break;
        case 'H':
        case 'h':
            enlargeList(morseBuffer, listSize, ".... ", 5);
            break;
        case 'I':
        case 'i':
            enlargeList(morseBuffer, listSize, ".. ", 3);
            break;
        case 'J':
        case 'j':
            enlargeList(morseBuffer, listSize, ".--- ", 5);
            break;
        case 'K':
        case 'k':
            enlargeList(morseBuffer, listSize, "-.- ", 4);
            break;
        case 'L':
        case 'l':
            enlargeList(morseBuffer, listSize, ".-.. ", 5);
            break;
        case 'M':
        case 'm':
            enlargeList(morseBuffer, listSize, "-- ", 3);
            break;
        case 'N':
        case 'n':
            enlargeList(morseBuffer, listSize, "-. ", 3);
            break;
        case 'O':
        case 'o':
            enlargeList(morseBuffer, listSize, "--- ", 4);
            break;
        case 'P':
        case 'p':
            enlargeList(morseBuffer, listSize, ".--. ", 5);
            break;
        case 'Q':
        case 'q':
            enlargeList(morseBuffer, listSize, "--.- ", 5);
            break;
        case 'R':
        case 'r':
            enlargeList(morseBuffer, listSize, ".-. ", 4);
            break;
        case 'S':
        case 's':
            enlargeList(morseBuffer, listSize, "... ", 4);
            break;
        case 'T':
        case 't':
            enlargeList(morseBuffer, listSize, "- ", 2);
            break;
        case 'U':
        case 'u':
            enlargeList(morseBuffer, listSize, "..- ", 4);
            break;
        case 'V':
        case 'v':
            enlargeList(morseBuffer, listSize, "...- ", 5);
            break;
        case 'W':
        case 'w':
            enlargeList(morseBuffer, listSize, ".-- ", 4);
            break;
        case 'X':
        case 'x':
            enlargeList(morseBuffer, listSize, "-..- ", 5);
            break;
        case 'Y':
        case 'y':
            enlargeList(morseBuffer, listSize, "-.-- ", 5);
            break;
        case 'Z':
        case 'z':
            enlargeList(morseBuffer, listSize, "--.. ", 5);
            break;
        default:
            enlargeList(morseBuffer, listSize, " ", 1);
            break;
        }
    }

    enlargeList(morseBuffer, listSize, "\0", 1);

    return morseBuffer;

};


void playMorseSound(const char* morseCode)
{

    while (*morseCode != '\0')
    {
        switch (*morseCode)
        {
            case '.':
                Beep(1000, dotWait);  // Dot: 1000 Hz for Single Unit Of Time
                break;
            case '-':
                Beep(998, dashWait);  // Dash: 998 Hz For Three Units Of Time
                break;
            case ' ':
                Sleep(spaceWait);  // Space: Pause For Three Units Of Time (Double Space Will Give Space 6 Units Of Time)
                break;
        }
        morseCode++;
    }

};


int main()
{

    char* userInput = new char[100];
    
    std::cout << "Enter Message In English To Convert Into Morse: \n";
	std::cin.getline(userInput, 100);

    char* morseUserInput = alphabetToMorse(userInput);
    std::cout << morseUserInput << std::endl;

    playMorseSound(morseUserInput);

}