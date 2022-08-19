/*
 * The Shadow Simulator
 * See LICENSE for licensing information
 */
// clang-format off


#ifndef main_bindings_h
#define main_bindings_h

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "main/bindings/c/bindings-opaque.h"
#include "main/core/worker.h"
#include "main/host/descriptor/descriptor_types.h"
#include "main/host/status_listener.h"
#include "main/host/syscall_handler.h"
#include "main/host/syscall_types.h"
#include "main/host/thread.h"
#include "main/host/tracker_types.h"
#include "main/routing/dns.h"
#include "main/routing/packet.minimal.h"

// Memory allocated by Shadow, in a remote address space.
typedef struct AllocdMem_u8 AllocdMem_u8;

typedef struct Arc_AtomicRefCell_AbstractUnixNamespace Arc_AtomicRefCell_AbstractUnixNamespace;

// A queue of bytes that supports reading and writing stream and/or packet data.
//
// Both stream and packet data can be pushed onto the buffer and their order will be preserved.
// Data is stored internally as a linked list of chunks. Each chunk stores either stream or packet
// data. Consecutive stream data may be merged into a single chunk, but consecutive packets will
// always be contained in their own chunks.
//
// To avoid memory copies when moving bytes from one `ByteQueue` to another, you can use
// `pop_chunk()` to remove a chunk from the queue, and use `push_chunk()` to add it to another
// queue.
typedef struct ByteQueue ByteQueue;

// Utility for monitoring a set of child pid's, calling registered callbacks
// when one exits or is killed. Starts a background thread, which is shut down
// when the object is dropped.
typedef struct ChildPidWatcher ChildPidWatcher;

typedef struct CliOptions CliOptions;

// Shadow configuration options after processing command-line and configuration file options.
typedef struct ConfigOptions ConfigOptions;

typedef struct Controller Controller;

// The main counter object that maps individual keys to count values.
typedef struct Counter Counter;

// A file descriptor that reference an open file. Also contains flags that change the behaviour of
// this file descriptor.
typedef struct Descriptor Descriptor;

// Map of file handles to file descriptors. Typically owned by a Process.
typedef struct DescriptorTable DescriptorTable;

typedef struct Event Event;

// A wrapper for any type of file object.
typedef struct File File;

// Tool for assigning IP addresses to graph nodes.
typedef struct IpAssignment_u32 IpAssignment_u32;

// A set of `n` logical processors
typedef struct LogicalProcessors LogicalProcessors;

// Provides accessors for reading and writing another process's memory.
// When in use, any operation that touches that process's memory must go
// through the MemoryManager to ensure soundness. See MemoryManager::new.
typedef struct MemoryManager MemoryManager;

// A network graph containing the petgraph graph and a map from gml node ids to petgraph node
// indexes.
typedef struct NetworkGraph NetworkGraph;

// Represents a POSIX file description, or a Linux `struct file`. An `OpenFile` wraps a reference
// to a `File`. Once there are no more `OpenFile` objects for a given `File`, the `File` will be
// closed. Typically this means that holding an `OpenFile` will ensure that the file remains open
// (the file's status will not become `FileStatus::CLOSED`), but the underlying file may close
// itself in extenuating circumstances (for example if the file has an internal error).
//
// **Safety:** If an `OpenFile` for a specific file already exists, it is an error to create a new
// `OpenFile` for that file. You must clone the existing `OpenFile` object. A new `OpenFile` object
// should probably only ever be created for a newly created file object. Otherwise for existing
// file objects, it won't be clear if there are already-existing `OpenFile` objects for that file.
//
// There must also not be any existing mutable borrows of the file when an `OpenFile` is created.
typedef struct OpenFile OpenFile;

typedef struct PcapWriter_BufWriter_File PcapWriter_BufWriter_File;

// A mutable reference to a slice of plugin memory. Implements `DerefMut<[T]>`,
// allowing, e.g.:
//
// ```
// let tpp = TypedPluginPtr::<u32>::new(ptr, 10);
// let pmr = memory_manager.memory_ref_mut(ptr);
// assert_eq!(pmr.len(), 10);
// pmr[5] = 100;
// ```
//
// The object must be disposed of by calling `flush` or `noflush`.  Dropping
// the object without doing so will result in a panic.
typedef struct ProcessMemoryRefMut_u8 ProcessMemoryRefMut_u8;

