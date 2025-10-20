// OTA program entry: Download, Install

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdio.h>
#include <string.h>
#include "register.h"
#include "bt_pan_ota.h"
// Add the relevant Bluetooth header files.
#include "bts2_app_inc.h"
#include "bt_connection_manager.h"
#include "ble_connection_manager.h"
#include "bf0_sibles_internal.h"
#define LOG_TAG "ota_install"
#include "log.h"

#if defined(RT_USING_DFS_MNTTABLE)
#include "dfs_posix.h"
#include "flash_map.h"
const struct dfs_mount_tbl mount_table[] =
{
    FS_MOUNT_TABLE
};
#endif

#define CUSTOM_OTA_MODE_REBOOT_TO_OTA 1
#define CUSTOM_OTA_MODE_NONE          0

// Bluetooth PAN-related Definitions
#define BT_APP_READY 1
#define BT_APP_CONNECT_PAN  2
#define PAN_TIMER_MS        3000


static rt_mailbox_t g_bt_app_mb;
BOOL dfu_pan_connected = FALSE;
#define BLUETOOTH_NAME "sifli-pan"
#define OTA_BT_APP_CONNECT_PAN_SUCCESS 1

//UI-related parameters
#include "dfu_pan_ui.h"
#define DFU_PAN_UI_THREAD_STACK_SIZE (6144)
static struct rt_thread dfu_pan_ui_thread;
static uint8_t dfu_pan_ui_thread_stack[6144];

