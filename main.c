#include "system.h"

int main(void)
{
    /* system 保存整个固件运行过程中要反复使用的状态和数据。 */
    static SystemContext system;

    /* 上电后先做一次完整初始化，失败则停在这里等待排查。 */
    if (System_Init(&system) != SSCB_OK)
    {
        for (;;)
        {
        }
    }

    /* 初始化成功后，主循环不断重复执行一次系统任务。 */
    for (;;)
    {
        System_RunOnce(&system);
    }
}