// An immutable reference to a slice of plugin memory. Implements `Deref<[T]>`,
// allowing, e.g.:
//
// ```
// let tpp = TypedPluginPtr::<u32>::new(ptr, 10);
// let pmr = memory_manager.memory_ref(ptr);
// assert_eq!(pmr.len(), 10);
// let x = pmr[5];
// ```
typedef struct ProcessMemoryRef_u8 ProcessMemoryRef_u8;

typedef struct Random Random;

// Routing information for paths between nodes.
typedef struct RoutingInfo_u32 RoutingInfo_u32;

typedef struct SyscallHandler SyscallHandler;

// Mostly for interoperability with C APIs.
// In Rust code that doesn't need to interact with C, it may make more sense
// to directly use a `Fn(&mut Host)` trait object.
typedef struct TaskRef TaskRef;

// A wrapper for [`EventQueue`] that uses interior mutability to make the ffi simpler.
typedef struct ThreadSafeEventQueue ThreadSafeEventQueue;

typedef struct Timer Timer;

typedef struct TokenBucket TokenBucket;

typedef uint64_t WatchHandle;

typedef HostId HostId;

typedef void (*TaskCallbackFunc)(Host*, void*, void*);

typedef void (*TaskObjectFreeFunc)(void*);

typedef void (*TaskArgumentFreeFunc)(void*);

#define EMUTIME_INVALID UINT64_MAX

#define EMUTIME_MAX (UINT64_MAX - 1)

#define EMUTIME_MIN 0

















struct ByteQueue *bytequeue_new(uintptr_t default_chunk_size);

void bytequeue_free(struct ByteQueue *bq_ptr);

uintptr_t bytequeue_numBytes(struct ByteQueue *bq);

bool bytequeue_hasBytes(struct ByteQueue *bq);

void bytequeue_pushStream(struct ByteQueue *bq, const unsigned char *src, size_t len);

void bytequeue_pushPacket(struct ByteQueue *bq, const unsigned char *src, size_t len);

size_t bytequeue_pop(struct ByteQueue *bq, unsigned char *dst, size_t len);

struct ChildPidWatcher *childpidwatcher_new(void);

void childpidwatcher_free(struct ChildPidWatcher *watcher);

int32_t childpidwatcher_forkWatchable(const struct ChildPidWatcher *watcher,
                                      void (*child_fn)(void*),
                                      void *child_fn_data);

// Register interest in `pid`, and associate it with `read_fd`.
//
// `read_fd` should be the read end of a pipe, whose write end is owned
// *solely* by `pid`, causing `read_fd` to become invalid when `pid` exits.
// In a multi-threaded program care must be taken to prevent a concurrent
// fork from leaking the write end of the pipe into other children. One way
// to avoid this is to use O_CLOEXEC when creating the pipe, and then unset
// O_CLOEXEC in the child before calling exec.
//
// Be sure to close the parent's write-end of the pipe.
//
// Takes ownership of `read_fd`, and will close it when appropriate.
void childpidwatcher_registerPid(const struct ChildPidWatcher *watcher,
                                 int32_t pid,
                                 int32_t read_fd);

void childpidwatcher_unregisterPid(const struct ChildPidWatcher *watcher, int32_t pid);

// Call `callback` exactly once from another thread after the child `pid`
// has exited, including if it has already exited. Does *not* reap the
// child itself.
//
// The returned handle is guaranteed to be non-zero.
//
// Panics if `pid` doesn't exist.
//
// SAFETY: It must be safe for `callback` to execute and manipulate `data`
// from another thread. e.g. typically this means that `data` must be `Send`
// and `Sync`.
WatchHandle childpidwatcher_watch(const struct ChildPidWatcher *watcher,
                                  pid_t pid,
                                  void (*callback)(pid_t, void*),
                                  void *data);

// Unregisters a callback. After returning, the corresponding callback is
// guaranteed either to have already run, or to never run. i.e. it's safe to
// free data that the callback might otherwise access.
//
// Calling with pids or handles that no longer exist is safe.
void childpidwatcher_unwatch(const struct ChildPidWatcher *watcher, pid_t pid, WatchHandle handle);

struct Counter *counter_new(void);

void counter_free(struct Counter *counter_ptr);

int64_t counter_add_value(struct Counter *counter, const char *id, int64_t value);

int64_t counter_sub_value(struct Counter *counter, const char *id, int64_t value);

void counter_add_counter(struct Counter *counter, const struct Counter *other);

