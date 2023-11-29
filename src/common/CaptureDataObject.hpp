#ifndef  H__CAPTURE_DATA_OBJECT__H
#define  H__CAPTURE_DATA_OBJECT__H

#include  <memory>
#include  <cstdint>
#include  "Ensuring.hpp"

class CaptureDataObject
{
    public:
        const void* Data;

        const uint64_t SizeOfData;

        const uint64_t Length;

        CaptureDataObject(const void* const data, std::uint64_t sizeOfData, std::uint64_t length)
            : Data(data), SizeOfData(sizeOfData), Length(length)
        {
            ThrowExceptionIfNull(this->Data);
            ThrowExceptionIfZero(this->SizeOfData);
            ThrowExceptionIfZero(this->Length);
        }
};

#endif  // H__CAPTURE_DATA_OBJECT__H

