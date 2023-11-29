#ifndef  H__ICAPTURABLE__H
#define  H__ICAPTURABLE__H

#include  "CaptureDataObject.hpp"

class ICapturable
{
	public:
		virtual ~ICapturable(){}

		virtual bool Capture(const CaptureDataObject *) = 0;

        virtual uint64_t GetNBytes() = 0;

        virtual uint64_t GetLength() = 0;
};

#endif  /* H__ICAPTURABLE__H */