void counter_sub_counter(struct Counter *counter, const struct Counter *other);

bool counter_equals_counter(const struct Counter *counter, const struct Counter *other);

// Creates a new string representation of the counter, e.g., for logging.
// The returned string must be free'd by passing it to counter_free_string.
char *counter_alloc_string(struct Counter *counter);

// Frees a string previously returned from counter_alloc_string.
void counter_free_string(struct Counter *counter, char *ptr);

// A new packet capture writer. Each packet (header and payload) captured will be truncated to
// a length `capture_len`.
struct PcapWriter_BufWriter_File *pcapwriter_new(const char *path, uint32_t capture_len);

void pcapwriter_free(struct PcapWriter_BufWriter_File *pcap);

// If there's an error, returns 1. Otherwise returns 0. If there's an error, the pcap file is
// likely to be corrupt.
int pcapwriter_writePacket(struct PcapWriter_BufWriter_File *pcap,
                           uint32_t ts_sec,
                           uint32_t ts_usec,
                           const Packet *packet);

struct Random *random_new(uint64_t seed);

void random_free(struct Random *rng);

// Returns a pseudo-random integer in the range \[0, [`libc::RAND_MAX`]\].
uint32_t random_rand(struct Random *rng);

// Returns a pseudo-random float in the range \[0, 1).
double random_nextDouble(struct Random *rng);

// Returns a pseudo-random integer in the range \[0, [`u32::MAX`]\].
uint32_t random_nextU32(struct Random *rng);

// Fills the buffer with pseudo-random bytes.
void random_nextNBytes(struct Random *rng, uint8_t *buf, uintptr_t len);

// Get the backtrace. This function is slow. The string must be freed using `backtrace_free()`.
char *backtrace(void);

void backtrace_free(char *backtrace);

void controller_updateMinRunahead(const struct Controller *controller,
                                  SimulationTime min_path_latency);

void controller_incrementPluginErrors(const struct Controller *controller);

// Flush Rust's log::logger().
void rustlogger_flush(void);

// Set the max (noisiest) logging level to `level`.
void rustlogger_setLevel(LogLevel level);

// Whether logging is currently enabled for `level`.
int rustlogger_isEnabled(LogLevel level);

// Log to Rust's log::logger().
void rustlogger_log(LogLevel level,
                    const char *file_name,
                    const char *fn_name,
                    int32_t line,
                    const char *format,
                    void *va_list);

// When disabled, the logger thread is notified to write each record as
// soon as it's created.  The calling thread still isn't blocked on the
// record actually being written, though.
void shadow_logger_setEnableBuffering(int32_t buffering_enabled);

struct LogicalProcessors *lps_new(int n);

void lps_free(struct LogicalProcessors *lps);

int lps_n(const struct LogicalProcessors *lps);

void lps_readyPush(const struct LogicalProcessors *lps, int lpi, int worker);

int lps_popWorkerToRunOn(const struct LogicalProcessors *lps, int lpi);

void lps_donePush(const struct LogicalProcessors *lps, int lpi, int worker);

void lps_finishTask(struct LogicalProcessors *lps);

int lps_cpuId(const struct LogicalProcessors *lps, int lpi);

double lps_idleTimerElapsed(const struct LogicalProcessors *lps, int lpi);

void lps_idleTimerContinue(const struct LogicalProcessors *lps, int lpi);

void lps_idleTimerStop(const struct LogicalProcessors *lps, int lpi);

int main_runShadow(int argc, const char *const *argv);

int manager_saveProcessedConfigYaml(const struct ConfigOptions *config, const char *filename);

void clioptions_freeString(char *string);

bool clioptions_getGdb(const struct CliOptions *options);

bool clioptions_getShmCleanup(const struct CliOptions *options);

bool clioptions_getShowBuildInfo(const struct CliOptions *options);

bool clioptions_getShowConfig(const struct CliOptions *options);

char *clioptions_getConfig(const struct CliOptions *options);

void config_freeString(char *string);

void config_showConfig(const struct ConfigOptions *config);

bool config_getUseSchedFifo(const struct ConfigOptions *config);

bool config_getUseExplicitBlockMessage(const struct ConfigOptions *config);

bool config_getUseSyscallCounters(const struct ConfigOptions *config);

bool config_getUseMemoryManager(const struct ConfigOptions *config);

bool config_getUseShimSyscallHandler(const struct ConfigOptions *config);

