


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "wlan_types.h"

#include "oal_net.h"
#include "oal_cfg80211.h"
#include "oal_ext_if.h"
#include "frw_ext_if.h"

#include "dmac_ext_if.h"

#include "mac_device.h"
#include "mac_vap.h"
#include "mac_ie.h"
#include "mac_resource.h"

#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_ext_if.h"
#include "hmac_vap.h"
#include "hmac_p2p.h"

#include "wal_linux_cfg80211.h"
#include "wal_linux_scan.h"
#include "wal_linux_event.h"
#include "wal_main.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "wal_regdb.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_flowctl.h"

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#include "wal_linux_cfgvendor.h"
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif //#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

#ifdef _PRE_WLAN_FEATURE_AP_PM
#include "mac_pm.h"
#endif
#include "hmac_dfs.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "board.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CFG80211_C
#define WAL_BCN_BSSID_LENGTH_ADDR    (37)
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_DFR
extern  hmac_dfr_info_stru    g_st_dfr_info_etc;
#endif // _PRE_WLAN_FEATURE_DFR
#ifdef _PRE_WLAN_FEATURE_AP_PM
extern oal_uint8 g_uc_is_cpu_sleep;
#endif
#define hi1151_a_rates           (hi1151_rates + 4)
#define hi1151_a_rates_size      8
#define hi1151_g_rates           (hi1151_rates + 0)
#define hi1151_g_rates_size      12

/* 设备支持的速率 */
OAL_STATIC oal_ieee80211_rate hi1151_rates[] = {
    RATETAB_ENT(10,  0x1,   0),
    RATETAB_ENT(20,  0x2,   0),
    RATETAB_ENT(55,  0x4,   0),
    RATETAB_ENT(110, 0x8,   0),
    RATETAB_ENT(60,  0x10,  0),
    RATETAB_ENT(90,  0x20,  0),
    RATETAB_ENT(120, 0x40,  0),
    RATETAB_ENT(180, 0x80,  0),
    RATETAB_ENT(240, 0x100, 0),
    RATETAB_ENT(360, 0x200, 0),
    RATETAB_ENT(480, 0x400, 0),
    RATETAB_ENT(540, 0x800, 0),
};

/* 2.4G 频段 */
OAL_STATIC oal_ieee80211_channel g_ast_supported_channels_2ghz_info[] = {
    CHAN2G(1, 2412, 0),
    CHAN2G(2, 2417, 0),
    CHAN2G(3, 2422, 0),
    CHAN2G(4, 2427, 0),
    CHAN2G(5, 2432, 0),
    CHAN2G(6, 2437, 0),
    CHAN2G(7, 2442, 0),
    CHAN2G(8, 2447, 0),
    CHAN2G(9, 2452, 0),
    CHAN2G(10, 2457, 0),
    CHAN2G(11, 2462, 0),
    CHAN2G(12, 2467, 0),
    CHAN2G(13, 2472, 0),
    CHAN2G(14, 2484, 0),
};

/* 5G 频段 */
OAL_STATIC oal_ieee80211_channel g_ast_supported_channels_5ghz_info[] = {
    CHAN5G(36, 0),
    CHAN5G(40, 0),
    CHAN5G(44, 0),
    CHAN5G(48, 0),
    CHAN5G(52, 0),
    CHAN5G(56, 0),
    CHAN5G(60, 0),
    CHAN5G(64, 0),
    CHAN5G(100, 0),
    CHAN5G(104, 0),
    CHAN5G(108, 0),
    CHAN5G(112, 0),
    CHAN5G(116, 0),
    CHAN5G(120, 0),
    CHAN5G(124, 0),
    CHAN5G(128, 0),
    CHAN5G(132, 0),
    CHAN5G(136, 0),
    CHAN5G(140, 0),
    CHAN5G(144, 0),
    CHAN5G(149, 0),
    CHAN5G(153, 0),
    CHAN5G(157, 0),
    CHAN5G(161, 0),
    CHAN5G(165, 0),
    /* 4.9G */
    CHAN4_9G(184, 0),
    CHAN4_9G(188, 0),
    CHAN4_9G(192, 0),
    CHAN4_9G(196, 0),
};

/* 设备支持的加密套件 */
OAL_STATIC const oal_uint32 g_ast_wlan_supported_cipher_suites[] = {
    WLAN_CIPHER_SUITE_WEP40,
    WLAN_CIPHER_SUITE_WEP104,
    WLAN_CIPHER_SUITE_TKIP,
    WLAN_CIPHER_SUITE_CCMP,
#ifdef _PRE_WLAN_FEATURE_GCMP_256_CCMP256
    WLAN_CIPHER_SUITE_GCMP,
    WLAN_CIPHER_SUITE_GCMP_256,
    WLAN_CIPHER_SUITE_CCMP_256,
#endif
    WLAN_CIPHER_SUITE_AES_CMAC,
    WLAN_CIPHER_SUITE_BIP_CMAC_256,
    WLAN_CIPHER_SUITE_SMS4,
};

extern OAL_CONST wal_ioctl_mode_map_stru g_ast_mode_map_etc[];

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

/* 2.4G 频段信息 */
oal_ieee80211_supported_band g_st_supported_band_2ghz_info = {
    .channels   = g_ast_supported_channels_2ghz_info,
    .n_channels = sizeof(g_ast_supported_channels_2ghz_info)/sizeof(oal_ieee80211_channel),
    .bitrates   = hi1151_g_rates,
    .n_bitrates = hi1151_g_rates_size,
    .ht_cap = {
        .ht_supported = OAL_TRUE,
        .cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
    },
};

/* 5G 频段信息 */
OAL_STATIC oal_ieee80211_supported_band g_st_supported_band_5ghz_info = {
    .channels   = g_ast_supported_channels_5ghz_info,
    .n_channels = sizeof(g_ast_supported_channels_5ghz_info)/sizeof(oal_ieee80211_channel),
    .bitrates   = hi1151_a_rates,
    .n_bitrates = hi1151_a_rates_size,
    .ht_cap = {
        .ht_supported = OAL_TRUE,
        .cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
    },
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
    .vht_cap = {
        .vht_supported = OAL_TRUE,
        .cap = IEEE80211_VHT_CAP_SHORT_GI_80 | IEEE80211_VHT_CAP_HTC_VHT | IEEE80211_VHT_CAP_SHORT_GI_160 | IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ,
    },
#endif
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
#ifdef _PRE_WLAN_FEATURE_P2P
OAL_STATIC oal_ieee80211_iface_limit sta_p2p_limits[] = {
    {
    .max = 2,
    .types = BIT(NL80211_IFTYPE_STATION),
    },
    {
    .max = 2,
    .types = BIT(NL80211_IFTYPE_P2P_GO) | BIT(NL80211_IFTYPE_P2P_CLIENT),
    },
    {
    .max = 1,
    .types = BIT(NL80211_IFTYPE_P2P_DEVICE),
    },
};

OAL_STATIC oal_ieee80211_iface_combination
sta_p2p_iface_combinations[] = {
    {
    .num_different_channels = 2,
    .max_interfaces = 3,
    .limits = sta_p2p_limits,
    .n_limits = OAL_ARRAY_SIZE(sta_p2p_limits),
    },
};

#else  /* 非p2p场景 */
OAL_STATIC oal_ieee80211_iface_limit ap_no_limits[] = {
    {
 #if (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5)
    .max = 2,
 #elif defined(_PRE_WLAN_PRODUCT_1151V200)
    .max = 16,
 #else
    .max = 4,
 #endif
    .types = BIT(NL80211_IFTYPE_AP),
    },
    {
 #ifdef _PRE_WLAN_PRODUCT_1151V200
    .max = 3,
 #else
    .max = 1,
 #endif
    .types = BIT(NL80211_IFTYPE_STATION),
    },
};

OAL_STATIC oal_ieee80211_iface_combination
ap_iface_combinations[] = {
    {
    .num_different_channels = 2,
 #if (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5)
    .max_interfaces = 2,
 #elif defined(_PRE_WLAN_PRODUCT_1151V200)
    .max_interfaces = 16,
 #else
    .max_interfaces = 4,
 #endif
    .limits = ap_no_limits,
    .n_limits = OAL_ARRAY_SIZE(ap_no_limits),
    },
};

#endif

/* There isn't a lot of sense in it, but you can transmit anything you like */
static const struct ieee80211_txrx_stypes
wal_cfg80211_default_mgmt_stypes[NUM_NL80211_IFTYPES] = {
    [NL80211_IFTYPE_ADHOC] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4)
    },
    [NL80211_IFTYPE_STATION] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
    },
    [NL80211_IFTYPE_AP] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |//lint !e572
        BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        BIT(IEEE80211_STYPE_DISASSOC >> 4) |
        BIT(IEEE80211_STYPE_AUTH >> 4) |
        BIT(IEEE80211_STYPE_DEAUTH >> 4) |
        BIT(IEEE80211_STYPE_ACTION >> 4)
    },
    [NL80211_IFTYPE_AP_VLAN] = {
        /* copy AP */
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |//lint !e572
        BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        BIT(IEEE80211_STYPE_DISASSOC >> 4) |
        BIT(IEEE80211_STYPE_AUTH >> 4) |
        BIT(IEEE80211_STYPE_DEAUTH >> 4) |
        BIT(IEEE80211_STYPE_ACTION >> 4)
    },
    [NL80211_IFTYPE_P2P_CLIENT] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
    },
    [NL80211_IFTYPE_P2P_GO] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |//lint !e572
        BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        BIT(IEEE80211_STYPE_DISASSOC >> 4) |
        BIT(IEEE80211_STYPE_AUTH >> 4) |
        BIT(IEEE80211_STYPE_DEAUTH >> 4) |
        BIT(IEEE80211_STYPE_ACTION >> 4)
    },
#if defined(_PRE_WLAN_FEATURE_P2P)
    [NL80211_IFTYPE_P2P_DEVICE] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
    },
#endif /* WL_CFG80211_P2P_DEV_IF */
};

#endif   /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44) */

#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)

/* 2.4G 频段信息 */
OAL_STATIC struct ieee80211_supported_band g_st_supported_band_2ghz_info = {
    g_ast_supported_channels_2ghz_info,
    hi1151_g_rates,
    IEEE80211_BAND_2GHZ,
    sizeof(g_ast_supported_channels_2ghz_info)/sizeof(oal_ieee80211_channel),
    hi1151_g_rates_size,
    {
        IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
        OAL_TRUE,
    },
};

/* 5G 频段信息 */
OAL_STATIC oal_ieee80211_supported_band g_st_supported_band_5ghz_info = {
    g_ast_supported_channels_5ghz_info,
    hi1151_a_rates,
    IEEE80211_BAND_5GHZ,
    sizeof(g_ast_supported_channels_5ghz_info)/sizeof(oal_ieee80211_channel),
    hi1151_a_rates_size,
    {
        IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
        OAL_TRUE,
    },
};
#endif

oal_uint8               g_uc_cookie_array_bitmap_etc = 0;   /* 每个bit 表示cookie array 中是否使用，1 - 已使用；0 - 未使用 */
cookie_arry_stru        g_cookie_array_etc[WAL_COOKIE_ARRAY_SIZE];

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_UAPSD

OAL_STATIC oal_uint32 wal_find_wmm_uapsd_etc(oal_uint8 *puc_wmm_ie)
{
    /* 判断 WMM UAPSD 是否使能 */
    if (puc_wmm_ie[1] < MAC_WMM_QOS_INFO_POS) {
        return OAL_FALSE;
    }

    if (puc_wmm_ie[MAC_WMM_QOS_INFO_POS] & BIT7) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}
#endif


oal_uint32 wal_cfg80211_open_wmm_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == puc_param))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_open_wmm_etc::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 针对配置vap做保护 */
    if (WLAN_VAP_MODE_CONFIG == pst_mac_vap->en_vap_mode)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_open_wmm_etc::this is config vap! can't get info.}");
        return OAL_FAIL;
    }

    /* host侧,开关WMM，更新mib信息位中的Qos位置 */
    return hmac_config_open_wmm(pst_mac_vap, us_len, puc_param);
}


oal_void wal_parse_wpa_wpa2_ie_etc(oal_beacon_parameters *pst_beacon_info,
                                 mac_beacon_param_stru *pst_beacon_param)
{
    oal_ieee80211_mgmt      *pst_mgmt;
    oal_uint16               us_capability_info;
    oal_uint8               *puc_ie_tmp;

    /* 判断是否加密 */
    pst_mgmt = (oal_ieee80211_mgmt *)pst_beacon_info->head;

    us_capability_info = pst_mgmt->u.beacon.capab_info;
    pst_beacon_param->en_privacy = OAL_FALSE;

    if (WLAN_WITP_CAPABILITY_PRIVACY & us_capability_info)
    {
        pst_beacon_param->en_privacy = OAL_TRUE;

        /* 查找 RSN 信息元素 */
        puc_ie_tmp = mac_find_ie_etc(MAC_EID_RSN, pst_beacon_info->tail, pst_beacon_info->tail_len);
        if (OAL_PTR_NULL != puc_ie_tmp)
        {
            oal_memcopy(pst_beacon_param->auc_rsn_ie, puc_ie_tmp, MAC_MAX_RSN_LEN);
        }

        /* 查找 WPA 信息元素，并解析出认证类型 */
        puc_ie_tmp = mac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA, pst_beacon_info->tail, pst_beacon_info->tail_len);
        if (OAL_PTR_NULL != puc_ie_tmp)
        {
            oal_memcopy(pst_beacon_param->auc_wpa_ie, puc_ie_tmp, MAC_MAX_RSN_LEN);
        }
    }
}
#ifdef _PRE_WLAN_FEATURE_11R_AP

oal_uint32 wal_parse_md_ie(oal_beacon_parameters *pst_beacon_info,
                                 mac_beacon_param_stru *pst_beacon_param)
{
    oal_uint8               *puc_ie_tmp;
    pst_beacon_param->en_ft_bss_transition = OAL_FALSE;
    /* 查找 MD 信息元素 */
    puc_ie_tmp = mac_find_ie_etc(MAC_EID_MOBILITY_DOMAIN, pst_beacon_info->tail, pst_beacon_info->tail_len);
    if (OAL_PTR_NULL != puc_ie_tmp)
    {
        pst_beacon_param->en_ft_bss_transition = OAL_TRUE;
        pst_beacon_param->us_mdid = (puc_ie_tmp[3] << 8) + puc_ie_tmp[2];
        pst_beacon_param->en_ft_over_ds = puc_ie_tmp[4] & BIT0;
        pst_beacon_param->en_ft_resource_req = puc_ie_tmp[4] & BIT1;
        OAM_WARNING_LOG3(0, OAM_SF_CFG, "{wal_parse_md_ie::puc_ie_tmp2-4[0x%x][0x%x][0x%x].}",
        puc_ie_tmp[2], puc_ie_tmp[3], puc_ie_tmp[4]);
    }
    OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_parse_md_ie::en_ft_bss_transition=%d.}", pst_beacon_param->en_ft_bss_transition);

    return OAL_SUCC;
}
#endif


oal_uint32 wal_parse_wmm_ie_etc(oal_net_device_stru *pst_dev,
                                   mac_vap_stru *pst_mac_vap, oal_beacon_parameters *pst_beacon_info)
{
    oal_uint8               *puc_wmm_ie;
    oal_uint16               us_len = OAL_SIZEOF(oal_uint8);
    oal_uint8                uc_wmm = OAL_TRUE;
    oal_uint32               ul_ret = OAL_SUCC;

#ifdef _PRE_WLAN_FEATURE_UAPSD
    oal_uint8    uc_uapsd = OAL_TRUE;
    wal_msg_write_stru    st_write_msg;
#endif

    /*  查找wmm_ie  */
    puc_wmm_ie = mac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WMM,
                                            pst_beacon_info->tail, pst_beacon_info->tail_len);

    if (OAL_PTR_NULL == puc_wmm_ie)
    {
    /* wmm ie未找到，则说明wmm 关 */
        uc_wmm = OAL_FALSE;
    }
#ifdef _PRE_WLAN_FEATURE_UAPSD
    /*  找到wmm ie，顺便判断下uapsd是否使能 */
    else
    {
        if(OAL_FALSE == wal_find_wmm_uapsd_etc(puc_wmm_ie))
        {
        /* 对应UAPSD 关*/
            uc_uapsd = OAL_FALSE;
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_parse_wmm_ie_etc::uapsd is disabled!!}");
        }

        /* 填写 msg 消息头*/
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_UAPSD_EN, OAL_SIZEOF(uc_uapsd));

        /* 填写 msg 消息体 */
#ifdef _PRE_WLAN_FEATURE_P2P
#if defined(_PRE_PRODUCT_ID_HI1103_HOST)
        /* 该功能目前按照驱动设置来实现，后续需要结合上层设置，TBD */
        if (IS_P2P_GO(pst_mac_vap))
        {
            uc_uapsd = WLAN_FEATURE_UAPSD_IS_OPEN;
            OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                "{wal_parse_wmm_ie_etc:: (1103)It is a Go, set uapsd = WLAN_FEATURE_UAPSD_IS_OPEN)}");
        }
#elif defined(_PRE_PRODUCT_ID_HI1102_HOST)
       uc_uapsd = OAL_FALSE;
#endif
       OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
           "{wal_parse_wmm_ie_etc:: uapsd = %d)}", uc_uapsd);
#endif

        oal_memcopy(st_write_msg.auc_value, &uc_uapsd, OAL_SIZEOF(uc_uapsd));

        /* 发送消息 */
        ul_ret = (oal_uint32)wal_send_cfg_event_etc(pst_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(uc_uapsd),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);

        if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
        {
            ul_ret = OAL_FAIL;
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_parse_wmm_ie_etc::uapsd switch set failed[%d].}",ul_ret);
        }
    }
#endif
    /*  wmm 开启/关闭 标记  */
    ul_ret = wal_cfg80211_open_wmm_etc(pst_mac_vap, us_len, &uc_wmm);
    if (OAL_SUCC != ul_ret)
    {
        ul_ret = OAL_FAIL;
        OAM_WARNING_LOG0(0, OAM_SF_TX, "{wal_parse_wmm_ie_etc::can not open wmm!}\r\n");
    }

    return ul_ret;
}


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)


oal_uint32 wal_cfg80211_add_vap_etc(mac_cfg_add_vap_param_stru *pst_add_vap_param)
{
    
    oal_int32                   l_ret;
    oal_net_device_stru        *pst_net_dev;
    oal_net_device_stru        *pst_cfg_net_dev;
    wal_msg_write_stru          st_write_msg;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32                  ul_err_code;

    oal_wireless_dev_stru      *pst_wdev;
    mac_wiphy_priv_stru        *pst_wiphy_priv;

    mac_vap_stru               *pst_cfg_mac_vap;
    hmac_vap_stru              *pst_cfg_hmac_vap;
    mac_device_stru            *pst_mac_device;
    wlan_vap_mode_enum_uint8    en_vap_mode;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8    en_p2p_mode = WLAN_LEGACY_VAP_MODE;
    en_p2p_mode = pst_add_vap_param->en_p2p_mode;
#endif

    en_vap_mode = pst_add_vap_param->en_vap_mode;

    /* 获取mac device */
    pst_net_dev     = pst_add_vap_param->pst_net_dev;
    pst_wdev        = pst_net_dev->ieee80211_ptr;
    pst_wiphy_priv  = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    pst_mac_device  = pst_wiphy_priv->pst_mac_device;
    pst_cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->uc_cfg_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_cfg_mac_vap))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_add_vap_etc::pst_cfg_mac_vap is null mac_vap:%d!}\r\n",pst_mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_cfg_hmac_vap))
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_add_vap_etc::pst_cfg_hmac_vap is null vap_id:%d!}\r\n",pst_mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_cfg_net_dev))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_add_vap_etc::pst_mac_device is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }


    if (WLAN_VAP_MODE_BSS_AP == en_vap_mode)
    {
        pst_wdev->iftype = NL80211_IFTYPE_AP;
    }
    else if (WLAN_VAP_MODE_BSS_STA == en_vap_mode)
    {
        pst_wdev->iftype = NL80211_IFTYPE_STATION;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    if (WLAN_P2P_DEV_MODE == en_p2p_mode)
    {
        pst_wdev->iftype = NL80211_IFTYPE_P2P_DEVICE;
    }
    else if (WLAN_P2P_CL_MODE == en_p2p_mode)
    {
        pst_wdev->iftype = NL80211_IFTYPE_P2P_CLIENT;
    }
    else if (WLAN_P2P_GO_MODE == en_p2p_mode)
    {
        pst_wdev->iftype = NL80211_IFTYPE_P2P_GO;
    }
#endif  /* _PRE_WLAN_FEATURE_P2P */

    OAL_NETDEVICE_FLAGS(pst_net_dev) &= ~OAL_IFF_RUNNING;  /* 将net device的flag设为down */

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 填写消息 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ADD_VAP, OAL_SIZEOF(mac_cfg_add_vap_param_stru));
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev     = pst_net_dev;
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->en_vap_mode     = en_vap_mode;
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->uc_cfg_vap_indx = pst_cfg_mac_vap->uc_vap_id;
#ifdef _PRE_WLAN_FEATURE_P2P
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode      = en_p2p_mode;
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->bit_11ac2g_enable = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_11AC2G_ENABLE);
    ((mac_cfg_add_vap_param_stru *)st_write_msg.auc_value)->bit_disable_capab_2ght40 = g_st_wlan_customize_etc.uc_disable_capab_2ght40;
#endif
    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_add_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAL_MEM_FREE(pst_wdev, OAL_FALSE);
        oal_net_free_netdev(pst_net_dev);
        OAM_WARNING_LOG1(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_vap_etc::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    /* 读取返回的错误码 */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_vap_etc::hmac add vap fail, ul_err_code[%u]!}\r\n",
                          ul_err_code);
        /* 异常处理，释放内存 */
        OAL_MEM_FREE(pst_wdev, OAL_FALSE);
        oal_net_free_netdev(pst_net_dev);
        return ul_err_code;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    wal_set_random_mac_to_mib_etc(pst_net_dev); /* set random mac to mib ; for hi1102-cb */
#endif

    return OAL_SUCC;
}


oal_uint32 wal_cfg80211_del_vap_etc(mac_cfg_del_vap_param_stru *pst_del_vap_param)
{
    wal_msg_write_stru           st_write_msg;
    wal_msg_stru                *pst_rsp_msg = OAL_PTR_NULL;
    oal_net_device_stru         *pst_net_dev;
    mac_vap_stru                *pst_mac_vap;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_del_vap_param))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap_etc::pst_del_vap_param null ptr !}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_net_dev = pst_del_vap_param->pst_net_dev;

    /* 设备在up状态不允许删除，必须先down */
    if (OAL_UNLIKELY(0 != (OAL_IFF_RUNNING & OAL_NETDEVICE_FLAGS(pst_net_dev))))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_vap_etc::device is busy, please down it first %d!}\r\n", OAL_NETDEVICE_FLAGS(pst_net_dev));
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_UNLIKELY(NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_del_vap_etc::can't get mac vap from netdevice priv data!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设备在up状态不允许删除，必须先down */
#ifdef _PRE_WLAN_FEATURE_P2P
    if ((pst_mac_vap->en_vap_state != MAC_VAP_STATE_INIT)
        &&(pst_mac_vap->en_vap_state != MAC_VAP_STATE_STA_SCAN_COMP))
#else
    if (pst_mac_vap->en_vap_state != MAC_VAP_STATE_INIT)
#endif
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_vap_etc::device is busy, please down it first %d!}\r\n", pst_mac_vap->en_vap_state);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* 不需要释放net_device 结构下的wireless_dev 成员 */
    //OAL_MEM_FREE(OAL_NETDEVICE_WDEV(pst_net_dev), OAL_TRUE);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 初始化删除vap 参数 */
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = pst_del_vap_param->en_p2p_mode;
#endif
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEL_VAP, OAL_SIZEOF(mac_cfg_del_vap_param_stru));

    if (OAL_SUCC != wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_del_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap_etc::wal_send_cfg_event_etc fail!}");
        return OAL_EFAIL;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp_etc(pst_rsp_msg))
    {
         OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap_etc::wal_check_and_release_msg_resp_etc fail!}");
        return OAL_EFAIL;
    }

    return OAL_SUCC;
}
#else

oal_uint32 wal_cfg80211_add_vap_etc(mac_cfg_add_vap_param_stru *pst_add_vap_param)
{
    return OAL_SUCC;
}

oal_uint32 wal_cfg80211_del_vap_etc(mac_cfg_del_vap_param_stru *pst_del_vap_param)
{
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_VOWIFI

oal_uint32 wal_cfg80211_vowifi_report_etc(frw_event_mem_stru *pst_event_mem)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    frw_event_stru                  *pst_event;
    hmac_vap_stru                   *pst_hmac_vap;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
      OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_vowifi_report_etc::pst_event_mem is null!}");
      return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event  = frw_get_event_stru(pst_event_mem);
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if(OAL_PTR_NULL == pst_hmac_vap)
    {
      OAM_ERROR_LOG1(0, OAM_SF_TX, "{wal_cfg80211_vowifi_report_etc::pst_hmac_vap null.vap_id[%d]}", pst_event->st_event_hdr.uc_vap_id);
      return OAL_ERR_CODE_PTR_NULL;
    }

    /* 上报vowifi切换申请 */
    oal_cfg80211_vowifi_report_etc(pst_hmac_vap->pst_net_device, GFP_KERNEL);
#endif /* (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */

    return OAL_SUCC;

}
#endif /* _PRE_WLAN_FEATURE_VOWIFI */


oal_uint32 wal_cfg80211_cac_report(frw_event_mem_stru *pst_event_mem)
{
#if (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5) || (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_CPE)
    frw_event_stru                  *pst_event;
    hmac_vap_stru                   *pst_hmac_vap;
    hmac_cac_event_stru             *pst_cac_event;
    oal_uint32                       ul_ret;
    enum PRIV_NL80211_RADAR_EVENT    en_event;
    enum PRIV_NL80211_CHAN_WIDTH     en_chan_width;


    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_cac_report::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_event  = (frw_event_stru *)pst_event_mem->puc_data;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if(OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_cac_report::pst_hmac_vap null.vap_id[%d]}", pst_event->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cac_event  = (hmac_cac_event_stru *)(pst_event->auc_event_data);
    switch(pst_cac_event->en_type)
    {
       case HMAC_CAC_STARTED:
            en_event = PRIV_NL80211_RADAR_CAC_STARTED;
            break;
        case HMAC_CAC_FINISHED:
            en_event = PRIV_NL80211_RADAR_CAC_FINISHED;
            break;
        case HMAC_CAC_ABORTED:
             en_event = PRIV_NL80211_RADAR_CAC_ABORTED;
             break;
        default:
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_cac_report::error event type[%d]}", pst_cac_event->en_type);
             return OAL_FAIL;
    }

    switch(pst_cac_event->en_bw_mode)
    {
       case WLAN_BAND_WIDTH_20M:
            en_chan_width= PRIV_NL80211_CHAN_WIDTH_20;
            break;
       case WLAN_BAND_WIDTH_40PLUS:
       case WLAN_BAND_WIDTH_40MINUS:
            en_chan_width = PRIV_NL80211_CHAN_WIDTH_40;
            break;
       case WLAN_BAND_WIDTH_80PLUSPLUS:
       case WLAN_BAND_WIDTH_80PLUSMINUS:
       case WLAN_BAND_WIDTH_80MINUSPLUS:
       case WLAN_BAND_WIDTH_80MINUSMINUS:
             en_chan_width = PRIV_NL80211_CHAN_WIDTH_80;
             break;
#ifdef _PRE_WLAN_FEATURE_160M
       case WLAN_BAND_WIDTH_160PLUSPLUSPLUS:
       case WLAN_BAND_WIDTH_160PLUSPLUSMINUS:
       case WLAN_BAND_WIDTH_160PLUSMINUSPLUS:
       case WLAN_BAND_WIDTH_160PLUSMINUSMINUS:
       case WLAN_BAND_WIDTH_160MINUSPLUSPLUS:
       case WLAN_BAND_WIDTH_160MINUSPLUSMINUS:
       case WLAN_BAND_WIDTH_160MINUSMINUSPLUS:
       case WLAN_BAND_WIDTH_160MINUSMINUSMINUS:
             en_chan_width = PRIV_NL80211_CHAN_WIDTH_160;
             break;
#endif
        default:
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_cac_report::error chan width type[%d]}", pst_cac_event->en_bw_mode);
             return OAL_FAIL;
    }
    ul_ret = oal_cfg80211_notify_cac_event(pst_hmac_vap->pst_net_device, pst_cac_event->ul_freq,en_chan_width,en_event,GFP_ATOMIC);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_cac_report::oal_cfg80211_notify_cac_event failed.}");
        return OAL_FAIL;
    }
#endif

    return OAL_SUCC;

}


//#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44))

OAL_STATIC oal_bool_enum wal_check_support_basic_rate_6M(oal_uint8   *puc_supported_rates_ie,
                                                            oal_uint8    uc_supported_rates_num,
                                                            oal_uint8   *puc_extended_supported_rates_ie,
                                                            oal_uint8    uc_extended_supported_rates_num)
{
    oal_uint8     uc_loop;
    oal_bool_enum en_support = OAL_FALSE;
    for (uc_loop = 0; uc_loop < uc_supported_rates_num; uc_loop++)
    {
        if (OAL_PTR_NULL == puc_supported_rates_ie)
        {
            break;
        }
        if (0x8c == puc_supported_rates_ie[2 + uc_loop])
        {
            en_support = OAL_TRUE;
        }
    }

    for (uc_loop = 0; uc_loop < uc_extended_supported_rates_num; uc_loop++)
    {
        if (OAL_PTR_NULL == puc_extended_supported_rates_ie)
        {
            break;
        }
        if (0x8c == puc_extended_supported_rates_ie[2 + uc_loop])
        {
            en_support = OAL_TRUE;
        }
    }

    return en_support;
}


OAL_STATIC oal_uint32  wal_parse_protocol_mode(
                wlan_channel_band_enum_uint8 en_band,
                oal_beacon_parameters       *pst_beacon_info,
                oal_uint8                   *puc_ht_ie,
                oal_uint8                   *puc_vht_ie,
                wlan_protocol_enum_uint8    *pen_protocol)
{
    oal_uint8   *puc_supported_rates_ie             = OAL_PTR_NULL;
    oal_uint8   *puc_extended_supported_rates_ie    = OAL_PTR_NULL;
    oal_uint8    uc_supported_rates_num             = 0;
    oal_uint8    uc_extended_supported_rates_num    = 0;
    oal_uint16   us_offset;


    if (OAL_PTR_NULL != puc_vht_ie)
    {
        /* 设置AP 为11ac 模式 */
        *pen_protocol = WLAN_VHT_MODE;
        return OAL_SUCC;
    }
    if (OAL_PTR_NULL != puc_ht_ie)
    {
        /* 设置AP 为11n 模式 */
        *pen_protocol = WLAN_HT_MODE;
        return OAL_SUCC;
    }

    /* hostapd 先设置频段，后设置add beacon 因此在add beacon流程中，可以使用频段信息和速率信息设置AP 模式(a/b/g) */
    if (WLAN_BAND_5G == en_band)
    {
        *pen_protocol = WLAN_LEGACY_11A_MODE;
        return OAL_SUCC;
    }
    if (WLAN_BAND_2G == en_band)
    {
        us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
        //puc_supported_rates_ie = mac_get_supported_rates_ie(pst_beacon_info->head + 24, (oal_uint16)pst_beacon_info->head_len, us_offset);
        puc_supported_rates_ie = mac_find_ie_etc(MAC_EID_RATES, pst_beacon_info->head + 24 + us_offset, pst_beacon_info->head_len - us_offset);
        if (OAL_PTR_NULL != puc_supported_rates_ie)
        {
            uc_supported_rates_num = puc_supported_rates_ie[1];
        }
        //puc_extended_supported_rates_ie = mac_get_exsup_rates_ie(pst_beacon_info->tail, (oal_uint16)pst_beacon_info->tail_len, 0);
        puc_extended_supported_rates_ie = mac_find_ie_etc(MAC_EID_XRATES, pst_beacon_info->tail, pst_beacon_info->tail_len);
        if (OAL_PTR_NULL != puc_extended_supported_rates_ie)
        {
            uc_extended_supported_rates_num = puc_extended_supported_rates_ie[1];
        }

        if ((uc_supported_rates_num + uc_extended_supported_rates_num) < uc_supported_rates_num) {
            *pen_protocol = WLAN_PROTOCOL_BUTT;
            return OAL_FAIL;
        }

        if (4 == uc_supported_rates_num + uc_extended_supported_rates_num)
        {
            *pen_protocol = WLAN_LEGACY_11B_MODE;
            return OAL_SUCC;
        }
        if (8 == uc_supported_rates_num + uc_extended_supported_rates_num)
        {
            *pen_protocol = WLAN_LEGACY_11G_MODE;
            return OAL_SUCC;
        }
        if (12 == uc_supported_rates_num + uc_extended_supported_rates_num)
        {
            /* 根据基本速率区分为 11gmix1 还是 11gmix2 */
            /* 如果基本速率集支持 6M , 则判断为 11gmix2 */
            *pen_protocol = WLAN_MIXED_ONE_11G_MODE;
            if (OAL_TRUE == wal_check_support_basic_rate_6M(puc_supported_rates_ie,
                                                            uc_supported_rates_num,
                                                            puc_extended_supported_rates_ie,
                                                            uc_extended_supported_rates_num))
            {
                *pen_protocol = WLAN_MIXED_TWO_11G_MODE;
            }
            return OAL_SUCC;
        }
    }

    /* 其他情况，认为配置不合理 */
    *pen_protocol = WLAN_PROTOCOL_BUTT;

    return OAL_FAIL;
}


