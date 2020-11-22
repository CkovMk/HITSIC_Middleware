#include "app_menu_nvm.hpp"

#if defined(HITSIC_MENU_USE_NVM) && (HITSIC_MENU_USE_NVM > 0)

/*!
 * @addtogroup menu_nvm
 * @{
 */
#define SYSLOG_TAG  ("MENU.KVDB")
#define SYSLOG_LVL  (HITSIC_MENU_KVDB_LOG_LVL)
#include "inc_syslog.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * ********** NVM存储变量定义 **********
     */

    /**
     * @brief : 每个扇区包含的字节数
     */

    /**
     * @brief : 全局存储 Global Storage
     */
    uint32_t menu_nvm_glAddrOffset;/// 全局存储区地址偏移

    /**
     * @brief : 局部存储 Region Storage
     */


    // 每个局部存储区占用的扇区数
    uint32_t menu_nvm_rgSectOffset
    [HITSIC_MENU_NVM_REGION_CNT];/// 三个局部存储区的扇区偏移

    uint32_t menu_nvm_rgAddrOffset
    [HITSIC_MENU_NVM_REGION_CNT];/// 三个局部存储区的地址偏移

    /**
     * @brief : 菜单存储占用的总扇区数
     */
    uint32_t menu_nvm_totalSectCnt;
    /**
     * @brief : 每个菜单项保存时占用的字节数
     */
    uint32_t menu_nvm_dataSize = 32u;

    /**
     * @brief : 菜单项写入缓存。
     * 当改写第N个扇区时，menu_nvm_cachedSector = N, menu_nvm_cache分配4KB缓存
     * 并读入第N扇区的所有内容。此时能且仅能修改第N扇区的内容。对第N扇区内容的修改
     * 将缓存至上述内存。
     */
    uint8_t *menu_nvm_cache = NULL;
    uint32_t menu_nvm_cachedSector = 0;
    /**
     * @brief : 菜单进行全局擦除/保存的次数，可用于估计Flash寿命
     */
    uint32_t menu_nvm_eraseCnt = 0;

    /**
     * ********** NVM存储操作接口 **********
     */

    status_t MENU_NvmRead(uint32_t _addr, void *_buf, uint32_t _byteCnt)
    {
        SYSLOG_V("Rx Addr = 0x%8.8x, Size = %4.4d", _addr, _byteCnt);
        if (HITSIC_MENU_NVM_RETVAL_SUCCESS ==
                HITSIC_MENU_NVM_AddressRead(_addr, _buf, _byteCnt))
        {
            return kStatus_Success;
        }
        else
        {
            return kStatus_Fail;
        }
    }

    bool MENU_NvmCacheable(uint32_t _addr)
    {
        if (menu_nvm_cache == NULL ||
                _addr / flash_sectorSize == menu_nvm_cachedSector)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    status_t MENU_NvmCacheSector(uint32_t _sect)
    {
        if (menu_nvm_cache != NULL)
        {
            return kStatus_Fail;
        }
        menu_nvm_cache = (uint8_t *)malloc(flash_sectorSize);
        if (menu_nvm_cache == NULL)
        {
            SYSLOG_E("Cached Sector %2.2d\n Failed! [-MemMalloc]",
                    menu_nvm_cachedSector);
            return kStatus_Fail;
        }
        if (HITSIC_MENU_NVM_RETVAL_SUCCESS !=
                HITSIC_MENU_NVM_SectorRead(menu_nvm_cachedSector,
                        (void *)menu_nvm_cache))
        {
            free(menu_nvm_cache);
            menu_nvm_cache = NULL;
            SYSLOG_E("Cached Sector %2.2d\n Failed! [-FlashRead]",
                    menu_nvm_cachedSector);
            return kStatus_Fail;
        }
        menu_nvm_cachedSector = _sect;
        SYSLOG_V("Cached Sector %2.2d", menu_nvm_cachedSector);
        return kStatus_Success;
    }

    status_t MENU_NvmWriteCache(uint32_t _addr, void *_buf, uint32_t _byteCnt)
    {
        if (menu_nvm_cache == NULL)
        {
            if (HITSIC_MENU_NVM_RETVAL_SUCCESS !=
                    MENU_NvmCacheSector(_addr / HITSIC_MENU_NVM_SECTOR_SIZE))
            {
                return kStatus_Fail;
            }
        }
        memcpy(menu_nvm_cache + _addr % flash_sectorSize, _buf, _byteCnt);
        SYSLOG_V("Tx Addr = 0x%8.8x, Size = %4.4d", _addr, _byteCnt);
        return kStatus_Success;
    }

    status_t MENU_NvmUpdateCache(void)
    {
        if (HITSIC_MENU_NVM_RETVAL_SUCCESS !=
                HITSIC_MENU_NVM_SectorWrite(menu_nvm_cachedSector, menu_nvm_cache))
        {
            return kStatus_Fail;
        }
        // void* readBuf = malloc(flash_sectorSize);
        // FLASH_SectorRead(menu_nvm_cachedSector, readBuf);
        SYSLOG_V("Update Cached Sector %2.2d", menu_nvm_cachedSector);
        //		if(memcmp(readBuf, menu_nvm_cache, flash_sectorSize) != 0)
        //		{
        //			HITSIC_MENU_PRINTF("Warning: MENU: Nvm Update Cache Fail.\n");
        //		}
        //		free(readBuf);
        //		readBuf = NULL;

        free(menu_nvm_cache);
        menu_nvm_cache = NULL;
        return kStatus_Success;
    }

#ifdef __cplusplus
}
#endif