int32_t config_getPreloadSpinMax(const struct ConfigOptions *config);

uint64_t config_getMaxUnappliedCpuLatency(const struct ConfigOptions *config);

SimulationTime config_getUnblockedSyscallLatency(const struct ConfigOptions *config);

SimulationTime config_getUnblockedVdsoLatency(const struct ConfigOptions *config);

uint32_t config_getParallelism(const struct ConfigOptions *config);

SimulationTime config_getBootstrapEndTime(const struct ConfigOptions *config);

bool config_getUseLegacyWorkingDir(const struct ConfigOptions *config);

bool config_getProgress(const struct ConfigOptions *config);

bool config_getModelUnblockedSyscallLatency(const struct ConfigOptions *config);

LogLevel config_getHostHeartbeatLogLevel(const struct ConfigOptions *config);

LogInfoFlags config_getHostHeartbeatLogInfo(const struct ConfigOptions *config);

SimulationTime config_getHostHeartbeatInterval(const struct ConfigOptions *config);

enum StraceFmtMode config_getStraceLoggingMode(const struct ConfigOptions *config);

// Parses a string as bits-per-second. Returns '-1' on error.
int64_t parse_bandwidth(const char *s);

// Parses a string as a time in nanoseconds. Returns '-1' on error.
int64_t parse_time_nanosec(const char *s);

EmulatedTime emutime_add_simtime(EmulatedTime lhs, SimulationTime rhs);

SimulationTime emutime_sub_emutime(EmulatedTime lhs, EmulatedTime rhs);

SimulationTime simtime_from_timeval(struct timeval val);

SimulationTime simtime_from_timespec(struct timespec val);

__attribute__((warn_unused_result))
bool simtime_to_timeval(SimulationTime val,
                        struct timeval *out);

__attribute__((warn_unused_result))
bool simtime_to_timespec(SimulationTime val,
                         struct timespec *out);

struct Event *event_new(struct TaskRef *task_ref,
                        SimulationTime time,
                        Host *src_host,
                        HostId dst_host_id);

void event_free(struct Event *event);

// Execute the event. **This frees the event.**
void event_executeAndFree(struct Event *event, Host *host);

HostId event_getHostID(struct Event *event);

SimulationTime event_getTime(const struct Event *event);

void event_setTime(struct Event *event, SimulationTime time);

const struct ThreadSafeEventQueue *eventqueue_new(void);

void eventqueue_drop(const struct ThreadSafeEventQueue *queue);

const struct ThreadSafeEventQueue *eventqueue_cloneArc(const struct ThreadSafeEventQueue *queue_ptr);

// Takes ownership of the event.
void eventqueue_push(const struct ThreadSafeEventQueue *queue, struct Event *event);

struct Event *eventqueue_pop(const struct ThreadSafeEventQueue *queue);

EmulatedTime eventqueue_nextEventTime(const struct ThreadSafeEventQueue *queue);

// Create a new reference-counted task that can only be executed on the
// given host. The callbacks can safely assume that they will only be called
// with the lock for the specified host held.
//
// SAFETY:
// * `object` and `argument` must meet the requirements
//    for `HostTreePointer::new`.
// * Given that the host lock is held when execution of a callback
//   starts, they must not cause `object` or `argument` to be dereferenced
//   without the host lock held. (e.g. by releasing the host lock or exfiltrating
//   the pointers to be dereferenced by other code that might not hold the lock).
//
// There must still be some coordination between the creator of the TaskRef
// and the callers of `taskref_execute` and `taskref_drop` to ensure that
// the callbacks don't conflict with other accesses in the same thread
// (e.g. that the caller isn't holding a Rust mutable reference to one of
// the pointers while the callback transforms the pointer into another Rust
// reference).
struct TaskRef *taskref_new_bound(HostId host_id,
                                  TaskCallbackFunc callback,
                                  void *object,
                                  void *argument,
                                  TaskObjectFreeFunc object_free,
                                  TaskArgumentFreeFunc argument_free);

// Create a new reference-counted task that may be executed on any Host.
//
// SAFETY:
// * The callbacks must be safe to call with `object` and `argument`
//   with *any* Host. (e.g. even if task is expected to execute on another Host,
//   must be safe to execute or free the Task from the current Host.)
//
// There must still be some coordination between the creator of the TaskRef
// and the callers of `taskref_execute` and `taskref_drop` to ensure that
// the callbacks don't conflict with other accesses in the same thread
// (e.g. that the caller isn't holding a Rust mutable reference to one of
// the pointers while the callback transforms the pointer into another Rust
// reference).
struct TaskRef *taskref_new_unbound(TaskCallbackFunc callback,
                                    void *object,
                                    void *argument,
                                    TaskObjectFreeFunc object_free,
                                    TaskArgumentFreeFunc argument_free);