OAL_STATIC oal_uint32 wal_parse_ht_vht_ie(
                mac_vap_stru           *pst_mac_vap,
                oal_beacon_parameters  *pst_beacon_info,
                mac_beacon_param_stru  *pst_beacon_param)
{
    oal_uint8              *puc_ht_ie;
    oal_uint8              *puc_vht_ie;
    oal_uint32              ul_ret;
    mac_frame_ht_cap_stru  *pst_ht_cap;
    mac_vht_cap_info_stru  *pst_vht_cap;

    //puc_ht_ie  = mac_get_ht_cap_ie(pst_beacon_info->tail, (oal_uint16)pst_beacon_info->tail_len, 0);
    //puc_vht_ie = mac_get_vht_cap_ie(pst_beacon_info->tail, (oal_uint16)pst_beacon_info->tail_len, 0);
    puc_ht_ie  = mac_find_ie_etc(MAC_EID_HT_CAP, pst_beacon_info->tail, pst_beacon_info->tail_len);
    puc_vht_ie = mac_find_ie_etc(MAC_EID_VHT_CAP, pst_beacon_info->tail, pst_beacon_info->tail_len);

    /* 解析协议模式 */
    ul_ret = wal_parse_protocol_mode(pst_mac_vap->st_channel.en_band, pst_beacon_info, puc_ht_ie, puc_vht_ie, &pst_beacon_param->en_protocol);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_parse_ht_vht_ie::return err code!}\r\n", ul_ret);

        return ul_ret;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    /* 定制化实现P2P GO 2.4G下默认支持11ac 协议模式 */
    if(IS_P2P_GO(pst_mac_vap)
       && (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band))
    {
       pst_beacon_param->en_protocol = ((OAL_TRUE == pst_mac_vap->st_cap_flag.bit_11ac2g) ? WLAN_VHT_MODE : WLAN_HT_MODE);
    }

#endif /* _PRE_WLAN_FEATURE_P2P */

	/* 如果ht vht均为空，返回 */
    if ((OAL_PTR_NULL == puc_ht_ie)&&(OAL_PTR_NULL == puc_vht_ie))
    {
        return OAL_SUCC;
    }
    /* 解析short gi能力 */
    if (OAL_PTR_NULL != puc_ht_ie)
    {
        /* 使用ht cap ie中数据域的2个字节 */
        if (puc_ht_ie[1] < OAL_SIZEOF(mac_frame_ht_cap_stru))
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_parse_ht_vht_ie::invalid ht cap ie len[%d]!}\r\n", puc_ht_ie[1]);
            return OAL_SUCC;
        }

        pst_ht_cap = (mac_frame_ht_cap_stru *)(puc_ht_ie + MAC_IE_HDR_LEN);

        pst_beacon_param->en_shortgi_20 = (oal_uint8)pst_ht_cap->bit_short_gi_20mhz;
    	pst_beacon_param->en_shortgi_40 = 0;

    	if ((pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_20M)
    		&& (WLAN_BAND_WIDTH_BUTT != pst_mac_vap->st_channel.en_bandwidth))
    	{
            pst_beacon_param->en_shortgi_40 = (oal_uint8)pst_ht_cap->bit_short_gi_40mhz;
    	}

    }

//    pst_beacon_param->uc_smps_mode  = (oal_uint8)pst_ht_cap->bit_sm_power_save;

    if (OAL_PTR_NULL == puc_vht_ie)
    {
        return OAL_SUCC;
    }

    /* 使用vht cap ie中数据域的4个字节 */
    if (puc_vht_ie[1] < OAL_SIZEOF(mac_vht_cap_info_stru))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_parse_ht_vht_ie::invalid ht cap ie len[%d]!}\r\n", puc_vht_ie[1]);
        return OAL_SUCC;
    }

    pst_vht_cap = (mac_vht_cap_info_stru *)(puc_vht_ie + MAC_IE_HDR_LEN);

    pst_beacon_param->en_shortgi_80 = 0;

	if ((pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_40MINUS)
		&& (WLAN_BAND_WIDTH_BUTT != pst_mac_vap->st_channel.en_bandwidth))
	{
        pst_beacon_param->en_shortgi_80 = pst_vht_cap->bit_short_gi_80mhz;
	}

    return OAL_SUCC;
}

//#endif
#if 0

OAL_STATIC oal_uint32 wal_parse_beacon_wps_ie(mac_vap_stru *pst_mac_vap, oal_beacon_parameters *pst_beacon_info)
{
    oal_app_ie_stru  st_wps_ie;
    oal_uint8       *puc_wps_ie         = OAL_PTR_NULL;
    oal_uint32       ul_ret;


    puc_wps_ie  = mac_get_wps_ie(pst_beacon_info->tail, (oal_uint16)pst_beacon_info->tail_len, 0);
    if (OAL_PTR_NULL == puc_wps_ie)
    {
        /* WPS 信息元素为空，设置WPS 功能为空 */
        pst_mac_vap->st_cap_flag.bit_wps = OAL_FALSE;
        if (OAL_PTR_NULL != pst_mac_vap->ast_app_ie[OAL_APP_BEACON_IE].puc_ie)
        {
            OAL_MEM_FREE(pst_mac_vap->ast_app_ie[OAL_APP_BEACON_IE].puc_ie, OAL_TRUE);
            pst_mac_vap->ast_app_ie[OAL_APP_BEACON_IE].puc_ie = OAL_PTR_NULL;
        }
        pst_mac_vap->ast_app_ie[OAL_APP_BEACON_IE].ul_ie_len     = 0;
        pst_mac_vap->ast_app_ie[OAL_APP_BEACON_IE].ul_ie_max_len = 0;
        return OAL_SUCC;
    }

    /* 添加WPS信息元素BEACON 帧到VAP 结构中 */
    st_wps_ie.puc_ie    = puc_wps_ie;
    st_wps_ie.ul_ie_len = puc_wps_ie[1];
    ul_ret = hmac_config_set_app_ie_to_vap_etc(pst_mac_vap, &st_wps_ie, OAL_APP_BEACON_IE);

    return ul_ret;
}
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
OAL_STATIC oal_int32 wal_cfg80211_sched_scan_stop(oal_wiphy_stru          *pst_wiphy,
                                                  oal_net_device_stru     *pst_netdev,
                                                  oal_uint64               ul_reqid);

#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
OAL_STATIC oal_int32 wal_cfg80211_sched_scan_stop(oal_wiphy_stru          *pst_wiphy,
                                                  oal_net_device_stru     *pst_netdev);
#else
/* do nothing */
#endif


OAL_STATIC oal_int32 wal_p2p_stop_roc(mac_vap_stru *pst_mac_vap, oal_net_device_stru *pst_netdev)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_vap_stru     *pst_hmac_vap;
    hmac_device_stru  *pst_hmac_device;
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_hmac_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_p2p_stop_roc:: pst_hmac_device[%d] null!}\r\n", pst_mac_vap->uc_device_id);
        return -OAL_EFAIL;
    }

    /* tx mgmt roc 优先级低,可以被自己的80211 roc以及80211 scan打断 */
    if (MAC_VAP_STATE_STA_LISTEN == pst_mac_vap->en_vap_state)
    {
        if (pst_hmac_device->st_scan_mgmt.en_is_scanning != OAL_TRUE)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_p2p_stop_roc::not in scan state but vap is listen state!}");
            return OAL_SUCC;
        }

        pst_hmac_vap  = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hmac_vap))
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_p2p_stop_roc:: pst_hmac_vap null!}\r\n");
            return -OAL_EFAIL;
        }
        pst_hmac_vap->en_wait_roc_end = OAL_TRUE;
        OAL_INIT_COMPLETION(&(pst_hmac_vap->st_roc_end_ready));
        wal_force_scan_complete_etc(pst_netdev, OAL_TRUE);
        if(0 == oal_wait_for_completion_timeout(&(pst_hmac_vap->st_roc_end_ready), (oal_uint32)OAL_MSECS_TO_JIFFIES(200)))
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_p2p_stop_roc::cancel old roc timout!}");
            return -OAL_EFAIL;
        }
    }
#endif
    return OAL_SUCC;
}

/*lint -e801*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))//TBD:确认正确的 Linux 版本号
OAL_STATIC oal_int32  wal_cfg80211_scan(
                    oal_wiphy_stru                        *pst_wiphy,
                    oal_cfg80211_scan_request_stru        *pst_request)
#else
OAL_STATIC oal_int32  wal_cfg80211_scan(
                    oal_wiphy_stru                        *pst_wiphy,
                    oal_net_device_stru                   *pst_netdev,
                    oal_cfg80211_scan_request_stru        *pst_request)
#endif
{
    hmac_device_stru               *pst_hmac_device;
    mac_vap_stru                   *pst_mac_vap;
    hmac_scan_stru                 *pst_scan_mgmt;
    oal_int32                       l_ret = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))//TBD:确认正确的 Linux 版本号
    oal_net_device_stru            *pst_netdev;
#endif

    if (OAL_ANY_NULL_PTR2(pst_wiphy, pst_request))
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{wal_cfg80211_scan::scan failed, null ptr, pst_wiphy[%p], pst_request[%p]!}", pst_wiphy, pst_request);
        goto fail;
    }

    /* 判断扫描传入内存长度不能大于后续缓存空间大小，避免拷贝内存越界 */
    if (pst_request->ie_len > WLAN_WPS_IE_MAX_SIZE) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_scan:: scan ie is too large to save. [%d]!}",
            pst_request->ie_len);
        return -OAL_EFAIL;
    }


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))//TBD:确认正确的 Linux 版本号
    pst_netdev = pst_request->wdev->netdev;
#endif
    if (OAL_PTR_NULL == pst_netdev)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_scan::scan failed, null ptr, pst_netdev = null!}");
        goto fail;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_scan:: dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    pst_mac_vap    = OAL_NET_DEV_PRIV(pst_netdev);
    if(NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_scan::can't get mac vap from netdevice priv data!}");
        goto fail;
    }

#ifdef _PRE_WLAN_FEATURE_WAPI
    if (IS_P2P_SCAN_REQ(pst_request)
    && (OAL_TRUE == hmac_user_is_wapi_connected_etc(pst_mac_vap->uc_device_id)))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{stop p2p scan under wapi!}");
        goto fail;
    }
#endif/* #ifdef _PRE_WLAN_FEATURE_WAPI */

    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
    if (NULL == pst_hmac_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_scan::scan failed, null ptr, pst_hmac_device = null device_id:%d!}",pst_mac_vap->uc_device_id);
        goto fail;
    }
    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::start a new normal scan. n_channels[%d], ie_len[%d], n_ssid[%d]}",
      pst_request->n_channels, pst_request->ie_len, pst_request->n_ssids);

    /* TBD: gong 不可以重叠扫描, 若上次未结束，需要返回busy  */
    /* 等待上次的扫描请求完成 */
    /*lint -e730*//* info, boolean argument to function */
    l_ret = OAL_WAIT_EVENT_INTERRUPTIBLE(pst_scan_mgmt->st_wait_queue,(OAL_PTR_NULL == pst_scan_mgmt->pst_request));
    /*lint +e730*/
    if (l_ret < 0)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::start a new scan failed, wait return error.}");
        /*lint -e801*/
        goto fail;
        /*lint +e801*/
    }

    /* p2p normal scan自己可以打断自己上一次的roc */
    if (MAC_VAP_STATE_STA_LISTEN == pst_mac_vap->en_vap_state)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::stop roc scan, before normal scan.}");
        l_ret = wal_p2p_stop_roc(pst_mac_vap, pst_netdev);
        if (l_ret < 0)
        {
            /*lint -e801*/
            goto fail;
            /*lint +e801*/
        }
    }

    if (pst_mac_vap->en_vap_state >= MAC_VAP_STATE_STA_JOIN_COMP &&
        pst_mac_vap->en_vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::stop normal scan while connecting.}");
        /*lint -e801*/
        goto fail;
        /*lint +e801*/
    }

    /* 保存当前内核下发的扫描请求到本地 */
    pst_scan_mgmt->pst_request = pst_request;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
    /* 如果当前调度扫描在运行，先暂停调度扫描 */
    if (OAL_PTR_NULL != pst_scan_mgmt->pst_sched_scan_req)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::stop sched scan, before normal scan.}");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
        wal_cfg80211_sched_scan_stop(pst_wiphy, pst_netdev, 0);
#else
        wal_cfg80211_sched_scan_stop(pst_wiphy, pst_netdev);
#endif
    }
#endif

    /* 进入扫描 */
    if (OAL_SUCC != wal_scan_work_func_etc(pst_scan_mgmt, pst_netdev, pst_request))
    {
        pst_scan_mgmt->pst_request = OAL_PTR_NULL;
        return -OAL_EFAIL;
    }

    return OAL_SUCC;

fail:
    return -OAL_EFAIL;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))

OAL_STATIC oal_void wal_set_crypto_info(mac_conn_param_stru *pst_conn_param, oal_cfg80211_conn_stru *pst_sme)
{
    oal_uint8               *puc_ie_tmp  = OAL_PTR_NULL;
    oal_uint8               *puc_wep_key = OAL_PTR_NULL;

#ifdef _PRE_WLAN_FEATURE_WAPI
    if(WITP_WAPI_VERSION == pst_sme->crypto.wpa_versions)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "wal_cfg80211_connect::crypt ver is wapi!");
        pst_conn_param->uc_wapi = OAL_TRUE;
    }
    else
    {
        pst_conn_param->uc_wapi = OAL_FALSE;
    }
#endif

    if (pst_sme->privacy)
    {
        if ((0 != pst_sme->key_len)&&(OAL_PTR_NULL != pst_sme->key))
        {
            /* 设置wep加密信息 */
            pst_conn_param->uc_wep_key_len            = pst_sme->key_len;
            pst_conn_param->uc_wep_key_index          = pst_sme->key_idx;

            puc_wep_key = (oal_uint8 *)oal_memalloc(pst_sme->key_len);
            if (OAL_PTR_NULL == puc_wep_key)
            {
                OAM_ERROR_LOG2(0, OAM_SF_CFG, "{wal_set_crypto_info::puc_wep_key(%d)(%p) alloc mem return null ptr!}",
                (oal_uint32)(pst_sme->key_len),(oal_uint8 *)pst_sme->key);
                return;
            }
            oal_memcopy(puc_wep_key, (oal_uint8 *)pst_sme->key, (oal_uint32)(pst_sme->key_len));
            pst_conn_param->puc_wep_key = puc_wep_key;

            return;
        }

        /* 优先查找 RSN 信息元素 */
        puc_ie_tmp = mac_find_ie_etc(MAC_EID_RSN, (oal_uint8 *)pst_sme->ie, (oal_int32)pst_sme->ie_len);
        if (OAL_PTR_NULL != puc_ie_tmp)
        {
            mac_ie_get_rsn_cipher(puc_ie_tmp, &pst_conn_param->st_crypto);

            OAM_WARNING_LOG2(0, OAM_SF_WPA, "wal_set_crypto_info::get_rsn_cipher pair_suite[0]:0x%x pair_suite[1]:0x%x",
                             pst_conn_param->st_crypto.aul_pair_suite[0], pst_conn_param->st_crypto.aul_pair_suite[1]);
        }
        else
        {
            /* WPA */
            puc_ie_tmp = mac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA, (oal_uint8 *)pst_sme->ie, (oal_int32)pst_sme->ie_len);
            if (OAL_PTR_NULL != puc_ie_tmp)
            {
                mac_ie_get_wpa_cipher(puc_ie_tmp, &pst_conn_param->st_crypto);
                OAM_WARNING_LOG2(0, OAM_SF_WPA, "wal_set_crypto_info::get_wpa_cipher pair_suite[0]:0x%x pair_suite[1]:0x%x",
                                 pst_conn_param->st_crypto.aul_pair_suite[0], pst_conn_param->st_crypto.aul_pair_suite[1]);
            }
        }
    }
}

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)

OAL_STATIC oal_bool_enum_uint8 wal_is_p2p_device(oal_net_device_stru *pst_net_device)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_vap_stru                       *pst_mac_vap;
    hmac_vap_stru                      *pst_hmac_vap;

    pst_mac_vap  = OAL_NET_DEV_PRIV(pst_net_device);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        return OAL_TRUE;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if ((pst_hmac_vap != OAL_PTR_NULL)
        && (pst_hmac_vap->pst_p2p0_net_device != OAL_PTR_NULL)
        && (pst_net_device == pst_hmac_vap->pst_p2p0_net_device))
    {
        return OAL_TRUE;
    }
    else
    {
        return OAL_FALSE;
    }
#else
    return OAL_FALSE;
#endif /* _PRE_WLAN_FEATURE_P2P */
}


OAL_STATIC oal_void free_connect_param_resource(mac_conn_param_stru *pst_conn_param)
{
    if (OAL_PTR_NULL != pst_conn_param->puc_wep_key)
    {
        oal_free(pst_conn_param->puc_wep_key);
        pst_conn_param->puc_wep_key = OAL_PTR_NULL;
    }
    if (OAL_PTR_NULL != pst_conn_param->puc_ie)
    {
        oal_free(pst_conn_param->puc_ie);
        pst_conn_param->puc_ie = OAL_PTR_NULL;
    }
}


OAL_STATIC oal_int32  wal_cfg80211_connect(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_device, oal_cfg80211_conn_stru *pst_sme)
{
    mac_conn_param_stru                 st_mac_conn_param;
    oal_int32                           l_ret;
#ifdef _PRE_WLAN_FEATURE_AP_PM
    oal_uint8                           uc_para = 0;
    oal_uint32                          ul_ret;
#endif
    oal_uint8                          *puc_ie = OAL_PTR_NULL;

    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_net_device) || (OAL_PTR_NULL == pst_sme))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_connect::connect failed, pst_wiphy, pst_netdev, pst_sme %p, %p, %p!}\r\n",
                       pst_wiphy, pst_net_device, pst_sme);

        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_connect::dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }

#endif //#ifdef _PRE_WLAN_FEATURE_DFR
    if (wal_is_p2p_device(pst_net_device))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "wal_cfg80211_connect:connect stop, p2p device should not connect.");
        return -OAL_EINVAL;
    }

    /* 初始化驱动连接参数 */
    OAL_MEMZERO(&st_mac_conn_param, OAL_SIZEOF(mac_conn_param_stru));

    /* 解析内核下发的 ssid */
    st_mac_conn_param.uc_ssid_len   = (oal_uint8)pst_sme->ssid_len;
    oal_memcopy(st_mac_conn_param.auc_ssid, (oal_uint8 *)pst_sme->ssid, st_mac_conn_param.uc_ssid_len);

#if (defined(_PRE_PRODUCT_ID_HI110X_HOST))
    if (pst_sme->bssid)
    {
        oal_memcopy(st_mac_conn_param.auc_bssid, (oal_uint8 *)pst_sme->bssid, OAL_MAC_ADDR_LEN);
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0))
    else if (pst_sme->bssid_hint)
    {
        oal_memcopy(st_mac_conn_param.auc_bssid, (oal_uint8 *)pst_sme->bssid_hint, OAL_MAC_ADDR_LEN);
    }
#endif
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_ASSOC, "{wal_cfg80211_connect::bssid and bssid_hint is NULL.}");
        return -OAL_EFAIL;
    }
#else
    /* 解析内核下发的 bssid */
    oal_memcopy(st_mac_conn_param.auc_bssid, (oal_uint8 *)pst_sme->bssid, OAL_MAC_ADDR_LEN);
#endif
    /* 解析内核下发的安全相关参数 */
    /* 设置认证类型 */
    st_mac_conn_param.en_auth_type  = pst_sme->auth_type;

    /* 设置加密能力 */
    st_mac_conn_param.en_privacy    = pst_sme->privacy;

    /* 获取内核下发的pmf是使能的结果 */
    st_mac_conn_param.en_mfp        = pst_sme->mfp;

    if (OAL_PTR_NULL != pst_sme->channel)
    {
        st_mac_conn_param.uc_channel = oal_ieee80211_frequency_to_channel(pst_sme->channel->center_freq);
    }

    OAM_WARNING_LOG4(0, OAM_SF_ANY, "{wal_cfg80211_connect::start a new connect, ssid_len[%d], auth_type[%d], privacy[%d], mfp[%d]}\r\n",
      pst_sme->ssid_len, pst_sme->auth_type, pst_sme->privacy, pst_sme->mfp);

    /* 设置加密参数 */
    wal_set_crypto_info(&st_mac_conn_param, pst_sme);
    if ((OAL_PTR_NULL == st_mac_conn_param.puc_wep_key)&&(pst_sme->key_len > 0)&&(OAL_PTR_NULL != pst_sme->key))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ASSOC, "{wal_cfg80211_connect::puc_wep_key is NULL.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 设置关联P2P/WPS ie */
    st_mac_conn_param.ul_ie_len     = (oal_uint32)(pst_sme->ie_len);
    if((st_mac_conn_param.ul_ie_len > 0)&&(OAL_PTR_NULL != pst_sme->ie))
    {
        puc_ie = (oal_uint8 *)oal_memalloc(pst_sme->ie_len);
        if (OAL_PTR_NULL == puc_ie)
        {
            OAM_ERROR_LOG2(0, OAM_SF_CFG, "{wal_scan_work_func_etc::puc_ie(%d)(%p) alloc mem return null ptr!}",
            (oal_uint32)(pst_sme->ie_len),(oal_uint8 *)pst_sme->ie);
            free_connect_param_resource(&st_mac_conn_param);
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
        oal_memcopy(puc_ie, (oal_uint8 *)pst_sme->ie, st_mac_conn_param.ul_ie_len);
        st_mac_conn_param.puc_ie = puc_ie;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wlan_pm_set_timeout_etc(WLAN_SLEEP_LONG_CHECK_CNT);
#endif

#ifdef _PRE_WLAN_FEATURE_AP_PM
    ul_ret = wal_config_sta_scan_connect_event(pst_net_device, &uc_para);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_connect:: exit wow fail!}\r\n");
    }
#endif

    
    wal_force_scan_complete_etc(pst_net_device, OAL_TRUE);
    

    /* 抛事件给驱动，启动关联 */
    l_ret = wal_cfg80211_start_connect_etc(pst_net_device, &st_mac_conn_param);

    if( OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_connect::wal_cfg80211_start_connect_etc fail %d!}\r\n", l_ret);
        /*事件下发失败在WAL释放，下发成功，无论succ fail HMAC会释放*/
        if((-OAL_EFAIL != l_ret)&&(-OAL_ETIMEDOUT != l_ret))
        {
            free_connect_param_resource(&st_mac_conn_param);
        }
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_cfg80211_disconnect(
                    oal_wiphy_stru                   *pst_wiphy,
                    oal_net_device_stru              *pst_net_device,
                    oal_uint16                        us_reason_code)
{
    mac_cfg_kick_user_param_stru    st_mac_cfg_kick_user_param;
    oal_int32                       l_ret;

    mac_user_stru                   *pst_mac_user;
    mac_vap_stru                    *pst_mac_vap;

    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_net_device))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_disconnect::input param pointer is null,pst_wiphy, pst_netdev %d, %d!}\r\n", pst_wiphy, pst_net_device);

        return -OAL_EINVAL;
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_disconnect:: deauth us_reason_code=%d}", us_reason_code);

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_disconnect::dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }

#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_device);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_disconnect::mac vap has already been del!}");
        OAL_IO_PRINT("wal_cfg80211_disconnect:: mac vap has already been del!ifname %s\r\n", pst_net_device->name);
        return OAL_SUCC;
    }

    /* 填写和sta关联的ap mac 地址*/
    pst_mac_user = mac_res_get_mac_user_etc(pst_mac_vap->us_assoc_vap_id);
    if (OAL_PTR_NULL == pst_mac_user)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_disconnect::mac_res_get_mac_user_etc pst_mac_user is null, user idx[%d]!}\r\n", pst_mac_vap->us_assoc_vap_id);
        return OAL_SUCC;
    }

    /* 解析内核下发的connect参数 */
    oal_memset(&st_mac_cfg_kick_user_param, 0, OAL_SIZEOF(mac_cfg_kick_user_param_stru));

    /* 解析内核下发的去关联原因  */
    st_mac_cfg_kick_user_param.us_reason_code = us_reason_code;

    oal_memcopy(st_mac_cfg_kick_user_param.auc_mac_addr, pst_mac_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);

    l_ret = wal_cfg80211_start_disconnect_etc(pst_net_device, &st_mac_cfg_kick_user_param);
    if( OAL_SUCC != l_ret)
    {
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC oal_int32 wal_cfg80211_add_key(oal_wiphy_stru          *pst_wiphy,
                                          oal_net_device_stru     *pst_netdev,
                                          oal_uint8                uc_key_index,
                                        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44)
                                          bool                     en_pairwise,
                                        #endif
                                        #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
                                          OAL_CONST oal_uint8     *puc_mac_addr,
                                        #else
                                          oal_uint8               *puc_mac_addr,
                                        #endif
                                          oal_key_params_stru     *pst_params)
#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
OAL_STATIC oal_int32 wal_cfg80211_add_key(oal_wiphy_stru          *pst_wiphy,
                                          oal_net_device_stru     *pst_netdev,
                                          oal_uint8                uc_key_index,
                                          oal_bool_enum            en_pairwise,
                                          OAL_CONST oal_uint8     *puc_mac_addr,
                                          oal_key_params_stru     *pst_params)

#endif
{
    wal_msg_write_stru             st_write_msg;
    mac_addkey_param_stru          st_payload_params;
    wal_msg_stru                  *pst_rsp_msg = NULL;

    /*1.1 入参检查*/
    if ((OAL_PTR_NULL == pst_wiphy)
        || (OAL_PTR_NULL == pst_netdev)
        || (OAL_PTR_NULL == pst_params))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_add_key::Param Check ERROR,pst_wiphy, pst_netdev, pst_params %d, %d, %d!}\r\n",
                       pst_wiphy, pst_netdev, pst_params);
        return -OAL_EINVAL;
    }

    /*1.2 key长度检查，防止拷贝越界*/
    if ((pst_params->key_len > OAL_WPA_KEY_LEN) || (pst_params->key_len < 0)
        || (pst_params->seq_len > OAL_WPA_SEQ_LEN) || (pst_params->seq_len < 0))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_add_key::Param Check ERROR! key_len[%x]  seq_len[%x]!}\r\n",
                      (oal_int32)pst_params->key_len, (oal_int32)pst_params->seq_len);
        return -OAL_EINVAL;
    }


    /*2.1 消息参数准备*/
    oal_memset(&st_payload_params, 0, OAL_SIZEOF(st_payload_params));
    st_payload_params.uc_key_index = uc_key_index;

    OAL_MEMZERO(st_payload_params.auc_mac_addr, WLAN_MAC_ADDR_LEN);
    if (OAL_PTR_NULL != puc_mac_addr)
    {
        /*不能使用内核下发的mac指针，可能被释放，需要拷贝到本地再使用*/
        oal_memcopy(st_payload_params.auc_mac_addr, puc_mac_addr, WLAN_MAC_ADDR_LEN);
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44)
    st_payload_params.en_pairwise  = en_pairwise;
#else
    st_payload_params.en_pairwise  = (OAL_PTR_NULL != puc_mac_addr) ? OAL_TRUE : OAL_FALSE;
#endif
#else
    st_payload_params.en_pairwise  = en_pairwise;
#endif

    /*2.2 获取相关密钥值*/
    st_payload_params.st_key.key_len = pst_params->key_len;
    st_payload_params.st_key.seq_len = pst_params->seq_len;
    st_payload_params.st_key.cipher  = pst_params->cipher;
    oal_memcopy(st_payload_params.st_key.auc_key, pst_params->key, (oal_uint32)pst_params->key_len);
    oal_memcopy(st_payload_params.st_key.auc_seq, pst_params->seq, (oal_uint32)pst_params->seq_len);
    OAM_INFO_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_add_key::key_len:%d, seq_len:%d, cipher:0x%08x!}\r\n",
                  pst_params->key_len, pst_params->seq_len, pst_params->cipher);

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /*3.1 填写 msg 消息头*/
    st_write_msg.en_wid = WLAN_CFGID_ADD_KEY;
    st_write_msg.us_len = OAL_SIZEOF(mac_addkey_param_stru);

    /*3.2 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_payload_params, OAL_SIZEOF(mac_addkey_param_stru));

    /*由于消息中使用了局部变量指针，因此需要将发送该函数设置为同步，否则hmac处理时会使用野指针*/
    if (OAL_SUCC != wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_addkey_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_add_key::wal_send_cfg_event_etc fail!}");
        return -OAL_EFAIL;
    }
    if (OAL_SUCC != wal_check_and_release_msg_resp_etc(pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_add_key::wal_check_and_release_msg_resp_etc fail!}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

 
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
 OAL_STATIC oal_int32 wal_cfg80211_get_key(oal_wiphy_stru          *pst_wiphy,
                                           oal_net_device_stru     *pst_netdev,
                                           oal_uint8                uc_key_index,
                                       #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44)
                                           bool            en_pairwise,
                                       #endif
                                       #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
                                           OAL_CONST oal_uint8     *puc_mac_addr,
                                       #else
                                           oal_uint8               *puc_mac_addr,
                                       #endif
                                           void                    *cookie,
                                           void                    (*callback)(void *cookie, oal_key_params_stru*))
#else
OAL_STATIC oal_int32 wal_cfg80211_get_key(oal_wiphy_stru *pst_wiphy,
                                         oal_net_device_stru     *pst_netdev,
                                         oal_uint8                uc_key_index,
                                         oal_bool_enum            en_pairwise,
                                         OAL_CONST oal_uint8     *puc_mac_addr,
                                         void                    *cookie,
                                         void                    (*callback)(void *cookie, oal_key_params_stru*))


#endif

{
    wal_msg_write_stru            st_write_msg;
    mac_getkey_param_stru         st_payload_params  = {0};
    oal_uint8                     auc_mac_addr[WLAN_MAC_ADDR_LEN];
    wal_msg_stru                  *pst_rsp_msg = NULL;

    /*1.1 入参检查*/
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_wiphy) || OAL_UNLIKELY(OAL_PTR_NULL == pst_netdev)
        ||OAL_UNLIKELY(OAL_PTR_NULL == cookie)|| OAL_UNLIKELY(OAL_PTR_NULL == callback))
    {
        OAM_ERROR_LOG4(0, OAM_SF_ANY, "{wal_cfg80211_get_key::Param Check ERROR,pst_wiphy, pst_netdev, cookie, callback %d, %d, %d, %d!}\r\n", pst_wiphy, pst_netdev, cookie, callback);
        return -OAL_EINVAL;
    }

    /*2.1 消息参数准备*/
    st_payload_params.pst_netdev   = pst_netdev;
    st_payload_params.uc_key_index = uc_key_index;

    if (OAL_PTR_NULL != puc_mac_addr)
    {
        /*不能使用内核下发的mac指针，可能被释放，需要拷贝到本地再使用*/
        oal_memcopy(auc_mac_addr, puc_mac_addr, WLAN_MAC_ADDR_LEN);
        st_payload_params.puc_mac_addr = auc_mac_addr;
    }
    else
    {
        st_payload_params.puc_mac_addr = OAL_PTR_NULL;
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44)
    st_payload_params.en_pairwise  = en_pairwise;
