#include "HelperFunctions.h"
#include <iostream>
#include <bitset>
#include <array>
#include <vector>

using namespace std;

class SerialMessageParser {
private:
    typedef bitset<8> BYTE;

    static const uint8_t numHeaderBytes = 5; // 5 bytes = 1 message ID + 3 reserved + 1 separator
    static const uint8_t startFlag = 0xFF; // 1 byte
    static const uint8_t separatorFlag = 0xFE; // 1 byte
    static const uint8_t numPayloadUnitBytes = 5; // 5 bytes = 4 + 1 separator
    static const uint8_t numChecksumBytesWithSeparator = 5; // 5 bytes

    typedef array<BYTE, numPayloadUnitBytes-1> mPayloadUnit; // without separator

    array<BYTE, numHeaderBytes> mHeader; // message header
    array<BYTE, numChecksumBytesWithSeparator> mChecksum; // message checksum
    
    vector<mPayloadUnit> mPayloadVector;

    // this function reads the byte in hex from user input
    BYTE ReadByteFromUser()
    {
        uint16_t userInputInHex;
        cout << "Enter a byte in hex (ff): ";
        cin >> hex >> userInputInHex;

        bitset<8> userInputByte { userInputInHex }; // in 8 bit binary
        return userInputByte;
    }

    void DetectStartFlagPattern()
    {
        uint8_t startByteCount = 0;

        while (startByteCount < 5)
        {
            BYTE byteRead = ReadByteFromUser();

            if (byteRead == startFlag) // 0xFF
                ++startByteCount;
            else
                startByteCount = 0;
        }
        cout << "Start Pattern Detected" << endl;
    }

    bool GetMessageHeader()
    {
        for (size_t i = 0; i < numHeaderBytes; ++i) // populate header from user input
        {
            mHeader[i] = ReadByteFromUser();
        }

        return (mHeader.back() == separatorFlag); // makes sure the separator is present

        //cout << "Message Header: " << endl;

        //for (size_t i = 0; i < headerSize; ++i)
        //{
        //    cout << hex << tempHeader[i].to_ulong() << endl;
        //}
        //cout << endl;
    }

    bool GetMessageChecksum()
    {
        mChecksum = {};
        for (size_t i = 0; i < numChecksumBytesWithSeparator; ++i)
        {
            mChecksum[i] = ReadByteFromUser();
        }

        return (mChecksum.back() == separatorFlag); // makes sure the separator is present
    }

    mPayloadUnit GetMessagePayloadUnit()
    {
        
        array<BYTE, numPayloadUnitBytes> newMessage; // raw message from user with separator

        mPayloadUnit messageContent = {};

        for (size_t i = 0; i < numPayloadUnitBytes; ++i)
        {
            newMessage[i] = ReadByteFromUser();
        }

        if (newMessage.back() != separatorFlag) // incorrect message as 0XFE not there so get the message again
        {
            messageContent = GetMessagePayloadUnit();
        }

        for (size_t i = 0; i < numPayloadUnitBytes-1; ++i) // copy it in the messageContent and discard the separator byte
        {
            messageContent[i] = newMessage[i];
        }

        return messageContent;
    }

    string Get32BitStringFrom4ByteArray(array<BYTE, 4>& byteArray)
    {
        string bitString = "";
        for (size_t i = 0; i < 4; ++i)
        {
            bitString += byteArray[i].to_string();
        }
        return bitString;
    }

    bitset<32> CalculateChecksumFromPayload()
    {
        bitset<32> checksumResult{ 0x00 };

        // convert header to 32 bit representation
        array<BYTE, 4> simpleHeader = { mHeader[0], mHeader[1], mHeader[2], mHeader[3]};
        bitset<32> checksumHeader{ Get32BitStringFrom4ByteArray(simpleHeader)};

        cout << "Header checksum: " << checksumHeader.to_ulong() << endl;

        bitset<32> checksumMessage{ Get32BitStringFrom4ByteArray(mPayloadVector[0]) };
        for (size_t i = 1; i < mPayloadVector.size(); ++i)
        {
            bitset<32> temp{ Get32BitStringFrom4ByteArray(mPayloadVector[i]) };
            checksumMessage = checksumMessage ^ temp;
        }
        cout << "Message checksum: " << checksumMessage.to_ulong() << endl;

        checksumResult = checksumHeader ^ checksumMessage;

        return checksumResult;
    }


public:
    ~SerialMessageParser() {} // destructor

    SerialMessageParser() // constructor
    {
        mHeader = {}; // clean up the header
        mChecksum = {}; // clean up the checksum
        mPayloadVector = {}; // clean up the message payload unit vector aka message content
    }

    void DiscardMessage()
    {
        cout << "Message Discarded" << endl;
        mHeader = {}; // clean up the header
        mChecksum = {}; // clean up the checksum
        mPayloadVector = {}; // clean up the message payload unit vector aka message content
    }

    void ParseMessage()
    {
        // first detect start pattern by continously asking the user
        cout << "Input STARTFLAG (5 bytes)" << endl;
        DetectStartFlagPattern();

        // then get header and proceed if valid separator exists
        cout << "Input HEADER (5 bytes)" << endl;
        if (GetMessageHeader() == false)
        {
            DiscardMessage();
            return;
        }

        // note: messageSize is the number of bytes to read for payload unit (5 bytes) + 4 bytes of checksum + separator
        size_t messageSize = get_message_size_from_message_id((int)this->mHeader[0].to_ulong());
        size_t numRepeatedPayloads = (messageSize - numChecksumBytesWithSeparator) / numPayloadUnitBytes;

        mPayloadVector.resize(numRepeatedPayloads);

        for (size_t i = 0; i < mPayloadVector.size(); ++i)
        {
            cout << "Input MESSAGE PAYLOAD UNIT # " << (i + 1) << endl;
            mPayloadVector[i] = GetMessagePayloadUnit();
        }

        // then get checksum and proceed if valid separator exists
        cout << "Input CHECKSUM (5 bytes)" << endl;
        if (GetMessageChecksum() == false)
        {
            DiscardMessage();
            return;
        }

        // calculate checksum from message header and message payload unit content
        bitset<32> computedChecksumResult = CalculateChecksumFromPayload();

        // parse the checksum content into 32 bit 
        array<BYTE, 4> simpleChecksumMesssage{ mChecksum[0], mChecksum[1], mChecksum[2], mChecksum[3]};
        bitset<32> receivedChecksum{ Get32BitStringFrom4ByteArray(simpleChecksumMesssage)};

        // verify it against received checksum inside the message
        bool checksumValid = (computedChecksumResult == receivedChecksum);

        if (checksumValid == false) // checksum mismatch throw away the message
        {
            cout << "Invalid checksum" << endl;
            DiscardMessage();
            return;
        }
        else
        {
            cout << "Message SUCCESSFULLY PARSED" << endl;
            cout << "Calling process_message() with message_id: " << mHeader[0].to_ulong() << endl;
        }
    }
};

int main()
{
    SerialMessageParser myParserInstance;
    myParserInstance.ParseMessage();

    return 0;
}