// Destroys this reference to the `Task`, dropping the `Task` if no references remain.
//
// Panics if task's Host lock isn't held.
//
// SAFETY: `task` must be legally dereferencable.
void taskref_drop(struct TaskRef *task);

DNS *worker_getDNS(void);

SimulationTime worker_getLatency(in_addr_t src, in_addr_t dst);

float worker_getReliability(in_addr_t src, in_addr_t dst);

uint64_t worker_getBandwidthDownBytes(in_addr_t ip);

uint64_t worker_getBandwidthUpBytes(in_addr_t ip);

bool worker_isRoutable(in_addr_t src, in_addr_t dst);

void worker_incrementPacketCount(in_addr_t src, in_addr_t dst);

// Initialize a Worker for this thread.
void worker_newForThisThread(WorkerPool *worker_pool,
                             int32_t worker_id,
                             SimulationTime bootstrap_end_time);

// Returns NULL if there is no live Worker.
struct Counter *_worker_objectAllocCounter(void);

// Implementation for counting allocated objects. Do not use this function directly.
// Use worker_count_allocation instead from the call site.
void worker_increment_object_alloc_counter(const char *object_name);

// Returns NULL if there is no live Worker.
struct Counter *_worker_objectDeallocCounter(void);

// Implementation for counting deallocated objects. Do not use this function directly.
// Use worker_count_deallocation instead from the call site.
void worker_increment_object_dealloc_counter(const char *object_name);

// Returns NULL if there is no live Worker.
struct Counter *_worker_syscallCounter(void);

// Aggregate the given syscall counts in a worker syscall counter.
void worker_add_syscall_counts(const struct Counter *syscall_counts);

// ID of the current thread's Worker. Panics if the thread has no Worker.
int32_t worker_threadID(void);

void worker_setActiveHost(Host *host);

void worker_setActiveProcess(Process *process);

void worker_setActiveThread(Thread *thread);

void worker_setRoundEndTime(SimulationTime t);

SimulationTime _worker_getRoundEndTime(void);

void worker_setCurrentEmulatedTime(EmulatedTime t);

void worker_clearCurrentTime(void);

SimulationTime worker_getCurrentSimulationTime(void);

EmulatedTime worker_getCurrentEmulatedTime(void);

void worker_updateMinHostRunahead(SimulationTime t);

bool worker_isBootstrapActive(void);

WorkerPool *_worker_pool(void);

bool worker_isAlive(void);

// Add the counters to their global counterparts, and clear the provided counters.
void worker_addToGlobalAllocCounters(struct Counter *alloc_counter,
                                     struct Counter *dealloc_counter);

// Add the counters to their global counterparts, and clear the provided counters.
void worker_addToGlobalSyscallCounter(struct Counter *syscall_counter);

// Create an object that can be used to store all descriptors created by a
// process. When the table is no longer required, use descriptortable_free
// to release the reference.
struct DescriptorTable *descriptortable_new(void);

// Free the table.
void descriptortable_free(struct DescriptorTable *table);

// Store the given descriptor at the given index. Any previous descriptor that was
// stored there will be returned. This consumes a ref to the given descriptor as in
// add(), and any returned descriptor must be freed manually.
struct Descriptor *descriptortable_set(struct DescriptorTable *table,
                                       int index,
                                       struct Descriptor *descriptor);

// This is a helper function that handles some corner cases where some
// descriptors are linked to each other and we must remove that link in
// order to ensure that the reference count reaches zero and they are properly
// freed. Otherwise the circular reference will prevent the free operation.
// TODO: remove this once the TCP layer is better designed.
void descriptortable_shutdownHelper(struct DescriptorTable *table);

// Close all descriptors. The `host` option is a legacy option for legacy files.
void descriptortable_removeAndCloseAll(struct DescriptorTable *table, Host *host);

struct Arc_AtomicRefCell_AbstractUnixNamespace *abstractunixnamespace_new(void);

void abstractunixnamespace_free(struct Arc_AtomicRefCell_AbstractUnixNamespace *ns);

