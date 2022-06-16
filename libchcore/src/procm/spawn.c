#include <chcore/procm.h>
#include <chcore/ipc.h>
#include <chcore/assert.h>
#include <chcore/internal/server_caps.h>
#include <string.h>

static struct ipc_struct *procm_ipc_struct = NULL;

static void connect_procm_server(void)
{
        int procm_cap = __chcore_get_procm_cap();
        chcore_assert(procm_cap >= 0);
        procm_ipc_struct = ipc_register_client(procm_cap);
        chcore_assert(procm_ipc_struct);
}

int chcore_procm_spawn(const char *path, int *mt_cap_out)
{
        if (!procm_ipc_struct) {
                connect_procm_server();
        }

        struct ipc_msg *ipc_msg = ipc_create_msg(
                procm_ipc_struct, sizeof(struct procm_ipc_data), 0);
        chcore_assert(ipc_msg);

        struct procm_ipc_data *procm_ipc_data =
                (struct procm_ipc_data *)ipc_get_msg_data(ipc_msg);
        procm_ipc_data->request = PROCM_IPC_REQ_SPAWN;
        strncpy(procm_ipc_data->spawn.args.path, path, PROCM_PATH_LEN - 1);
        procm_ipc_data->spawn.args.path[PROCM_PATH_LEN - 1] = '\0';
        int ret = ipc_call(procm_ipc_struct, ipc_msg);
        if (ret == 0) {
                ret = procm_ipc_data->spawn.returns.pid;
                if (mt_cap_out) {
                        *mt_cap_out = ipc_get_msg_cap(ipc_msg, 0);
                }
        }
        ipc_destroy_msg(procm_ipc_struct, ipc_msg);
        return ret;
}
