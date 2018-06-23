////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2011 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

//! @file mdrv_rqct_st.h
//! @author MStar Semiconductor Inc.
//! @brief VHE Driver IOCTL User's Interface.
//! \ingroup rqct_group VHE driver
//! @{
#ifndef _MDRV_RQCT_ST_H_
#define _MDRV_RQCT_ST_H_

#define RQCTIF_MAJ              1   //!< major version: Major number of driver-I/F version.
#define RQCTIF_MIN              1   //!< minor version: Minor number of driver-I/F version.
#define RQCTIF_EXT              1   //!< extended code: Extended number of version. It should increase when "mdrv_rqct_io.h/mdrv_rqct_st.h" changed.

//! RQCT Interface version number.
#define RQCTIF_VERSION_ID       ((RQCTIF_MAJ<<22)|(RQCTIF_MIN<<12)|(RQCTIF_EXT))
//! Acquire version number.
#define RQCTIF_GET_VER(v)       (((v)>>12))
//! Acquire major version number.
#define RQCTIF_GET_MJR(v)       (((v)>>22)&0x3FF)
//! Acquire minor version number.
#define RQCTIF_GET_MNR(v)       (((v)>>12)&0x3FF)
//! Acquire extended number.
#define RQCTIF_GET_EXT(v)       (((v)>> 0)&0xFFF)

//! rqct-method : support 3 types - CQP, CBR, VBR.
enum rqct_method
{
    RQCT_METHOD_CQP = 0,    //!< constant QP.
    RQCT_METHOD_CBR,        //!< constant bitrate.
    RQCT_METHOD_VBR,        //!< variable bitrate.
};

//! mvhe_superfrm_mode indicates super frame mechanism
enum rqct_superfrm_mode
{
    RQCT_SUPERFRM_NONE = 0, //!< super frame mode none.
    RQCT_SUPERFRM_DISCARD,  //!< super frame mode discard.
    RQCT_SUPERFRM_REENCODE, //!< super frame mode reencode.
};

//! rqct_conf is used to apply/query rq-configs during streaming period.
typedef union rqct_conf
{
    //! rqct-config type.
    enum rqct_conf_e
    {
        RQCT_CONF_SEQ = 0,  //!< set sequence rate-control.
        RQCT_CONF_DQP,      //!< set delta-QP between I/P.
        RQCT_CONF_QPR,      //!< set QP range.
        RQCT_CONF_LOG,      //!< turn on/off rqct log message.
        RQCT_CONF_PEN,      //!< penalties for mhe.
        RQCT_CONF_SPF,      //!< super frame settings.
        RQCT_CONF_LTR,      //!< long term reference setting
        RQCT_CONF_END,      //!< end of rqct-conf-enum.
    } type;             //!< indicating config. type.

    //! set rqct seq. config.
    struct _seq
    {
        enum rqct_conf_e    i_type;     //!< i_type MUST be RQCT_CFG_SEQ.
        enum rqct_method    i_method;   //!< indicating rqct-method.
        int                 i_period;   //!< ip-period.
        int                 i_leadqp;   //!< leadqp.
        int                 i_btrate;   //!< btrate.
        int                 b_passiveI; //!< Passive encode I frame.
    } seq;          //!< rqct configs of seq. setting.

    //! set rqct lt config
    struct _ltr
    {
        enum rqct_conf_e    i_type;         //!< i_type MUST be RQCT_CONF_LTR.
        int                 i_period;       //!< ltr period
    } ltr;

    //! set rqct dqp. config.
    struct _dqp
    {
        enum rqct_conf_e    i_type;     //!< i_type MUST be RQCT_CFG_DQP.
        int                 i_dqp;      //!< dif-qp between I/P.
    } dqp;          //!< rqct configs of dqp. setting.

    //! set rqct qpr. config.
    struct _qpr
    {
        enum rqct_conf_e    i_type;     //!< i_type MUST be RQCT_CFG_QPR.
        int                 i_iupperq;  //!< I frame upperq.
        int                 i_ilowerq;  //!< I frame lowerq.
        int                 i_pupperq;  //!< P frame upperq.
        int                 i_plowerq;  //!< P frame lowerq.
    } qpr;          //!< rqct configs of dqp. setting.

    //! set rqct log. config.
    struct _log
    {
        enum rqct_conf_e    i_type;     //!< i_type MUST be RQCT_CFG_LOG.
        int                 b_logm;     //!< switch of log-message.
    } log;          //!< rqct configs of dqp. setting.

    //! set rqct pen. config.
    struct _pen
    {
        enum rqct_conf_e    i_type;     //!< i_type MUST be RQCT_CFG_PEN.
        short               b_ia8xlose;   //!< Avoid CU8 intra to be chosen
        short               b_ir8xlose;   //!< Avoid CU8 AMVP to be chosen
        short               b_ia16lose;   //!< Avoid CU16 intra to be chosen
        short               b_ir16lose;   //!< Avoid CU16 AMVP to be chosen
        short               b_ir16mlos;   //!< Avoid CU16 Merge to be chosen
        short               b_ir16slos;   //!< Avoid CU16 AMVP no-coef to be chosen. forcing coef to be all 0's
        short               b_ir16mslos;  //!< Avoid CU16 Merge no-coef to be chosen. forcing coef to be all 0's
        short               b_ia32lose;   //!< Avoid CU32 intra to be chosen
        short               b_ir32mlos;   //!< Avoid CU32 Merge to be chosen
        short               b_ir32mslos;  //!< Avoid CU32 Merge no-coef to be chosen. forcing coef to be all 0's

        unsigned short int  u_ia8xpen;    //!< CU8 intra. Must >=0. Larger value means less preferred.
        unsigned short int  u_ir8xpen;    //!< CU8 AMVP. Must >=0. Larger value means less preferred
        unsigned short int  u_ia16pen;    //!< CU16 intra. Must >=0. Larger value means less preferred
        unsigned short int  u_ir16pen;    //!< CU16 AMVP. Must >=0. Larger value means less preferred
        unsigned short int  u_ir16mpen;   //!< CU16 Merge. Must >=0. Larger value means less preferred
        unsigned short int  u_ir16spen;   //!< CU16 AMVP no-coef. Must >=0. Larger value means less preferred. forcing coef to be all 0's
        unsigned short int  u_ir16mspen;  //!< CU16 Merge no-coef. Must >=0. Larger value means less preferred. forcing coef to be all 0's
        unsigned short int  u_ia32pen;    //!< CU32 intra. Must >=0. Larger value means less preferred
        unsigned short int  u_ir32mpen;   //!< CU32 Merge. Must >=0. Larger value means less preferred
        unsigned short int  u_ir32mspen;  //!< CU32 Merge no-coef. Must >=0. Larger value means less preferred., forcing coef to be all 0's

    } pen;          //!< rqct configs of dqp. setting.

    struct _spf
    {
        enum rqct_conf_e    i_type;     //!< i_type MUST be MVHE_PARM_SPF.
        enum rqct_superfrm_mode    i_spfrm;
        int  i_IFrmBitsThr;
        int  i_PFrmBitsThr;
        int  i_BFrmBitsThr;
    } spf;          //!< used to set super frame skip mode configuration.
} rqct_conf;

#endif //_MDRV_RQCT_ST_H_
//! @}