// The new descriptor takes ownership of the reference to the legacy file and does not
// increment its ref count, but will decrement the ref count when this descriptor is
// freed/dropped with `descriptor_free()`. The descriptor flags must be either 0 or
// `O_CLOEXEC`.
struct Descriptor *descriptor_fromLegacyFile(LegacyFile *legacy_file, int descriptor_flags);

// If the descriptor is a legacy file, returns a pointer to the legacy file object. Otherwise
// returns NULL. The legacy file's ref count is not modified, so the pointer must not outlive
// the lifetime of the descriptor.
LegacyFile *descriptor_asLegacyFile(const struct Descriptor *descriptor);

// If the descriptor is a new/rust descriptor, returns a pointer to the reference-counted
// `OpenFile` object. Otherwise returns NULL. The `OpenFile` object's ref count is not
// modified, so the returned pointer must not outlive the lifetime of the descriptor.
const struct OpenFile *descriptor_borrowOpenFile(const struct Descriptor *descriptor);

// If the descriptor is a new/rust descriptor, returns a pointer to the reference-counted
// `OpenFile` object. Otherwise returns NULL. The `OpenFile` object's ref count is incremented,
// so the returned pointer must always later be passed to `openfile_drop()`, otherwise the
// memory will leak.
const struct OpenFile *descriptor_newRefOpenFile(const struct Descriptor *descriptor);

// The descriptor flags must be either 0 or `O_CLOEXEC`.
void descriptor_setFlags(struct Descriptor *descriptor, int flags);

// Decrement the ref count of the `OpenFile` object. The pointer must not be used after calling
// this function.
void openfile_drop(const struct OpenFile *file);

// Get the state of the `OpenFile` object.
Status openfile_getStatus(const struct OpenFile *file);

// Add a status listener to the `OpenFile` object. This will increment the status listener's
// ref count, and will decrement the ref count when this status listener is removed or when the
// `OpenFile` is freed/dropped.
void openfile_addListener(const struct OpenFile *file, StatusListener *listener);

// Remove a listener from the `OpenFile` object.
void openfile_removeListener(const struct OpenFile *file, StatusListener *listener);

// Get the canonical handle for an `OpenFile` object. Two `OpenFile` objects refer to the same
// underlying data if their handles are equal.
uintptr_t openfile_getCanonicalHandle(const struct OpenFile *file);

// If the descriptor is a new/rust descriptor, returns a pointer to the reference-counted
// `File` object. Otherwise returns NULL. The `File` object's ref count is incremented, so the
// pointer must always later be passed to `file_drop()`, otherwise the memory will leak.
const struct File *descriptor_newRefFile(const struct Descriptor *descriptor);

// Decrement the ref count of the `File` object. The pointer must not be used after calling
// this function.
void file_drop(const struct File *file);

// Get the state of the `File` object.
Status file_getStatus(const struct File *file);

// Add a status listener to the `File` object. This will increment the status listener's ref
// count, and will decrement the ref count when this status listener is removed or when the
// `File` is freed/dropped.
void file_addListener(const struct File *file, StatusListener *listener);

// Remove a listener from the `File` object.
void file_removeListener(const struct File *file, StatusListener *listener);

// Get the canonical handle for a `File` object. Two `File` objects refer to the same
// underlying data if their handles are equal.
uintptr_t file_getCanonicalHandle(const struct File *file);

// # Safety
// * `thread` must point to a valid object.
struct MemoryManager *memorymanager_new(pid_t pid);

// # Safety
// * `mm` must point to a valid object.
void memorymanager_free(struct MemoryManager *mm);

struct AllocdMem_u8 *allocdmem_new(Thread *thread, uintptr_t len);

void allocdmem_free(Thread *thread, struct AllocdMem_u8 *allocd_mem);

PluginPtr allocdmem_pluginPtr(const struct AllocdMem_u8 *allocd_mem);

// Initialize the MemoryMapper if it isn't already initialized. `thread` must
// be running and ready to make native syscalls.
void memorymanager_initMapperIfNeeded(struct MemoryManager *memory_manager, Thread *thread);

void memorymanager_freeRef(struct ProcessMemoryRef_u8 *memory_ref);

const void *memorymanagerref_ptr(const struct ProcessMemoryRef_u8 *memory_ref);

size_t memorymanagerref_sizeof(const struct ProcessMemoryRef_u8 *memory_ref);