#else
    if (OAL_PTR_NULL != puc_mac_addr)
    {
        st_payload_params.en_pairwise  = OAL_TRUE;
    }
    else
    {
        st_payload_params.en_pairwise  = OAL_FALSE;
    }
#endif
#else
    st_payload_params.en_pairwise  = en_pairwise;
#endif
    st_payload_params.cookie       = cookie;
    st_payload_params.callback     = callback;

    OAM_INFO_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_get_key::key_idx:%d, en_pairwise:%d!}\r\n", uc_key_index, st_payload_params.en_pairwise);
    if (OAL_PTR_NULL != puc_mac_addr)
    {
        OAM_INFO_LOG4(0, OAM_SF_ANY, "{wal_cfg80211_get_key::MAC ADDR: %02X:XX:XX:%02X:%02X:%02X!}\r\n",
                puc_mac_addr[0], puc_mac_addr[3], puc_mac_addr[4], puc_mac_addr[5]);
    }
    else
    {
        OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::MAC ADDR IS null!}\r\n");
    }
    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /*3.1 填写 msg 消息头*/
    st_write_msg.en_wid = WLAN_CFGID_GET_KEY;
    st_write_msg.us_len = OAL_SIZEOF(mac_getkey_param_stru);

    /*3.2 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_payload_params, OAL_SIZEOF(mac_getkey_param_stru));

    /*由于消息中使用了局部变量指针，因此需要将发送该函数设置为同步，否则hmac处理时会使用野指针*/
    if (OAL_SUCC != wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_getkey_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::wal_send_cfg_event_etc fail.}");
        return -OAL_EINVAL;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp_etc(pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::wal_check_and_release_msg_resp_etc fail.}");
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC oal_int32 wal_cfg80211_remove_key(oal_wiphy_stru         *pst_wiphy,
                                             oal_net_device_stru    *pst_netdev,
                                             oal_uint8               uc_key_index,
                                         #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,37)
                                             bool           en_pairwise,
                                         #endif
                                         #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
                                             OAL_CONST oal_uint8    *puc_mac_addr)
                                         #else
                                             oal_uint8              *puc_mac_addr)
                                         #endif
#else
OAL_STATIC oal_int32 wal_cfg80211_remove_key(oal_wiphy_stru *pst_wiphy,
                                              oal_net_device_stru    *pst_netdev,
                                              oal_uint8               uc_key_index,
                                              oal_bool_enum           en_pairwise,
                                              OAL_CONST oal_uint8    *puc_mac_addr)


#endif

{
    mac_removekey_param_stru         st_payload_params  = {0};
    wal_msg_write_stru               st_write_msg       = {0};
    wal_msg_stru                    *pst_rsp_msg = NULL;

    /*1.1 入参检查*/
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_wiphy) || OAL_UNLIKELY(OAL_PTR_NULL == pst_netdev))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::Param Check ERROR,pst_wiphy, pst_netdev %d, %d!}\r\n", pst_wiphy, pst_netdev);
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return OAL_SUCC;
    }
#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    /*2.1 消息参数准备*/
    st_payload_params.uc_key_index = uc_key_index;
    OAL_MEMZERO(st_payload_params.auc_mac_addr, OAL_MAC_ADDR_LEN);
    if (OAL_PTR_NULL != puc_mac_addr)
    {
        /*不能使用内核下发的mac指针，可能被释放，需要拷贝到本地再使用*/
        oal_memcopy(st_payload_params.auc_mac_addr, puc_mac_addr, WLAN_MAC_ADDR_LEN);
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,37)
    st_payload_params.en_pairwise  = en_pairwise;
#else
    if (OAL_PTR_NULL != puc_mac_addr)
    {
        st_payload_params.en_pairwise  = OAL_TRUE;
        OAM_INFO_LOG4(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::MAC ADDR: %02X:XX:XX:%02X:%02X:%02X!}\r\n",
            puc_mac_addr[0], puc_mac_addr[3], puc_mac_addr[4], puc_mac_addr[5]);
    }
    else
    {
        st_payload_params.en_pairwise  = OAL_FALSE;
        OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::MAC ADDR IS null!}\r\n");
    }
#endif
#else
    st_payload_params.en_pairwise  = en_pairwise;
#endif

    OAM_INFO_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::uc_key_index:%d, en_pairwise:%d!}\r\n", uc_key_index, st_payload_params.en_pairwise);

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /*3.1 填写 msg 消息头*/
    st_write_msg.en_wid = WLAN_CFGID_REMOVE_KEY;
    st_write_msg.us_len = OAL_SIZEOF(mac_removekey_param_stru);

    /*3.2 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_payload_params, OAL_SIZEOF(mac_removekey_param_stru));

    if(OAL_SUCC != wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_removekey_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::wal_send_cfg_event_etc fail.}");
        return -OAL_EFAIL;
    }

    if(OAL_SUCC != wal_check_and_release_msg_resp_etc(pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::wal_check_and_release_msg_resp_etc fail.}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC oal_int32 wal_cfg80211_set_default_key(oal_wiphy_stru *pst_wiphy,
                                                    oal_net_device_stru     *pst_netdev,
                                                    oal_uint8                uc_key_index
                                                #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,38)
                                                      ,
                                                    bool            en_unicast,
                                                    bool            en_multicast
                                                #endif
                                                        )
#else
OAL_STATIC oal_int32 wal_cfg80211_set_default_key(oal_wiphy_stru *pst_wiphy,
                                                      oal_net_device_stru     *pst_netdev,
                                                      oal_uint8                uc_key_index,
                                                      oal_bool_enum            en_unicast,
                                                      oal_bool_enum            en_multicast
                                                        )

#endif
{
    mac_setdefaultkey_param_stru  st_payload_params  = {0};
    oal_int32                     l_ret;
    wal_msg_write_stru            st_write_msg      = {0};

    /*1.1 入参检查*/
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_wiphy) || OAL_UNLIKELY(OAL_PTR_NULL == pst_netdev))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_set_default_key::pst_wiphy or pst_netdev ptr is null,error %d, %d!}\r\n",
                       pst_wiphy, pst_netdev);
        return -OAL_EINVAL;
    }

    /*2.1 消息参数准备*/
    st_payload_params.uc_key_index = uc_key_index;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44)
    st_payload_params.en_unicast   = en_unicast;
    st_payload_params.en_multicast = en_multicast;
#else
    st_payload_params.en_unicast   = OAL_TRUE;
    st_payload_params.en_multicast = OAL_TRUE;
#endif
#else
    st_payload_params.en_unicast   = en_unicast;
    st_payload_params.en_multicast = en_multicast;
#endif

    OAM_INFO_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_set_default_key::key_index:%d, unicast:%d, multicast:%d!}\r\n",
                  uc_key_index, st_payload_params.en_unicast, st_payload_params.en_multicast);

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /*3.1 填写 msg 消息头*/
    st_write_msg.en_wid = WLAN_CFGID_DEFAULT_KEY;
    st_write_msg.us_len = OAL_SIZEOF(mac_setdefaultkey_param_stru);

    /*3.2 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_payload_params, OAL_SIZEOF(mac_setdefaultkey_param_stru));

    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_setdefaultkey_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_default_key::wal_send_cfg_event_etc return err code %d!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_int32 wal_cfg80211_set_default_mgmt_key_etc(oal_wiphy_stru          *pst_wiphy,
                                            oal_net_device_stru     *pst_netdev,
                                            oal_uint8                uc_key_index)
{
    mac_setdefaultkey_param_stru  st_payload_params  = {0};
    oal_int32                     l_ret;
    wal_msg_write_stru            st_write_msg       = {0};

    /*1.1 入参检查*/
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_wiphy) || OAL_UNLIKELY(OAL_PTR_NULL == pst_netdev))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_set_default_mgmt_key_etc::pst_wiphy or pst_netdev ptr is null,error %d, %d!}\r\n",
                       pst_wiphy, pst_netdev);
        return -OAL_EINVAL;
    }

    /*2.1 消息参数准备*/
    st_payload_params.uc_key_index = uc_key_index;
    st_payload_params.en_unicast   = OAL_FALSE;
    st_payload_params.en_multicast = OAL_TRUE;

    OAM_INFO_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_set_default_mgmt_key_etc::key_index:%d, unicast:%d, multicast:%d!}\r\n",
              uc_key_index, st_payload_params.en_unicast, st_payload_params.en_multicast);

    /***************************************************************************
    抛事件到wal层处理
    ***************************************************************************/
    /*3.1 填写 msg 消息头*/
    st_write_msg.en_wid = WLAN_CFGID_DEFAULT_KEY;
    st_write_msg.us_len = OAL_SIZEOF(mac_setdefaultkey_param_stru);

    /*3.2 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_payload_params, OAL_SIZEOF(mac_setdefaultkey_param_stru));

    l_ret = wal_send_cfg_event_etc(pst_netdev,
                           WAL_MSG_TYPE_WRITE,
                           WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_setdefaultkey_param_stru),
                           (oal_uint8 *)&st_write_msg,
                           OAL_FALSE,
                           OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_default_mgmt_key_etc::wal_send_cfg_event_etc return err code %d!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_uint8 wal_cfg80211_convert_value_to_vht_width(oal_int32 l_channel_value)
{
    oal_uint8 uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_20_40;

    switch (l_channel_value)
    {
        case 20:
        case 40:
            uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
            break;
        case 80:
            uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_80;
            break;
        case 160:
            uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_160;
            break;
        default:
            break;
    }

    return uc_channel_vht_width;
}


/*lint -e40*/
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 34)) || ((_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION) ||(_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION) )
OAL_STATIC oal_int32 wal_cfg80211_set_channel(oal_wiphy_stru           *pst_wiphy,
                                                    oal_ieee80211_channel    *pst_chan,
                                                    oal_nl80211_channel_type  en_channel_type)
{
#if 1
    /* 通过HOSTAPD 设置信道采用接口wal_ioctl_set_channel */
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_set_channel::should not call this function. call wal_ioctl_set_channel!}\r\n");
    return -OAL_EFAIL;
#else
    oal_net_device_stru         *pst_cfg_net_dev;
    mac_cfg_channel_param_stru  *pst_channel_param;
    wal_msg_write_stru           st_write_msg;
    wlan_channel_bandwidth_enum_uint8   en_bandwith;
    oal_int32                    l_ret;

    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_chan))
    {
        return -OAL_EINVAL;
    }

    pst_cfg_net_dev = oal_dev_get_by_name("Hisilicon0");    /* 获取配置VAP 的net_device 结构 */

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 进行内核带宽值和WITP 带宽值转换 */
    switch(en_channel_type)
    {
        case NL80211_CHAN_NO_HT:
            en_bandwith = WLAN_BAND_WIDTH_20M;
            break;
        case NL80211_CHAN_HT20:
            en_bandwith = WLAN_BAND_WIDTH_20M;
            break;
        case NL80211_CHAN_HT40MINUS:
            en_bandwith = WLAN_BAND_WIDTH_40MINUS;
            break;
        case NL80211_CHAN_HT40PLUS:
            en_bandwith = WLAN_BAND_WIDTH_40PLUS;
            break;
        default:
            return -OAL_EINVAL;
    }

    /* 填写消息 */
    pst_channel_param = (mac_cfg_channel_param_stru *)(st_write_msg.auc_value);
    pst_channel_param->uc_channel   = (oal_uint8)pst_chan->hw_value;
    pst_channel_param->en_band      = pst_chan->band;
    pst_channel_param->en_bandwidth = en_bandwith;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_SET_CHANNEL, OAL_SIZEOF(mac_cfg_channel_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_channel_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
#endif
}
#endif
/*lint +e40*/


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
OAL_STATIC oal_int32 wal_cfg80211_set_wiphy_params(oal_wiphy_stru *pst_wiphy, oal_uint32 ul_changed)
{
#if 1
        /* 通过HOSTAPD 设置RTS 门限，分片门限 采用接口wal_ioctl_set_frag， wal_ioctl_set_rts */
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_set_wiphy_params::should not call this function.call wal_ioctl_set_frag/wal_ioctl_set_rts!}\r\n");
        return OAL_SUCC;
#else

    oal_net_device_stru         *pst_cfg_net_dev;
    mac_cfg_wiphy_param_stru    *pst_wiphy_param;
    wal_msg_write_stru           st_write_msg;
    oal_int32                    l_ret;

    if (OAL_PTR_NULL == pst_wiphy)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_set_wiphy_params::pst_wiphy = NULL!}\r\n");
        return -OAL_EINVAL;
    }

    pst_cfg_net_dev = oal_dev_get_by_name("Hisilicon0");    /* 获取配置VAP 的net_device 结构 */
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(pst_cfg_net_dev);

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/

    /* 填写消息 */
    pst_wiphy_param = (mac_cfg_wiphy_param_stru *)(st_write_msg.auc_value);
    oal_memset(pst_wiphy_param, 0, sizeof(mac_cfg_wiphy_param_stru));

    if (ul_changed & WIPHY_PARAM_FRAG_THRESHOLD)
    {
        OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_wiphy_params::wiphy->frag_threshold = %08X!}\r\n", pst_wiphy->frag_threshold);
        pst_wiphy_param->ul_frag_threshold         = pst_wiphy->frag_threshold;
        pst_wiphy_param->uc_frag_threshold_changed = OAL_TRUE;

        if (pst_wiphy_param->ul_frag_threshold > WAL_MAX_FRAG_THRESHOLD)
        {
            pst_wiphy_param->ul_frag_threshold = WAL_MAX_FRAG_THRESHOLD;
        }
        if (pst_wiphy_param->ul_frag_threshold < WAL_MIN_FRAG_THRESHOLD)
        {
            pst_wiphy_param->ul_frag_threshold = WAL_MIN_FRAG_THRESHOLD;
        }
    }

    if (ul_changed & WIPHY_PARAM_RTS_THRESHOLD)
    {
        OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_wiphy_params::wiphy->frag_threshold = %08X!}\r\n", pst_wiphy->frag_threshold);
        pst_wiphy_param->ul_rts_threshold         = pst_wiphy->rts_threshold;
        pst_wiphy_param->uc_rts_threshold_changed = OAL_TRUE;

        if (pst_wiphy_param->ul_rts_threshold < WAL_MIN_RTS_THRESHOLD)
        {
            pst_wiphy_param->ul_rts_threshold = WAL_MIN_RTS_THRESHOLD;
        }

        if (pst_wiphy_param->ul_rts_threshold > WAL_MAX_RTS_THRESHOLD)
        {
            pst_wiphy_param->ul_rts_threshold = WAL_MAX_RTS_THRESHOLD;
        }
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_SET_WIPHY_PARAMS, OAL_SIZEOF(mac_cfg_wiphy_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_wiphy_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_wiphy_params::return err code %d!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
#endif
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44))

OAL_STATIC oal_int32 wal_cfg80211_set_beacon(
                oal_wiphy_stru        *pst_wiphy,
                oal_net_device_stru   *pst_dev,
                oal_beacon_parameters *pst_beacon_info)
{
    mac_beacon_param_stru    st_beacon_param;  /* beacon info struct */
    wal_msg_write_stru       st_write_msg;
    mac_vap_stru            *pst_mac_vap;
    oal_uint32               ul_ret = 0;
    oal_int32                l_ret = 0;
    oal_uint8                uc_vap_id;
    oal_uint8               *puc_ie;
    mac_cfg_ssid_param_stru *pst_ssid_param;
    oal_uint8                uc_ssid_len;

    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_dev) || (OAL_PTR_NULL == pst_beacon_info))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_set_beacon::pst_wiphy = %x, pst_dev = %x, pst_beacon_info = %x!}\r\n",
                         pst_wiphy, pst_dev, pst_beacon_info);
        return -OAL_EINVAL;
    }

    if(OAL_PTR_NULL == pst_beacon_info->head)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_beacon::pst_beacon_info->head %x!}\r\n", pst_beacon_info->head);
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_dev);
    uc_vap_id   = pst_mac_vap->uc_vap_id;

    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    oal_memset(&st_beacon_param, 0, sizeof(mac_beacon_param_stru));
    st_beacon_param.l_interval    = pst_beacon_info->interval;
    st_beacon_param.l_dtim_period = pst_beacon_info->dtim_period;

    puc_ie = mac_get_ssid_etc(pst_beacon_info->head + MAC_80211_FRAME_LEN, (pst_beacon_info->head_len - MAC_80211_FRAME_LEN), &uc_ssid_len);
    /* 隐藏SSID两种情况:(1)长度为0，(2)长度为实际ssid长度，但内容为空 */
    st_beacon_param.uc_hidden_ssid = 0;
    if (OAL_TRUE == mac_is_hide_ssid(puc_ie, uc_ssid_len))
    {
        st_beacon_param.uc_hidden_ssid = 1;
    }

    /*****************************************************************************
        1. 下发安全配置消息
    *****************************************************************************/
    /* 获取 WPA/WPA2 信息元素 */
    if (OAL_PTR_NULL == pst_beacon_info->tail || OAL_PTR_NULL == pst_beacon_info->head)
    {
        OAM_ERROR_LOG2(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_beacon::beacon frame error tail = %d, head = %d!}\r\n",
                       pst_beacon_info->tail, pst_beacon_info->head);
        return -OAL_EINVAL;
    }

    wal_parse_wpa_wpa2_ie_etc(pst_beacon_info, &st_beacon_param);

#ifdef _PRE_WLAN_FEATURE_11R_AP
    /* 获取 MD 信息元素 */
    ul_ret = wal_parse_md_ie(pst_beacon_info, &st_beacon_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_beacon::failed to parse MD ie!}\r\n");
        return -OAL_EINVAL;
    }
#endif

    ul_ret = wal_parse_ht_vht_ie(pst_mac_vap, pst_beacon_info, &st_beacon_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_beacon::Failed to parse HT/VHT ie!}\r\n");
        return -OAL_EINVAL;
    }

 #ifdef _PRE_WLAN_FEATURE_11D
    /* 对日本14信道作特殊判断，只在11b模式下才能启用14，非11b模式 降为11b */
    if ((14 == pst_mac_vap->st_channel.uc_chan_number) && (WLAN_LEGACY_11B_MODE != st_beacon_param.en_protocol))
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_beacon::Now change protocol to 11b!}\r\n");
        st_beacon_param.en_protocol = WLAN_LEGACY_11B_MODE;
    }
#endif

    /*设置操作类型*/
    st_beacon_param.en_operation_type = MAC_SET_BEACON;

    /* 填写 msg 消息头*/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, OAL_SIZEOF(mac_beacon_param_stru));

    /* 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_beacon_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::Failed to start addset beacon!}\r\n");
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        2. 下发SSID配置消息
    *****************************************************************************/
    if (0 != uc_ssid_len)
    {
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SSID, OAL_SIZEOF(mac_cfg_ssid_param_stru));

        pst_ssid_param = (mac_cfg_ssid_param_stru *)(st_write_msg.auc_value);
        pst_ssid_param->uc_ssid_len = uc_ssid_len;
        oal_memcopy(pst_ssid_param->ac_ssid, puc_ie, uc_ssid_len);

        l_ret = wal_send_cfg_event_etc(pst_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ssid_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::fail to send ssid cfg msg!}\r\n");
            return -OAL_EFAIL;
        }

    }

    /* 3. 配置WPS信息元素 */
#if 0
    ul_ret = wal_parse_beacon_wps_ie(pst_mac_vap, pst_beacon_info);
    if (OAL_SUCC != ul_ret)
    {
        return -OAL_EINVAL;
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_add_beacon(
                oal_wiphy_stru        *pst_wiphy,
                oal_net_device_stru   *pst_dev,
                oal_beacon_parameters *pst_beacon_info)
{
    mac_beacon_param_stru    st_beacon_param;  /* beacon info struct */
    wal_msg_write_stru       st_write_msg;
    mac_vap_stru            *pst_mac_vap;
    oal_uint32               ul_ret = 0;
    oal_int32                l_ret = 0;
    oal_uint8                uc_vap_id;
    oal_uint8               *puc_ie;
    mac_cfg_ssid_param_stru *pst_ssid_param;
    oal_uint8                uc_ssid_len;

    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_dev) || (OAL_PTR_NULL == pst_beacon_info))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_add_beacon::pst_wiphy = %x, pst_dev = %x, pst_beacon_info = %x!}\r\n",
                         pst_wiphy, pst_dev, pst_beacon_info);
        return -OAL_EINVAL;
    }

    if(OAL_PTR_NULL == pst_beacon_info->head)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_add_beacon::pst_beacon_info->head %x!}\r\n", pst_beacon_info->head);
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_dev);
    uc_vap_id   = pst_mac_vap->uc_vap_id;

    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    oal_memset(&st_beacon_param, 0, sizeof(mac_beacon_param_stru));
    st_beacon_param.l_interval    = pst_beacon_info->interval;
    st_beacon_param.l_dtim_period = pst_beacon_info->dtim_period;

    puc_ie = mac_get_ssid_etc(pst_beacon_info->head + MAC_80211_FRAME_LEN, (pst_beacon_info->head_len - MAC_80211_FRAME_LEN), &uc_ssid_len);
    /* 隐藏SSID两种情况:(1)长度为0，(2)长度为实际ssid长度，但内容为空 */
    st_beacon_param.uc_hidden_ssid = 0;
    if (OAL_TRUE == mac_is_hide_ssid(puc_ie, uc_ssid_len))
    {
        st_beacon_param.uc_hidden_ssid = 1;
    }

    /*****************************************************************************
        1. 下发安全配置消息
    *****************************************************************************/
    /* 获取 WPA/WPA2 信息元素 */
    if (OAL_PTR_NULL == pst_beacon_info->tail || OAL_PTR_NULL == pst_beacon_info->head)
    {
        OAM_ERROR_LOG2(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::beacon frame error tail = %d, head = %d!}\r\n",
                       pst_beacon_info->tail, pst_beacon_info->head);
        return -OAL_EINVAL;
    }

    wal_parse_wpa_wpa2_ie_etc(pst_beacon_info, &st_beacon_param);

#ifdef _PRE_WLAN_FEATURE_11R_AP
    /* 获取 MD 信息元素 */
    ul_ret = wal_parse_md_ie(pst_beacon_info, &st_beacon_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::failed to parse MD ie!}\r\n");
        return -OAL_EINVAL;
    }
#endif

    ul_ret = wal_parse_ht_vht_ie(pst_mac_vap, pst_beacon_info, &st_beacon_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::Failed to parse HT/VHT ie!}\r\n");
        return -OAL_EINVAL;
    }

 #ifdef _PRE_WLAN_FEATURE_11D
    /* 对日本14信道作特殊判断，只在11b模式下才能启用14，非11b模式 降为11b */
    if ((14 == pst_mac_vap->st_channel.uc_chan_number) && (WLAN_LEGACY_11B_MODE != st_beacon_param.en_protocol))
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::Now change protocol to 11b!}\r\n");
        st_beacon_param.en_protocol = WLAN_LEGACY_11B_MODE;
    }
#endif

    /*设置操作类型*/
    st_beacon_param.en_operation_type = MAC_ADD_BEACON;

    /* 填写 msg 消息头*/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, OAL_SIZEOF(mac_beacon_param_stru));

    /* 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_beacon_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::Failed to start addset beacon!}\r\n");
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        2. 下发SSID配置消息
    *****************************************************************************/
    if (0 != uc_ssid_len)
    {
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SSID, OAL_SIZEOF(mac_cfg_ssid_param_stru));

        pst_ssid_param = (mac_cfg_ssid_param_stru *)(st_write_msg.auc_value);
        pst_ssid_param->uc_ssid_len = uc_ssid_len;
        oal_memcopy(pst_ssid_param->ac_ssid, puc_ie, uc_ssid_len);

        l_ret = wal_send_cfg_event_etc(pst_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ssid_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::fail to send ssid cfg msg!}\r\n");
            return -OAL_EFAIL;
        }

    }

    /* 3.配置Wmm信息元素 */
    ul_ret = wal_parse_wmm_ie_etc(pst_dev, pst_mac_vap, pst_beacon_info);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::Failed to parse wmm ie!}\r\n");
        return -OAL_EINVAL;
    }

    /* 5.启动AP */
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    l_ret = wal_start_vap_etc(pst_dev);
#else
    l_ret = wal_netdev_open_etc(pst_dev,OAL_FALSE);
#endif
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_beacon::failed to start ap, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_del_beacon(oal_wiphy_stru        *pst_wiphy,
                                                oal_net_device_stru *pst_netdev)
{
    wal_msg_write_stru           st_write_msg;
    mac_vap_stru                *pst_mac_vap;
    oal_int32                    l_ret = 0;
    oal_uint8                    uc_vap_id;

#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_wireless_dev_stru   *pst_wdev;
#endif
#ifdef _PRE_WLAN_FEATURE_AP_PM
    mac_device_stru         *pst_device;
    mac_pm_arbiter_stru     *pst_pm_arbiter;
#endif

    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_netdev))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_del_beacon::pst_wiphy = %x, pst_netdev = %x!}",
                       pst_wiphy, pst_netdev);
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_beacon::dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return OAL_SUCC;
    }

#endif

    /* 获取vap id */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_netdev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_beacon::can't get mac vap from netdevice priv data!}");
        return -OAL_EINVAL;
    }

    uc_vap_id   = pst_mac_vap->uc_vap_id;

    /* 判断是否为非ap模式 */
    if (WLAN_VAP_MODE_BSS_AP != pst_mac_vap->en_vap_mode)
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_del_beacon::vap is not in ap mode!}");
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_AP_PM
    pst_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (pst_device != OAL_PTR_NULL)
    {
        pst_pm_arbiter = (mac_pm_arbiter_stru*)(pst_device->pst_pm_arbiter);

        if (pst_pm_arbiter != OAL_PTR_NULL)
        {
            if ((OAL_TRUE == pst_device->en_pm_enable)
                && (DEV_PWR_STATE_WOW == pst_pm_arbiter->uc_cur_state)
                && (OAL_TRUE == g_uc_is_cpu_sleep))
            {
                OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_beacon:: device is in WOW state!}");
                return OAL_SUCC;
            }
        }
    }
#endif
    /*****************************************************************************
        发送消息，停用ap
    *****************************************************************************/
    /* 填写消息 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DOWN_VAP, OAL_SIZEOF(mac_cfg_start_vap_param_stru));

#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev    = pst_netdev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_beacon::wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}\r\n");
        return -OAL_EINVAL;
    }
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_beacon::en_p2p_mode=%u}\r\n", en_p2p_mode);

    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;
#endif

    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_netdev;

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_start_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_del_beacon::failed to stop ap, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

#endif

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44)) || (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION))
#if 0

OAL_STATIC oal_int32 wal_cfg80211_bringup_ap(oal_net_device_stru *pst_netdev)
{
    wal_msg_write_stru st_write_msg;
    oal_int32          l_ret = 0;

    /* 填写消息 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_START_VAP, OAL_SIZEOF(mac_cfg_start_vap_param_stru));
    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_netdev;

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_start_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_bringup_ap::failed to bring up ap, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_int32 wal_cfg80211_set_ssid(oal_net_device_stru *pst_netdev,
                                                oal_uint8           *puc_ssid_ie,
                                                oal_uint8            uc_ssid_len)
{
    wal_msg_write_stru           st_write_msg;
    mac_cfg_ssid_param_stru     *pst_ssid_param;
    oal_int32                    l_ret = 0;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SSID, OAL_SIZEOF(mac_cfg_ssid_param_stru));

    pst_ssid_param = (mac_cfg_ssid_param_stru *)(st_write_msg.auc_value);
    pst_ssid_param->uc_ssid_len = uc_ssid_len;
    oal_memcopy(pst_ssid_param->ac_ssid, (oal_int8 *)puc_ssid_ie, uc_ssid_len);

    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ssid_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_ssid::fail to send ssid cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_set_auth_mode(oal_net_device_stru *pst_netdev, oal_uint8 en_auth_algs)
{
    wal_msg_write_stru           st_write_msg;
    oal_int32                    l_ret = 0;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_AUTH_MODE, OAL_SIZEOF(oal_uint32));
    *((oal_uint32 *)(st_write_msg.auc_value)) = en_auth_algs;  /* 设置配置命令参数 */

    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_auth_mode::fail to send auth_tpye cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}



OAL_STATIC oal_int32 wal_cfg80211_fill_beacon_param(oal_net_device_stru *pst_netdev,
                                                           oal_beacon_data_stru        *pst_beacon_info,
                                                           mac_beacon_param_stru       *pst_beacon_param)
{
    oal_beacon_parameters st_beacon_info_tmp;
    oal_uint32    ul_ret;
    oal_uint8    *puc_beacon_info_tmp;
    oal_uint32    ul_beacon_head_len;
    oal_uint32    ul_beacon_tail_len;
    oal_uint8     uc_vap_id;
    oal_uint32    ul_offset;
    mac_vap_stru *pst_mac_vap;

    if (OAL_PTR_NULL == pst_netdev
       || OAL_PTR_NULL == pst_beacon_info
       || OAL_PTR_NULL == pst_beacon_param)
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::param is NULL. pst_mac_vap=0x%x, pst_beacon_info=0x%x, pst_beacon_param=0x%x",
           pst_netdev, pst_beacon_info, pst_beacon_param);
        return -OAL_EINVAL;
    }
     /* 获取vap id */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_netdev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::pst_mac_vap = %x}", pst_mac_vap);
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;
    /*****************************************************************************
        1.安全配置ie消息等
    *****************************************************************************/
    if (OAL_PTR_NULL == pst_beacon_info->tail || OAL_PTR_NULL == pst_beacon_info->head)
    {
        OAM_ERROR_LOG2(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::beacon frame error tail = %x, head = %x!}",
                       pst_beacon_info->tail, pst_beacon_info->head);
        return -OAL_EINVAL;
    }

    ul_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    /* oal_ieee80211_mgmt 前面公共部分size为 MAC_80211_FRAME_LEN(24)  */
    if (pst_beacon_info->head_len < (ul_offset + MAC_80211_FRAME_LEN))
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_fill_beacon_param::pst_beacon_info head_len[%d] error.}", pst_beacon_info->head_len);
        return -OAL_EINVAL;
    }

    ul_beacon_head_len = (oal_uint32)pst_beacon_info->head_len;
    ul_beacon_tail_len = (oal_uint32)pst_beacon_info->tail_len;
    puc_beacon_info_tmp = (oal_uint8 *)(oal_memalloc(ul_beacon_head_len + ul_beacon_tail_len));
    if (OAL_PTR_NULL == puc_beacon_info_tmp)
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::puc_beacon_info_tmp memalloc failed.}");
        return -OAL_EINVAL;
    }
    else
    {
        oal_memcopy(puc_beacon_info_tmp, pst_beacon_info->head, ul_beacon_head_len);
        oal_memcopy(puc_beacon_info_tmp + ul_beacon_head_len, pst_beacon_info->tail, ul_beacon_tail_len);
    }

    /* 为了复用51的解析接口，将新内核结构中的内容赋值给51接口识别的结构体，进而获取信息元素 */
    OAL_MEMZERO(&st_beacon_info_tmp, sizeof(st_beacon_info_tmp));
    st_beacon_info_tmp.head     = puc_beacon_info_tmp;
    st_beacon_info_tmp.head_len = (oal_int32)ul_beacon_head_len;
    st_beacon_info_tmp.tail     = puc_beacon_info_tmp + ul_beacon_head_len;
    st_beacon_info_tmp.tail_len = (oal_int32)ul_beacon_tail_len;

    /* 获取 WPA/WPA2 信息元素 */
    wal_parse_wpa_wpa2_ie_etc(&st_beacon_info_tmp, pst_beacon_param);

    /* 此接口需要修改，linux上没问题，但是win32有错 TBD */
    ul_ret = wal_parse_ht_vht_ie(pst_mac_vap, &st_beacon_info_tmp, pst_beacon_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::failed to parse HT/VHT ie!}");
        oal_free(puc_beacon_info_tmp);
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_11R_AP
    /* 获取 MD 信息元素 */
    ul_ret = wal_parse_md_ie(&st_beacon_info_tmp, pst_beacon_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::failed to parse MD ie!}");
        oal_free(puc_beacon_info_tmp);
        return -OAL_EINVAL;
    }
