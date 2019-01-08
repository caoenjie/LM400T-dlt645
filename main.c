/*******************************************************************************
*                                 AMetal
*                       ----------------------------
*                       innovating embedded platform
*
* Copyright (c) 2001-2018 Guangzhou ZHIYUAN Electronics Co., Ltd.
* All rights reserved.
*
* Contact information:
* web site:    http://www.zlg.cn/
*******************************************************************************/

/**
 * \file
 * \brief
 *
 * \internal
 * \par Modification history
 * - 1.00 15-10-21  zsp, first implementation.
 * \endinternal
 */

/**
 * \addtogroup
 * \copydoc
 */

#include "ametal.h"
#include "am_board.h"
#include "am_vdebug.h"
#include "am_delay.h"
#include "am_gpio.h"
#include "demo_all_entries.h"
//#include "meter.h"
#include "dlt645.h"

int am_main()
{
    AM_DBG_INFO("Start up successful!\r\n");

//    demo_lorawan_class_ac_otaa_entry();
//    demo_read_meter();
//    demo_kl26_std_uart_polling_entry();
//    demo_kl26_std_uart_ringbuf_entry();
//    MeterData_Get();
    meter_test();



    while (1) {

        am_led_toggle(LED_1);

        am_mdelay(300);
    }
}

/* end of file */
