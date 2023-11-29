#include  "MultiThreadCaptureController.hpp"
#include  <cstdarg>

// using GCC extended syntax
#define logMessage(str, fmt, ...)  __logMessage(__LINE__, str, fmt, ##__VA_ARGS__)


/* ----- Public ----- */

MultiThreadCaptureController::MultiThreadCaptureController(
    ICapturable* cap,
    bool disposeCaptureObejct,
    bool isDebug
) :
    isReady_(false), isActive_(false), isQuit_(false),
    ownerThreadId_(-1), captureThreadId_(-1),
    idx_latest_(notApplicatable_), idx_previous_(notApplicatable_), idx_update_(notApplicatable_), idx_locked_(notApplicatable_),
    cap_(cap), disposeCaptureObejct_(disposeCaptureObejct), isDebug_(isDebug)
{
    ThrowExceptionIfNull(cap_);

    auto length = cap_->GetLength();
    auto nbytes = cap_->GetNBytes();

    for (int idx = 0; idx < maxNumCaptureData_; ++idx)
    {
        captureData_[idx] = std::shared_ptr<CaptureDataObject>(new CaptureDataObject(reinterpret_cast<void*>(new uint8_t[length * nbytes]), nbytes, length));
        capturedTimes_[idx] = (uint64_t)0;
    }
}

MultiThreadCaptureController::~MultiThreadCaptureController()
{
    Finalize();
}

bool MultiThreadCaptureController::Setup(void)
{
    logMessage("D", "entry to Setup");

    thread_ = std::thread(&MultiThreadCaptureController::Main, this);

    mtxToSyncThread_.lock();
    ownerThreadId_ = std::this_thread::get_id();
    mtxToSyncThread_.unlock();

    logMessage("D", "exit from Setup");

    return true;
}

bool MultiThreadCaptureController::StartCapture(void)
{
    logMessage("D", "entry to StartCapture");

    mtxToSyncThread_.lock();
    {
        isActive_ = true;
    }
    mtxToSyncThread_.unlock();

    logMessage("D", "exit from StartCapture");

    return true;
}

bool MultiThreadCaptureController::StopCapture(void)
{
    logMessage("D", "entry to StopCapture");

    mtxToSyncThread_.lock();
    {
        isActive_ = false;
    }
    mtxToSyncThread_.unlock();

    logMessage("D", "exit from StopCapture");

    return true;
}

bool MultiThreadCaptureController::FinishCapture(void)
{
    logMessage("D", "entry to FinishCapture");

    mtxToSyncThread_.lock();
    {
        isQuit_ = true;
    }
    mtxToSyncThread_.unlock();

    thread_.join();

    logMessage("D", "exit from FinishCapture");

    return true;
}

std::tuple<std::shared_ptr<CaptureDataObject>, uint64_t> MultiThreadCaptureController::Read(void)
{
    if (IsEnd())
    {
        return std::make_tuple<std::shared_ptr<CaptureDataObject>, uint64_t>(nullptr, -1);
    }

    if (!IsFirstCaptured())
    {
        // wait to become ready for reading capturedData
        WaitForReady();
    }

    auto idx_latest = GetLatestIndex();

    auto capturedData = captureData_[idx_latest];
    auto time_stamp = capturedTimes_[idx_latest];

    return std::tuple<std::shared_ptr<CaptureDataObject>, int>(capturedData, time_stamp);
}

std::tuple<std::shared_ptr<CaptureDataObject>, uint64_t> MultiThreadCaptureController::ReadWithSync(uint64_t sync_time)
{
    if (IsEnd())
    {
        return std::make_tuple<std::shared_ptr<CaptureDataObject>, uint64_t>(nullptr, -1);
    }

    if (!IsFirstCaptured())
    {
        // wait to become ready for reading captured data
        auto lk = std::unique_lock<std::mutex>(mtxToConditionalWait_);
        cvarToWaitThread_.wait(lk);
    }

    auto idx_locked = GetNearestIndex(sync_time);

    auto capturedData = captureData_[idx_locked];
    auto captured_time = capturedTimes_[idx_locked];

    return std::tuple<std::shared_ptr<CaptureDataObject>, int>(capturedData, captured_time);
}


/* ----- Private ----- */

void MultiThreadCaptureController::Main(void)
{
    auto ret = true;
    ret &= Initialize();

    while (ret)
    {
        ret &= Action();
    }

    Finalize();
}

bool MultiThreadCaptureController::Initialize(void)
{
    logMessage("D", "entry to Initialize");

    mtxToSyncThread_.lock();
    isReady_ = true;
    mtxToSyncThread_.unlock();

    captureThreadId_ = std::this_thread::get_id();

    logMessage("D", "exit from Initialize");

    return true;
}

bool MultiThreadCaptureController::Action(void)
{
    auto ret = true;

    if (IsReady())
    {
        // upon recieving a termination request
        if (IsEnd()) { return false; }

        auto idx_update = GetUpdateIndex();

        auto capturedData = captureData_[idx_update];
        ret &= cap_->Capture(capturedData.get());
        if (!ret)
        {
            // end of capture by loading all the videos, or fail to capture from camera
            isQuit_ = true;
            return false;
        }

        auto time = GetTimeAsUs();
        capturedTimes_[idx_update] = time;

        if (!IsFirstCaptured())
        {
            OnCaptureReady();
        }
    }

    return ret;
}