#endif

    /* 解析是否隐藏SSID */
    if (0 == puc_beacon_info_tmp[WAL_BCN_BSSID_LENGTH_ADDR])
    {
        pst_beacon_param->uc_hidden_ssid = 1;
    }

    /* 配置Wmm信息元素 */
    ul_ret = wal_parse_wmm_ie_etc(pst_netdev, pst_mac_vap, &st_beacon_info_tmp);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::Failed to parse wmm ie!}");
        return -OAL_EINVAL;
    }
    /* 释放临时申请的内存 */
    oal_free(puc_beacon_info_tmp);

#ifdef _PRE_WLAN_FEATURE_11D
    /* 对日本14信道作特殊判断，只在11b模式下才能启用14，非11b模式 降为11b */
    if ((14 == pst_mac_vap->st_channel.uc_chan_number) && (WLAN_LEGACY_11B_MODE != pst_beacon_param->en_protocol))
    {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY,
                       "{wal_cfg80211_fill_beacon_param::ch 14 should in 11b, but is %d!}", pst_beacon_param->en_protocol);
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::change protocol to 11b!}");
        pst_beacon_param->en_protocol = WLAN_LEGACY_11B_MODE;
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_change_beacon(oal_wiphy_stru         *pst_wiphy,
                                                        oal_net_device_stru     *pst_netdev,
                                                        oal_beacon_data_stru    *pst_beacon_info)
{
    mac_beacon_param_stru        st_beacon_param;  /* beacon info struct */
    wal_msg_write_stru           st_write_msg;
    oal_int32                    l_ret = 0;

    OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::enter here.}");

    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_netdev) || (OAL_PTR_NULL == pst_beacon_info))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::pst_wiphy = %x, pst_netdev = %x, pst_beacon_info = %x!}",
                         pst_wiphy, pst_netdev, pst_beacon_info);
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return OAL_SUCC;
    }
#endif //#ifdef _PRE_WLAN_FEATURE_DFR


    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    oal_memset(&st_beacon_param, 0, sizeof(mac_beacon_param_stru));

    l_ret = wal_cfg80211_fill_beacon_param(pst_netdev, pst_beacon_info, &st_beacon_param);
    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::failed to fill beacon param, error[%d]}", l_ret);
        return -OAL_EINVAL;
    }

    /*设置操作类型*/
    st_beacon_param.en_operation_type = MAC_SET_BEACON;

    /* 填写 msg 消息头*/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, OAL_SIZEOF(mac_beacon_param_stru));

    /* 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_beacon_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::Failed to start addset beacon, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_convert_width_to_value(oal_int32 l_channel_width)
{
    oal_int32 l_channel_width_value = 0;

    switch (l_channel_width)
    {
        case 0:
        case 1:
            l_channel_width_value = 20;
            break;
        case 2:
            l_channel_width_value = 40;
            break;
        case 3:
        case 4:
            l_channel_width_value = 80;
            break;
        case 5:
            l_channel_width_value = 160;
            break;
        default:
            break;
    }

    return l_channel_width_value;
}


OAL_STATIC oal_int32 wal_cfg80211_set_channel_info(oal_wiphy_stru      *pst_wiphy,
                                                          oal_net_device_stru *pst_netdev)
{
    mac_cfg_channel_param_stru          *pst_channel_param;
    oal_ieee80211_channel               *pst_channel;
    wlan_channel_bandwidth_enum_uint8    en_bandwidth;
    wal_msg_write_stru                   st_write_msg;
    oal_uint32                           ul_err_code;
    oal_int32                            l_channel;
    oal_int32                            l_center_freq1;
    oal_int32                            l_center_freq2;
    oal_int32                            l_bandwidth;
    oal_int32                            l_bandwidth_value;
    oal_uint8                            uc_vht_width;
    wal_msg_stru                        *pst_rsp_msg = OAL_PTR_NULL;
    oal_int32                            l_ret;
    mac_vap_stru                        *pst_mac_vap;
    oal_uint8                            uc_vap_id;
    oal_int32                            l_channel_center_freq0;
    oal_int32                            l_channel_center_freq1;

    /* 获取vap id */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_netdev);
    uc_vap_id   = pst_mac_vap->uc_vap_id;

    l_bandwidth    = pst_netdev->ieee80211_ptr->preset_chandef.width;
    l_center_freq1 = pst_netdev->ieee80211_ptr->preset_chandef.center_freq1;
    l_center_freq2 = pst_netdev->ieee80211_ptr->preset_chandef.center_freq2;
    pst_channel    = pst_netdev->ieee80211_ptr->preset_chandef.chan;
    l_channel      = pst_channel->hw_value;

    OAM_WARNING_LOG4(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_channel_info::l_bandwidth = %d, l_center_freq1 = %d, l_center_freq2 = %d, l_channel = %d.}",
                  l_bandwidth, l_center_freq1, l_center_freq2, l_channel);

    /* 判断信道在不在管制域内 */
    l_ret = (oal_int32)mac_is_channel_num_valid_etc(pst_channel->band, (oal_uint8)l_channel);
    if (l_ret != OAL_SUCC)
    {
        OAM_WARNING_LOG2(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_channel_info::channel num is invalid. band, ch num [%d] [%d]!}\r\n", pst_channel->band, l_channel);
        return -OAL_EINVAL;
    }

    /* 进行内核带宽值和WITP 带宽值转换 */
    l_channel_center_freq0 = oal_ieee80211_frequency_to_channel(l_center_freq1);
    l_channel_center_freq1 = oal_ieee80211_frequency_to_channel(l_center_freq2);
    l_bandwidth_value = wal_cfg80211_convert_width_to_value(l_bandwidth);
    uc_vht_width = wal_cfg80211_convert_value_to_vht_width(l_bandwidth_value);
    if (0 == l_bandwidth_value)
    {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_channel_info::channel width is invalid, l_bandwidth = %d.\r\n", l_bandwidth);
        return -OAL_EINVAL;
    }

    if (80 == l_bandwidth_value || 160 == l_bandwidth_value)
    {
        en_bandwidth = mac_get_bandwith_from_center_freq_seg0_seg1(uc_vht_width, (oal_uint8)l_channel, (oal_uint8)l_channel_center_freq0, (oal_uint8)l_channel_center_freq1);
    }
    else if (40 == l_bandwidth_value)
    {
        switch (l_channel_center_freq0 - l_channel)
        {
            case -2:
                en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
                break;
            case 2:
                en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
                break;
            default:
                en_bandwidth = WLAN_BAND_WIDTH_20M;
                break;
        }
    }
    else
    {
        en_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/

    /* 填写消息 */
    pst_channel_param = (mac_cfg_channel_param_stru *)(st_write_msg.auc_value);
    pst_channel_param->uc_channel   = (oal_uint8)pst_channel->hw_value;
    pst_channel_param->en_band      = pst_channel->band;
    pst_channel_param->en_bandwidth = en_bandwidth;

    OAM_WARNING_LOG3(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_channel::uc_channel = %d, en_band = %d, en_bandwidth = %d.}",
                     pst_channel_param->uc_channel,
                     pst_channel_param->en_band,
                     pst_channel_param->en_bandwidth);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_SET_CHANNEL, OAL_SIZEOF(mac_cfg_channel_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_channel_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_cfg80211_set_channel_info::wal_send_cfg_event_etc return err code: [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    /* 读取返回的错误码 */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_channel_info::wal_check_and_release_msg_resp_etc fail return err code: [%u].}",
                       ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_start_ap(oal_wiphy_stru           *pst_wiphy,
                                                oal_net_device_stru      *pst_netdev,
                                                oal_ap_settings_stru     *pst_ap_settings)
{
    mac_beacon_param_stru        st_beacon_param;  /* beacon info struct */
    wal_msg_write_stru           st_write_msg;
    mac_vap_stru                *pst_mac_vap;
    oal_beacon_data_stru        *pst_beacon_info;
    oal_uint8                   *puc_ssid_ie;
    oal_int32                    l_ret = 0;
    oal_uint8                    uc_ssid_len;
    oal_uint8                    uc_vap_id;
    oal_uint8                    auc_ssid_ie[32];
    oal_int32                    l_ssid_len = 0;
    oal_netdev_priv_stru        *pst_netdev_priv = OAL_PTR_NULL;
    wlan_auth_alg_mode_enum_uint8     en_auth_algs;

    OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::enter here.}");

    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_netdev) || (OAL_PTR_NULL == pst_ap_settings))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::pst_wiphy = %x, pst_netdev = %x, pst_ap_settings = %x!}",
                       pst_wiphy, pst_netdev, pst_ap_settings);
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_netdev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::pst_mac_vap = %x}", pst_mac_vap);
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;

    /*****************************************************************************
        1.设置信道
    *****************************************************************************/
    l_ret = wal_cfg80211_set_channel_info(pst_wiphy, pst_netdev);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed to set channel, return err code[%x]}", l_ret);
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        2.1 设置ssid信息
    *****************************************************************************/
    l_ssid_len = pst_ap_settings->ssid_len;
    if ((l_ssid_len > 32) || (l_ssid_len <= 0))
    {
         OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::ssid len error, len[%d].}", l_ssid_len);
         return -OAL_EFAIL;
    }
    oal_memset(auc_ssid_ie, 0, sizeof(auc_ssid_ie));
    oal_memcopy(auc_ssid_ie, pst_ap_settings->ssid, (oal_uint32)l_ssid_len);
    puc_ssid_ie = auc_ssid_ie;
    uc_ssid_len = (oal_uint8)l_ssid_len;

    if (0 != uc_ssid_len)
    {
        l_ret = wal_cfg80211_set_ssid(pst_netdev, puc_ssid_ie, uc_ssid_len);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::fail to send ssid cfg msg, error[%d]}", l_ret);
            return -OAL_EFAIL;
        }
    }

    /*****************************************************************************
        2.2 设置auth mode信息
    *****************************************************************************/
    en_auth_algs = pst_ap_settings->auth_type;
    if((WLAN_WITP_AUTH_OPEN_SYSTEM != en_auth_algs)
    && (WLAN_WITP_AUTH_SHARED_KEY != en_auth_algs)
    && (WLAN_WITP_AUTH_AUTOMATIC != en_auth_algs))
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::en_auth_algs error[%d].}", en_auth_algs);
        return -OAL_EFAIL;
    }

    l_ret = wal_cfg80211_set_auth_mode(pst_netdev, en_auth_algs);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::fail to send auth_tpye cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        3.设置beacon时间间隔、tim period以及安全配置消息等
    *****************************************************************************/

    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    oal_memset(&st_beacon_param, 0, sizeof(mac_beacon_param_stru));
    st_beacon_param.l_interval    = pst_ap_settings->beacon_interval;
    st_beacon_param.l_dtim_period = pst_ap_settings->dtim_period;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))//TBD:1102 需要，暂时用内核版本号区分
    st_beacon_param.uc_hidden_ssid = (pst_ap_settings->hidden_ssid == 1);

    OAM_WARNING_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::beacon_interval=%d, dtim_period=%d, hidden_ssid=%d!}",
                  pst_ap_settings->beacon_interval,
                  pst_ap_settings->dtim_period,
                  pst_ap_settings->hidden_ssid);
#endif
    pst_beacon_info               = &(pst_ap_settings->beacon);
    l_ret = wal_cfg80211_fill_beacon_param(pst_netdev, pst_beacon_info, &st_beacon_param);
    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed to fill beacon param, error[%d]}", l_ret);
        return -OAL_EINVAL;
    }

    /*设置操作类型*/
    st_beacon_param.en_operation_type = MAC_ADD_BEACON;

    /* 填写 msg 消息头*/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, OAL_SIZEOF(mac_beacon_param_stru));

    /* 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_beacon_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed to start addset beacon, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }
    /*****************************************************************************
        4.启动ap
    *****************************************************************************/
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    l_ret = wal_start_vap_etc(pst_netdev);
#else
    l_ret = wal_netdev_open_etc(pst_netdev,OAL_FALSE);
#endif
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed to start ap, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    pst_netdev_priv = (oal_netdev_priv_stru *)OAL_NET_DEV_WIRELESS_PRIV(pst_netdev);
    if ((pst_netdev_priv->uc_napi_enable) && (!pst_netdev_priv->uc_state))
    {
        oal_napi_enable(&pst_netdev_priv->st_napi);
        pst_netdev_priv->uc_state = 1;
    }


    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_stop_ap(oal_wiphy_stru        *pst_wiphy,
                                                oal_net_device_stru *pst_netdev)
{
    wal_msg_write_stru           st_write_msg;
    mac_vap_stru                *pst_mac_vap;
    oal_int32                    l_ret = 0;
    oal_uint8                    uc_vap_id;
    oal_netdev_priv_stru        *pst_netdev_priv;

#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_wireless_dev_stru   *pst_wdev;
#endif
#ifdef _PRE_WLAN_FEATURE_AP_PM
    mac_device_stru         *pst_device;
    mac_pm_arbiter_stru     *pst_pm_arbiter;
#endif

    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_netdev))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::pst_wiphy = %x, pst_netdev = %x!}",
                       pst_wiphy, pst_netdev);
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return OAL_SUCC;
    }

#endif

    /* 获取vap id */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_netdev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::can't get mac vap from netdevice priv data!}");
        return -OAL_EINVAL;
    }

    uc_vap_id   = pst_mac_vap->uc_vap_id;

    /* 判断是否为非ap模式 */
    if (WLAN_VAP_MODE_BSS_AP != pst_mac_vap->en_vap_mode)
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_stop_ap::vap is not in ap mode!}");
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_AP_PM
    pst_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    pst_pm_arbiter = (mac_pm_arbiter_stru*)(pst_device->pst_pm_arbiter);

    if ((pst_device != OAL_PTR_NULL) && (pst_pm_arbiter != OAL_PTR_NULL))
    {
        if ((OAL_TRUE == pst_device->en_pm_enable)
            && (DEV_PWR_STATE_WOW == pst_pm_arbiter->uc_cur_state)
            && (OAL_TRUE == g_uc_is_cpu_sleep))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap:: device is in WOW state!}");
            return OAL_SUCC;
        }
    }
#endif

    /* 如果netdev不是running状态，则不需要down */
    if (0 == (OAL_NETDEVICE_FLAGS(pst_netdev) & OAL_IFF_RUNNING))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::vap is already down!}\r\n");
        return OAL_SUCC;
    }

    /*****************************************************************************
        发送消息，停用ap
    *****************************************************************************/
    /* 填写消息 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DOWN_VAP, OAL_SIZEOF(mac_cfg_start_vap_param_stru));

#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev    = pst_netdev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}\r\n");
        return -OAL_EINVAL;
    }
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::en_p2p_mode=%u}\r\n", en_p2p_mode);

    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;
#endif

    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_netdev;

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_start_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_stop_ap::failed to stop ap, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    pst_netdev_priv = (oal_netdev_priv_stru *)OAL_NET_DEV_WIRELESS_PRIV(pst_netdev);
    if (pst_netdev_priv->uc_napi_enable)
    {
    #ifndef WIN32
        oal_netbuf_list_purge(&pst_netdev_priv->st_rx_netbuf_queue);
    #endif
        oal_napi_disable(&pst_netdev_priv->st_napi);
        pst_netdev_priv->uc_state = 0;
    }

    return OAL_SUCC;
}

#if 0

OAL_STATIC oal_int32 wal_cfg80211_get_mode(oal_net_device_stru      *pst_netdev,
                                                  wlan_protocol_enum_uint8 *puc_protocol_mode)
{
    oal_int32                       l_ret;
    wal_msg_query_stru              st_query_msg;
    wal_msg_stru                   *pst_rsp_msg;
    wal_msg_rsp_stru               *pst_query_rsp_msg;
    oal_uint8                       uc_prot_idx;
    mac_cfg_mode_param_stru        *pst_mode_param;
    mac_vap_stru                   *pst_mac_vap;

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/

    st_query_msg.en_wid = WLAN_CFGID_MODE;

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_get_mode::wal_alloc_cfg_event_etc return err code %d!}", l_ret);
        return l_ret;
    }
#error  wal_send_cfg_event_etc API had changed,please recode!

    /* 处理返回消息 */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_get_mode::rsp msg wid %d!}", pst_query_rsp_msg->en_wid);

    pst_mode_param = (mac_cfg_mode_param_stru *)(pst_query_rsp_msg->auc_value);

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_netdev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_get_mode::null pointer.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        for (uc_prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map_etc[uc_prot_idx].pc_name; uc_prot_idx++)
        {
            if ((g_ast_mode_map_etc[uc_prot_idx].en_mode == pst_mode_param->en_protocol) &&
                (g_ast_mode_map_etc[uc_prot_idx].en_band == pst_mode_param->en_band) &&
                (g_ast_mode_map_etc[uc_prot_idx].en_bandwidth == pst_mode_param->en_bandwidth))
            {
                break;
            }
        }
    }
    /* STA模式下频段和频段在和AP关联之后自适应，此处仅比较协议模式 */
    else if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        for (uc_prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map_etc[uc_prot_idx].pc_name; uc_prot_idx++)
        {
            if (g_ast_mode_map_etc[uc_prot_idx].en_mode == pst_mode_param->en_protocol)
            {
                break;
            }
        }
    }
    else
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_get_mode::vap mode error, vap mode = %d.}",
                       pst_mac_vap->en_vap_mode);
        return OAL_SUCC;
    }

    if (OAL_PTR_NULL == g_ast_mode_map_etc[uc_prot_idx].pc_name)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_get_mode::uc_prot_idx[%d], en_protocol[%d].}",
                       uc_prot_idx, pst_mode_param->en_protocol);
        return OAL_SUCC;
    }

    *puc_protocol_mode = g_ast_mode_map_etc[uc_prot_idx].en_mode;
    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_set_mode(oal_net_device_stru      *pst_netdev,
                                                  wlan_protocol_enum_uint8 *puc_protocol_mode)
{
    mac_cfg_mode_param_stru    *pst_mode_param;
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;
    oal_uint8                   uc_prot_idx;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_netdev) || (OAL_PTR_NULL == puc_protocol_mode)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_set_mode::pst_net_dev = %x, puc_protocol_mode = %x.}",
                       pst_netdev, puc_protocol_mode);
        return -OAL_EFAUL;
    }

    for (uc_prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map_etc[uc_prot_idx].pc_name; uc_prot_idx++)
    {
        if (g_ast_mode_map_etc[uc_prot_idx].en_mode == *puc_protocol_mode)
        {
            break;
        }
    }

    if (OAL_PTR_NULL == g_ast_mode_map_etc[uc_prot_idx].pc_name)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_mode::un recognized protocol mode[%d]!}",
                         *puc_protocol_mode);
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_MODE, OAL_SIZEOF(mac_cfg_mode_param_stru));

    pst_mode_param = (mac_cfg_mode_param_stru *)(st_write_msg.auc_value);
    pst_mode_param->en_protocol  = g_ast_mode_map_etc[uc_prot_idx].en_mode;
    pst_mode_param->en_band      = g_ast_mode_map_etc[uc_prot_idx].en_band;
    pst_mode_param->en_bandwidth = g_ast_mode_map_etc[uc_prot_idx].en_bandwidth;

    OAM_INFO_LOG3(0, OAM_SF_CFG, "{wal_cfg80211_set_mode::protocol[%d],band[%d],bandwidth[%d]!}\r\n",
                  pst_mode_param->en_protocol, pst_mode_param->en_band, pst_mode_param->en_bandwidth);

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_mode_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_mode::wal_alloc_cfg_event_etc return err code[%d]}", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_int32 wal_cfg80211_change_bss(oal_wiphy_stru        *pst_wiphy,
                                                    oal_net_device_stru   *pst_netdev,
                                                    oal_bss_parameters    *pst_bss_params)
{
#if 0
    mac_vap_stru                *pst_mac_vap;
    oal_int32                    l_ret = 0;
    oal_uint8                    uc_vap_id;
    wlan_protocol_enum_uint8     en_prev_protocol_mode = WLAN_PROTOCOL_BUTT;
    wlan_protocol_enum_uint8     en_curr_protocol_mode = WLAN_PROTOCOL_BUTT;

    OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_change_bss::enter here.}");

    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_netdev) || (OAL_PTR_NULL == pst_bss_params))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_change_bss::pst_wiphy = %p, pst_netdev = %p, pst_bss_params = %p!}",
                       pst_wiphy, pst_netdev, pst_bss_params);
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_netdev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_change_bss::pst_mac_vap is null!}");
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;

    /* 如果下发的bss参数中cts保护打开，不设置驱动 */
    if (pst_bss_params->use_cts_prot >= 0)
    {
        /* do noting */
    }

    /* 如果下发的bss参数中短前导参数大于0，不设置参数 */
    if (pst_bss_params->use_short_preamble >= 0)
    {
        /* do noting */
    }

    if (pst_bss_params->use_short_slot_time >= 0)
    {
        /* do noting */
    }

    /* set ap isolate */
    if (pst_bss_params->ap_isolate >= 0)
    {
        /* do noting */
    }

    /* 根据支持的最大速率，获取当前支持的模式 */
    if (pst_bss_params->basic_rates)
    {
        switch ((int)(pst_bss_params->basic_rates[pst_bss_params->basic_rates_len - 1]))
        {
            case 22: /* B only , rate 11 */
                en_curr_protocol_mode = WLAN_LEGACY_11B_MODE;
                break;
            case 108: /* G only , rate 54 */
                en_curr_protocol_mode = WLAN_MIXED_TWO_11G_MODE;
                break;
            default:
                en_curr_protocol_mode = WLAN_LEGACY_11B_MODE;
                break;
        }
    }

    if (pst_bss_params->ht_opmode >= 0)
    {
        en_curr_protocol_mode = WLAN_HT_MODE;
    }

    /*****************************************************************************
        1.获取原先的协议模式，如果和将要配置的相同，则直接返回
    *****************************************************************************/
    l_ret = wal_cfg80211_get_mode(pst_netdev, &en_prev_protocol_mode);
    if (OAL_SUCC == l_ret)
    {
        /* 如果要设置的协议模式和原先的协议模式一致，直接返回成功 */
        if (en_prev_protocol_mode == en_curr_protocol_mode)
        {
            OAM_INFO_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_change_bss::old protocol mode = new protocol mode.}");
            return OAL_SUCC;
        }
    }
    else
    {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_change_bss:: get mode failed, error[%d]}", l_ret);
    }

    /*****************************************************************************
        2.停用ap
    *****************************************************************************/
    l_ret = wal_cfg80211_stop_ap(pst_wiphy, pst_netdev);
    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_change_bss::failed to stop ap, error[%d].}", l_ret);
        return -OAL_EINVAL;
    }

    /*****************************************************************************
        3.设置协议模式
    *****************************************************************************/
    l_ret = wal_cfg80211_set_mode(pst_netdev, &en_curr_protocol_mode);
    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_change_bss::failed to set mode, error[%d].}", l_ret);
        return -OAL_EINVAL;
    }

    /*****************************************************************************
        4.重新启用ap
    *****************************************************************************/
    l_ret = wal_netdev_open_etc(pst_netdev,OAL_FALSE);;
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed to start ap, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
#else
    return OAL_SUCC;
#endif
}


OAL_STATIC oal_void  wal_cfg80211_print_sched_scan_req_info(oal_cfg80211_sched_scan_request_stru  *pst_request)
{
    oal_int8      ac_tmp_buff[200];
    oal_int32     l_loop = 0;

    /* 打印基本参数 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
    OAM_WARNING_LOG4(0, OAM_SF_SCAN,"wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]ms,flags[%d],rssi_thold[%d]",
                pst_request->n_channels,
                pst_request->scan_plans[0].interval,
                pst_request->flags,
                pst_request->min_rssi_thold);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    OAM_WARNING_LOG4(0, OAM_SF_SCAN,"wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]ms,flags[%d],rssi_thold[%d]",
                pst_request->n_channels,
                pst_request->interval,
                pst_request->flags,
                pst_request->min_rssi_thold);
#else
    OAM_WARNING_LOG4(0, OAM_SF_SCAN,"wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]ms,flags[%d],rssi_thold[%d]",
                pst_request->n_channels,
                pst_request->interval,
                pst_request->flags,
                pst_request->rssi_thold);
#endif


    /* 打印ssid集合的信息 */
    for (l_loop = 0; l_loop < pst_request->n_match_sets; l_loop++)
    {
        OAL_MEMZERO(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff));
        OAL_SPRINTF(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), "mactch_sets[%d] info, ssid_len[%d], ssid: %.32s.\n",
                   l_loop, pst_request->match_sets[l_loop].ssid.ssid_len, pst_request->match_sets[l_loop].ssid.ssid);
        oam_print_etc(ac_tmp_buff);
    }

    for (l_loop = 0; l_loop < pst_request->n_ssids; l_loop++)
    {
        OAL_MEMZERO(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff));
        OAL_SPRINTF(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), "ssids[%d] info, ssid_len[%d], ssid: %.32s.\n",
                   l_loop, pst_request->ssids[l_loop].ssid_len, pst_request->ssids[l_loop].ssid);
        oam_print_etc(ac_tmp_buff);
    }

    return;
}



OAL_STATIC oal_bool_enum_uint8 wal_pno_scan_with_assigned_ssid(oal_cfg80211_ssid_stru *pst_ssid,
                                                                        oal_cfg80211_ssid_stru *pst_ssid_list,
                                                                        oal_int32 l_count)
{
    oal_int32 l_loop;

    if ((OAL_PTR_NULL == pst_ssid) || (OAL_PTR_NULL == pst_ssid_list))
    {
        return OAL_FALSE;
    }

    for (l_loop = 0; l_loop < l_count; l_loop++)
    {
        if ((pst_ssid->ssid_len == pst_ssid_list[l_loop].ssid_len)
            && (0 == oal_memcmp(pst_ssid->ssid, pst_ssid_list[l_loop].ssid, pst_ssid->ssid_len)))
        {
           return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}


OAL_STATIC oal_int32 wal_cfg80211_sched_scan_start(oal_wiphy_stru                        *pst_wiphy,
                                                           oal_net_device_stru                   *pst_netdev,
                                                           oal_cfg80211_sched_scan_request_stru  *pst_request)
{
    hmac_device_stru               *pst_hmac_device;
    hmac_scan_stru                 *pst_scan_mgmt;
    mac_vap_stru                   *pst_mac_vap;
    oal_cfg80211_ssid_stru         *pst_ssid_tmp;
    oal_cfg80211_ssid_stru         *pst_scan_ssid_list;
    mac_pno_scan_stru               st_pno_scan_info;
    oal_int32                       l_loop = 0;
    oal_uint32                      ul_ret;

    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_netdev) || (OAL_PTR_NULL == pst_request))
    {
        OAM_ERROR_LOG3(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start::input param pointer is null, pst_wiphy[%p], pst_netdev[%p], pst_request[%p]!}",
                       pst_wiphy, pst_netdev, pst_request);
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_sched_scan_start:: dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    pst_mac_vap    = OAL_NET_DEV_PRIV(pst_netdev);
    if(NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start:: pst_mac_vap is null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_hmac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start:: pst_mac_device is null!}");
        return -OAL_EINVAL;
    }

    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);

    /* 如果当前设备处于扫描状态，abort当前扫描 */
    if (OAL_PTR_NULL != pst_scan_mgmt->pst_request)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start:: device is busy, stop current scan!}");

        wal_force_scan_complete_etc(pst_netdev, OAL_TRUE);
    }

    /* 检测内核下发的需要匹配的ssid集合的个数是否合法 */
    if (pst_request->n_match_sets <= 0)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{wal_cfg80211_sched_scan_start::match_sets = %d!}", pst_request->n_match_sets);
        return -OAL_EINVAL;
    }

    /* 初始化pno扫描的结构体信息 */
    OAL_MEMZERO(&st_pno_scan_info, OAL_SIZEOF(st_pno_scan_info));

    /* 将内核下发的匹配的ssid集合复制到本地 */
    pst_scan_ssid_list = OAL_PTR_NULL;
    if (pst_request->n_ssids > 0)
    {
        pst_scan_ssid_list = pst_request->ssids;
    }
    for (l_loop = 0; l_loop < OAL_MIN(pst_request->n_match_sets, MAX_PNO_SSID_COUNT); l_loop++)
    {
        pst_ssid_tmp = &(pst_request->match_sets[l_loop].ssid);
        if (pst_ssid_tmp->ssid_len >= WLAN_SSID_MAX_LEN)
        {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_cfg80211_sched_scan_start:: wrong ssid_len[%d]!}", pst_ssid_tmp->ssid_len);
            continue;
        }
        oal_memcopy(st_pno_scan_info.ast_match_ssid_set[l_loop].auc_ssid, pst_ssid_tmp->ssid, pst_ssid_tmp->ssid_len);
        st_pno_scan_info.ast_match_ssid_set[l_loop].auc_ssid[pst_ssid_tmp->ssid_len] = '\0';
        st_pno_scan_info.ast_match_ssid_set[l_loop].en_scan_ssid = wal_pno_scan_with_assigned_ssid(pst_ssid_tmp, pst_scan_ssid_list, pst_request->n_ssids);
        st_pno_scan_info.l_ssid_count++;
    }

    /* 其它参数赋值 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    st_pno_scan_info.l_rssi_thold         = pst_request->min_rssi_thold;
#else
    st_pno_scan_info.l_rssi_thold         = pst_request->rssi_thold;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
    //st_pno_scan_info.ul_pno_scan_interval = pst_request->interval;        /* 上层下发的默认值为10s */
    st_pno_scan_info.ul_pno_scan_interval = PNO_SCHED_SCAN_INTERVAL;        /* 驱动自己定义为30s */
    st_pno_scan_info.uc_pno_scan_repeat   = MAX_PNO_REPEAT_TIMES;

    /* 保存当前的PNO调度扫描请求指针 */
    pst_scan_mgmt->pst_sched_scan_req     = pst_request;
    pst_scan_mgmt->en_sched_scan_complete = OAL_FALSE;

    /* 维测打印上层下发的调度扫描请求参数信息 */
    wal_cfg80211_print_sched_scan_req_info(pst_request);

    /* 下发pno扫描请求到hmac */
    ul_ret = wal_cfg80211_start_sched_scan_etc(pst_netdev, &st_pno_scan_info);
    if (ul_ret != OAL_SUCC)
    {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_cfg80211_sched_scan_start::wal_cfg80211_start_sched_scan_etc fail[%d]!}",ul_ret);
        return -OAL_EBUSY;
    }

    return OAL_SUCC;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
OAL_STATIC oal_int32 wal_cfg80211_sched_scan_stop(oal_wiphy_stru          *pst_wiphy,
                                                  oal_net_device_stru     *pst_netdev,
                                                  oal_uint64               ul_reqid)

#else
OAL_STATIC oal_int32 wal_cfg80211_sched_scan_stop(oal_wiphy_stru          *pst_wiphy,
                                                  oal_net_device_stru     *pst_netdev)
#endif
{
    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_netdev))
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_stop::input param pointer is null, pst_wiphy[%p], pst_netdev[%p]!}",
                       pst_wiphy, pst_netdev);
        return -OAL_EINVAL;
    }

    return wal_stop_sched_scan_etc(pst_netdev);
}

#endif


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0))
oal_int32 wal_cfg80211_change_virtual_intf_etc(oal_wiphy_stru        *pst_wiphy,
                                           oal_net_device_stru   *pst_net_dev,
                                           enum nl80211_iftype    en_type,
                                           oal_vif_params_stru   *pst_params)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
oal_int32 wal_cfg80211_change_virtual_intf_etc(oal_wiphy_stru        *pst_wiphy,
                                           oal_net_device_stru   *pst_net_dev,
                                           enum nl80211_iftype    en_type,
                                           oal_uint32            *pul_flags,
                                           oal_vif_params_stru   *pst_params)
#else
oal_int32 wal_cfg80211_change_virtual_intf_etc(oal_wiphy_stru        *pst_wiphy,
                                           oal_int32              l_ifindex,
                                           enum nl80211_iftype    en_type,
                                           oal_uint32            *pul_flags,
                                           oal_vif_params_stru   *pst_params)
