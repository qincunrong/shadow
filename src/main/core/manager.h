/*
 * The Shadow Simulator
 *
 * Copyright (c) 2010-2011 Rob Jansen <jansen@cs.umn.edu>
 * Copyright (c) 2011-2013
 * To the extent that a federal employee is an author of a portion
 * of this software or a derivative work thereof, no copyright is
 * claimed by the United States Government, as represented by the
 * Secretary of the Navy ("GOVERNMENT") under Title 17, U.S. Code.
 * All Other Rights Reserved.
 *
 * Permission to use, copy, and modify this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * GOVERNMENT ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION
 * AND DISCLAIMS ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER
 * RESULTING FROM THE USE OF THIS SOFTWARE.
 */

#ifndef SHD_MANAGER_H_
#define SHD_MANAGER_H_

#include <glib.h>
#include <netinet/in.h>

#include "main/core/support/definitions.h"
#include "main/host/host_parameters.h"
#include "main/routing/dns.h"

typedef struct _Manager Manager;

Manager* manager_new(const Controller* controller, const ConfigOptions* config,
                     SimulationTime endTime, guint randomSeed);
void manager_free(Manager* manager);

ChildPidWatcher* manager_childpidwatcher(Manager* manager);

guint manager_getRawCPUFrequency(Manager* manager);
DNS* manager_getDNS(Manager* manager);
guint32 manager_getNodeBandwidthUp(Manager* manager, in_addr_t ip);
guint32 manager_getNodeBandwidthDown(Manager* manager, in_addr_t ip);
void manager_updateMinRunahead(Manager* manager, SimulationTime time);
SimulationTime manager_getLatencyForAddresses(Manager* manager, Address* sourceAddress,
                                              Address* destinationAddress);
gfloat manager_getReliabilityForAddresses(Manager* manager, Address* sourceAddress,
                                          Address* destinationAddress);
bool manager_isRoutable(Manager* manager, Address* sourceAddress, Address* destinationAddress);
void manager_incrementPacketCount(Manager* manager, Address* sourceAddress,
                                  Address* destinationAddress);

const ConfigOptions* manager_getConfig(Manager* manager);

void manager_incrementPluginError(Manager* manager);
const gchar* manager_getHostsRootPath(Manager* manager);

void manager_run(Manager*);

/* info received from controller to set up the simulation */
int manager_addNewVirtualHost(Manager* manager, HostParameters* params);
void manager_addNewVirtualProcess(Manager* manager, const gchar* hostName, const gchar* pluginName,
                                  SimulationTime startTime, SimulationTime stopTime,
                                  const gchar* const* argv, const char* environment,
                                  bool pause_for_debugging);

#endif /* SHD_MANAGER_H_ */
