/*
 * The Shadow Simulator
 * See LICENSE for licensing information
 */

#ifndef SHD_DEFINITIONS_H_
#define SHD_DEFINITIONS_H_

#include <glib.h>

// TODO put into a shd-types.h file
typedef struct _Process Process;
typedef struct _Host Host;

/**
 * Simulation time in nanoseconds. Allows for a consistent representation
 * of time throughput the simulator.
 */
typedef guint64 SimulationTime;

/**
 * Unique object id reference
 */
typedef guint ShadowID;

/**
 * Represents an invalid simulation time.
 */
#define SIMTIME_INVALID G_MAXUINT64

/**
 * Represents one nanosecond in simulation time.
 */
#define SIMTIME_ONE_NANOSECOND G_GUINT64_CONSTANT(1)

/**
 * Represents one microsecond in simulation time.
 */
#define SIMTIME_ONE_MICROSECOND G_GUINT64_CONSTANT(1000)

/**
 * Represents one millisecond in simulation time.
 */
#define SIMTIME_ONE_MILLISECOND G_GUINT64_CONSTANT(1000000)

/**
 * Represents one second in simulation time.
 */
#define SIMTIME_ONE_SECOND G_GUINT64_CONSTANT(1000000000)

/**
 * Represents one minute in simulation time.
 */
#define SIMTIME_ONE_MINUTE G_GUINT64_CONSTANT(60000000000)

/**
 * Represents one hour in simulation time.
 */
#define SIMTIME_ONE_HOUR G_GUINT64_CONSTANT(3600000000000)

/**
 * Emulation time in nanoseconds. Allows for a consistent representation
 * of time throughput the simulator. Emulation time is the simulation time
 * plus the EMULATION_TIME_OFFSET. This type allows us to explicitly
 * distinguish each type of time in the code.,
 */
typedef guint64 EmulatedTime;

/**
 * The number of nanoseconds from the epoch to January 1st, 2000 at 12:00am UTC.
 * This is used to emulate to applications that we are in a recent time.
 *
 * Duplicated as SIMULATION_START_SEC in `emulated_time.rs`.
 */
#define EMUTIME_SIMULATION_START (G_GUINT64_CONSTANT(946684800) * SIMTIME_ONE_SECOND)

/**
 * The Unix Epoch as EmulatedTime
 */
#define EMUTIME_UNIX_EPOCH (0)

/* Ensure it can be converted to EmulatedTime */
#define SIMTIME_MAX (EMUTIME_MAX - EMUTIME_SIMULATION_START)
#define SIMTIME_MIN 0

/**
 * The start of our random port range in host order, used if application doesn't
 * specify the port it wants to bind to, and for client connections.
 */
#define MIN_RANDOM_PORT 10000

/**
 * An upper limit to the maximum number of pending incoming connections.
 * On a laptop: net.core.somaxconn = 4096
 */
#define SHADOW_SOMAXCONN 4096

/**
 * We always use TCP_autotuning unless this is set to FALSE
 *
 * @todo change this to a command line option accessible via #Configuration
 */
#define CONFIG_TCPAUTOTUNE TRUE

/**
 * Minimum, default, and maximum values for TCP send and receive buffers
 * Normally specified in:
 *      /proc/sys/net/ipv4/tcp_rmem
 *      /proc/sys/net/ipv4/tcp_wmem
 */
#define CONFIG_TCP_WMEM_MIN 4096
#define CONFIG_TCP_WMEM_DEFAULT 16384
#define CONFIG_TCP_WMEM_MAX 4194304
#define CONFIG_TCP_RMEM_MIN 4096
#define CONFIG_TCP_RMEM_DEFAULT 87380
#define CONFIG_TCP_RMEM_MAX 6291456

/**
 * Default initial retransmission timeout and ranges,
 * TCP_TIMEOUT_INIT=1000ms, TCP_RTO_MIN=200ms and TCP_RTO_MAX=120000ms from net/tcp.h
 *
 * HZ is about 1 second, i.e., about 1000 milliseconds
 */
#define NET_TCP_HZ 1000
#define CONFIG_TCP_RTO_INIT NET_TCP_HZ
#define CONFIG_TCP_RTO_MIN (NET_TCP_HZ / 5)
#define CONFIG_TCP_RTO_MAX (NET_TCP_HZ * 120)

/**
 * Default delay ack times, from net/tcp.h
 */
#define CONFIG_TCP_DELACK_MIN (NET_TCP_HZ / 25)
#define CONFIG_TCP_DELACK_MAX (NET_TCP_HZ / 5)

/**
 * Minimum size of the send buffer per socket when TCP-autotuning is used.
 * This value was computed from "man tcp"
 *
 * @todo change this to a command line option accessible via #Configuration
 */
#define CONFIG_SEND_BUFFER_MIN_SIZE 16384

/**
 * Minimum size of the receive buffer per socket when TCP-autotuning is used.
 * This value was computed from "man tcp"
 *
 * @todo change this to a command line option accessible via #Configuration
 */
#define CONFIG_RECV_BUFFER_MIN_SIZE 87380

/**
 * Default size of the send buffer per socket if TCP-autotuning is not used.
 * This value was computed from "man tcp"
 */
#define CONFIG_SEND_BUFFER_SIZE 131072

/**
 * Default size of the receive buffer per socket if TCP-autotuning is not used
 * This value was computed from "man tcp"
 */
#define CONFIG_RECV_BUFFER_SIZE 174760

/**
 * Default size for pipes. Value taken from "man 7 pipe".
 */
#define CONFIG_PIPE_BUFFER_SIZE 65536

/**
 * Default batching time when the network interface receives packets
 */
#define CONFIG_RECEIVE_BATCH_TIME (10*SIMTIME_ONE_MILLISECOND)

/**
 * Default IP header size in bytes.
 */
#define CONFIG_HEADER_SIZE_IP 20

/**
 * Default UDP header size in bytes.
 */
#define CONFIG_HEADER_SIZE_UDP 8

/**
 * Default TCP header size in bytes.
 */
#define CONFIG_HEADER_SIZE_TCP 20

/**
 * Header size in bytes of a routable packet with UDP encapsulation; includes
 * the IP and UDP headers but excludes the ethernet header and packet payload.
 */
#define CONFIG_HEADER_SIZE_UDPIP (CONFIG_HEADER_SIZE_UDP+CONFIG_HEADER_SIZE_IP)

/**
 * Header size in bytes of a routable packet with TCP encapsulation; includes
 * the IP and TCP headers but excludes the ethernet header and packet payload.
 */
#define CONFIG_HEADER_SIZE_TCPIP (CONFIG_HEADER_SIZE_TCP+CONFIG_HEADER_SIZE_IP)

/**
 * Maximum size of an IP packet without fragmenting over Ethernetv2
 */
#define CONFIG_MTU 1500

/**
 * Maximum segment size of a TCP payload, not counting the IP or TCP header
 */
#define CONFIG_TCP_MAX_SEGMENT_SIZE (CONFIG_MTU-CONFIG_HEADER_SIZE_TCPIP)

/**
 * Maximum size of a datagram we are allowed to send out over the network
 */
#define CONFIG_DATAGRAM_MAX_SIZE 65507

/**
 * Delay in nanoseconds for a TCP close timer.
 */
#define CONFIG_TCPCLOSETIMER_DELAY (60 * SIMTIME_ONE_SECOND)

#endif /* SHD_DEFINITIONS_H_ */