#endif
{
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8    en_p2p_mode;
    wlan_vap_mode_enum_uint8    en_vap_mode;
    mac_cfg_del_vap_param_stru  st_del_vap_param;
    mac_cfg_add_vap_param_stru  st_add_vap_param;
    mac_vap_stru               *pst_mac_vap;
    mac_device_stru            *pst_mac_device;
    oal_int8                    ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    oal_int32                   l_ret;
#endif

    /* 1.1 入参检查 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    if (OAL_PTR_NULL == pst_net_dev)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_etc::pst_dev is null!}\r\n");
        return -OAL_EINVAL;
    }
#else
    oal_net_device_stru   *pst_net_dev;
    pst_net_dev = oal_dev_get_by_index(l_ifindex);
    if (OAL_PTR_NULL == pst_net_dev)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_etc::pst_dev is null!}\r\n");
        return -OAL_EINVAL;
    }
    oal_dev_put(pst_net_dev);/* 调用oal_dev_get_by_index后，必须调用oal_dev_put使net_dev的引用计数减一 */
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_change_virtual_intf_etc::dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return OAL_SUCC;
    }

#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    if ((OAL_PTR_NULL == pst_wiphy)
        || (OAL_PTR_NULL == pst_params))
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_etc::pst_wiphy or pul_flag or pst_params ptr is null,error %d, %d, %d!}\r\n",
                        pst_wiphy, pst_params);
        return -OAL_EINVAL;
    }

    /* 检查VAP 当前模式和目的模式是否相同，如果相同则直接返回 */
    if (pst_net_dev->ieee80211_ptr->iftype == en_type)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_etc::same iftype[%d],do not need change !}\r\n", en_type);
        return OAL_SUCC;
    }

    OAM_WARNING_LOG2(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_etc::[%d][%d]}\r\n",
                    (pst_net_dev->ieee80211_ptr->iftype), en_type);

    OAL_IO_PRINT("wal_cfg80211_change_virtual_intf_etc,dev_name is:%s\n", pst_net_dev->name);

    switch (en_type) {
        case NL80211_IFTYPE_MONITOR:
        case NL80211_IFTYPE_WDS:
        case NL80211_IFTYPE_MESH_POINT:
        case NL80211_IFTYPE_ADHOC:
            OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_etc::currently we do not support this type[%d]}\r\n", en_type);
            return -OAL_EINVAL;

        case NL80211_IFTYPE_STATION:
#if 0/*暂时无用的赋值*/
            en_vap_mode = WLAN_VAP_MODE_BSS_STA;
            en_p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
            if(NL80211_IFTYPE_AP == pst_net_dev->ieee80211_ptr->iftype)
            {
                /* 下电APUT，切换netdev状态为station */
                l_ret = wal_netdev_stop_ap_etc(pst_net_dev);
                return l_ret;
            }
#endif
            //if(pst_net_dev->ieee80211_ptr->iftype != NL80211_IFTYPE_P2P_CLIENT)
            {
                pst_net_dev->ieee80211_ptr->iftype = en_type;//TBD:P2P BUG P2P_DEVICE 提前创建，不需要通过wpa_supplicant 创建
                OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_etc::change to station}\r\n");
            }
            return OAL_SUCC;
            //break;
        case NL80211_IFTYPE_P2P_CLIENT:
            en_vap_mode = WLAN_VAP_MODE_BSS_STA;
            en_p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_AP:
        case NL80211_IFTYPE_AP_VLAN:
            en_vap_mode = WLAN_VAP_MODE_BSS_AP;
            en_p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            en_vap_mode = WLAN_VAP_MODE_BSS_AP;
            en_p2p_mode = WLAN_P2P_GO_MODE;
            break;
        default:
            OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_etc::currently we do not support this type[%d]}\r\n", en_type);
            return -OAL_EINVAL;
    }

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    if(NL80211_IFTYPE_AP == en_type)
    {
        l_ret = wal_setup_ap_etc(pst_net_dev);
        return l_ret;
    }
#endif

    /* 设备为P2P 设备才需要进行change virtual interface */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_etc::can't get mac vap from netdevice priv data.}\r\n");
        return -OAL_EINVAL;
    }

    if (IS_LEGACY_VAP(pst_mac_vap))
    {
        pst_net_dev->ieee80211_ptr->iftype = en_type;
        return OAL_SUCC;
    }

    pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_etc::mac_res_get_dev_etc fail device id[%d].}",pst_mac_vap->uc_device_id);
        return -OAL_EINVAL;
    }

    /* 通过device id获取netdev名字 */
    OAL_SPRINTF(ac_p2p_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "p2p%d", pst_mac_device->uc_device_id);

    if (0 == (oal_strcmp(ac_p2p_netdev_name, pst_net_dev->name)))
    {
        /* 解决异常情况下,wpa_supplicant下发p2p0设备切换到p2p go/cli模式导致fastboot的问题 */
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_etc::p2p0 netdevice can not change to P2P CLI/GO.}\r\n");
        return -OAL_EINVAL;
    }

    /* 如果当前模式和目的模式不同，则需要:
       1. 停止 VAP
       2. 删除 VAP
       3. 重新创建对应模式VAP
       4. 启动VAP
    */
    /* 停止VAP */
    wal_netdev_stop_etc(pst_net_dev);

    OAL_MEMZERO(&st_del_vap_param, OAL_SIZEOF(st_del_vap_param));
    /* 删除VAP */
    st_del_vap_param.pst_net_dev = pst_net_dev;
    /* 设备p2p 模式需要从net_device 中获取 */
    st_del_vap_param.en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(pst_net_dev->ieee80211_ptr->iftype);
    if (wal_cfg80211_del_vap_etc(&st_del_vap_param))
    {
        return -OAL_EFAIL;
    }

    OAL_MEMZERO(&st_add_vap_param, OAL_SIZEOF(st_add_vap_param));
    /* 重新创建对应模式VAP */
    st_add_vap_param.pst_net_dev = pst_net_dev;
    st_add_vap_param.en_vap_mode = en_vap_mode;
    st_add_vap_param.en_p2p_mode = en_p2p_mode;
    wal_cfg80211_add_vap_etc(&st_add_vap_param);

    /* 启动VAP */
    wal_netdev_open_etc(pst_net_dev,OAL_FALSE);
#endif /* _PRE_WLAN_FEATURE_P2P */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    pst_net_dev->ieee80211_ptr->iftype = en_type;
//#else
//    l_ifindex = l_ifindex;   /*comment due to codeDEX report*/
#endif
    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_add_station(oal_wiphy_stru            *pst_wiphy,
                                                oal_net_device_stru     *pst_dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
                                                const
#endif
                                                oal_uint8               *puc_mac,
                                                oal_station_parameters_stru *pst_sta_parms)
{
    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_del_station(oal_wiphy_stru        *pst_wiphy,
                                                oal_net_device_stru *pst_dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
                                                struct station_del_parameters *params
#else
                                                oal_uint8           *puc_mac
#endif
)
{
    mac_vap_stru                  *pst_mac_vap        = OAL_PTR_NULL;
    mac_cfg_kick_user_param_stru  st_kick_user_param;
    oal_int32                     int_user_count_ok   = 0;
    oal_int32                     int_user_count_fail = 0;
    oal_int32                     uint_ret            = OAL_FAIL;
    oal_uint8                     auc_mac_boardcast[OAL_MAC_ADDR_LEN];
    oal_uint16                    us_reason_code = MAC_INACTIVITY;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    oal_uint8                    *puc_mac;
    if (params == OAL_PTR_NULL)
    {
        return -OAL_EFAUL;
    }
    puc_mac = (oal_uint8 *)params->mac;

    us_reason_code = params->reason_code;
#endif

    if (OAL_PTR_NULL == pst_wiphy || OAL_PTR_NULL == pst_dev)
    {
        return -OAL_EFAUL;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_station::dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return OAL_SUCC;
    }

#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_station::can't get mac vap from netdevice priv data!}\r\n");
        return -OAL_EFAUL;
    }

    /* 判断是否是AP模式*/
    if (WLAN_VAP_MODE_BSS_AP != pst_mac_vap->en_vap_mode)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_del_station::WLAN_VAP_MODE_BSS_AP != vap_mode[%d]!}\r\n", pst_mac_vap->en_vap_mode);
        return -OAL_EINVAL;
    }

    if (OAL_PTR_NULL == puc_mac)
    {
        oal_memset(auc_mac_boardcast,0xff,OAL_MAC_ADDR_LEN);
        puc_mac = auc_mac_boardcast;
        OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_del_station::deleting all user!}\r\n");
    }
    else
    {
        OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_del_station::deleting user:%02X:XX:XX:%02X:%02X:%02X!}\r\n",
                    puc_mac[0], puc_mac[3], puc_mac[4], puc_mac[5]);
    }

    st_kick_user_param.us_reason_code = us_reason_code;
    oal_memcopy(st_kick_user_param.auc_mac_addr, puc_mac, OAL_MAC_ADDR_LEN);
    uint_ret = wal_cfg80211_start_disconnect_etc(pst_dev, &st_kick_user_param);
    if (OAL_SUCC != uint_ret)
    {
        /* 由于删除的时候可能用户已经删除，此时再进行用户查找，会返回错误，输出ERROR打印，修改为warning */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_del_station::hmac_config_kick_user_etc fail[%d]!}\r\n", uint_ret);
        int_user_count_fail++;
    }
    else
    {
        int_user_count_ok++;
    }

    if (int_user_count_fail > 0)
    {
        OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_del_station::%d user is failed to be deleted!}\r\n", int_user_count_fail);
        return -OAL_EINVAL;
    }

    OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_del_station::%d user is deleted!}\r\n", int_user_count_ok);
    return OAL_SUCC;

}


OAL_STATIC oal_int32 wal_cfg80211_change_station(oal_wiphy_stru                 *pst_wiphy,
                                                    oal_net_device_stru         *pst_dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
                                                    const
#endif
                                                    oal_uint8                   *puc_mac,
                                                    oal_station_parameters_stru *pst_sta_parms)
{
    return OAL_SUCC;
}

#define QUERY_STATION_INFO_TIME  (5 * OAL_TIME_HZ)

OAL_STATIC oal_void wal_cfg80211_fill_station_info(oal_station_info_stru  *pst_sta_info,
                                                       oal_station_info_stru  *pst_stats)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
    /* 适配linux 4.0.修改为BIT(NL80211_STA_INFO_SIGNAL)等. */

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_SIGNAL);

    pst_sta_info->signal = pst_stats->signal;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_RX_PACKETS);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_PACKETS);

    pst_sta_info->rx_packets = pst_stats->rx_packets;
    pst_sta_info->tx_packets = pst_stats->tx_packets;

    pst_sta_info->filled   |= BIT(NL80211_STA_INFO_RX_BYTES);
    pst_sta_info->filled   |= BIT(NL80211_STA_INFO_TX_BYTES);
    pst_sta_info->rx_bytes  = pst_stats->rx_bytes;
    pst_sta_info->tx_bytes  = pst_stats->tx_bytes;


    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_RETRIES);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_FAILED);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_RX_DROP_MISC);

    pst_sta_info->tx_retries       = pst_stats->tx_retries;
    pst_sta_info->tx_failed        = pst_stats->tx_failed;
    pst_sta_info->rx_dropped_misc  = pst_stats->rx_dropped_misc;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_BITRATE);
    /* kernel\net\wireless\util.c cfg80211_calculate_bitrate
        if (!(rate->flags & RATE_INFO_FLAGS_MCS) &&
            !(rate->flags & RATE_INFO_FLAGS_VHT_MCS))
        return rate->legacy;
    */
    if (pst_stats->txrate.flags & RATE_INFO_FLAGS_VHT_MCS)
    {
        pst_sta_info->txrate.legacy = pst_stats->txrate.legacy; /* dmac已将单位修改为100kbps */
        pst_sta_info->txrate.flags  = pst_stats->txrate.flags & 0xFC;   /* 清除标志位RATE_INFO_FLAGS_VHT_MCS RATE_INFO_FLAGS_MCS */
    }
    else
    {
        pst_sta_info->txrate.legacy = (oal_uint16)(pst_stats->txrate.legacy * 10); /* 内核中单位为100kbps */
        pst_sta_info->txrate.flags  = pst_stats->txrate.flags;
    }
    pst_sta_info->txrate.mcs    = pst_stats->txrate.mcs;
    pst_sta_info->txrate.nss    = pst_stats->txrate.nss;
    pst_sta_info->txrate.bw     = pst_stats->txrate.bw;

#else
    pst_sta_info->filled |= STATION_INFO_SIGNAL;

    pst_sta_info->signal = pst_stats->signal;

    pst_sta_info->filled |= STATION_INFO_RX_PACKETS;
    pst_sta_info->filled |= STATION_INFO_TX_PACKETS;

    pst_sta_info->rx_packets = pst_stats->rx_packets;
    pst_sta_info->tx_packets = pst_stats->tx_packets;

    pst_sta_info->filled   |= STATION_INFO_RX_BYTES;
    pst_sta_info->filled   |= STATION_INFO_TX_BYTES;
    pst_sta_info->rx_bytes  = pst_stats->rx_bytes;
    pst_sta_info->tx_bytes  = pst_stats->tx_bytes;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
    pst_sta_info->filled |= STATION_INFO_TX_RETRIES;
    pst_sta_info->filled |= STATION_INFO_TX_FAILED;
    pst_sta_info->filled |= STATION_INFO_RX_DROP_MISC;

    pst_sta_info->tx_retries       = pst_stats->tx_retries;
    pst_sta_info->tx_failed        = pst_stats->tx_failed;
    pst_sta_info->rx_dropped_misc  = pst_stats->rx_dropped_misc;
#endif

    pst_sta_info->filled |= STATION_INFO_TX_BITRATE ;
    pst_sta_info->txrate.legacy = (oal_uint16)(pst_stats->txrate.legacy * 10); /* 内核中单位为100kbps */
    pst_sta_info->txrate.flags  = pst_stats->txrate.flags;
    pst_sta_info->txrate.mcs    = pst_stats->txrate.mcs;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
    pst_sta_info->txrate.nss    = pst_stats->txrate.nss;
#endif

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
}


oal_uint8 wal_cfg80211_get_station_filter_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_mac)
{
    hmac_user_stru  *pst_hmac_user;
    oal_uint32       ul_current_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    oal_uint32       ul_runtime;
    oal_uint32       ul_get_station_threshold;
#ifndef WIN32
#ifdef _PRE_WLAN_FEATURE_VOWIFI
    mac_device_stru *pst_mac_dev;
#endif
#endif
    pst_hmac_user = mac_vap_get_hmac_user_by_addr_etc(pst_mac_vap, puc_mac);
    if(OAL_PTR_NULL == pst_hmac_user)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_get_station_filter_etc::user is null.}");
        return OAL_FALSE;
    }
#ifndef WIN32
#ifdef _PRE_WLAN_FEATURE_VOWIFI
    pst_mac_dev = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if(OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_get_station_filter_etc::dev is null.}");
        return OAL_FALSE;
    }
    
    if (IS_LEGACY_STA(pst_mac_vap) &&
        (OAL_PTR_NULL != pst_mac_vap->pst_vowifi_cfg_param && VOWIFI_DISABLE_REPORT == pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode))
    {/*亮屏且vowifi正在使用时*/
        ul_get_station_threshold = WAL_VOWIFI_GET_STATION_THRESHOLD;
    }
    else
#endif
#endif
    {
        ul_get_station_threshold = WAL_GET_STATION_THRESHOLD;
    }

    ul_runtime = (oal_uint32)OAL_TIME_GET_RUNTIME(pst_hmac_user->ul_rssi_last_timestamp, ul_current_time);

    if(ul_get_station_threshold > ul_runtime)
    {
        return OAL_FALSE;
    }

    pst_hmac_user->ul_rssi_last_timestamp = ul_current_time;
    return OAL_TRUE;
}


OAL_STATIC oal_int32 wal_cfg80211_get_station(oal_wiphy_stru        *pst_wiphy,
                                                   oal_net_device_stru   *pst_dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
                                                   const
#endif
                                                   oal_uint8             *puc_mac,
                                                   oal_station_info_stru *pst_sta_info)
{
    mac_vap_stru                          *pst_mac_vap;
    hmac_vap_stru                         *pst_hmac_vap;
    dmac_query_request_event               st_dmac_query_request_event;
    dmac_query_station_info_request_event *pst_query_station_info;
    wal_msg_write_stru                     st_write_msg;
    oal_int                                i_leftime;
    oal_int32                              l_ret;
    oal_uint8                              uc_vap_id;

    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_dev) ||
        (OAL_PTR_NULL == puc_mac)|| (OAL_PTR_NULL == pst_sta_info))
    {
        OAM_ERROR_LOG4(0, OAM_SF_ANY,
                       "{wal_cfg80211_get_station::pst_wiphy[0x%p],pst_dev[0x%p],puc_mac[0x%p],pst_sta_info[0x%p]!}",
                       pst_wiphy, pst_dev, puc_mac, pst_sta_info);
        return -OAL_EINVAL;
    }

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_get_station::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;

    pst_query_station_info = (dmac_query_station_info_request_event *)&st_dmac_query_request_event;
    pst_query_station_info->query_event = OAL_QUERY_STATION_INFO_EVENT;
    oal_set_mac_addr(pst_query_station_info->auc_query_sta_addr, (oal_uint8 *)puc_mac);

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_get_station::mac_res_get_hmac_vap fail.}");
        return -OAL_EINVAL;
    }

    /* 固定时间内最多更新一次RSSI */
    if(OAL_FALSE == wal_cfg80211_get_station_filter_etc(&pst_hmac_vap->st_vap_base_info, (oal_uint8 *)puc_mac))
    {
        wal_cfg80211_fill_station_info(pst_sta_info, &pst_hmac_vap->station_info);
#ifdef CONFIG_HW_GET_EXT_SIG
        pst_sta_info->filled |= BIT(NL80211_STA_INFO_NOISE);
        pst_sta_info->noise = pst_hmac_vap->station_info.noise;

        pst_sta_info->filled |= BIT(NL80211_STA_INFO_SNR);
        pst_sta_info->snr = OAL_MAX(pst_hmac_vap->st_station_info_extend.c_snr_ant0, pst_hmac_vap->st_station_info_extend.c_snr_ant1);

        pst_sta_info->filled |= BIT(NL80211_STA_INFO_CNAHLOAD);
        pst_sta_info->chload = pst_hmac_vap->station_info.chload;
#endif
        return OAL_SUCC;
    }

    pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;

    /********************************************************************************
        抛事件到wal层处理 ，对于低功耗需要做额外处理，不能像下层抛事件，直接起定时器
        低功耗会在接收beacon帧的时候主动上报信息。
    ********************************************************************************/
    /*3.1 填写 msg 消息头*/
    st_write_msg.en_wid = WLAN_CFGID_QUERY_STATION_STATS;
    st_write_msg.us_len = OAL_SIZEOF(st_dmac_query_request_event);

    /*3.2 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, pst_query_station_info, OAL_SIZEOF(dmac_query_station_info_request_event));

    l_ret = wal_send_cfg_event_etc(pst_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(dmac_query_station_info_request_event),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_get_station::wal_send_cfg_event_etc return err code %d!}", l_ret);
        return -OAL_EFAIL;
    }
    /*lint -e730*//* info, boolean argument to function */
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag),QUERY_STATION_INFO_TIME);
    /*lint +e730*/
    if (0 == i_leftime)
    {
        /* 超时还没有上报扫描结束 */
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_get_station::query info wait for %ld ms timeout!}",
                         ((QUERY_STATION_INFO_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_EINVAL;
    }
    else if (i_leftime < 0)
    {
        /* 定时器内部错误 */
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_get_station::query info wait for %ld ms error!}",
                         ((QUERY_STATION_INFO_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_EINVAL;
    }
    else
    {
        /* 正常结束  */
        wal_cfg80211_fill_station_info(pst_sta_info, &pst_hmac_vap->station_info);

#ifdef CONFIG_HW_GET_EXT_SIG
        pst_sta_info->filled |= BIT(NL80211_STA_INFO_NOISE);
        pst_sta_info->noise = pst_hmac_vap->station_info.noise;

        pst_sta_info->filled |= BIT(NL80211_STA_INFO_SNR);
        pst_sta_info->snr = OAL_MAX(pst_hmac_vap->st_station_info_extend.c_snr_ant0, pst_hmac_vap->st_station_info_extend.c_snr_ant1);

        pst_sta_info->filled |= BIT(NL80211_STA_INFO_CNAHLOAD);
        pst_sta_info->chload = pst_hmac_vap->station_info.chload;
#endif
        return OAL_SUCC;
    }
}


OAL_STATIC oal_int32 wal_cfg80211_dump_station(oal_wiphy_stru           *pst_wiphy,
                                                oal_net_device_stru     *pst_dev,
                                                oal_int32                int_index,
                                                oal_uint8               *puc_mac,
                                                oal_station_info_stru   *pst_sta_info)
{
    return OAL_SUCC;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44)) //TBD:1102 需要，暂时用内核版本号区分
#if (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5) && (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_CPE)

#ifdef _PRE_WLAN_FEATURE_P2P

OAL_STATIC oal_uint32 wal_is_p2p_group_exist(mac_device_stru *pst_mac_device)
{
    if (OAL_SUCC != hmac_check_p2p_vap_num_etc(pst_mac_device, WLAN_P2P_GO_MODE)
        || OAL_SUCC != hmac_check_p2p_vap_num_etc(pst_mac_device, WLAN_P2P_CL_MODE))
    {
        return OAL_TRUE;
    }
    else
    {
        return OAL_FALSE;
    }
}


oal_uint32 wal_del_p2p_group_etc(mac_device_stru *pst_mac_device)
{
    oal_uint8                    uc_vap_idx;
    mac_vap_stru                *pst_mac_vap;
    hmac_vap_stru               *pst_hmac_vap;
    oal_net_device_stru         *pst_net_dev;
    mac_cfg_del_vap_param_stru   st_del_vap_param;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
        {
            OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_del_p2p_group_etc::get mac vap resource fail! vap id is %d}",
                            pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hmac_vap))
        {
            OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_del_p2p_group_etc::get hmac vap resource fail! vap id is %d}",
                            pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        pst_net_dev = pst_hmac_vap->pst_net_device;
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
        {
            OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_del_p2p_group_etc::get net device fail! vap id is %d}",
                            pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (IS_P2P_GO(pst_mac_vap) || IS_P2P_CL(pst_mac_vap))
        {
            OAL_MEMZERO(&st_del_vap_param, OAL_SIZEOF(st_del_vap_param));
            OAL_IO_PRINT("wal_del_p2p_group_etc:: ifname %s\r\n", pst_net_dev->name);
            st_del_vap_param.pst_net_dev = pst_net_dev;
            st_del_vap_param.en_vap_mode = pst_mac_vap->en_vap_mode;
            st_del_vap_param.en_p2p_mode = mac_get_p2p_mode_etc(pst_mac_vap);
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_del_p2p_group_etc:: vap mode[%d], p2p mode[%d]}\r\n",
                                st_del_vap_param.en_vap_mode, st_del_vap_param.en_p2p_mode);
            /* 删除已经存在的P2P group */
            wal_force_scan_complete_etc(pst_net_dev, OAL_TRUE);
            wal_stop_vap_etc(pst_net_dev);
            if(OAL_SUCC == wal_cfg80211_del_vap_etc(&st_del_vap_param))
            {
                wal_cfg80211_unregister_netdev_etc(pst_net_dev);
            }
        }
    }
    return OAL_SUCC;
}
#endif



oal_uint32 wal_cfg80211_register_netdev_etc(oal_net_device_stru *pst_net_dev)
{
    oal_uint8    uc_rollback_lock = OAL_FALSE;
    oal_uint32   ul_ret = 0;
    oal_netdev_priv_stru        *pst_netdev_priv;

    if (rtnl_is_locked())
    {
        rtnl_unlock();
        uc_rollback_lock = OAL_TRUE;
    }

    pst_netdev_priv = (oal_netdev_priv_stru *)OAL_NET_DEV_WIRELESS_PRIV(pst_net_dev);
    pst_netdev_priv->uc_napi_enable     = OAL_TRUE;
    pst_netdev_priv->uc_gro_enable      = OAL_TRUE;
    pst_netdev_priv->uc_napi_weight     = NAPI_POLL_WEIGHT_LEV1;
    pst_netdev_priv->ul_queue_len_max   = NAPI_NETDEV_PRIV_QUEUE_LEN_MAX;
    pst_netdev_priv->uc_state           = 0;
    pst_netdev_priv->ul_period_pkts     = 0;
    pst_netdev_priv->ul_period_start    = 0;

    oal_netbuf_list_head_init(&pst_netdev_priv->st_rx_netbuf_queue);
    oal_netif_napi_add(pst_net_dev, &pst_netdev_priv->st_napi, hmac_rxdata_polling, NAPI_POLL_WEIGHT_LEV1);

    /* 内核注册net_device, 只返回0 */
    ul_ret = (oal_uint32)oal_net_register_netdev(pst_net_dev);

    if (uc_rollback_lock)
    {
        rtnl_lock();
    }

    return ul_ret;
}


oal_void wal_cfg80211_unregister_netdev_etc(oal_net_device_stru *pst_net_dev)
{
    oal_uint8    uc_rollback_lock = OAL_FALSE;

    if (rtnl_is_locked())
    {
        rtnl_unlock();
        uc_rollback_lock = OAL_TRUE;
    }

    /* 去注册netdev */
    oal_net_unregister_netdev(pst_net_dev);

    if (uc_rollback_lock)
    {
        rtnl_lock();
    }
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
OAL_STATIC oal_wireless_dev_stru * wal_cfg80211_add_virtual_intf(oal_wiphy_stru     *pst_wiphy,
                                                                const char          *puc_name,
                                                                unsigned char name_assign_type,
                                                                enum nl80211_iftype  en_type,
                                                                oal_vif_params_stru *pst_params)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
OAL_STATIC oal_wireless_dev_stru * wal_cfg80211_add_virtual_intf(oal_wiphy_stru     *pst_wiphy,
                                                                const char          *puc_name,
                                                                unsigned char name_assign_type,
                                                                enum nl80211_iftype  en_type,
                                                                oal_uint32          *pul_flags,
                                                                oal_vif_params_stru *pst_params)
#else
OAL_STATIC oal_wireless_dev_stru * wal_cfg80211_add_virtual_intf(oal_wiphy_stru     *pst_wiphy,
                                                                const char          *puc_name,
                                                                enum nl80211_iftype  en_type,
                                                                oal_uint32          *pul_flags,
                                                                oal_vif_params_stru *pst_params)
#endif
{
    oal_wireless_dev_stru      *pst_wdev    = OAL_PTR_NULL;
    wlan_p2p_mode_enum_uint8    en_p2p_mode;
    wlan_vap_mode_enum_uint8    en_vap_mode;
    oal_net_device_stru        *pst_net_dev;
    mac_wiphy_priv_stru        *pst_wiphy_priv;
    mac_device_stru            *pst_mac_device;
    hmac_device_stru           *pst_hmac_device;
#if (!defined(_PRE_PRODUCT_ID_HI110X_HOST))
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_vap_stru              *pst_p2p0_hmac_vap;
    oal_uint8                   uc_p2p0_vap_idx;
#endif
#endif
    wal_msg_write_stru          st_write_msg;
    wal_msg_stru               *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint8                   uc_cfg_vap_id;
    mac_vap_stru               *pst_cfg_mac_vap;
    hmac_vap_stru              *pst_cfg_hmac_vap;
    mac_vap_stru               *pst_mac_vap;
    oal_net_device_stru        *pst_cfg_net_dev;
    oal_uint32                  ul_ret;
    oal_int32                   l_ret;
    mac_cfg_add_vap_param_stru *pst_add_vap_param;
    oal_int8                   ac_name[OAL_IF_NAME_SIZE] = {0};
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_uint8                   uc_rollback_lock=0;
    oal_int32                   l_timeout;
#endif

    /* 1.1 入参检查 */
    if ((OAL_PTR_NULL == pst_wiphy)
        || (OAL_PTR_NULL == puc_name)
        || (OAL_PTR_NULL == pst_params))
    {
        OAM_ERROR_LOG3(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf:: ptr is null,error pst_wiphy %d, puc_name %d, pst_params %d!}\r\n",
                        pst_wiphy, puc_name, pst_params);
        return ERR_PTR(-EINVAL);
    }

    /* 入参检查无异常后赋值，并调用OAL统一接口 */
    pst_wiphy_priv = oal_wiphy_priv(pst_wiphy);
    if (pst_wiphy_priv == OAL_PTR_NULL)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::pst_wiphy_priv is null!}\r\n");
        return ERR_PTR(-EINVAL);
    }
    pst_mac_device = pst_wiphy_priv->pst_mac_device;
    if (pst_mac_device == OAL_PTR_NULL)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::pst_mac_device is null!}\r\n");
        return ERR_PTR(-EINVAL);
    }

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::en_type[%d]!}\r\n", en_type);
    /* 提示:SDT无法打印%s字符串形式 */
    OAL_IO_PRINT("wal_cfg80211_add_virtual_intf,dev_name is:%s\n", puc_name);

    switch (en_type)
    {
        case NL80211_IFTYPE_ADHOC:
        case NL80211_IFTYPE_AP_VLAN:
        case NL80211_IFTYPE_WDS:
        case NL80211_IFTYPE_MESH_POINT:
        case NL80211_IFTYPE_MONITOR:
            OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::Unsupported interface type[%d]!}\r\n", en_type);
            return ERR_PTR(-EINVAL);
#ifdef _PRE_WLAN_FEATURE_P2P
        case NL80211_IFTYPE_P2P_DEVICE:
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
            pst_net_dev       = pst_mac_device->st_p2p_info.pst_p2p_net_device;
            pst_wdev          = pst_net_dev->ieee80211_ptr;
#else
            /* p2p0 在加载wifi 驱动后创建，不需要通过该接口创建，直接返回已创建的p2p0 wireless device 即可 */
            uc_p2p0_vap_idx   = pst_mac_device->st_p2p_info.uc_p2p0_vap_idx;
            pst_p2p0_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_p2p0_vap_idx);
            if (OAL_UNLIKELY(OAL_PTR_NULL == pst_p2p0_hmac_vap))
            {
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::pst_p2p0_hmac_vap[id=%d] null!!}",
                            uc_p2p0_vap_idx);
                return ERR_PTR(-ENODEV);
            }
            pst_net_dev       = pst_p2p0_hmac_vap->pst_p2p0_net_device;
            pst_wdev          = pst_net_dev->ieee80211_ptr;
#endif
            return pst_wdev;
#endif
        case NL80211_IFTYPE_P2P_CLIENT:
            en_vap_mode = WLAN_VAP_MODE_BSS_STA;
            en_p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_STATION:
            en_vap_mode = WLAN_VAP_MODE_BSS_STA;
            en_p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            en_vap_mode = WLAN_VAP_MODE_BSS_AP;
            en_p2p_mode = WLAN_P2P_GO_MODE;
            break;
        case NL80211_IFTYPE_AP:
            en_vap_mode = WLAN_VAP_MODE_BSS_AP;
            en_p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        default:
            OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::Unsupported interface type[%d]!}\r\n", en_type);
            return ERR_PTR(-EINVAL);
    }
    /* 如果创建的net device已经存在，直接返回 */
    /* 根据dev_name找到dev */
    pst_net_dev = oal_dev_get_by_name(puc_name);
    if (OAL_PTR_NULL != pst_net_dev)
    {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(pst_net_dev);

        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::the net_device is already exist!}\r\n");
        pst_wdev = pst_net_dev->ieee80211_ptr;
        return pst_wdev;
    }

    /* 添加net_device 前先判断当前是否正在删除net_device 状态，
        如果正在删除net_device，则等待删除完成，再添加 */
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_device->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::hmac_device is null!}\r\n", pst_mac_device->uc_device_id);
        return ERR_PTR(-ENODEV);
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if (hmac_get_p2p_status_etc(pst_hmac_device->ul_p2p_intf_status, P2P_STATUS_IF_DELETING) == OAL_TRUE)
    {
        /* 等待删除完成 */
        if (rtnl_is_locked()) {
            rtnl_unlock();
            uc_rollback_lock = OAL_TRUE;
        }
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf:Released the lock and wait till IF_DEL is complete!}\r\n");
        l_timeout = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_device->st_netif_change_event,
                        (hmac_get_p2p_status_etc(pst_hmac_device->ul_p2p_intf_status, P2P_STATUS_IF_DELETING) == OAL_FALSE),
                        OAL_MSECS_TO_JIFFIES(WAL_MAX_WAIT_TIME));

        /* put back the rtnl_lock again */
        if (uc_rollback_lock) {
            rtnl_lock();
        }

        if (l_timeout > 0)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY,"{wal_cfg80211_add_virtual_intf::IF DEL is Success!}\r\n");
        }
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY,"{wal_cfg80211_add_virtual_intf::timeount < 0, return -EAGAIN!}\r\n");
            return ERR_PTR(-EAGAIN);
        }
    }

    /* 检查wifi 驱动中，P2P group 是否已经创建，如果P2P group 已经创建，
        则将该P2P group 删除，并且重新创建P2P group */
    if (OAL_TRUE == wal_is_p2p_group_exist(pst_mac_device))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::found exist p2p group, delet it first!}\r\n");
        if (OAL_SUCC != wal_del_p2p_group_etc(pst_mac_device))
        {
            return ERR_PTR(-EAGAIN);
        }
    }
