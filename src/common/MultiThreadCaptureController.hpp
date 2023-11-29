#ifndef  H__MULTI_THREAD_CAPTURE_CONTROLLER__H
#define  H__MULTI_THREAD_CAPTURE_CONTROLLER__H

#include  <thread>
#include  <mutex>
#include  <condition_variable>
#include  <chrono>
#include  <cstdio>
#include  <cstdint>
#include  "ICapturable.hpp"

class MultiThreadCaptureController
{
    private:
        static constexpr int maxNumCaptureData_ = 4;
        static constexpr int notApplicatable_ = -1;

        bool isReady_;  // This will become true when initialization is success
        bool isActive_;  // This will become true when getting the order to start capture
        bool isQuit_;  // This will become true when getting the order to finish capture

        std::mutex mtxToSyncThread_;
        std::mutex mtxToConditionalWait_;
        std::condition_variable cvarToWaitThread_;  // conditional variable for thread waiting

        std::thread thread_;  // sub thread
        std::thread::id ownerThreadId_;  // main thread's ID 
        std::thread::id captureThreadId_;  // sub thread's ID

        std::shared_ptr<CaptureDataObject> captureData_[maxNumCaptureData_];
        uint64_t capturedTimes_[maxNumCaptureData_];
        int idx_latest_;
        int idx_previous_;
        int idx_update_;
        int idx_locked_;

        struct timespec ts_;

        int Length_;
        int numBytesPerChannel_;

        ICapturable* cap_;  // User selected capture object
        bool disposeCaptureObejct_;

        bool isDebug_;

        static void ThrowExceptionIfNull(void* ptr)
        {
            if (ptr == nullptr)
            {
                throw new std::exception();
            }
        }

        void Main(void);

        bool Initialize(void);

        bool Action(void);

        void Finalize(void);

        bool IsEnd(void);

        bool IsReady(void);

        bool IsFirstCaptured(void);

        void UpdateStateIndicies(void);

        int GetUpdateIndex(void);

        int GetLatestIndex(void);

        int GetNearestIndex(uint64_t sync_time);

        void WaitForReady(void);

        void OnCaptureReady(void);

        uint64_t GetTimeAsUs(void);

        void __logMessage(int line, const char* str, const char* fmt, ...);

    public:

        MultiThreadCaptureController(
            ICapturable* cap,
            bool disposeCaptureObejct,
            bool isDebug = false
        );

        ~MultiThreadCaptureController();

        bool Setup(void);

        bool StartCapture(void);

        bool StopCapture(void);

        bool FinishCapture(void);

        std::tuple<std::shared_ptr<CaptureDataObject>, uint64_t> Read(void);

        std::tuple<std::shared_ptr<CaptureDataObject>, uint64_t> ReadWithSync(uint64_t sync_time);   

        std::tuple<int, int, int, int> __dbg_getindicies(void);
};

#endif  /* H__MULTI_THREAD_CAPTURE_CONTROLLER__H */

