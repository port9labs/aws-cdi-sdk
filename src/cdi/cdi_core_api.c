// ---------------------------------------------------------------------------
// Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
// ---------------------------------------------------------------------------

/**
 * @file
 * @brief
 * This file contains the definitions of the functions that comprise the CDI Core SDK's API.
 */

// Include headers in the following order: Related header, C system headers, other libraries' headers, your project's
// headers.

#include "cdi_core_api.h"

#include "internal.h"
#include "internal_rx.h"

//*********************************************************************************************************************
//***************************************** START OF DEFINITIONS AND TYPES ********************************************
//*********************************************************************************************************************

//*********************************************************************************************************************
//*********************************************** START OF VARIABLES **************************************************
//*********************************************************************************************************************

//*********************************************************************************************************************
//******************************************* START OF STATIC FUNCTIONS ***********************************************
//*********************************************************************************************************************

//*********************************************************************************************************************
//******************************************* START OF PUBLIC FUNCTIONS ***********************************************
//*********************************************************************************************************************

////////////////////////////////////////////////////////////////////////////////
// Doxygen commenting for these functions is in cdi_core_api.h.
////////////////////////////////////////////////////////////////////////////////

CdiReturnStatus CdiCoreInitialize(const CdiCoreConfigData* core_config_ptr)
{
    CdiReturnStatus rs = kCdiStatusOk;

    if (NULL == core_config_ptr) {
        return kCdiStatusInvalidParameter;
    }

    // Set up anything in global context required by CdiGlobalInitialization().
    rs = CdiGlobalInitialization(core_config_ptr);

    return rs;
}

CdiReturnStatus CdiCoreNetworkAdapterInitialize(CdiAdapterData* adapter_data_ptr,
                                                CdiAdapterHandle* ret_handle_ptr)
{
    CdiReturnStatus rs = kCdiStatusOk;

    if (!cdi_global_context.sdk_initialized) {
        return kCdiStatusFatal;
    }

    // initialize the adapter
    if (rs == kCdiStatusOk) {
        rs = AdapterInitializeInternal(adapter_data_ptr, ret_handle_ptr);
    }

    return rs;
}

CdiReturnStatus CdiCoreRxFreeBuffer(const CdiSgList* sgl_ptr)
{
    CdiReturnStatus rs = kCdiStatusOk;

    if (NULL == sgl_ptr) {
        return kCdiStatusInvalidParameter;
    }

    // Don't process an internally generated empty SGL.
    if (sgl_ptr->sgl_head_ptr != &cdi_global_context.empty_sgl_entry) {
        if (!IsValidMemoryHandle(sgl_ptr->internal_data_ptr)) {
            rs = kCdiStatusInvalidHandle;
        } else {
            // Return the packet buffers and SGL entries to the endpoint.
            rs = RxEnqueueFreeBuffer(sgl_ptr);
        }
    }

    return rs;
}

int CdiCoreGather(const CdiSgList* sgl_ptr, int offset, void* dest_data, int byte_count)
{
    if (NULL == sgl_ptr) {
        return kCdiStatusInvalidParameter;
    }

    if (NULL == dest_data) {
        return kCdiStatusInvalidParameter;
    }

    return CdiGatherInternal(sgl_ptr, offset, dest_data, byte_count);
}

CDI_INTERFACE CdiReturnStatus CdiCoreStatsReconfigure(CdiConnectionHandle handle, const CdiStatsConfigData* config_ptr)
{
    if (!IsValidConnectionHandle(handle)) {
        return kCdiStatusInvalidHandle;
    }

    // Use false here so settings are only applied if they have changed.
    return CoreStatsConfigureInternal(handle, config_ptr, false);
}

CdiReturnStatus CdiCoreConnectionDestroy(CdiConnectionHandle handle)
{
    if (!IsValidConnectionHandle(handle)) {
        return kCdiStatusInvalidHandle;
    }

    ConnectionDestroyInternal(handle);
    return kCdiStatusOk;
}

CdiReturnStatus CdiCoreShutdown(void)
{
    return SdkShutdownInternal();
}

void CdiCoreGetUtcTime(struct timespec* ret_time_ptr)
{
    CdiOsGetUtcTime(ret_time_ptr);
}

uint64_t CdiCoreGetUtcTimeMicroseconds(void)
{
    struct timespec utc_time;
    CdiCoreGetUtcTime(&utc_time);

    return (uint64_t)utc_time.tv_sec * 1000000L + (utc_time.tv_nsec / 1000L);
}


const char* CdiCoreStatusToString(CdiReturnStatus status)
{
    static const EnumStringKey key_array[] = {
        { kCdiStatusOk,                    "OK"                             },
        { kCdiStatusFatal,                 "fatal error encountered"        },
        { kCdiStatusNotEnoughMemory,       "not enough memory"              },
        { kCdiStatusNotInitialized,        "not initialized"                },
        { kCdiStatusMaxLatencyExceeded,    "maximum latency exceeded"       },
        { kCdiStatusInvalidHandle,         "invalid handle"                 },
        { kCdiStatusInvalidParameter,      "invalid parameter"              },
        { kCdiStatusNotConnected,          "not connected"                  },
        { kCdiStatusQueueFull,             "queue full"                     },
        { kCdiStatusInvalidConnectionType, "invalid connection type"        },
        { kCdiStatusRxPayloadError,        "receive payload error"          },
        { kCdiStatusRxWrongProtocolType,   "received wrong protocol type"   },
        { kCdiStatusCreateLogFailed,       "failed to create log"           },
        { kCdiStatusCreateThreadFailed,    "failed to create thread"        },
        { kCdiStatusShuttingDown,          "failed to shutdown"             },
        { kCdiStatusWrongDirection,        "wrong endpoint direction"       },
        { kCdiStatusGetPortFailed,         "failed to get port"             },
        { kCdiStatusNotReady,              "connection not ready"           },
        { kCdiStatusSendFailed,            "failed to send a packet"        },
        { kCdiStatusAllocationFailed,      "failed to allocate resource"    },
        { kCdiStatusOpenFailed,            "failed to open a port"          },
        { kCdiStatusDuplicate,             "duplicate connection error"     },
        { kCdiStatusInvalidSgl,            "scatter-gather list is invalid" },
        { kCdiStatusEndpointManagerState,  "endpoint manager state changed" },
        { kCdiStatusBufferOverflow,        "buffer overflowed"              },
        { kCdiStatusArraySizeExceeded,     "array size exceeded"            },
        { kCdiStatusNonFatal,              "non-fatal error encountered"    },
        { kCdiStatusCloudWatchNotEnabled,  "CloudWatch SDK not enabled"     },
        { kCdiStatusCloudWatchThrottling,  "CloudWatch throttling - retry"  },
        { kCdiStatusCloudWatchInvalidCredentials, "CloudWatch invalid credentials" },
        { CDI_INVALID_ENUM_VALUE,          "<invalid>"                      },
    };

    return CdiUtilityEnumValueToString(key_array, status);
}