#endif

    /* 获取配置VAP 结构 */
    uc_cfg_vap_id   = pst_mac_device->uc_cfg_vap_id;
    pst_cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(uc_cfg_vap_id);
    if (OAL_PTR_NULL == pst_cfg_mac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::pst_cfg_mac_vap is null vap_id:%d!}\r\n",uc_cfg_vap_id);
        return ERR_PTR(-ENODEV);
    }
    pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_cfg_vap_id);
    if (OAL_PTR_NULL == pst_cfg_hmac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::pst_cfg_hmac_vap is null vap_id:%d!}\r\n",uc_cfg_vap_id);
        return ERR_PTR(-ENODEV);
    }
    pst_cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
    oal_memcopy(ac_name, puc_name, OAL_STRLEN(puc_name));
#if defined(_PRE_WLAN_FEATURE_FLOWCTL)
    pst_net_dev = oal_net_alloc_netdev_mqs(OAL_SIZEOF(oal_netdev_priv_stru), ac_name, oal_ether_setup, WAL_NETDEV_SUBQUEUE_MAX_NUM, 1);    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
#elif defined(_PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL)
    pst_net_dev = oal_net_alloc_netdev_mqs(OAL_SIZEOF(oal_netdev_priv_stru), ac_name, oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1);    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
#else
    pst_net_dev = oal_net_alloc_netdev(OAL_SIZEOF(oal_netdev_priv_stru), ac_name, oal_ether_setup);    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_dev))
    {
        OAM_ERROR_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::pst_net_dev null ptr error!}\r\n");
        return ERR_PTR(-ENOMEM);
    }

    pst_wdev = (oal_wireless_dev_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,OAL_SIZEOF(oal_wireless_dev_stru), OAL_FALSE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_wdev))
    {
        OAM_ERROR_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::alloc mem, pst_wdev is null ptr!}\r\n");
        /* 异常处理，释放内存 */
        oal_net_free_netdev(pst_net_dev);
        return ERR_PTR(-ENOMEM);
    }

    oal_memset(pst_wdev, 0, OAL_SIZEOF(oal_wireless_dev_stru));

    /* 对netdevice进行赋值 */
    /* 对新创建的net_device 初始化对应参数 */
#ifdef CONFIG_WIRELESS_EXT
    pst_net_dev->wireless_handlers             = &g_st_iw_handler_def_etc;
#endif /* CONFIG_WIRELESS_EXT */
    /* OAL_NETDEVICE_OPS(pst_net_dev)             = &g_st_wal_net_dev_ops_etc; */
    pst_net_dev->netdev_ops                    = &g_st_wal_net_dev_ops_etc;

    pst_net_dev->ethtool_ops                   = &g_st_wal_ethtool_ops_etc;

    OAL_NETDEVICE_DESTRUCTOR(pst_net_dev)      = oal_net_free_netdev;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,44))
    OAL_NETDEVICE_MASTER(pst_net_dev)          = OAL_PTR_NULL;
#endif

    OAL_NETDEVICE_IFALIAS(pst_net_dev)         = OAL_PTR_NULL;
    OAL_NETDEVICE_WATCHDOG_TIMEO(pst_net_dev)  = 5;
    OAL_NETDEVICE_WDEV(pst_net_dev)            = pst_wdev;
    OAL_NETDEVICE_QDISC(pst_net_dev, OAL_PTR_NULL);
#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
//    OAL_NETDEVICE_TX_QUEUE_LEN(pst_net_dev) = 0;
#endif

    pst_wdev->iftype = en_type;
    pst_wdev->wiphy  = pst_mac_device->pst_wiphy;
    pst_wdev->netdev = pst_net_dev;/* 给wdev 中的net_device 赋值 */
#ifdef _PRE_WLAN_FEATURE_P2P
    if ((WLAN_LEGACY_VAP_MODE == en_p2p_mode) && (WLAN_VAP_MODE_BSS_STA == en_vap_mode))
    {
        /* 如果创建wlan0， 则保存wlan0 为主net_device,p2p0 和p2p-p2p0 MAC 地址从主netdevice 获取 */
        if (pst_mac_device->st_p2p_info.pst_primary_net_device == OAL_PTR_NULL)
        {
            /* 创建wlan0 在加载wifi 驱动时，不需要通过该接口 */
            OAM_ERROR_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::should not go here!}\r\n");
            OAL_MEM_FREE(pst_wdev, OAL_FALSE);
            oal_net_free_netdev(pst_net_dev);
            return ERR_PTR(-ENODEV);
        }
    }
#endif

    OAL_NETDEVICE_FLAGS(pst_net_dev) &= ~OAL_IFF_RUNNING;   /* 将net device的flag设为down */

    ul_ret = wal_cfg80211_register_netdev_etc(pst_net_dev);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        /* 注册不成功，释放资源 */
        OAL_MEM_FREE(pst_wdev, OAL_FALSE);
        oal_net_free_netdev(pst_net_dev);
        return ERR_PTR(-EBUSY);
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 填写消息 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ADD_VAP, OAL_SIZEOF(mac_cfg_add_vap_param_stru));

    pst_add_vap_param = (mac_cfg_add_vap_param_stru *)(st_write_msg.auc_value);
    pst_add_vap_param->pst_net_dev     = pst_net_dev;
    pst_add_vap_param->en_vap_mode     = en_vap_mode;
    pst_add_vap_param->uc_cfg_vap_indx = uc_cfg_vap_id;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_add_vap_param->en_p2p_mode     = en_p2p_mode;
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    pst_add_vap_param->bit_11ac2g_enable = (oal_uint8)!!hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_11AC2G_ENABLE);
    pst_add_vap_param->bit_disable_capab_2ght40 = g_st_wlan_customize_etc.uc_disable_capab_2ght40;
#endif
    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_add_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::wal_send_cfg_event_etc return err code: [%d]!}", l_ret);
        /*lint -e801*/
        goto ERR_STEP;
        /*lint +e801*/
    }

    /* 读取返回的错误码 */
    if(OAL_SUCC != wal_check_and_release_msg_resp_etc(pst_rsp_msg))
    {
        OAM_WARNING_LOG0(pst_cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::wal_check_and_release_msg_resp_etc fail:ul_err_code!}");
        /*lint -e801*/
        goto ERR_STEP;
        /*lint +e801*/
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    wal_set_random_mac_to_mib_etc(pst_net_dev); /* set random mac to mib ; for hi1102-cb */
#endif

    /* 设置netdevice的MAC地址，MAC地址在HMAC层被初始化到MIB中 */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_add_virtual_intf::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}");
        /*lint -e801*/
        goto ERR_STEP;
         /*lint +e801*/
    }
    oal_set_mac_addr((oal_uint8 *)OAL_NETDEVICE_MAC_ADDR(pst_net_dev), mac_mib_get_StationID(pst_mac_vap));

    /* 设置VAP UP */
    wal_netdev_open_etc(pst_net_dev,OAL_FALSE);

    OAM_WARNING_LOG2(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::succ. en_type[%d],vap_id[%d]!}\r\n",
                    en_type, pst_mac_vap->uc_vap_id);

    return pst_wdev;


/* 异常处理 */
ERR_STEP:
    wal_cfg80211_unregister_netdev_etc(pst_net_dev);
    /* 先去注册，后释放 */
    OAL_MEM_FREE(pst_wdev, OAL_FALSE);
    return ERR_PTR(-EAGAIN);
}

OAL_STATIC oal_bool_enum_uint8 wal_cfg80211_check_is_primary_netdev(oal_wiphy_stru *wiphy,
    oal_net_device_stru *net_dev)
{
    mac_device_stru *mac_device;
    mac_wiphy_priv_stru *wiphy_priv;
    wiphy_priv = oal_wiphy_priv(wiphy);
    if (wiphy_priv == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_check_is_primary_netdev::pst_wiphy_priv is null!}");
        return OAL_FALSE;
    }
    mac_device = wiphy_priv->pst_mac_device;
    if (mac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_check_is_primary_netdev::pst_mac_device is null!}");
        return OAL_FALSE;
    }
    return mac_device->st_p2p_info.pst_primary_net_device == net_dev;
}


OAL_STATIC oal_int32 wal_cfg80211_del_virtual_intf(oal_wiphy_stru           *pst_wiphy,
                                                   oal_wireless_dev_stru    *pst_wdev)
{
    /* 异步去注册net_device */
    wal_msg_write_stru           st_write_msg;
    wal_msg_stru               *pst_rsp_msg = {0};
    oal_int32                    l_ret;
    oal_net_device_stru         *pst_net_dev;
    mac_vap_stru                *pst_mac_vap;
    hmac_vap_stru               *pst_hmac_vap;
    hmac_device_stru            *pst_hmac_device;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_wdev)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::pst_wiphy or pst_wdev null ptr error %d, %d!}\r\n",
                       pst_wiphy, pst_wdev);
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return OAL_SUCC;
    }

#endif //#ifdef _PRE_WLAN_FEATURE_DFR

    pst_net_dev = pst_wdev->netdev;
    if (pst_net_dev == OAL_PTR_NULL)
    {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::pst_net_dev is null by netdev, mode[%d]!}\r\n");
            return -OAL_EINVAL;
    }

    pst_mac_vap  = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::mac_vap is null by netdev, mode[%d]!}\r\n", pst_net_dev->ieee80211_ptr->iftype);
        return -OAL_EINVAL;
    }
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(0,OAM_SF_ANY,"{wal_cfg80211_del_virtual_intf::mac_res_get_hmac_vap fail.vap_id[%u]}",pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }
    if (wal_cfg80211_check_is_primary_netdev(pst_wiphy, pst_net_dev)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::cannot del primary netdev}");
        return -OAL_EINVAL;
    }

    oal_net_tx_stop_all_queues(pst_net_dev);
    wal_netdev_stop_etc(pst_net_dev);
    /* 在释放完net_device 后释放wireless device */
    //OAL_MEM_FREE(OAL_NETDEVICE_WDEV(pst_net_dev), OAL_TRUE);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/

    /* 初始化删除vap 参数 */
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_wdev = pst_net_dev->ieee80211_ptr;
    if (wal_wireless_iftype_to_mac_p2p_mode_etc(pst_wdev->iftype) == WLAN_P2P_BUTT)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::wal_wireless_iftype_to_mac_p2p_mode_etc return BUTT}\r\n");
        return -OAL_EINVAL;
    }

    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = mac_get_p2p_mode_etc(pst_mac_vap);
#endif

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEL_VAP, OAL_SIZEOF(mac_cfg_del_vap_param_stru));

    /* 设置删除net_device 标识 */
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::hmac_device is null!}\r\n", pst_mac_vap->uc_device_id);
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_set_p2p_status_etc(&pst_hmac_device->ul_p2p_intf_status, P2P_STATUS_IF_DELETING);

    /* 启动linux work 删除net_device */
    pst_hmac_vap->pst_del_net_device = pst_net_dev;
    oal_workqueue_schedule(&(pst_hmac_vap->st_del_virtual_inf_worker));
#endif

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_del_vap_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if (OAL_SUCC != wal_check_and_release_msg_resp_etc(pst_rsp_msg))
    {
        OAM_WARNING_LOG0(0,OAM_SF_ANY,"{wal_cfg80211_del_virtual_intf::wal_check_and_release_msg_resp_etc fail}");
    }

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::return err code %d}\r\n", l_ret);
        l_ret = -OAL_EFAIL;
    }

    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::pst_hmac_device->ul_p2p_intf_status %d, del result: %d}\r\n",
                            pst_hmac_device->ul_p2p_intf_status, l_ret);

    return l_ret;
}
#endif  /* _PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5 */

#else  /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44) */

oal_uint32 wal_del_p2p_group_etc(mac_device_stru *pst_mac_device)
{
    return OAL_SUCC;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
#if (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5) && (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_CPE)


OAL_STATIC oal_int32 wal_cfg80211_mgmt_tx_cancel_wait(oal_wiphy_stru        *pst_wiphy,
                                                    oal_wireless_dev_stru   *pst_wdev,
                                                    oal_uint64               ull_cookie)
{
    /* TBD */
    return -OAL_EFAIL;
}
#endif  /* _PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5 */
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))//TBD:确认正确的 Linux 版本号
#if (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5) && (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_CPE)

OAL_STATIC oal_int32 wal_cfg80211_set_pmksa(oal_wiphy_stru           *pst_wiphy,
                                                      oal_net_device_stru      *pst_net_device,
                                                      oal_cfg80211_pmksa_stru  *pmksa)
{
    wal_msg_write_stru                       st_write_msg           = {0};
    mac_cfg_pmksa_param_stru                *pst_cfg_pmksa;
    oal_int32                                l_ret;

    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_net_device) || (OAL_PTR_NULL == pmksa))
    {
        OAM_ERROR_LOG3(0, OAM_SF_CFG,
                       "{wal_cfg80211_set_pmksa::param null! pst_wiphy[%d], pst_net_device[%d], pmksa[%d]!!}\r\n",
                       pst_wiphy, pst_net_device, pmksa);
        return -OAL_EINVAL;
    }

    if ((OAL_PTR_NULL == pmksa->bssid) || (OAL_PTR_NULL == pmksa->pmkid))
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{wal_cfg80211_set_pmksa::param null! bssid[%d] pmkid[%d]}\r\n",
                       pmksa->bssid, pmksa->pmkid);
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_SET_PMKSA, OAL_SIZEOF(mac_cfg_pmksa_param_stru));
    pst_cfg_pmksa = (mac_cfg_pmksa_param_stru *)st_write_msg.auc_value;
    oal_memcopy(pst_cfg_pmksa->auc_bssid, pmksa->bssid, WLAN_MAC_ADDR_LEN);
    oal_memcopy(pst_cfg_pmksa->auc_pmkid, pmksa->pmkid, WLAN_PMKID_LEN);

    l_ret = wal_send_cfg_event_etc(pst_net_device,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_pmksa_param_stru),
                                (oal_uint8 *)&st_write_msg,
                                OAL_FALSE,
                                OAL_PTR_NULL);

    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_set_pmksa::wal_send_cfg_event_etc fail[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_del_pmksa(oal_wiphy_stru           *pst_wiphy,
                                                      oal_net_device_stru      *pst_net_device,
                                                      oal_cfg80211_pmksa_stru  *pmksa)
{
    wal_msg_write_stru                       st_write_msg           = {0};
    mac_cfg_pmksa_param_stru                *pst_cfg_pmksa;
    oal_int32                                l_ret;

    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_net_device) || (OAL_PTR_NULL == pmksa))
    {
        OAM_ERROR_LOG3(0, OAM_SF_CFG,
                       "{wal_cfg80211_del_pmksa::param null! pst_wiphy[%d], pst_net_device[%d], pmksa[%d]!!}\r\n",
                       pst_wiphy, pst_net_device, pmksa);
        return -OAL_EINVAL;
    }

    if ((OAL_PTR_NULL == pmksa->bssid) || (OAL_PTR_NULL == pmksa->pmkid))
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{wal_cfg80211_del_pmksa::param null! bssid[%d] pmkid[%d]}\r\n",
                       pmksa->bssid, pmksa->pmkid);
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_DEL_PMKSA, OAL_SIZEOF(mac_cfg_pmksa_param_stru));
    pst_cfg_pmksa = (mac_cfg_pmksa_param_stru *)st_write_msg.auc_value;
    oal_memcopy(pst_cfg_pmksa->auc_bssid, pmksa->bssid, WLAN_MAC_ADDR_LEN);
    oal_memcopy(pst_cfg_pmksa->auc_pmkid, pmksa->pmkid, WLAN_PMKID_LEN);

    l_ret = wal_send_cfg_event_etc(pst_net_device,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_pmksa_param_stru),
                                (oal_uint8 *)&st_write_msg,
                                OAL_FALSE,
                                OAL_PTR_NULL);

    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_del_pmksa::wal_send_cfg_event_etc fail[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}



OAL_STATIC oal_int32 wal_cfg80211_flush_pmksa(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_device)
{
    wal_msg_write_stru              st_write_msg = {0};
    oal_int32                       l_ret;
    mac_vap_stru                   *pst_mac_vap;

    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_net_device))
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG,
                       "{wal_cfg80211_flush_pmksa::param null! pst_wiphy[%d], pst_net_device[%d]!!}\r\n",
                       pst_wiphy, pst_net_device);
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的mac_vap_stru 结构，以防wal_alloc_cfg_event返回ERROR */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_device);
    if (NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_flush_pmksa::can't get mac vap from netdevice priv data!}");
        return -OAL_EFAIL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_FLUSH_PMKSA, 0);

    l_ret = wal_send_cfg_event_etc(pst_net_device,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH,
                                (oal_uint8 *)&st_write_msg,
                                OAL_FALSE,
                                OAL_PTR_NULL);

    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_flush_pmksa::wal_send_cfg_event_etc fail[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_drv_remain_on_channel(oal_wiphy_stru           *pst_wiphy,
                                                    oal_wireless_dev_stru    *pst_wdev,
                                                    struct ieee80211_channel *pst_chan,
                                                    oal_uint32                ul_duration,
                                                    oal_uint64               *pull_cookie,
                                                    wlan_ieee80211_roc_type_uint8 en_roc_type)
{
    wal_msg_write_stru                       st_write_msg           = {0};
    wal_msg_stru                            *pst_rsp_msg            = OAL_PTR_NULL;
    oal_uint32                               ul_err_code;
    oal_net_device_stru                     *pst_netdev;
    mac_remain_on_channel_param_stru         st_remain_on_channel   = {0};
    oal_uint16                               us_center_freq;
    oal_int32                                l_channel;
    oal_int32                                l_ret;
    mac_device_stru                         *pst_mac_device;
    mac_vap_stru                            *pst_mac_vap;
    mac_vap_stru                            *pst_other_vap;
    oal_uint8                                uc_vap_idx;

    /* 1.1 入参检查 */
    if ((OAL_PTR_NULL == pst_wiphy)
        || (OAL_PTR_NULL == pst_wdev)
        || (OAL_PTR_NULL == pst_chan)
        || (OAL_PTR_NULL == pull_cookie))
    {
        OAM_ERROR_LOG4(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::pst_wiphy or pst_wdev or pst_chan or pull_cookie ptr is null,error %x, %x, %x, %x!}\r\n",
                        pst_wiphy, pst_wdev, pst_chan, pull_cookie);
        return -OAL_EINVAL;
    }

    pst_netdev = pst_wdev->netdev;
    if (OAL_PTR_NULL == pst_netdev)
    {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::pst_netdev ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    pst_mac_vap    = OAL_NET_DEV_PRIV(pst_netdev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::can't get mac vap from netdevice priv data!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_device = (mac_device_stru *)mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::pst_mac_device ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_WAPI
    if (OAL_TRUE == hmac_user_is_wapi_connected_etc(pst_mac_vap->uc_device_id))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{stop p2p remaining under wapi!}");
        return -OAL_EINVAL;
    }
#endif/* #ifdef _PRE_WLAN_FEATURE_WAPI */

    /* check all vap state in case other vap is connecting now */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_other_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_other_vap)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_drv_remain_on_channel::vap is null! vap id is %d}", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((( pst_other_vap->en_vap_state >= MAC_VAP_STATE_STA_JOIN_COMP) && (pst_other_vap->en_vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC))
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (MAC_VAP_STATE_ROAMING == pst_other_vap->en_vap_state)
#endif
        )
        {
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                "{wal_drv_remain_on_channel::invalid ROC[%d] for roaming or connecting state[%d]!}", en_roc_type, pst_other_vap->en_vap_state);
            return -OAL_EFAIL;
        }
    }

    /* tx mgmt roc 优先级低,可以被自己的80211 roc以及80211 scan打断 */
    if (MAC_VAP_STATE_STA_LISTEN == pst_mac_vap->en_vap_state)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_drv_remain_on_channel::new roc type[%d],cancel old roc!}", en_roc_type);
        l_ret = wal_p2p_stop_roc(pst_mac_vap, pst_netdev);
        if (l_ret < 0)
        {
            return -OAL_EFAIL;
        }
    }

    /* 2.1 消息参数准备 */
    us_center_freq  = pst_chan->center_freq;
    l_channel       = (oal_int32)oal_ieee80211_frequency_to_channel((oal_int32)us_center_freq);

    st_remain_on_channel.uc_listen_channel  = (oal_uint8)l_channel;
    st_remain_on_channel.ul_listen_duration = ul_duration;
    st_remain_on_channel.st_listen_channel  = *pst_chan;
    st_remain_on_channel.en_listen_channel_type =  WLAN_BAND_WIDTH_20M;
    st_remain_on_channel.en_roc_type =  en_roc_type;

    if (pst_chan->band == HISI_IEEE80211_BAND_2GHZ)
    {
        st_remain_on_channel.en_band = WLAN_BAND_2G;
    }
    else if (pst_chan->band == HISI_IEEE80211_BAND_5GHZ)
    {
        st_remain_on_channel.en_band = WLAN_BAND_5G;
    }
    else
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_drv_remain_on_channel::wrong band en_roc_type[%d]!}\r\n", pst_chan->band);
        return -OAL_EINVAL;
    }

    if (IEEE80211_ROC_TYPE_NORMAL == en_roc_type)
    {
        /* 设置cookie 值 */
        *pull_cookie = ++pst_mac_device->st_p2p_info.ull_last_roc_id;   /* cookie值上层调用需要判断是否是这次的发送导致的callback */
        if (*pull_cookie == 0)
        {
            *pull_cookie = ++pst_mac_device->st_p2p_info.ull_last_roc_id;
        }

        /* 保存cookie 值，下发给HMAC 和DMAC */
        st_remain_on_channel.ull_cookie         = pst_mac_device->st_p2p_info.ull_last_roc_id;
    }
    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 3.1 填写 msg 消息头 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_REMAIN_ON_CHANNEL, OAL_SIZEOF(mac_remain_on_channel_param_stru));


    /* 3.2 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_remain_on_channel, OAL_SIZEOF(mac_remain_on_channel_param_stru));

    /* 3.3 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_remain_on_channel_param_stru),
                                (oal_uint8 *)&st_write_msg,
                                OAL_TRUE,
                                &pst_rsp_msg);

    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_drv_remain_on_channel::wal_send_cfg_event_etc return err code:[%d]!}\r\n",l_ret);
        return -OAL_EFAIL;
    }

    /* 4.1 读取返回的错误码 */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_drv_remain_on_channel::wal_send_cfg_event_etc return err code:[%u]!}\r\n",
                         ul_err_code);
        return -OAL_EFAIL;
    }

    if (IEEE80211_ROC_TYPE_NORMAL == en_roc_type)
    {
        /* 上报暂停在指定信道成功 */
        oal_cfg80211_ready_on_channel_etc(pst_wdev, *pull_cookie, pst_chan, ul_duration, GFP_KERNEL);
    }

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                  "{wal_drv_remain_on_channel::SUCC! l_channel=%d, ul_duration=%d, cookie 0x%x, roc type=%d!}\r\n",
                  l_channel, ul_duration, *pull_cookie, en_roc_type);
    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfg80211_remain_on_channel(oal_wiphy_stru           *pst_wiphy,
                                                    oal_wireless_dev_stru    *pst_wdev,
                                                    struct ieee80211_channel *pst_chan,
                                                    oal_uint32                ul_duration,
                                                    oal_uint64               *pull_cookie)
{
    return wal_drv_remain_on_channel(pst_wiphy, pst_wdev, pst_chan, ul_duration, pull_cookie, IEEE80211_ROC_TYPE_NORMAL);
}


OAL_STATIC oal_int32 wal_cfg80211_cancel_remain_on_channel(oal_wiphy_stru           *pst_wiphy,
                                                            oal_wireless_dev_stru   *pst_wdev,
                                                            oal_uint64               ull_cookie)
{
    wal_msg_write_stru                       st_write_msg                 = {0};
    mac_remain_on_channel_param_stru         st_cancel_remain_on_channel  = {0};
    wal_msg_stru                            *pst_rsp_msg                  = OAL_PTR_NULL;
    oal_uint32                               ul_err_code;
    oal_net_device_stru                     *pst_netdev;
    oal_int32                                l_ret;

    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_wdev))
    {
        OAM_ERROR_LOG2(0, OAM_SF_P2P, "{wal_cfg80211_cancel_remain_on_channel::pst_wiphy or pst_wdevis null,error %d, %d!}\r\n", pst_wiphy, pst_wdev);
        return -OAL_EINVAL;
    }

    pst_netdev = pst_wdev->netdev;
    if (OAL_PTR_NULL == pst_netdev)
    {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{wal_cfg80211_cancel_remain_on_channel::pst_netdev ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    OAM_WARNING_LOG1(0, OAM_SF_P2P, "wal_cfg80211_cancel_remain_on_channel[0x%x].", ull_cookie);

    /* 3.1 填写 msg 消息头 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_CANCEL_REMAIN_ON_CHANNEL, OAL_SIZEOF(mac_remain_on_channel_param_stru));


    /* 3.2 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_cancel_remain_on_channel, OAL_SIZEOF(mac_remain_on_channel_param_stru));

    /* 3.3 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_remain_on_channel_param_stru),
                                (oal_uint8 *)&st_write_msg,
                                OAL_TRUE,
                                &pst_rsp_msg);

    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_cfg80211_cancel_remain_on_channel::wal_send_cfg_event_etc return err code: [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    /* 4.1 读取返回的错误码 */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{wal_cfg80211_cancel_remain_on_channel::wal_check_and_release_msg_resp_etc fail return err code:[%u]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


oal_uint8 wal_find_oldest_cookie(cookie_arry_stru  *pst_cookie_array)
{
    oal_uint8           uc_loops = 0;
    oal_uint8           uc_target_index = 0;

    /* 外部保证只有在cookie用满的情况下才会调用该函数，因此初始index选0即可 */
    for (uc_loops = 1; uc_loops < WAL_COOKIE_ARRAY_SIZE; uc_loops++)
    {
        if(oal_time_after32(pst_cookie_array[uc_target_index].ul_record_time, pst_cookie_array[uc_loops].ul_record_time))
        {
            uc_target_index = uc_loops;
        }
    }

    return uc_target_index;
}


oal_void wal_check_cookie_timeout(cookie_arry_stru  *pst_cookie_array,
                                    oal_uint8       *puc_cookie_bitmap,
                                    oal_uint32       ul_current_time)
{
    oal_uint8           uc_loops = 0;
    cookie_arry_stru   *pst_tmp_cookie;

    OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_check_cookie_timeout::time_out!}\r\n");
    for (uc_loops = 0; uc_loops < WAL_COOKIE_ARRAY_SIZE; uc_loops++)
    {
        pst_tmp_cookie = &pst_cookie_array[uc_loops];
        if (oal_time_after32(OAL_TIME_JIFFY, pst_tmp_cookie->ul_record_time + OAL_MSECS_TO_JIFFIES(WAL_MGMT_TX_TIMEOUT_MSEC)))
        {
            /* cookie array 中保存的cookie 值超时 */
            /* 清空cookie array 中超时的cookie */
            pst_tmp_cookie->ul_record_time = 0;
            pst_tmp_cookie->ull_cookie     = 0;
            /* 清除占用的cookie bitmap位 */
            oal_bit_clear_bit_one_byte(puc_cookie_bitmap, uc_loops);
        }
    }
}


oal_uint32 wal_del_cookie_from_array(cookie_arry_stru   *pst_cookie_array,
                                        oal_uint8       *puc_cookie_bitmap,
                                        oal_uint8        uc_cookie_idx)
{
    cookie_arry_stru   *pst_tmp_cookie;

    /* 清除对应cookie bitmap 位 */
    oal_bit_clear_bit_one_byte(puc_cookie_bitmap, uc_cookie_idx);

    /* 清空cookie array 中超时的cookie */
    pst_tmp_cookie = &pst_cookie_array[uc_cookie_idx];
    pst_tmp_cookie->ull_cookie     = 0;
    pst_tmp_cookie->ul_record_time = 0;
    return OAL_SUCC;
}



oal_uint32 wal_add_cookie_to_array(cookie_arry_stru *pst_cookie_array,
                                    oal_uint8       *puc_cookie_bitmap,
                                    oal_uint64      *pull_cookie,
                                    oal_uint8       *puc_cookie_idx)
{
    oal_uint8           uc_idx;
    cookie_arry_stru   *pst_tmp_cookie;

    if (WAL_COOKIE_FULL_MASK == *puc_cookie_bitmap)
    {
        /* cookie array 满，返回错误 */
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_add_cookie_to_array::array full!}\r\n");
        return OAL_FAIL;
    }

    /* 将cookie 添加到array 中 */
    uc_idx = oal_bit_find_first_zero_one_byte(*puc_cookie_bitmap);
    oal_bit_set_bit_one_byte(puc_cookie_bitmap, uc_idx);

    pst_tmp_cookie = &pst_cookie_array[uc_idx];
    pst_tmp_cookie->ull_cookie      = *pull_cookie;
    pst_tmp_cookie->ul_record_time  = OAL_TIME_JIFFY;

    *puc_cookie_idx = uc_idx;
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_check_cookie_from_array(oal_uint8 *puc_cookie_bitmap,oal_uint8 uc_cookie_idx)
{
    /* 从cookie bitmap中查找相应的cookie index，如果位图为0，表示已经被del */
    if(*puc_cookie_bitmap & (BIT(uc_cookie_idx)))
    {
        return OAL_SUCC;
    }
    /* 找不到则返回FAIL */
    return OAL_FAIL;
}


OAL_STATIC oal_uint32 wal_mgmt_do_tx(oal_net_device_stru    *pst_netdev,
                                                mac_mgmt_frame_stru *pst_mgmt_tx_param,
                                                bool                 en_offchan,
                                                oal_uint32           ul_wait)
{
    mac_vap_stru                    *pst_mac_vap;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_mgmt_tx_stru                *pst_mgmt_tx;
    wal_msg_write_stru               st_write_msg;
    oal_int32                        l_ret = 0;
    oal_int                          i_leftime;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_netdev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_mgmt_do_tx::can't get mac vap from netdevice priv data.}\r\n");
        return OAL_FAIL;
    }

    if( !IS_P2P_GO(pst_mac_vap))
    {
        // 如果不是Go，则做如下判断
        if (OAL_TRUE == en_offchan)
        {
            if (pst_mac_vap->en_vap_state != MAC_VAP_STATE_STA_LISTEN)
            {
                OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_mgmt_do_tx::pst_mac_vap state[%d]not in listen!}\r\n", pst_mac_vap->en_vap_state);
                return DMAC_TX_INVALID; //返回TX_INVALID,软件不再重传,告诉上层此次tx mgmt失败结束
            }
        }
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_mgmt_do_tx::pst_hmac_vap ptr is null!}\r\n");
        return OAL_FAIL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 3.1 填写 msg 消息头 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_MGMT_TX, OAL_SIZEOF(mac_mgmt_frame_stru));

    /* 3.2 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, pst_mgmt_tx_param, OAL_SIZEOF(mac_mgmt_frame_stru));

    /* 3.3 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_mgmt_frame_stru),
                                (oal_uint8 *)&st_write_msg,
                                OAL_FALSE,
                                OAL_PTR_NULL);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_mgmt_do_tx::wal_send_cfg_event_etc return err code %d!}\r\n", l_ret);
        return OAL_FAIL;
    }

    pst_mgmt_tx = &(pst_hmac_vap->st_mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete= OAL_FALSE;
    pst_mgmt_tx->mgmt_tx_status  = OAL_FAIL;
    /*lint -e730*/
    OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_mgmt_do_tx:: It will wait %d ms at most for the tx status report!}\r\n", ul_wait);
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_mgmt_tx->st_wait_queue,
        OAL_TRUE== pst_mgmt_tx->mgmt_tx_complete,
        (oal_uint32)OAL_MSECS_TO_JIFFIES(ul_wait));

    if ( 0 == i_leftime)
    {
        /* 定时器超时 */
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_mgmt_do_tx::mgmt tx timeout!}\r\n");
        return OAL_FAIL;
    }
    else if (i_leftime < 0)
    {
        /* 定时器内部错误 */
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_mgmt_do_tx::mgmt tx timer error!}\r\n");
        return OAL_FAIL;
    }
    else
    {
        /* 正常结束  */
        OAM_INFO_LOG0(0, OAM_SF_ANY, "{wal_mgmt_do_tx::mgmt tx commpleted!}\r\n");

        return pst_mgmt_tx->mgmt_tx_status;
    }
}