void MultiThreadCaptureController::Finalize(void)
{
    logMessage("D", "entry to Finalize");

    mtxToSyncThread_.lock();
    {
        idx_latest_ = notApplicatable_;
        idx_previous_ = notApplicatable_;
        idx_update_ = notApplicatable_;
        idx_locked_ = notApplicatable_;

        if (disposeCaptureObejct_)
        {
            delete cap_;
            cap_ = nullptr;
            disposeCaptureObejct_ = false;
        }
    }
    mtxToSyncThread_.unlock();

    logMessage("D", "exit from Finalize");
}

bool MultiThreadCaptureController::IsEnd(void)
{
    auto is_quit = false;

    mtxToSyncThread_.lock();
    {
        is_quit = isQuit_;
    }
    mtxToSyncThread_.unlock();

    return is_quit;
}
    
bool MultiThreadCaptureController::IsReady(void)
{
    return isReady_ && isActive_;
}
    
bool MultiThreadCaptureController::IsFirstCaptured(void)
{
    auto ret = false;

    mtxToSyncThread_.lock();
    {
        ret = (idx_latest_ != notApplicatable_);
    }
    mtxToSyncThread_.unlock();

    return ret;
}
    
void MultiThreadCaptureController::WaitForReady(void)
{
    logMessage("D", "entry to WaitForReady");

    auto lk = std::unique_lock<std::mutex>(mtxToConditionalWait_);
    cvarToWaitThread_.wait(lk);

    logMessage("D", "exit from WaitForReady");
}

void MultiThreadCaptureController::OnCaptureReady(void)
{
    logMessage("D", "entry to OnCaptureReady");

    auto lk = std::unique_lock<std::mutex>(mtxToConditionalWait_);
    cvarToWaitThread_.notify_one();

    logMessage("D", "exit from OnCaptureReady");
}
    
void MultiThreadCaptureController::UpdateStateIndicies(void)
{
    mtxToSyncThread_.lock();
    {
        idx_previous_ = idx_latest_;
        idx_latest_ = idx_update_;
    }
    mtxToSyncThread_.unlock();

    //logMessage("D", "latest=%d, previous=%d, update=%d, locked=%d", idx_latest_, idx_previous_, idx_update_, idx_locked_);
}

int MultiThreadCaptureController::GetUpdateIndex(void)
{
    int ret = 0;

    mtxToSyncThread_.lock();
    {
        for (int idx = 0; idx < maxNumCaptureData_; ++idx)
        {
            if ((idx != idx_latest_) || (idx != idx_update_) || (idx != idx_locked_))
            {
                ret = idx;
                break;
            }
        }

        idx_previous_ = idx_latest_;
        idx_latest_ = idx_update_;
        idx_update_ = ret;
    }
    mtxToSyncThread_.unlock();

    logMessage("GetUpdateIndex", "latest=%d, previous=%d, update=%d, locked=%d", idx_latest_, idx_previous_, idx_update_, idx_locked_);

    return ret;
}

int MultiThreadCaptureController::GetLatestIndex(void)
{
    int ret = notApplicatable_;

    mtxToSyncThread_.lock();
    {
        for (int idx = 0; idx < maxNumCaptureData_; ++idx)
        {
            if ((idx != idx_previous_) || (idx != idx_update_) || (idx != idx_locked_))
            {
                ret = idx;
                break;
            }
        }

        idx_previous_ = idx_locked_;
        idx_locked_ = ret;
        idx_latest_ = notApplicatable_;
    }
    mtxToSyncThread_.unlock();

    logMessage("GetLatestIndex", "latest=%d, previous=%d, update=%d, locked=%d", idx_latest_, idx_previous_, idx_update_, idx_locked_);

    return ret;
}

int MultiThreadCaptureController::GetNearestIndex(uint64_t sync_time)
{
    auto ret = notApplicatable_;
    return ret;
}

uint64_t MultiThreadCaptureController::GetTimeAsUs(void)
{
    return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count()
        );
}

void MultiThreadCaptureController::__logMessage(int line, const char* str, const char* fmt, ...)
{
    if (isDebug_)
    {
        char buf[1024]; // 1023bytes + '\0'
        va_list ap;

        va_start(ap, fmt);
        vsprintf(buf, fmt, ap);
        va_end(ap);

        auto thread_id = (captureThreadId_ == std::this_thread::get_id()) ? 1 : 0;

        std::printf("[%d][%s] %s (Line:%d @%s)\n", thread_id, str, buf, line, __FILE__);
    }
}


/* ----- Debug Method ----- */

std::tuple<int, int, int, int> MultiThreadCaptureController::__dbg_getindicies(void)
{
    return std::tuple<int, int, int, int>(idx_latest_, idx_previous_, idx_update_, idx_locked_);
}