#include "easyflash.h"

namespace menu{

class easyflashBackend_t//FIXME
{
private:
    static const char registryKey = "menu.registry";
    // key: menu.reg
    __PACKED struct registry_t
    {
        uint16_t uid;
        menu_itemType_t type;
        uint8_t region;
        char nameStr[menu_nameStrSize];
    };

//    __PACKED struct storageItem_t
//    {
//        uint32_t itemData;
//    };

    std::unique_ptr<registry_t> registry;

    /**
     * @brief 将菜单项转换为注册表项
     *
     * @param reg 注册表项
     * @param item 要转换的菜单项
     */
    void ConvertRegistry(registry_t *reg, menu_itemIfce_t *item)
    {
        reg->uid = item->saveAddr;
        reg->type = item->type;
        reg->region = menu_currRegionNum;
        memcpy(data->nameStr, item->nameStr, menu_nameStrSize);
    }

    bool MatchRegistry(registry_t *reg, menu_itemIfce_t *item)
    {
        bool result = true;
        result = reg->uid == item->saveAddr;
        result = reg->type == item->type;
        result = reg->region == menu_currRegionNum;
        result = memcmp(data->nameStr, item->nameStr, menu_nameStrSize);
        return result;
    }

public:
    easyflashBackend_t(void)
    {
        registry = nullptr;
    }

    status_t Init(void)
    {
        uint32_t result= 0U;
        uint32_t size = 0U;
        /** 获取注册表大小 */
        result = ef_get_env_blob(registryKey, nullptr, 0, &size);
        if(0U == result)
        {
            SYSLOG_I("Registry not found. Creating new registry first.");
            return kStatus_Fail;
        }
        SYSLOG_I("Registry found. %6.6d byte(s), %4.4d item(s).", size, size / sizeof(registry_t));
        registry = new registry_t[size / sizeof(registry_t)];
        result = ef_get_env_blob(registryKey, (void*)registry, size, nullptr);
        if(result != size)
        {
            SYSLOG_W("Registry size error ! \"%s\" line %d.", __FILE__, __LINE__);
        }

    }

    status_t SaveItem(menu_itemIfce_t *item)
    {

    }

    status_t LoadItem(menu_itemIfce_t *item)
    {

    }

};


} // ! namespace menu

/* @} */

#endif // ! HITSIC_MENU_USE_NVM