OAL_STATIC oal_int32 wal_cfg80211_mgmt_tx(oal_wiphy_stru                *pst_wiphy,
                                            oal_wireless_dev_stru       *pst_wdev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
                                            struct cfg80211_mgmt_tx_params *pst_params,
#else
                                            oal_ieee80211_channel       *pst_chan,
                                            bool                         en_offchan,
                                            oal_uint32                   ul_wait,
                                            OAL_CONST oal_uint8         *puc_buf,
                                            oal_size_t                   ul_len,
                                            bool                         en_no_cck,
                                            bool                         en_dont_wait_for_ack,
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
                                            oal_uint64                  *pull_cookie)
{
    oal_net_device_stru             *pst_netdev;
    mac_device_stru                 *pst_mac_device;
    mac_vap_stru                    *pst_mac_vap;
    OAL_CONST oal_ieee80211_mgmt    *pst_mgmt;
    oal_int32                       ul_ret = 0;
    mac_mgmt_frame_stru             st_mgmt_tx;
    oal_uint8                       uc_cookie_idx;
    oal_uint8                       uc_retry;
    mac_p2p_info_stru               *pst_p2p_info;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_mgmt_tx_stru                *pst_mgmt_tx;
    oal_wireless_dev_stru           *pst_roc_wireless_dev = pst_wdev;
    unsigned long                    ul_start_time_stamp;
    bool                             en_need_offchan = OAL_FALSE;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    oal_ieee80211_channel       *pst_chan;
    OAL_CONST oal_uint8         *puc_buf;
    oal_size_t                   ul_len;
    oal_uint32                   ul_wait;
    bool                         en_offchan = OAL_FALSE;
    if (pst_params == OAL_PTR_NULL)
    {
        return -OAL_EINVAL;
    }
    pst_chan   = pst_params->chan;
    puc_buf    = pst_params->buf;
    ul_len     = pst_params->len;
    en_offchan = pst_params->offchan;
    ul_wait    = pst_params->wait;
#endif

    /* 1.1 入参检查 */
    if ((OAL_PTR_NULL == pst_wiphy)
        || (OAL_PTR_NULL == pst_wdev)
        || (OAL_PTR_NULL == pst_chan)
        || (OAL_PTR_NULL == pull_cookie)
        || (OAL_PTR_NULL == puc_buf))
    {
        OAM_ERROR_LOG4(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_wiphy or pst_wdev or pst_chan or pull_cookie or puc_buf ptr is null,error %x, %x, %x, %x!}\r\n",
                        pst_wiphy, ul_wait, pst_chan, pull_cookie);
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    pst_netdev = pst_wdev->netdev;
    if (OAL_PTR_NULL == pst_netdev)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_netdev ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_vap    = OAL_NET_DEV_PRIV(pst_netdev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::can't get mac vap from netdevice priv data!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_device = (mac_device_stru *)mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_mac_device ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_hmac_vap ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_p2p_info = &pst_mac_device->st_p2p_info;
    *pull_cookie = pst_p2p_info->ull_send_action_id++;   /* cookie值上层调用需要判断是否是这次的发送导致的callback */
    if (*pull_cookie == 0)
    {
        *pull_cookie = pst_p2p_info->ull_send_action_id++;
    }
    pst_mgmt = (const struct ieee80211_mgmt *)puc_buf;
    if (oal_ieee80211_is_probe_resp(pst_mgmt->frame_control))
    {
        *pull_cookie = 0; /* set cookie default value */
        /* host should not send PROE RESPONSE,
           device will send immediately when receive probe request packet */
        oal_cfg80211_mgmt_tx_status_etc(pst_wdev, *pull_cookie, puc_buf, ul_len, OAL_TRUE, GFP_KERNEL);
        return OAL_SUCC;
    }

    /* 2.1 消息参数准备 */
    OAL_MEMZERO(&st_mgmt_tx, OAL_SIZEOF(st_mgmt_tx));
    st_mgmt_tx.channel = oal_ieee80211_frequency_to_channel(pst_chan->center_freq);

    if(WAL_COOKIE_FULL_MASK == g_uc_cookie_array_bitmap_etc)
    {
        uc_cookie_idx = wal_find_oldest_cookie(g_cookie_array_etc);
        wal_del_cookie_from_array(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, uc_cookie_idx);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::cookies is [0x%x] after clear}", g_uc_cookie_array_bitmap_etc);
    }

    ul_ret = wal_add_cookie_to_array(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, pull_cookie, &uc_cookie_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::Failed to add cookies, ul_ret[%d]!}\r\n", ul_ret);
        return -OAL_EINVAL;
    }
    else
    {
        st_mgmt_tx.mgmt_frame_id = uc_cookie_idx;
    }

    st_mgmt_tx.us_len       = ul_len;
    st_mgmt_tx.puc_frame    = puc_buf;

    pst_mgmt_tx = &(pst_hmac_vap->st_mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete= OAL_FALSE;
    pst_mgmt_tx->mgmt_tx_status = OAL_FAIL;

    switch (pst_hmac_vap->st_vap_base_info.en_vap_mode)
    {
        case WLAN_VAP_MODE_BSS_AP:
            // 此处应该是channel number，而不是channel index
            OAM_WARNING_LOG3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::p2p mode[%d](0=Legacy, 1=Go, 2=Dev, 3=Gc), tx mgmt vap channel[%d], mgmt tx channel[%d]",
                               pst_hmac_vap->st_vap_base_info.en_p2p_mode, pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_number, st_mgmt_tx.channel);
            if ((pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_number != st_mgmt_tx.channel) && IS_P2P_GO(&pst_hmac_vap->st_vap_base_info))
            {
                if (OAL_PTR_NULL == pst_mac_device->st_p2p_info.pst_p2p_net_device)
                {
                    OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::go mode but p2p dev is null");
                    return -OAL_EINVAL;
                }
                pst_roc_wireless_dev = OAL_NETDEVICE_WDEV(pst_mac_device->st_p2p_info.pst_p2p_net_device); //使用p2p0 roc
                en_need_offchan = OAL_TRUE;
            }
        break;

        /* P2P CL DEV */
        case WLAN_VAP_MODE_BSS_STA:
            if ((OAL_TRUE == en_offchan) && (pst_wiphy->flags & WIPHY_FLAG_OFFCHAN_TX))
            {
                en_need_offchan = OAL_TRUE;
            }
            if ((WLAN_LEGACY_VAP_MODE == pst_hmac_vap->st_vap_base_info.en_p2p_mode)
                && (MAC_VAP_STATE_UP == pst_hmac_vap->st_vap_base_info.en_vap_state)
                && (st_mgmt_tx.channel == pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_number))
            {
                en_need_offchan = OAL_FALSE;
            }
        break;

        default:
        break;
    }

    if ((OAL_TRUE == en_need_offchan) && !pst_chan)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::need offchannle but channel is null}\r\n", en_offchan);
        return -OAL_EINVAL;
    }

    if (0 == ul_wait)
    {
        ul_wait = WAL_MGMT_TX_TIMEOUT_MSEC;
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::wait time is 0, set it to %d ms}", ul_wait);
    }

    OAM_WARNING_LOG4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::offchannel[%d].channel[%d]vap state[%d],wait[%d]}\r\n",
            en_need_offchan, st_mgmt_tx.channel, pst_hmac_vap->st_vap_base_info.en_vap_state, ul_wait);

    /* 需要offchannel,按照入参切到相应的信道XXms */
    if (OAL_TRUE == en_need_offchan)
    {
        ul_ret = wal_drv_remain_on_channel(pst_wiphy, pst_roc_wireless_dev, pst_chan, ul_wait, pull_cookie, IEEE80211_ROC_TYPE_MGMT_TX);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::wal_drv_remain_on_channel[%d]!!!offchannel[%d].channel[%d],vap state[%d]}\r\n",
                    ul_ret, en_need_offchan, st_mgmt_tx.channel, pst_hmac_vap->st_vap_base_info.en_vap_state);
            return -OAL_EBUSY;
        }
    }

    ul_start_time_stamp = OAL_TIME_JIFFY;

    uc_retry = 0;
    /* 发送失败，则尝试重传 */
    do
    {
        ul_ret = wal_mgmt_do_tx(pst_netdev, &st_mgmt_tx, en_need_offchan, ul_wait);
        uc_retry++;
    }while((ul_ret != DMAC_TX_SUCC) && (ul_ret != DMAC_TX_INVALID) && (uc_retry <= WAL_MGMT_TX_RETRY_CNT)
        && time_before(OAL_TIME_JIFFY, ul_start_time_stamp + OAL_MSECS_TO_JIFFIES(ul_wait)));/*lint !e666*/
    if (uc_retry > WAL_MGMT_TX_RETRY_CNT)
    {
        OAM_WARNING_LOG3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::retry count[%d]>max[%d],tx status[%d],stop tx mgmt}\r\n",
                uc_retry, WAL_MGMT_TX_RETRY_CNT, ul_ret);
    }

    if (ul_ret != DMAC_TX_SUCC)
    {
        /* 发送失败，处理超时帧的bitmap */
        wal_check_cookie_timeout(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, OAL_TIME_JIFFY);

        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::vap status[%d], tx mgmt timeout}\r\n", pst_mac_vap->en_vap_state);

#ifdef _PRE_WLAN_FEATURE_P2P
        if (MAC_VAP_STATE_STA_LISTEN == pst_mac_vap->en_vap_state)
        {
            mac_vap_state_change_etc(&pst_hmac_vap->st_vap_base_info, pst_mac_device->st_p2p_info.en_last_vap_state);

            /* 抛事件到DMAC ，返回监听信道 */
            hmac_p2p_send_listen_expired_to_device_etc(pst_hmac_vap);
        }
#endif

        oal_cfg80211_mgmt_tx_status_etc(pst_wdev, *pull_cookie, puc_buf, ul_len, OAL_FALSE, GFP_KERNEL);
    }
    else
    {
        /* 正常结束  */
        *pull_cookie = g_cookie_array_etc[pst_mgmt_tx->mgmt_frame_id].ull_cookie;
        wal_del_cookie_from_array(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, pst_mgmt_tx->mgmt_frame_id);
        oal_cfg80211_mgmt_tx_status_etc(pst_wdev, *pull_cookie, puc_buf, ul_len, OAL_TRUE, GFP_KERNEL);
    }

    OAM_WARNING_LOG3(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::tx status [%d], retry cnt[%d]}, delta_time[%d]\r\n",
                    ul_ret, uc_retry, OAL_JIFFIES_TO_MSECS(OAL_TIME_JIFFY - ul_start_time_stamp));

#if 0
    if (OAL_TRUE == en_need_offchan) /* cancel roc */
    {
        oal_int32 l_ret = 0;

        /* l_ret = wal_cfg80211_cancel_remain_on_channel(pst_wiphy, pst_wdev, *pull_cookie);
        if (OAL_SUCC != l_ret)
        {
            OAM_WARNING_LOG4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                    "{wal_cfg80211_mgmt_tx::wal_cfg80211_cancel_remain_on_channel[%d]!!!offchannel[%d].channel[%d],vap state[%d]}\r\n",
                    l_ret, en_need_offchan, st_mgmt_tx.channel, pst_hmac_vap->st_vap_base_info.en_vap_state);
            return -OAL_EBUSY;
        } */
        /* stop roc which including scan  */
        if (MAC_VAP_STATE_STA_LISTEN == pst_mac_vap->en_vap_state)
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_cfg80211_mgmt_tx::wal_p2p_stop_roc::cancel roc!}");
            l_ret = wal_p2p_stop_roc(pst_mac_vap, pst_netdev);
            if (l_ret < 0)
            {
                OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_cfg80211_mgmt_tx::wal_p2p_stop_roc::cancel roc fail l_ret=%d!}", l_ret);
                return -OAL_EFAIL;
            }
        }
    }
#endif

    return OAL_SUCC;
}


oal_uint32  wal_cfg80211_mgmt_tx_status_etc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                  *pst_event;
    dmac_crx_mgmt_tx_status_stru    *pst_mgmt_tx_status_param;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_mgmt_tx_stru                *pst_mgmt_tx;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx_status_etc::pst_event_mem is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event  = frw_get_event_stru(pst_event_mem);
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if(OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_TX, "{wal_cfg80211_mgmt_tx_status_etc::pst_hmac_vap null.vap_id[%d]}", pst_event->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mgmt_tx_status_param   = (dmac_crx_mgmt_tx_status_stru *)(pst_event->auc_event_data);
    pst_mgmt_tx = &(pst_hmac_vap->st_mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete = OAL_TRUE;
    pst_mgmt_tx->mgmt_tx_status   = pst_mgmt_tx_status_param->uc_dscr_status;
    pst_mgmt_tx->mgmt_frame_id    = pst_mgmt_tx_status_param->mgmt_frame_id;

    /* 找不到相应的cookie值，说明已经超时被处理，不需要再唤醒 */
    if(OAL_SUCC == wal_check_cookie_from_array(&g_uc_cookie_array_bitmap_etc, pst_mgmt_tx->mgmt_frame_id))
    {
         /* 让编译器优化时保证OAL_WAIT_QUEUE_WAKE_UP在最后执行 */
        OAL_SMP_MB();
        OAL_WAIT_QUEUE_WAKE_UP_INTERRUPT(&pst_mgmt_tx->st_wait_queue);
    }

    return OAL_SUCC;
}

/* P2P 补充缺失的CFG80211接口 */
oal_void wal_cfg80211_mgmt_frame_register_etc(struct wiphy *wiphy,
                                        struct wireless_dev *wdev,
                                        oal_uint16 frame_type,
                                        bool reg)
{
    //OAL_IO_PRINT("wal_cfg80211_mgmt_frame_register_etc::net_device_name:%s, if_type:%d", wdev->netdev->name, wdev->iftype);
    OAM_INFO_LOG3(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_frame_register_etc::enter.frame_type[0x%04x], reg[%d], if_type[%d]}",
                    frame_type, reg, wdev->iftype);

    return;
}

oal_int32 wal_cfg80211_set_bitrate_mask_etc(struct wiphy *wiphy,
                                        struct net_device *dev,
                                        const u8 *peer,
                                        const struct cfg80211_bitrate_mask *mask)
{
    OAM_INFO_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_set_bitrate_mask_etc::enter 000.}");

    return OAL_SUCC;
}

#endif  /* _PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5 */

#else

oal_void wal_check_cookie_timeout(cookie_arry_stru  *pst_cookie_array,
                                    oal_uint8       *puc_cookie_bitmap,
                                    oal_uint32       ul_current_time)
{
    oal_uint8           uc_loops = 0;
    cookie_arry_stru   *pst_tmp_cookie;

    OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_check_cookie_timeout::time_out!}\r\n");
    for (uc_loops = 0; uc_loops < WAL_COOKIE_ARRAY_SIZE; uc_loops++)
    {
        pst_tmp_cookie = &pst_cookie_array[uc_loops];
        if (oal_time_is_before(pst_tmp_cookie->ul_record_time + OAL_MSECS_TO_JIFFIES(WAL_MGMT_TX_TIMEOUT_MSEC)))
        {
            /* cookie array 中保存的cookie 值超时 */
            /* 清空cookie array 中超时的cookie */
            pst_tmp_cookie->ul_record_time = 0;
            pst_tmp_cookie->ull_cookie     = 0;
            /* 清除占用的cookie bitmap位 */
            oal_bit_clear_bit_one_byte(puc_cookie_bitmap, uc_loops);
        }
    }
}


oal_uint32 wal_del_cookie_from_array(cookie_arry_stru   *pst_cookie_array,
                                        oal_uint8       *puc_cookie_bitmap,
                                        oal_uint8        uc_cookie_idx)
{
    cookie_arry_stru   *pst_tmp_cookie;

    /* 清除对应cookie bitmap 位 */
    oal_bit_clear_bit_one_byte(puc_cookie_bitmap, uc_cookie_idx);

    /* 清空cookie array 中超时的cookie */
    pst_tmp_cookie = &pst_cookie_array[uc_cookie_idx];
    pst_tmp_cookie->ull_cookie     = 0;
    pst_tmp_cookie->ul_record_time = 0;
    return OAL_SUCC;
}



oal_uint32 wal_add_cookie_to_array(cookie_arry_stru *pst_cookie_array,
                                    oal_uint8       *puc_cookie_bitmap,
                                    oal_uint64      *pull_cookie,
                                    oal_uint8       *puc_cookie_idx)
{
    oal_uint8           uc_idx;
    cookie_arry_stru   *pst_tmp_cookie;

    if (*puc_cookie_bitmap == 0xFF)
    {
        /* cookie array 满，返回错误 */
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_add_cookie_to_array::array full!}\r\n");
        return OAL_FAIL;
    }

    /* 将cookie 添加到array 中 */
    uc_idx = oal_bit_get_num_one_byte(*puc_cookie_bitmap);
    oal_bit_set_bit_one_byte(puc_cookie_bitmap, uc_idx);

    pst_tmp_cookie = &pst_cookie_array[uc_idx];
    pst_tmp_cookie->ull_cookie      = *pull_cookie;
    pst_tmp_cookie->ul_record_time  = OAL_TIME_JIFFY;

    *puc_cookie_idx = uc_idx;
    return OAL_SUCC;
}



oal_int32 wal_cfg80211_mgmt_tx(oal_wiphy_stru                *pst_wiphy,
                                            oal_wireless_dev_stru       *pst_wdev,
                                            oal_ieee80211_channel       *pst_chan,
                                            oal_bool_enum_uint8          en_offchan,
                                            oal_uint32                   ul_wait,
                                            OAL_CONST oal_uint8         *puc_buf,
                                            size_t                       ul_len,
                                            oal_bool_enum_uint8          en_no_cck,
                                            oal_bool_enum_uint8          en_dont_wait_for_ack,
                                            oal_uint64                  *pull_cookie)
{
    wal_msg_write_stru              st_write_msg;
    oal_net_device_stru             *pst_netdev;
    mac_device_stru                 *pst_mac_device;
    mac_vap_stru                    *pst_mac_vap;
    OAL_CONST oal_ieee80211_mgmt    *pst_mgmt;
    oal_int32                       l_ret = 0;
    oal_uint32                      ul_ret = 0;
    mac_mgmt_frame_stru             st_mgmt_tx;
    oal_int                         i_leftime;
    oal_uint8                       uc_cookie_idx;
    mac_p2p_info_stru               *pst_p2p_info;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_mgmt_tx_stru                *pst_mgmt_tx;

    /* 1.1 入参检查 */
    if ((OAL_PTR_NULL == pst_wiphy)
        || (OAL_PTR_NULL == pst_wdev)
        || (OAL_PTR_NULL == pst_chan)
        || (OAL_PTR_NULL == pull_cookie)
        || (OAL_PTR_NULL == puc_buf))
    {
        OAM_ERROR_LOG4(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_wiphy or pst_wdev or pst_chan or pull_cookie or puc_buf ptr is null,error %x, %x, %x, %x!}\r\n",
                        pst_wiphy, pst_wdev, pst_chan, pull_cookie);
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    pst_netdev = pst_wdev->netdev;
    if (OAL_PTR_NULL == pst_netdev)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_netdev ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_vap    = OAL_NET_DEV_PRIV(pst_netdev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_mac_vap ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_device = (mac_device_stru *)mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_mac_device ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::pst_hmac_vap ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    pst_p2p_info = &pst_mac_device->st_p2p_info;
    *pull_cookie = pst_p2p_info->ull_send_action_id++;   /* cookie值上层调用需要判断是否是这次的发送导致的callback */
    if (*pull_cookie == 0)
    {
        *pull_cookie = pst_p2p_info->ull_send_action_id++;
    }
    pst_mgmt = (const struct ieee80211_mgmt *)puc_buf;
    if (oal_ieee80211_is_probe_resp(pst_mgmt->frame_control))
    {
        *pull_cookie = 0; /* set cookie default value */
        /* host should not send PROE RESPONSE,
           device will send immediately when receive probe request packet */
        oal_cfg80211_mgmt_tx_status_etc(pst_wdev, *pull_cookie, puc_buf, ul_len, OAL_TRUE, GFP_KERNEL);
        return OAL_SUCC;
    }

    /* 2.1 消息参数准备 */
    OAL_MEMZERO(&st_mgmt_tx, OAL_SIZEOF(st_mgmt_tx));
    st_mgmt_tx.channel = oal_ieee80211_frequency_to_channel((oal_int32)pst_chan->center_freq);
    ul_ret = wal_add_cookie_to_array(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, pull_cookie, &uc_cookie_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::Failed to add cookies!}\r\n");
        return -OAL_EINVAL;
    }
    else
    {
        st_mgmt_tx.mgmt_frame_id = uc_cookie_idx;
    }
    st_mgmt_tx.us_len = (oal_uint16)ul_len;
    st_mgmt_tx.puc_frame = puc_buf;

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 3.1 填写 msg 消息头 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_MGMT_TX, OAL_SIZEOF(st_mgmt_tx));


    /* 3.2 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_mgmt_tx, OAL_SIZEOF(st_mgmt_tx));

    /* 3.3 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_mgmt_tx),
                                (oal_uint8 *)&st_write_msg,
                                OAL_FALSE,
                                NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::wal_send_cfg_event_etc return err code %d!}\r\n", l_ret);
        return -OAL_EINVAL;
    }

    pst_mgmt_tx = &(pst_hmac_vap->st_mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete= OAL_FALSE;
    pst_mgmt_tx->mgmt_tx_status = OAL_FAIL;
    /*lint -e730*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_mgmt_tx->st_wait_queue, OAL_TRUE== pst_mgmt_tx->mgmt_tx_complete,  (oal_uint32)OAL_MSECS_TO_JIFFIES(WAL_MGMT_TX_TIMEOUT_MSEC));
   /*lint +e730*/

    if ( 0 == i_leftime)
    {
        /* 超时还没有上报扫描结束 */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::mgmt tx wait for %ld ms timeout!}\r\n",((oal_uint32)WAL_MGMT_TX_TIMEOUT_MSEC));
        wal_check_cookie_timeout(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, OAL_TIME_JIFFY);

    }
    else if (i_leftime < 0)
    {
        /* 定时器内部错误 */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::mgmt tx wait for %ld ms error!}\r\n",((oal_uint32)WAL_MGMT_TX_TIMEOUT_MSEC));
    }
    else
    {
        /* 正常结束  */
        OAM_INFO_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::mgmt tx wait for %ld ms complete!}\r\n",((oal_uint32)WAL_MGMT_TX_TIMEOUT_MSEC));
        *pull_cookie = g_cookie_array_etc[pst_mgmt_tx->mgmt_frame_id].ull_cookie;
        wal_del_cookie_from_array(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, pst_mgmt_tx->mgmt_frame_id);
    }


    if(pst_mgmt_tx->mgmt_tx_status != DMAC_TX_SUCC)
    {
        oal_cfg80211_mgmt_tx_status_etc(pst_wdev, *pull_cookie, puc_buf, ul_len, OAL_FALSE, GFP_KERNEL);
        return -OAL_EINVAL;
    }
    else
    {
        oal_cfg80211_mgmt_tx_status_etc(pst_wdev, *pull_cookie, puc_buf, ul_len, OAL_TRUE, GFP_KERNEL);
        return OAL_SUCC;
    }
}
/*lint +e774*/



oal_uint32  wal_cfg80211_mgmt_tx_status_etc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                  *pst_event;
    dmac_crx_mgmt_tx_status_stru    *pst_mgmt_tx_status_param;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_mgmt_tx_stru                *pst_mgmt_tx;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx_status_etc::pst_event_mem is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_event  = frw_get_event_stru(pst_event_mem);
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if(OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{wal_cfg80211_mgmt_tx_status_etc::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mgmt_tx_status_param   = (dmac_crx_mgmt_tx_status_stru *)(pst_event->auc_event_data);
    pst_mgmt_tx = &(pst_hmac_vap->st_mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete = OAL_TRUE;
    pst_mgmt_tx->mgmt_tx_status   = pst_mgmt_tx_status_param->uc_dscr_status;
    pst_mgmt_tx->mgmt_frame_id    = pst_mgmt_tx_status_param->mgmt_frame_id;

     /* 让编译器优化时保证OAL_WAIT_QUEUE_WAKE_UP在最后执行 */
    OAL_SMP_MB();
    OAL_WAIT_QUEUE_WAKE_UP_INTERRUPT(&pst_mgmt_tx->st_wait_queue);

    return OAL_SUCC;
}

#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
#if (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5) && (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_CPE)


OAL_STATIC oal_int32 wal_cfg80211_start_p2p_device(oal_wiphy_stru       *pst_wiphy,
                                                oal_wireless_dev_stru   *pst_wdev)
{
    /* TBD */
    return -OAL_EFAIL;
}



OAL_STATIC void wal_cfg80211_stop_p2p_device(oal_wiphy_stru       *pst_wiphy,
                                                oal_wireless_dev_stru   *pst_wdev)
{
    /* TBD */
}


static oal_int32 wal_cfg80211_set_power_mgmt(oal_wiphy_stru  *pst_wiphy,
				oal_net_device_stru *pst_netdev,
				bool enabled, oal_int32 timeout)
{
#ifdef _PRE_WLAN_FEATURE_STA_PM
    wal_msg_write_stru           st_write_msg;
    mac_cfg_ps_open_stru        *pst_sta_pm_open;
    oal_int32                    l_ret = 0;
    mac_vap_stru                *pst_mac_vap;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_netdev)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_cfg80211_set_power_mgmt::pst_wiphy or pst_wdev null ptrerror %d, %d!}\r\n",pst_wiphy, pst_netdev);
        return -OAL_EINVAL;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_STA_PM_ON, OAL_SIZEOF(mac_cfg_ps_open_stru));

#if 1
    pst_mac_vap    = OAL_NET_DEV_PRIV(pst_netdev);
    if (OAL_UNLIKELY(NULL == pst_mac_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "{wal_cfg80211_set_power_mgmt::get mac vap failed.}");
		return OAL_SUCC;
    }

    /* P2P dev不下发 */
    if (IS_P2P_DEV(pst_mac_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_PWR,"wal_cfg80211_set_power_mgmt:vap is p2p dev return");
        return OAL_SUCC;
    }

    OAM_WARNING_LOG3(0, OAM_SF_PWR, "{wal_cfg80211_set_power_mgmt::vap mode[%d]p2p mode[%d]set pm:[%d]}",pst_mac_vap->en_vap_mode,pst_mac_vap->en_p2p_mode,enabled);
#endif
    pst_sta_pm_open = (mac_cfg_ps_open_stru *)(st_write_msg.auc_value);
    /* MAC_STA_PM_SWITCH_ON / MAC_STA_PM_SWITCH_OFF */
    pst_sta_pm_open->uc_pm_enable      = enabled;
    pst_sta_pm_open->uc_pm_ctrl_type   = MAC_STA_PM_CTRL_TYPE_HOST;

    l_ret = wal_send_cfg_event_etc(pst_netdev,
                           WAL_MSG_TYPE_WRITE,
                           WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ps_open_stru),
                           (oal_uint8 *)&st_write_msg,
                           OAL_FALSE,
                           OAL_PTR_NULL);


    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_set_power_mgmt::fail to send pm cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }
#endif
    return OAL_SUCC;

}
#ifdef _PRE_WLAN_FEATURE_11R


OAL_STATIC oal_int32 wal_cfg80211_update_ft_ies(oal_wiphy_stru                  *pst_wiphy,
                                                         oal_net_device_stru             *pst_netdev,
                                                         oal_cfg80211_update_ft_ies_stru *pst_fties)
{
    wal_msg_write_stru                  st_write_msg;
    mac_cfg80211_ft_ies_stru           *pst_mac_ft_ies;
    wal_msg_stru                       *pst_rsp_msg;
    oal_uint32                          ul_err_code;
    oal_int32                           l_ret;

    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_netdev) || (OAL_PTR_NULL == pst_fties))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_update_ft_ies::param is null.}\r\n");

        return -OAL_EINVAL;
    }

    if ((pst_fties->ie == OAL_PTR_NULL) || (pst_fties->ie_len == 0) || (MAC_MAX_FTE_LEN <= pst_fties->ie_len))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_update_ft_ies::unexpect ie or len[%d].}\r\n", pst_fties->ie_len);

        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    st_write_msg.en_wid = WLAN_CFGID_SET_FT_IES;
    st_write_msg.us_len = OAL_SIZEOF(mac_cfg80211_ft_ies_stru);

    pst_mac_ft_ies = (mac_cfg80211_ft_ies_stru *)st_write_msg.auc_value;
    pst_mac_ft_ies->us_mdid = pst_fties->md;
    pst_mac_ft_ies->us_len  = pst_fties->ie_len;
    oal_memcopy(pst_mac_ft_ies->auc_ie, pst_fties->ie, pst_fties->ie_len);

    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg80211_ft_ies_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_update_ft_ies::wal_send_cfg_event_etc: return err code %d!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_update_ft_ies::wal_check_and_release_msg_resp_etc fail return err code:[%u]!}\r\n",
                        ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;


}
#endif //_PRE_WLAN_FEATURE_11R
#endif  /* _PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5 */
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))

oal_int32   wal_cfg80211_dump_survey_etc(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
                            oal_int32 l_idx, oal_survey_info_stru *pst_info)
{
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info_etc.bit_device_reset_process_flag)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfg80211_dump_survey_etc::dfr_process_status[%d]!}",
            g_st_dfr_info_etc.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif //#ifdef _PRE_WLAN_FEATURE_DFR
    return hmac_cfg80211_dump_survey_etc(pst_wiphy, pst_netdev, l_idx, pst_info);
}
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
OAL_STATIC oal_void wal_cfg80211_abort_scan(oal_wiphy_stru         *pst_wiphy,
                                            oal_wireless_dev_stru  *pst_wdev)
{
    oal_net_device_stru *pst_netdev;

    /* 1.1 入参检查 */
    if ((OAL_PTR_NULL == pst_wiphy) || (OAL_PTR_NULL == pst_wdev))
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{wal_cfg80211_abort_scan::wiphy or wdev is null, %p, %p!}\r\n",
                        pst_wiphy, pst_wdev);
        return;
    }

    pst_netdev = pst_wdev->netdev;
    if (OAL_PTR_NULL == pst_netdev)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_abort_scan::netdev is null!}\r\n");
        return;
    }
    OAM_WARNING_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_abort_scan::enter!}\r\n");
    wal_force_scan_complete_etc(pst_netdev, OAL_TRUE);
    return;
}
#endif

/* 不同操作系统函数指针结构体方式不同*/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC oal_cfg80211_ops_stru  g_wal_cfg80211_ops =
{
    .scan                     = wal_cfg80211_scan,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
    .connect                  = wal_cfg80211_connect,
    .disconnect               = wal_cfg80211_disconnect,
#endif
    .add_key                  = wal_cfg80211_add_key,
    .get_key                  = wal_cfg80211_get_key,
    .del_key                  = wal_cfg80211_remove_key,
    .set_default_key          = wal_cfg80211_set_default_key,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44)//TBD:确认正确的 Linux 版本号
    .set_default_mgmt_key     = wal_cfg80211_set_default_mgmt_key_etc,
#else
    .set_default_mgmt_key     = wal_cfg80211_set_default_key,
#endif
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34))
    .set_channel              = wal_cfg80211_set_channel,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
    .set_wiphy_params         = wal_cfg80211_set_wiphy_params,
#endif
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34))
    .add_beacon               = wal_cfg80211_add_beacon,
    .set_beacon               = wal_cfg80211_set_beacon,
    .del_beacon               = wal_cfg80211_del_beacon,
#else/* Hi1102 修改AP 配置接口 */
    .change_beacon            = wal_cfg80211_change_beacon,
    .start_ap                 = wal_cfg80211_start_ap,
    .stop_ap                  = wal_cfg80211_stop_ap,
    .change_bss               = wal_cfg80211_change_bss,
    .sched_scan_start         = wal_cfg80211_sched_scan_start,
	.sched_scan_stop          = wal_cfg80211_sched_scan_stop,