// Get a read-accessor to the specified plugin memory.
// Must be freed via `memorymanager_freeReader`.
struct ProcessMemoryRef_u8 *memorymanager_getReadablePtr(const struct MemoryManager *memory_manager,
                                                         PluginPtr plugin_src,
                                                         uintptr_t n);

struct ProcessMemoryRef_u8 *memorymanager_getReadablePtrPrefix(const struct MemoryManager *memory_manager,
                                                               PluginPtr plugin_src,
                                                               uintptr_t n);

ssize_t memorymanager_readString(const struct MemoryManager *memory_manager,
                                 PluginPtr ptr,
                                 char *strbuf,
                                 size_t maxlen);

// Copy data from this reader's memory.
int32_t memorymanager_readPtr(const struct MemoryManager *memory_manager,
                              void *dst,
                              PluginPtr src,
                              uintptr_t n);

// Write data to this writer's memory.
int32_t memorymanager_writePtr(struct MemoryManager *memory_manager,
                               PluginPtr dst,
                               const void *src,
                               uintptr_t n);

// Get a writable pointer to this writer's memory. Initial contents are unspecified.
struct ProcessMemoryRefMut_u8 *memorymanager_getWritablePtr(struct MemoryManager *memory_manager,
                                                            PluginPtr plugin_src,
                                                            uintptr_t n);

// Get a readable and writable pointer to this writer's memory.
struct ProcessMemoryRefMut_u8 *memorymanager_getMutablePtr(struct MemoryManager *memory_manager,
                                                           PluginPtr plugin_src,
                                                           uintptr_t n);

void *memorymanagermut_ptr(struct ProcessMemoryRefMut_u8 *memory_ref);

size_t memorymanagermut_sizeof(struct ProcessMemoryRefMut_u8 *memory_ref);

// Write-back any previously returned writable memory, and free the writer.
int32_t memorymanager_freeMutRefWithFlush(struct ProcessMemoryRefMut_u8 *mref);

// Write-back any previously returned writable memory, and free the writer.
void memorymanager_freeMutRefWithoutFlush(struct ProcessMemoryRefMut_u8 *mref);

// Fully handles the `brk` syscall, keeping the "heap" mapped in our shared mem file.
SysCallReturn memorymanager_handleBrk(struct MemoryManager *memory_manager,
                                      Thread *thread,
                                      PluginPtr plugin_src);

// Fully handles the `mmap` syscall
SysCallReturn memorymanager_handleMmap(struct MemoryManager *memory_manager,
                                       Thread *thread,
                                       PluginPtr addr,
                                       uintptr_t len,
                                       int32_t prot,
                                       int32_t flags,
                                       int32_t fd,
                                       int64_t offset);

// Fully handles the `munmap` syscall
SysCallReturn memorymanager_handleMunmap(struct MemoryManager *memory_manager,
                                         Thread *thread,
                                         PluginPtr addr,
                                         uintptr_t len);

SysCallReturn memorymanager_handleMremap(struct MemoryManager *memory_manager,
                                         Thread *thread,
                                         PluginPtr old_addr,
                                         uintptr_t old_size,
                                         uintptr_t new_size,
                                         int32_t flags,
                                         PluginPtr new_addr);

SysCallReturn memorymanager_handleMprotect(struct MemoryManager *memory_manager,
                                           Thread *thread,
                                           PluginPtr addr,
                                           uintptr_t size,
                                           int32_t prot);

// Register a `Descriptor`. This takes ownership of the descriptor and you must not access it
// after.
int process_registerDescriptor(Process *proc, struct Descriptor *desc);

// Get a temporary reference to a descriptor.
const struct Descriptor *process_getRegisteredDescriptor(Process *proc, int handle);

// Get a temporary mutable reference to a descriptor.
struct Descriptor *process_getRegisteredDescriptorMut(Process *proc, int handle);

// Get a temporary reference to a legacy file.
LegacyFile *process_getRegisteredLegacyFile(Process *proc, int handle);

SysCallReturn log_syscall(Process *proc,
                          enum StraceFmtMode logging_mode,
                          pid_t tid,
                          const char *name,
                          const char *args,
                          SysCallReturn result);

struct SyscallHandler *rustsyscallhandler_new(void);

void rustsyscallhandler_free(struct SyscallHandler *handler_ptr);

SysCallReturn rustsyscallhandler_syscall(struct SyscallHandler *sys,
                                         SysCallHandler *csys,
                                         const SysCallArgs *args);