void bt_app_connect_pan_timeout_handle(void *parameter)
{
    LOG_I("bt_app_connect_pan_timeout_handle %x, %d", g_bt_app_mb,
          g_bt_app_env_ota.bt_connected);
    if ((g_bt_app_mb != NULL) && (g_bt_app_env_ota.bt_connected))
        rt_mb_send(g_bt_app_mb, BT_APP_CONNECT_PAN);
    return;
}
static int bt_app_interface_event_handle(uint16_t type, uint16_t event_id,
                                         uint8_t *data, uint16_t data_len)
{

    LOG_I("bt_app_interface_event_handle_type: %d\n",type);
    if (type == BT_NOTIFY_COMMON)
    {
        int pan_conn = 0;

        switch (event_id)
        {
        case BT_NOTIFY_COMMON_BT_STACK_READY:
        {
            LOG_I("BT_NOTIFY_COMMON_BT_STACK_READY\n");
            rt_mb_send(g_bt_app_mb, BT_APP_READY);
        }
        break;
        case BT_NOTIFY_COMMON_ACL_CONNECTED:
        {
            dfu_pan_ui_update_message(UI_MSG_UPDATE_BLE, UI_MSG_DATA_BLE_CONNECTED);
            LOG_I("BT_NOTIFY_COMMON_ACL_CONNECTED\n");

        }
        break;
        case BT_NOTIFY_COMMON_ACL_DISCONNECTED:
        {
            dfu_pan_ui_update_message(UI_MSG_UPDATE_BLE, UI_MSG_DATA_BLE_DISCONNECTED);

            bt_notify_device_base_info_t *info =
                (bt_notify_device_base_info_t *)data;
            LOG_I("disconnected(0x%.2x:%.2x:%.2x:%.2x:%.2x:%.2x) res %d",
                  info->mac.addr[5], info->mac.addr[4], info->mac.addr[3],
                  info->mac.addr[2], info->mac.addr[1], info->mac.addr[0],
                  info->res);
            g_bt_app_env_ota.bt_connected = FALSE;
            //  memset(&g_bt_app_env_ota.bd_addr, 0xFF,
            //  sizeof(g_bt_app_env_ota.bd_addr));
                 if (info->res == BT_NOTIFY_COMMON_SCO_DISCONNECTED) 
                {
                
                    LOG_I("Phone actively disconnected, prepare to enter sleep mode after 30 seconds");
                }
                else 
                {
                    LOG_I("Abnormal disconnection, start reconnect attempts");
                }
            
            if (g_bt_app_env_ota.pan_connect_timer)
                rt_timer_stop(g_bt_app_env_ota.pan_connect_timer);
        }
        break;
        case BT_NOTIFY_COMMON_ENCRYPTION:
        {
            bt_notify_device_mac_t *mac = (bt_notify_device_mac_t *)data;
            LOG_I("Encryption competed");
            g_bt_app_env_ota.bd_addr = *mac;
            pan_conn = 1;
        }
        break;
        case BT_NOTIFY_COMMON_PAIR_IND:
        {
            bt_notify_device_base_info_t *info =
                (bt_notify_device_base_info_t *)data;
            LOG_I("Pairing completed %d", info->res);
            if (info->res == BTS2_SUCC)
            {
                g_bt_app_env_ota.bd_addr = info->mac;
                pan_conn = 1;
            }
        }
        break;
        case BT_NOTIFY_COMMON_KEY_MISSING:
        {
            bt_notify_device_base_info_t *info =
                (bt_notify_device_base_info_t *)data;
            LOG_I("Key missing %d", info->res);
            memset(&g_bt_app_env_ota.bd_addr, 0xFF, sizeof(g_bt_app_env_ota.bd_addr));
            bt_cm_delete_bonded_devs_and_linkkey(info->mac.addr);
        }
        break;
        default:
            break;
        }

        if (pan_conn)
        {
            LOG_I("bd addr 0x%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
                  g_bt_app_env_ota.bd_addr.addr[5], g_bt_app_env_ota.bd_addr.addr[4],
                  g_bt_app_env_ota.bd_addr.addr[3], g_bt_app_env_ota.bd_addr.addr[2],
                  g_bt_app_env_ota.bd_addr.addr[1], g_bt_app_env_ota.bd_addr.addr[0]);
            g_bt_app_env_ota.bt_connected = TRUE;
            // Trigger PAN connection after PAN_TIMER_MS period to avoid SDP
            // confliction.
            if (!g_bt_app_env_ota.pan_connect_timer)
                g_bt_app_env_ota.pan_connect_timer = rt_timer_create(
                    "connect_pan", bt_app_connect_pan_timeout_handle,
                    (void *)&g_bt_app_env_ota,
                    rt_tick_from_millisecond(PAN_TIMER_MS),
                    RT_TIMER_FLAG_SOFT_TIMER);
            else
                rt_timer_stop(g_bt_app_env_ota.pan_connect_timer);
            rt_timer_start(g_bt_app_env_ota.pan_connect_timer);
        }
    }
    else if (type == BT_NOTIFY_PAN)
    {
        switch (event_id)
        {
        case BT_NOTIFY_PAN_PROFILE_CONNECTED:
        {
            LOG_I("pan connect successed \n");
            if ((g_bt_app_env_ota.pan_connect_timer))
            {
                rt_timer_stop(g_bt_app_env_ota.pan_connect_timer);
            }
            rt_mb_send(g_bt_app_mb, OTA_BT_APP_CONNECT_PAN_SUCCESS);
            dfu_pan_ui_update_message(UI_MSG_UPDATE_NET, UI_MSG_DATA_NET_CONNECTED);
            dfu_pan_connected = TRUE; 
        }
        break;
        case BT_NOTIFY_PAN_PROFILE_DISCONNECTED:
        {

            LOG_I("pan disconnect with remote device\n");
            dfu_pan_connected = FALSE; 
            dfu_pan_ui_update_message(UI_MSG_UPDATE_NET, UI_MSG_DATA_NET_DISCONNECTED);

        }
        break;
        default:
            break;
        }
    }
    else if (type == BT_NOTIFY_HID)
    {
        switch (event_id)
        {
        case BT_NOTIFY_HID_PROFILE_CONNECTED:
        {
            LOG_I("HID connected\n");
            if (!dfu_pan_connected)
            {
                if (g_bt_app_env_ota.pan_connect_timer)
                {
                    rt_timer_stop(g_bt_app_env_ota.pan_connect_timer);
                }
                bt_interface_conn_ext((char *)&g_bt_app_env_ota.bd_addr,
                                      BT_PROFILE_PAN);
            }
        }
        break;
        case BT_NOTIFY_HID_PROFILE_DISCONNECTED:
        {
            LOG_I("HID disconnected\n");
        }
        break;
        default:
            break;
        }
    }

    return 0;
}