#endif
    .change_virtual_intf      = wal_cfg80211_change_virtual_intf_etc,
    .add_station              = wal_cfg80211_add_station,
    .del_station              = wal_cfg80211_del_station,
    .change_station           = wal_cfg80211_change_station,
    .get_station              = wal_cfg80211_get_station,
    .dump_station             = wal_cfg80211_dump_station,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
    .dump_survey              = wal_cfg80211_dump_survey_etc,
#endif
#if (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5) && (_PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_CPE)  //E5等hostapd适配后统一调试
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))//TBD:确认正确的 Linux 版本号
    .set_pmksa                = wal_cfg80211_set_pmksa,
    .del_pmksa                = wal_cfg80211_del_pmksa,
    .flush_pmksa              = wal_cfg80211_flush_pmksa,
    .remain_on_channel        = wal_cfg80211_remain_on_channel,
    .cancel_remain_on_channel = wal_cfg80211_cancel_remain_on_channel,
    .mgmt_tx                  = wal_cfg80211_mgmt_tx,
    .mgmt_frame_register      = wal_cfg80211_mgmt_frame_register_etc,
    .set_bitrate_mask         = wal_cfg80211_set_bitrate_mask_etc,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))//TBD:1102 需要，暂时用内核版本号区分
    .add_virtual_intf         = wal_cfg80211_add_virtual_intf,
    .del_virtual_intf         = wal_cfg80211_del_virtual_intf,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
    .mgmt_tx_cancel_wait      = wal_cfg80211_mgmt_tx_cancel_wait,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
    .start_p2p_device         = wal_cfg80211_start_p2p_device,
    .stop_p2p_device          = wal_cfg80211_stop_p2p_device,
    .set_power_mgmt           = wal_cfg80211_set_power_mgmt,
#ifdef _PRE_WLAN_FEATURE_11R
    .update_ft_ies            = wal_cfg80211_update_ft_ies,
#endif //_PRE_WLAN_FEATURE_11R
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
    .abort_scan               = wal_cfg80211_abort_scan,
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)) */
#endif /* _PRE_CONFIG_TARGET_PRODUCT != _PRE_TARGET_PRODUCT_TYPE_E5 */
};
#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
OAL_STATIC oal_cfg80211_ops_stru  g_wal_cfg80211_ops =
{
    wal_cfg80211_add_key,
    wal_cfg80211_get_key,
    wal_cfg80211_remove_key,
    wal_cfg80211_set_default_key,
    wal_cfg80211_set_default_key,
    wal_cfg80211_scan,
    wal_cfg80211_connect,
    wal_cfg80211_disconnect,
    wal_cfg80211_set_channel,
    wal_cfg80211_set_wiphy_params,
    wal_cfg80211_add_beacon,
    wal_cfg80211_change_virtual_intf_etc,
    wal_cfg80211_add_station,
    wal_cfg80211_del_station,
    wal_cfg80211_change_station,
    wal_cfg80211_get_station,
    wal_cfg80211_dump_station,
    wal_cfg80211_change_beacon,
    wal_cfg80211_start_ap,
    wal_cfg80211_stop_ap,
    wal_cfg80211_change_bss,
};

#endif


oal_void wal_cfg80211_reset_bands_etc(oal_uint8  uc_dev_id)
{
    int i;

    /* 每次更新国家码,flags都会被修改,且上次修改的值不会被清除,相当于每次修改的国家码都会生效,因此更新国家需要清除flag标志 */
    for(i = 0; i < g_st_supported_band_2ghz_info.n_channels; i++)
    {
        g_st_supported_band_2ghz_info.channels[i].flags = 0;
    }

    if(mac_device_check_5g_enable(uc_dev_id))
    {
        for(i = 0; i < g_st_supported_band_5ghz_info.n_channels; i++)
        {
            g_st_supported_band_5ghz_info.channels[i].flags = 0;
        }
    }
}


oal_void wal_cfg80211_save_bands_etc(oal_uint8  uc_dev_id)
{
    int i;

    /* 每次更新国家码,flags都会被修改,且上次修改的值不会被清除,
       相当于每次修改的国家码都会生效,因此更新国家需要清除flag标志，
       每次设置国家码flag 后，保存当前设置到orig_flags中
     */
    for(i = 0; i < g_st_supported_band_2ghz_info.n_channels; i++)
    {
        g_st_supported_band_2ghz_info.channels[i].orig_flags = g_st_supported_band_2ghz_info.channels[i].flags;
    }

    if(mac_device_check_5g_enable(uc_dev_id))
    {
        for(i = 0; i < g_st_supported_band_5ghz_info.n_channels; i++)
        {
            g_st_supported_band_5ghz_info.channels[i].orig_flags = g_st_supported_band_5ghz_info.channels[i].flags;
        }
    }
}


oal_uint32  wal_cfg80211_init_etc(oal_void)
{
    oal_uint32           ul_chip;
    oal_uint8            uc_device;
    oal_int32            l_return;
    oal_uint8            uc_dev_id;
    mac_device_stru     *pst_device;
    oal_uint32           ul_chip_max_num;
    mac_board_stru      *pst_hmac_board;
    oal_wiphy_stru      *pst_wiphy;
    mac_wiphy_priv_stru *pst_wiphy_priv;
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    oal_int8             ac_vap_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
#endif
#ifdef _PRE_WLAN_WEB_CMD_COMM
    oal_net_device_stru        *pst_net_dev;
    hmac_vap_stru              *pst_cfg_hmac_vap;
#endif
    hmac_board_get_instance_etc(&pst_hmac_board);

    ul_chip_max_num = oal_bus_get_chip_num_etc();

    for (ul_chip = 0; ul_chip < ul_chip_max_num; ul_chip++)
    {
        for (uc_device = 0; uc_device < pst_hmac_board->ast_chip[ul_chip].uc_device_nums; uc_device++)
        {
            /* 获取device_id */
            uc_dev_id = pst_hmac_board->ast_chip[ul_chip].auc_device_id[uc_device];

            pst_device = mac_res_get_dev_etc(uc_dev_id);

            if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_init_etc::mac_res_get_dev_etc,pst_dev null!}\r\n");
                return OAL_FAIL;
            }

            pst_device->pst_wiphy = oal_wiphy_new(&g_wal_cfg80211_ops, OAL_SIZEOF(mac_wiphy_priv_stru));

            if (OAL_PTR_NULL == pst_device->pst_wiphy)
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_init_etc::oal_wiphy_new failed!}\r\n");
                return OAL_FAIL;
            }

            /* 初始化wiphy 结构体内容 */
            pst_wiphy = pst_device->pst_wiphy;

#ifdef _PRE_WLAN_FEATURE_P2P
            pst_wiphy->interface_modes  = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP)
                                            |BIT(NL80211_IFTYPE_P2P_CLIENT)
                                            |BIT(NL80211_IFTYPE_P2P_GO)
                                            |BIT(NL80211_IFTYPE_P2P_DEVICE);
#else
            pst_wiphy->interface_modes  = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP);
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
#ifdef _PRE_WLAN_FEATURE_P2P
            pst_wiphy->iface_combinations   = sta_p2p_iface_combinations;
            pst_wiphy->n_iface_combinations = OAL_ARRAY_SIZE(sta_p2p_iface_combinations);
            pst_wiphy->mgmt_stypes          = wal_cfg80211_default_mgmt_stypes;
            pst_wiphy->max_remain_on_channel_duration = 5000;
            pst_wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL | WIPHY_FLAG_OFFCHAN_TX;
            pst_wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME; /* 设置GO 能力位 */

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
            /* 1102注册支持pno调度扫描能力相关信息 */
            pst_wiphy->max_sched_scan_ssids  = MAX_PNO_SSID_COUNT;
	        pst_wiphy->max_match_sets        = MAX_PNO_SSID_COUNT;
	        pst_wiphy->max_sched_scan_ie_len = WAL_MAX_SCAN_IE_LEN;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0))
            pst_wiphy->max_sched_scan_reqs  = 1;
#else
            pst_wiphy->flags |= WIPHY_FLAG_SUPPORTS_SCHED_SCAN;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44) */
#endif   /* _PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST */

#else    /* 非p2p场景下起多vap */
            pst_wiphy->mgmt_stypes          = wal_cfg80211_default_mgmt_stypes;
            pst_wiphy->iface_combinations   = ap_iface_combinations;
            pst_wiphy->n_iface_combinations = OAL_ARRAY_SIZE(ap_iface_combinations);

#endif
#endif   /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44) */
#endif   /* _PRE_OS_VERSION_LINUX == _PRE_OS_VERSION */

            pst_wiphy->max_scan_ssids             = WLAN_SCAN_REQ_MAX_SSID;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
            pst_wiphy->max_scan_ie_len            = WAL_MAX_SCAN_IE_LEN;
            pst_wiphy->cipher_suites              = g_ast_wlan_supported_cipher_suites;
            pst_wiphy->n_cipher_suites            = sizeof(g_ast_wlan_supported_cipher_suites)/sizeof(oal_uint32);

            /* 不使能节能 */
            pst_wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */

#if defined (_PRE_WLAN_FEATURE_ROAM) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
            /* wifi 驱动上报支持FW_ROAM,关联时(cfg80211_connect)候使用bssid_hint 替代bssid。*/
            pst_wiphy->flags |= WIPHY_FLAG_SUPPORTS_FW_ROAM;
#endif

#ifdef _PRE_WLAN_FEATURE_DFS_OFFLOAD
            pst_wiphy->flags |= 0x200;
#endif


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
            /* linux 3.14 版本升级，管制域重新修改 */
            pst_wiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
            /* 管制域配置 */
            pst_wiphy->flags |= WIPHY_FLAG_CUSTOM_REGULATORY;
#else
             /* linux-2.6.30  管制域配置 */
            pst_wiphy->custom_regulatory = true;
#endif

            pst_wiphy->bands[IEEE80211_BAND_2GHZ] = &g_st_supported_band_2ghz_info;        /* 支持的频带信息 2.4G */
            if(mac_device_check_5g_enable(uc_dev_id))
            {
                pst_wiphy->bands[IEEE80211_BAND_5GHZ] = &g_st_supported_band_5ghz_info;        /* 支持的频带信息 5G */
            }
            pst_wiphy->signal_type                = CFG80211_SIGNAL_TYPE_MBM;

            oal_wiphy_apply_custom_regulatory(pst_wiphy, &g_st_default_regdom_etc);

#if (defined(_PRE_PRODUCT_ID_HI110X_HOST) || (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            wal_cfgvendor_init_etc(pst_wiphy);
#endif
            OAL_IO_PRINT("wiphy_register start.\n");
            l_return = oal_wiphy_register(pst_wiphy);
            if(l_return != 0)
            {
                oal_wiphy_free(pst_device->pst_wiphy);
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_init_etc::oal_wiphy_register failed!}\r\n");
                return (oal_uint32)l_return;
            }

            /* P2P add_virtual_intf 传入wiphy 参数，在wiphy priv 指针保存wifi 驱动mac_devie_stru 结构指针 */
            pst_wiphy_priv  = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wiphy));
            pst_wiphy_priv->pst_mac_device = pst_device;

#ifdef _PRE_WLAN_WEB_CMD_COMM
            pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_device->uc_cfg_vap_id);
            if (OAL_PTR_NULL == pst_cfg_hmac_vap)
            {
                OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg80211_init_etc::pst_cfg_hmac_vap is null vap_id:%d!}\r\n",pst_device->uc_cfg_vap_id);
                OAL_IO_PRINT("wal_cfg80211_init_etc::Get cfg vap Hisilicon[%d] failed, cfg vap id is %d\n", uc_dev_id, pst_device->uc_cfg_vap_id);
                oal_wiphy_free(pst_device->pst_wiphy);
                return OAL_FAIL;
            }
            pst_net_dev = pst_cfg_hmac_vap->pst_net_device;
            if (OAL_PTR_NULL == pst_net_dev)
            {
                OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_cfg80211_init_etc::pst_net_dev of cfg vap is null ptr!}\r\n");
                OAL_IO_PRINT("wal_cfg80211_init_etc::Get cfg vap Hisilicon[%d] net dev failed, cfg vap id is %d\n", uc_dev_id, pst_device->uc_cfg_vap_id);
                oal_wiphy_free(pst_device->pst_wiphy);
                return OAL_FAIL;
            }
            pst_net_dev->wireless_handlers = &g_st_iw_cfg_handler_def;
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

            OAL_IO_PRINT("wal_init_wlan_netdev_etc wlan and p2p[%d].\n",uc_dev_id);

            /* 主路辅路的netdev规格:主路wlan0 p2p0,辅路wlan1 p2p1 */
            OAL_SPRINTF(ac_vap_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "wlan%d", uc_dev_id);
            l_return = wal_init_wlan_netdev_etc(pst_wiphy, ac_vap_netdev_name);
            if(OAL_SUCC != l_return)
            {
                OAL_IO_PRINT("wal_init_wlan_netdev_etc wlan[%d] failed.l_return:%d\n",uc_dev_id,l_return);
                return (oal_uint32)l_return;
            }

            OAL_SPRINTF(ac_vap_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "p2p%d", uc_dev_id);
            l_return = wal_init_wlan_netdev_etc(pst_wiphy, ac_vap_netdev_name);
            if(OAL_SUCC != l_return)
            {
                OAL_IO_PRINT("wal_init_wlan_netdev_etc p2p[%d] failed.l_return:%d\n",uc_dev_id,l_return);

                /* 释放wlan网络设备资源 */
                OAL_MEM_FREE(OAL_NETDEVICE_WDEV(pst_device->st_p2p_info.pst_primary_net_device), OAL_TRUE);
                oal_net_unregister_netdev(pst_device->st_p2p_info.pst_primary_net_device);
                return (oal_uint32)l_return;
            }
#if 0
            /*信道跟随-add*/
            OAL_SPRINTF(ac_vap_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, "hwlan%d", uc_dev_id);
            l_return = wal_init_wlan_netdev_etc(pst_wiphy, ac_vap_netdev_name);
            if(OAL_SUCC != l_return)
            {
                OAL_IO_PRINT("wal_init_wlan_netdev_etc hwlan[%d] failed.l_return:%d\n",uc_dev_id,l_return);

                OAL_MEM_FREE(OAL_NETDEVICE_WDEV(pst_device->st_p2p_info.pst_p2p_net_device), OAL_TRUE);
                oal_net_unregister_netdev(pst_device->st_p2p_info.pst_p2p_net_device);

                /* 释放wlan网络设备资源 */
                OAL_MEM_FREE(OAL_NETDEVICE_WDEV(pst_device->st_p2p_info.pst_primary_net_device), OAL_TRUE);
                oal_net_unregister_netdev(pst_device->st_p2p_info.pst_primary_net_device);
                return (oal_uint32)l_return;
            }
#endif
#endif
        }
    }

    return OAL_SUCC;
}


oal_void  wal_cfg80211_exit_etc(oal_void)
{
    oal_uint32          ul_chip;
    oal_uint8           uc_device;
    oal_uint8           uc_dev_id;
    mac_device_stru    *pst_device;
    oal_uint32          ul_chip_max_num;
    mac_board_stru     *pst_hmac_board;

    hmac_board_get_instance_etc(&pst_hmac_board);

    ul_chip_max_num = oal_bus_get_chip_num_etc(); /* 这个地方待确定 */

    for (ul_chip = 0; ul_chip < ul_chip_max_num; ul_chip++)
    {
        for (uc_device = 0; uc_device < pst_hmac_board->ast_chip[ul_chip].uc_device_nums; uc_device++)
        {
            /* 获取device_id */
            uc_dev_id = pst_hmac_board->ast_chip[ul_chip].auc_device_id[uc_device];

            pst_device = mac_res_get_dev_etc(uc_dev_id);

            if (OAL_PTR_NULL == pst_device)
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_init_etc::mac_res_get_dev_etc pst_device is null!}\r\n");
                return ;
            }
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST) || (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            wal_cfgvendor_deinit_etc(pst_device->pst_wiphy);
#endif

            /* 注销注册 wiphy device*/
            oal_wiphy_unregister(pst_device->pst_wiphy);

            /* 卸载wiphy device */
            oal_wiphy_free(pst_device->pst_wiphy);
        }
    }

    return;
}


oal_uint32  wal_cfg80211_init_evt_handle_etc(frw_event_mem_stru *pst_event_mem)
{

    wal_cfg80211_init_etc();
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_WEB_CMD_COMM

OAL_STATIC oal_int32 wal_cfg2io_fill_beacon_param(mac_vap_stru               *pst_mac_vap,
                                                           oal_beacon_data_stru        *pst_beacon_info,
                                                           mac_beacon_param_stru       *pst_beacon_param)
{
    oal_beacon_parameters st_beacon_info_tmp;
    oal_uint32  ul_ret;
    oal_uint8  *puc_beacon_info_tmp;
    oal_uint32  ul_beacon_head_len;
    oal_uint32  ul_beacon_tail_len;
    oal_uint32  ul_offset;
    oal_uint8   uc_vap_id;

    if (OAL_PTR_NULL == pst_mac_vap
       || OAL_PTR_NULL == pst_beacon_info
       || OAL_PTR_NULL == pst_beacon_param)
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{wal_cfg2io_fill_beacon_param::param is NULL. pst_mac_vap=0x%x, pst_beacon_info=0x%x, pst_beacon_param=0x%x",
           pst_mac_vap, pst_beacon_info, pst_beacon_param);
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;
    /*****************************************************************************
        1.安全配置ie消息等
    *****************************************************************************/
    if (OAL_PTR_NULL == pst_beacon_info->tail || OAL_PTR_NULL == pst_beacon_info->head)
    {
        OAM_ERROR_LOG2(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_fill_beacon_param::beacon frame error tail = %x, head = %x!}",
                       pst_beacon_info->tail, pst_beacon_info->head);
        return -OAL_EINVAL;
    }

    ul_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    /* oal_ieee80211_mgmt 前面公共部分size为 MAC_80211_FRAME_LEN(24)  */
    if (pst_beacon_info->head_len < (ul_offset + MAC_80211_FRAME_LEN))
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_cfg2io_fill_beacon_param::pst_beacon_info head_len[%d] error. }", pst_beacon_info->head_len);
        return -OAL_EINVAL;
    }

    ul_beacon_head_len = (oal_uint32)pst_beacon_info->head_len;
    ul_beacon_tail_len = (oal_uint32)pst_beacon_info->tail_len;
    puc_beacon_info_tmp = (oal_uint8 *)(OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,(oal_uint16)(ul_beacon_head_len + ul_beacon_tail_len),OAL_TRUE));
    if (OAL_PTR_NULL == puc_beacon_info_tmp)
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_fill_beacon_param::puc_beacon_info_tmp memalloc failed.}");
        return -OAL_EINVAL;
    }
    else
    {
        if(oal_copy_from_user(puc_beacon_info_tmp, pst_beacon_info->head, ul_beacon_head_len))
        {
            OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_fill_beacon_param::puc_beacon_info_tmp oal_copy_from_user failed.}");
            OAL_MEM_FREE(puc_beacon_info_tmp,OAL_TRUE);
            return -OAL_EINVAL;
        }

        if(oal_copy_from_user(puc_beacon_info_tmp + ul_beacon_head_len, pst_beacon_info->tail, ul_beacon_tail_len))
        {
            OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_fill_beacon_param::puc_beacon_info_tmp oal_copy_from_user failed.}");
            OAL_MEM_FREE(puc_beacon_info_tmp,OAL_TRUE);
            return -OAL_EINVAL;
        }
    }

    /* 为了复用51的解析接口，将新内核结构中的内容赋值给51接口识别的结构体，进而获取信息元素 */
    OAL_MEMZERO(&st_beacon_info_tmp, sizeof(st_beacon_info_tmp));
    st_beacon_info_tmp.head     = puc_beacon_info_tmp;
    st_beacon_info_tmp.head_len = (oal_int32)ul_beacon_head_len;
    st_beacon_info_tmp.tail     = puc_beacon_info_tmp + ul_beacon_head_len;
    st_beacon_info_tmp.tail_len = (oal_int32)ul_beacon_tail_len;

    /* 获取 WPA/WPA2 信息元素 */
    wal_parse_wpa_wpa2_ie_etc(&st_beacon_info_tmp, pst_beacon_param);

#ifdef _PRE_WLAN_FEATURE_11R_AP
    /* 获取 MD 信息元素 */
    ul_ret = wal_parse_md_ie(&st_beacon_info_tmp, pst_beacon_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_fill_beacon_param::failed to parse MD ie!}");
        OAL_MEM_FREE(puc_beacon_info_tmp,OAL_TRUE);
        return -OAL_EINVAL;
    }
#endif

    pst_beacon_param->en_shortgi_20 = mac_mib_get_ShortGIOptionInTwentyImplemented(pst_mac_vap);
    pst_beacon_param->en_shortgi_40 = mac_mib_get_ShortGIOptionInFortyImplemented(pst_mac_vap);
    pst_beacon_param->en_shortgi_80 = mac_mib_get_VHTShortGIOptionIn80Implemented(pst_mac_vap);

    /* 释放临时申请的内存 */
    OAL_MEM_FREE(puc_beacon_info_tmp,OAL_TRUE);

#ifdef _PRE_WLAN_FEATURE_11D
    /* 对日本14信道作特殊判断，只在11b模式下才能启用14，非11b模式 降为11b */
    if ((14 == pst_mac_vap->st_channel.uc_chan_number) && (WLAN_LEGACY_11B_MODE != pst_beacon_param->en_protocol))
    {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY,
                       "{wal_cfg2io_fill_beacon_param::ch 14 should in 11b, but is %d!}", pst_beacon_param->en_protocol);
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_fill_beacon_param::change protocol to 11b!}");
        pst_beacon_param->en_protocol = WLAN_LEGACY_11B_MODE;
    }
#endif

    return OAL_SUCC;
}

#if 0

OAL_STATIC oal_int32 wal_cfg2io_set_channel_info(oal_net_device_stru *pst_netdev, mac_cfg_channel_param_stru *pst_chan)
{
    mac_cfg_channel_param_stru          *pst_channel_param;
    wal_msg_write_stru                   st_write_msg;
    oal_uint32                           ul_err_code;
    wal_msg_stru                        *pst_rsp_msg = OAL_PTR_NULL;
    oal_int32                            l_ret;


    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/

    /* 填写消息 */
    pst_channel_param = (mac_cfg_channel_param_stru *)(st_write_msg.auc_value);
    pst_channel_param->en_band = pst_chan->en_band;
    pst_channel_param->en_bandwidth = pst_chan->en_bandwidth;
    pst_channel_param->uc_channel = pst_chan->uc_channel;

    OAM_WARNING_LOG3(0, OAM_SF_CHAN, "{wal_cfg2io_set_channel_info::uc_channel = %d, en_band = %d, en_bandwidth = %d.}",
                     pst_channel_param->uc_channel,
                     pst_channel_param->en_band,
                     pst_channel_param->en_bandwidth);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_SET_CHANNEL, OAL_SIZEOF(mac_cfg_channel_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_channel_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);

    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CHAN, "{wal_cfg2io_set_channel_info::wal_send_cfg_event_etc return err code: [%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    /* 读取返回的错误码 */
    ul_err_code = wal_check_and_release_msg_resp_etc(pst_rsp_msg);
    if(OAL_SUCC != ul_err_code)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CHAN, "{wal_cfg2io_set_channel_info::wal_send_cfg_event_etc return err code: [%u].}",
                       ul_err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif

oal_int32 wal_cfg2io_set_beacon(
                oal_net_device_stru   *pst_dev,
                struct wlan_ap_settings *pst_ap_settings)
{
    mac_beacon_param_stru        st_beacon_param;  /* beacon info struct */
    wal_msg_write_stru           st_write_msg;
    mac_vap_stru                *pst_mac_vap;
    oal_beacon_data_stru        *pst_beacon_info;
    oal_int32                    l_ret = 0;
    oal_uint8                    uc_vap_id;
    wlan_auth_alg_mode_enum_uint8     en_auth_algs;
    mac_cfg_channel_param_stru        st_channel_param;
    #if 0
    oal_uint8                    uc_ssid_len;
    oal_uint8                   *puc_ssid_ie;
    oal_uint8                    auc_ssid_ie[32];
    oal_uint32                   ul_ssid_len = 0;
    #endif

    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == pst_dev) || (OAL_PTR_NULL == pst_ap_settings))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{wal_cfg2io_set_beacon::pst_netdev = %x, pst_ap_settings = %x!}",
                       pst_dev, pst_ap_settings);
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    pst_mac_vap = OAL_NET_DEV_PRIV(pst_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfg2io_set_beacon::pst_mac_vap = %x}", pst_mac_vap);
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;
    oal_memset(&st_beacon_param, 0, sizeof(mac_beacon_param_stru));

    /*****************************************************************************
        1.设置信道
    *****************************************************************************/
    l_ret = wal_io2cfg_get_chan_info(pst_dev, &st_channel_param, &st_beacon_param);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_set_beacon::failed to get channel, return err code[%x]}", l_ret);
        return -OAL_EFAIL;
    }
#if 0
    l_ret = wal_cfg2io_set_channel_info(pst_dev, &st_channel_param);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_set_beacon::failed to set channel, return err code[%x]}", l_ret);
        return -OAL_EFAIL;
    }
#endif
    /*****************************************************************************
        2.1 设置ssid信息
    *****************************************************************************/
    #if 0
    ul_ssid_len = pst_ap_settings->ssid_len;
    if ((ul_ssid_len > 32) || (0 == ul_ssid_len))
    {
         OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_set_beacon::ssid len error, len[%d].}", ul_ssid_len);
         return -OAL_EFAIL;
    }
    oal_memset(auc_ssid_ie, 0, sizeof(auc_ssid_ie));
    l_ret = (oal_int32)oal_copy_from_user(auc_ssid_ie, pst_ap_settings->ssid, ul_ssid_len);
    if(l_ret)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_set_beacon::ssid oal_copy_from_user fail(%d).}", l_ret);
         return -OAL_EFAIL;
    }
    puc_ssid_ie = auc_ssid_ie;
    uc_ssid_len = (oal_uint8)ul_ssid_len;

    l_ret = wal_cfg80211_set_ssid(pst_dev, puc_ssid_ie, uc_ssid_len);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_set_beacon::fail to send ssid cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }
    #endif

    /*****************************************************************************
        2.2 设置auth mode信息
    *****************************************************************************/
    en_auth_algs = (wlan_auth_alg_mode_enum_uint8)pst_ap_settings->auth_type;
    if((WLAN_WITP_AUTH_OPEN_SYSTEM != en_auth_algs)
    && (WLAN_WITP_AUTH_SHARED_KEY != en_auth_algs)
    && (WLAN_WITP_AUTH_AUTOMATIC != en_auth_algs))
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_set_beacon::en_auth_algs error[%d].}", en_auth_algs);
        return -OAL_EFAIL;
    }

    l_ret = wal_cfg80211_set_auth_mode(pst_dev, en_auth_algs);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_set_beacon::fail to send auth_tpye cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        3.设置beacon时间间隔、tim period以及安全配置消息等
    *****************************************************************************/

    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    st_beacon_param.l_interval    = 0;
    st_beacon_param.l_dtim_period = 0;
    st_beacon_param.uc_hidden_ssid = pst_mac_vap->st_cap_flag.bit_hide_ssid;
    pst_beacon_info               = &(pst_ap_settings->beacon);
    l_ret = wal_cfg2io_fill_beacon_param(pst_mac_vap, pst_beacon_info, &st_beacon_param);
    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_set_beacon::failed to fill beacon param, error[%d]}", l_ret);
        return -OAL_EINVAL;
    }

    /*设置操作类型*/
    st_beacon_param.en_operation_type = MAC_ADD_BEACON;

    /* 填写 msg 消息头*/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, OAL_SIZEOF(mac_beacon_param_stru));

    /* 填写 msg 消息体 */
    oal_memcopy(st_write_msg.auc_value, &st_beacon_param, OAL_SIZEOF(mac_beacon_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event_etc(pst_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_beacon_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{wal_cfg2io_set_beacon::failed to start addset beacon, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_M2S

oal_uint32 wal_cfg80211_m2s_status_report(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                      *pst_event;
    dmac_m2s_complete_syn_stru          *pst_m2s_switch_comp_status;
    wlan_m2s_mgr_vap_stru               *pst_m2s_vap_mgr;
    hmac_vap_stru                       *pst_hmac_vap;
    oal_uint8                            uc_vap_idx;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_M2S, "{wal_cfg80211_m2s_status_report::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event  = (frw_event_stru *)pst_event_mem->puc_data;

    /* 获取配置vap */
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if(OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_M2S, "{wal_cfg80211_m2s_status_report::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_m2s_switch_comp_status  = (dmac_m2s_complete_syn_stru *)(pst_event->auc_event_data);

    /* 需要上层提供oal接口，TBD */
    OAM_WARNING_LOG4(0, OAM_SF_M2S, "{wal_cfg80211_m2s_status_report::vap num[%d]device result[%d]state[%d]m2s_mode[%d].}",
        pst_m2s_switch_comp_status->pri_data.mss_result.uc_vap_num, pst_m2s_switch_comp_status->pri_data.mss_result.en_m2s_result,
        pst_m2s_switch_comp_status->uc_m2s_state, pst_m2s_switch_comp_status->pri_data.mss_result.uc_m2s_mode);

    for (uc_vap_idx = 0; uc_vap_idx < pst_m2s_switch_comp_status->pri_data.mss_result.uc_vap_num; uc_vap_idx++)
    {
        pst_m2s_vap_mgr = &(pst_m2s_switch_comp_status->ast_m2s_comp_vap[uc_vap_idx]);

        OAM_WARNING_LOG3(0, OAM_SF_M2S,
            "{wal_cfg80211_m2s_status_report::Notify to host, ap index[%d]action type[%d]state[%d].}",
                uc_vap_idx, pst_m2s_vap_mgr->en_action_type, pst_m2s_vap_mgr->en_m2s_result);

        OAM_WARNING_LOG4(0, OAM_SF_M2S,
            "{wal_cfg80211_m2s_status_report::Notify to host, ap addr->%02x:XX:XX:%02x:%02x:%02x.}",
            pst_m2s_vap_mgr->auc_user_mac_addr[0], pst_m2s_vap_mgr->auc_user_mac_addr[3],
            pst_m2s_vap_mgr->auc_user_mac_addr[4], pst_m2s_vap_mgr->auc_user_mac_addr[5]);
    }

    /* 上报内核 */
    oal_cfg80211_m2s_status_report(pst_hmac_vap->pst_net_device, GFP_KERNEL,
        (oal_uint8 *)pst_m2s_switch_comp_status, OAL_SIZEOF(dmac_m2s_complete_syn_stru));

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH

oal_uint32 wal_cfg80211_tas_rssi_access_report(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                      *pst_event;
    hmac_vap_stru                       *pst_hmac_vap;
    oal_uint8                            uc_vap_idx;
    dmac_tas_rssi_notify_stru           *pst_tas_rssi_comp_status;
    oal_int32                            l_tas_state;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfg80211_tas_rssi_access_report::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event  = (frw_event_stru *)pst_event_mem->puc_data;

    /* 获取配置vap */
    uc_vap_idx   = pst_event->st_event_hdr.uc_vap_id;
    pst_hmac_vap = mac_res_get_hmac_vap(uc_vap_idx);
    if(OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(uc_vap_idx, OAM_SF_ANY, "{wal_cfg80211_tas_rssi_access_report::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_tas_rssi_comp_status  = (dmac_tas_rssi_notify_stru *)(pst_event->auc_event_data);

    l_tas_state = board_get_wifi_tas_gpio_state();
    OAM_WARNING_LOG3(uc_vap_idx, OAM_SF_ANY, "{wal_cfg80211_tas_rssi_access_report::core[%d] c_ant%d_rssi[%d].}",
                     pst_tas_rssi_comp_status->l_core_idx, l_tas_state, pst_tas_rssi_comp_status->l_rssi);

    /* 上报内核 */
    oal_cfg80211_tas_rssi_access_report(pst_hmac_vap->pst_net_device, GFP_KERNEL, (oal_uint8 *)pst_tas_rssi_comp_status,
                                        OAL_SIZEOF(dmac_tas_rssi_notify_stru));

    return OAL_SUCC;
}
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