// Create a new Timer that synchronously executes `task` on expiration.
// `task` should not call mutable methods of the enclosing `Timer`; if it needs
// to do so it should schedule a new task to do so.
struct Timer *timer_new(const struct TaskRef *task);

void timer_drop(struct Timer *timer);

uint64_t timer_getExpirationCount(const struct Timer *timer);

uint64_t timer_consumeExpirationCount(struct Timer *timer);

// Returns the remaining time until the next expiration. Returns 0 if the
// timer isn't armed.
SimulationTime timer_getRemainingTime(const struct Timer *timer);

SimulationTime timer_getInterval(const struct Timer *timer);

void timer_arm(struct Timer *timer,
               Host *host,
               EmulatedTime nextExpireTime,
               SimulationTime expireInterval);

void timer_disarm(struct Timer *timer);

struct NetworkGraph *networkgraph_load(const struct ConfigOptions *config);

void networkgraph_free(struct NetworkGraph *graph);

// Check if the node exists in the graph.
__attribute__((warn_unused_result))
bool networkgraph_nodeExists(struct NetworkGraph *graph,
                             uint32_t node_id);

// Get the downstream bandwidth of the graph node if it exists. A non-zero return value means
// that the node did not have a downstream bandwidth and that `bandwidth_down` was not updated.
__attribute__((warn_unused_result))
int networkgraph_nodeBandwidthDownBits(struct NetworkGraph *graph,
                                       uint32_t node_id,
                                       uint64_t *bandwidth_down);

// Get the upstream bandwidth of the graph node if it exists. A non-zero return value means
// that the node did not have an upstream bandwidth and that `bandwidth_up` was not updated.
__attribute__((warn_unused_result))
int networkgraph_nodeBandwidthUpBits(struct NetworkGraph *graph,
                                     uint32_t node_id,
                                     uint64_t *bandwidth_up);

struct IpAssignment_u32 *ipassignment_new(void);

void ipassignment_free(struct IpAssignment_u32 *ip_assignment);

// Get an unused address and assign it to a node.
__attribute__((warn_unused_result))
int ipassignment_assignHost(struct IpAssignment_u32 *ip_assignment,
                            uint32_t node_id,
                            in_addr_t *ip_addr);

// Assign an address to a node.
__attribute__((warn_unused_result))
int ipassignment_assignHostWithIp(struct IpAssignment_u32 *ip_assignment,
                                  uint32_t node_id,
                                  in_addr_t ip_addr);

struct RoutingInfo_u32 *routinginfo_new(struct NetworkGraph *graph,
                                        struct IpAssignment_u32 *ip_assignment,
                                        bool use_shortest_paths);

void routinginfo_free(struct RoutingInfo_u32 *routing_info);

// Checks if the addresses are assigned to hosts, and if so it must be routable since the
// graph is connected.
bool routinginfo_isRoutable(const struct IpAssignment_u32 *ip_assignment,
                            in_addr_t src,
                            in_addr_t dst);

// Get the packet latency from one host to another. The given addresses must be assigned to
// hosts.
uint64_t routinginfo_getLatencyNs(const struct RoutingInfo_u32 *routing_info,
                                  const struct IpAssignment_u32 *ip_assignment,
                                  in_addr_t src,
                                  in_addr_t dst);

// Get the packet reliability from one host to another. The given addresses must be assigned
// to hosts.
float routinginfo_getReliability(const struct RoutingInfo_u32 *routing_info,
                                 const struct IpAssignment_u32 *ip_assignment,
                                 in_addr_t src,
                                 in_addr_t dst);

// Increment the number of packets sent from one host to another. The given addresses must be
// assigned to hosts.
void routinginfo_incrementPacketCount(struct RoutingInfo_u32 *routing_info,
                                      const struct IpAssignment_u32 *ip_assignment,
                                      in_addr_t src,
                                      in_addr_t dst);

uint64_t routinginfo_smallestLatencyNs(struct RoutingInfo_u32 *routing_info);

struct TokenBucket *tokenbucket_new(uint64_t capacity,
                                    uint64_t refill_size,
                                    uint64_t refill_interval_nanos);

void tokenbucket_free(struct TokenBucket *tokenbucket_ptr);

bool tokenbucket_consume(struct TokenBucket *tokenbucket_ptr,
                         uint64_t count,
                         uint64_t *remaining_tokens,
                         uint64_t *nanos_until_refill);

#endif /* main_bindings_h */