int main(void)
{


    rt_err_t result = rt_thread_init(&dfu_pan_ui_thread,
                                     "dfu_pan_ui",
                                     dfu_pan_ui_task,
                                     NULL,
                                     &dfu_pan_ui_thread_stack[0],
                                     DFU_PAN_UI_THREAD_STACK_SIZE,
                                     30,
                                     10);
    if (result == RT_EOK)
    {
        rt_kprintf("xiaozhi UI thread init success\n");
        rt_thread_startup(&dfu_pan_ui_thread);
    }
    else
    {
        rt_kprintf("Failed to init xiaozhi UI thread\n");
    }



    g_bt_app_mb = rt_mb_create("bt_app", 8, RT_IPC_FLAG_FIFO);

#ifdef BSP_BT_CONNECTION_MANAGER
    bt_cm_set_profile_target(BT_CM_HID, BT_LINK_PHONE, 1);
#endif // BSP_BT_CONNECTION_MANAGER

    bt_interface_status_t status = bt_interface_register_bt_event_notify_callback(bt_app_interface_event_handle);
    sifli_ble_enable();

    LOG_I("---sifli_ble_enable---\n");


    while (1)
    {

        uint32_t value;

        // handle pan connect event
        rt_mb_recv(g_bt_app_mb, (rt_uint32_t *)&value, RT_WAITING_FOREVER);

        if (value == BT_APP_CONNECT_PAN)
        {
            LOG_I("BT_APP_CONNECT_PAN\n");
            if (g_bt_app_env_ota.bt_connected)
            {
                bt_interface_conn_ext((char *)&g_bt_app_env_ota.bd_addr,
                                      BT_PROFILE_PAN);
            }
        }
        else if (value == BT_APP_READY)
        {
            LOG_I("BT/BLE stack and profile ready");

#ifdef BT_NAME_MAC_ENABLE
            LOG_I("BT_NAME_MAC_Local-name: %s", BT_NAME_MAC);
            char local_name[32];
            bd_addr_t addr;
            ble_get_public_address(&addr);
            sprintf(local_name, "%s-%02x:%02x:%02x:%02x:%02x:%02x",
                    BLUETOOTH_NAME, addr.addr[0], addr.addr[1], addr.addr[2],
                    addr.addr[3], addr.addr[4], addr.addr[5]);
#else
            const char *local_name = BLUETOOTH_NAME;
#endif

        LOG_I("------Set_local_name------: %s\n", local_name);

            bt_interface_set_local_name(strlen(local_name), (void *)local_name);
        }
        else if (value == OTA_BT_APP_CONNECT_PAN_SUCCESS)
        {
            rt_kputs("OTA_BT_APP_CONNECT_PAN_SUCCESS\r\n");
            LOG_I("------start_ota------\n");
        }
    }

    return RT_EOK;
}


static void ota_cmd_start(int argc, char **argv)
{
    if (strcmp(argv[1], "del_bond") == 0)
    {
#ifdef BSP_BT_CONNECTION_MANAGER
        bt_cm_delete_bonded_devs();
        LOG_D("Delete bond");
#endif // BSP_BT_CONNECTION_MANAGER
    }
    // only valid after connection setup but phone didn't enable pernal hop
    else if (strcmp(argv[1], "conn_pan") == 0)
        bt_app_connect_pan_timeout_handle(NULL);
}
MSH_CMD_EXPORT(ota_cmd_start, Connect PAN to last paired device);

static void dfu_pan_print_files_cmd(int argc, char **argv)
{

    dfu_pan_print_files();

}
MSH_CMD_EXPORT(dfu_pan_print_files_cmd, Print OTA version files status);

static void dfu_pan_finish_cmd(int argc, char **argv)
{
    dfu_pan_test_update_flags();
}
MSH_CMD_EXPORT(dfu_pan_finish_cmd, OTA finish verification command);


static void dfu_pan_down_files_cmd(int argc, char **argv)
{
    if (argc < 2) {
        rt_kprintf("Usage: ota_down_files_cmd <version_index>\n");
        rt_kprintf("Available versions:\n");
        
        // Modify to use the dfu_pan_get_version_info function to read the version information
        for (int i = 0; i < MAX_VERSION_COUNT; i++) {
            struct version_info temp_version;
            if (dfu_pan_get_version_info(i, &temp_version) == 0 && temp_version.name[0] != '\0') {
                rt_kprintf("[%d] %s - %s\n", i, temp_version.name, 
                          temp_version.needs_update ? "Available for update" : "Current or older version");
            }
        }
        return;
    }
    
    int version_index = atoi(argv[1]);
    
    // Verify the validity of the index
    if (version_index < 0 || version_index >= MAX_VERSION_COUNT) {
        rt_kprintf("Invalid version index. Please choose a valid index.\n");
        return;
    }
    
    // Read version information
    struct version_info selected_version;
    if (dfu_pan_get_version_info(version_index, &selected_version) != 0 || selected_version.name[0] == '\0') {
        rt_kprintf("Invalid version index or failed to read version info. Please choose a valid index from the list above.\n");
        return;
    }
    
    rt_kprintf("Downloading version: %s\n", selected_version.name);
    dfu_pan_parse_package_from_url(selected_version.zippath);
}

MSH_CMD_EXPORT(dfu_pan_down_files_cmd, Download specific OTA version files);
