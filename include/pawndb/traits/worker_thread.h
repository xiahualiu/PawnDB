#ifndef PAWNDB_TRAITS_WORKER_THREAD_H
#define PAWNDB_TRAITS_WORKER_THREAD_H

namespace PawnDB {
/**
 * @brief Worker thread operation error codes
 */
enum class WorkerError {
    None,         /**< Operation successful */
    InitFailed,   /**< Initialization failed */
    JobFull,      /**< Job queue full */
    Terminated    /**< Thread terminated */
};

template<typename Derived, typename Context, typename JobType>
class WorkerThread {
public:

    WorkerError initialize(const Context&& context) noexcept {
        return static_cast<Derived*>(this)->trait_initialize(context);
    }


    WorkerError process_job(const JobType& job) noexcept {
        return static_cast<Derived*>(this)->trait_process_job(job);
    }

    WorkerError cleanup() noexcept {
        return static_cast<Derived*>(this)->trait_cleanup();
    }


    bool is_running() const noexcept {
        return static_cast<const Derived*>(this)->trait_is_running();
    }

    void terminate() noexcept {
        static_cast<Derived*>(this)->trait_terminate();
    }

protected:
    WorkerThread() = default;
    ~WorkerThread() = default;

};

} // namespace PawnDB

#endif // PAWNDB_TRAITS_WORKER_THREAD_H
