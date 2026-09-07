#ifndef IRQ_NUMBER_H
#define IRQ_NUMBER_H
// clang-format off
typedef enum{
    SysTimerSW_IRQn          = 3,                 /*!< System Timer SW interrupt */

    SOC_TRNG_IRQn            = 32,                /*!< Device Interrupt */
    SOC_INT33_IRQn           = 33,                /*!< Device Interrupt */
    SOC_HASH0_IRQn           = 34,                /*!< Device Interrupt */
    SOC_HASH1_IRQn           = 35,                /*!< Device Interrupt */
    SOC_SKE0_IRQn            = 36,                /*!< Device Interrupt */
    SOC_SKE1_IRQn            = 37,                /*!< Device Interrupt */
    SOC_PKE_IRQn             = 38,                /*!< Device Interrupt */
    SOC_INT39_IRQn           = 39,                /*!< Device Interrupt */
    SOC_CHACHA_IRQn          = 40,                /*!< Device Interrupt */
    SOC_INT41_IRQn           = 41,                /*!< Device Interrupt */
    SOC_WDT_IRQn             = 42,                /*!< Device Interrupt */
    SOC_WDT1_IRQn            = 43,                /*!< Device Interrupt */
    SOC_INT44_IRQn           = 44,                /*!< Device Interrupt */
    SOC_UART_IRQn            = 45,                /*!< Device Interrupt */
    SOC_INT46_IRQn           = 46,                /*!< Device Interrupt */
    SOC_TIMER0_IRQn          = 47,                /*!< Device Interrupt */
    SOC_TIMER1_IRQn          = 48,                /*!< Device Interrupt */
    SOC_INT49_IRQn           = 49,                /*!< Device Interrupt */
    SOC_IPATCH_IRQn          = 50,                /*!< Device Interrupt */
    SOC_INT51_IRQn           = 51,                /*!< Device Interrupt */
    SOC_UTC_TIMER_IRQn       = 52,                /*!< Device Interrupt */
    SOC_INT53_IRQn           = 53,                /*!< Device Interrupt */
    SOC_MON_COUNTER_IRQn     = 54,                /*!< Device Interrupt */
    SOC_INT55_IRQn           = 55,                /*!< Device Interrupt */
    SOC_SYSTEM_IRQn          = 56,                /*!< Device Interrupt */
    SOC_INT57_IRQn           = 57,                /*!< Device Interrupt */
    SOC_MAILBOX_IRQn         = 58,                /*!< Device Interrupt */
    SOC_INT59_IRQn           = 59,                /*!< Device Interrupt */
    SOC_EMU_IRQn             = 60,                /*!< Device Interrupt */
    SOC_INT61_IRQn           = 61,                /*!< Device Interrupt */
    SOC_INT62_IRQn           = 62,                /*!< Device Interrupt */
    SOC_INT63_IRQn           = 63,                /*!< Device Interrupt */
    SOC_INT64_IRQn           = 64,                /*!< Device Interrupt */
    SOC_INT65_IRQn           = 65,                /*!< Device Interrupt */
    SOC_INT66_IRQn           = 66,                /*!< Device Interrupt */
    SOC_INT67_IRQn           = 67,                /*!< Device Interrupt */
    SOC_INT68_IRQn           = 68,                /*!< Device Interrupt */
    SOC_INT69_IRQn           = 69,                /*!< Device Interrupt */
    SOC_INT70_IRQn           = 70,                /*!< Device Interrupt */
    SOC_INT71_IRQn           = 71,                /*!< Device Interrupt */
    SOC_INT72_IRQn           = 72,                /*!< Device Interrupt */
    SOC_INT73_IRQn           = 73,                /*!< Device Interrupt */
    SOC_INT74_IRQn           = 74,                /*!< Device Interrupt */
    SOC_INT75_IRQn           = 75,                /*!< Device Interrupt */
    SOC_INT76_IRQn           = 76,                /*!< Device Interrupt */
    SOC_INT77_IRQn           = 77,                /*!< Device Interrupt */
    SOC_INT78_IRQn           = 78,                /*!< Device Interrupt */
    SOC_INT79_IRQn           = 79,                /*!< Device Interrupt */
    SOC_INT80_IRQn           = 80,                /*!< Device Interrupt */
    SOC_INT81_IRQn           = 81,                /*!< Device Interrupt */
    SOC_INT82_IRQn           = 82,                /*!< Device Interrupt */
    SOC_INT83_IRQn           = 83,                /*!< Device Interrupt */
    SOC_INT84_IRQn           = 84,                /*!< Device Interrupt */
    SOC_INT85_IRQn           = 85,                /*!< Device Interrupt */
    SOC_INT86_IRQn           = 86,                /*!< Device Interrupt */
    SOC_INT87_IRQn           = 87,                /*!< Device Interrupt */
    SOC_INT88_IRQn           = 88,                /*!< Device Interrupt */
    SOC_INT89_IRQn           = 89,                /*!< Device Interrupt */
    SOC_INT90_IRQn           = 90,                /*!< Device Interrupt */
    SOC_INT91_IRQn	         = 91,                /*!< Device Interrupt */
    SOC_INT92_IRQn           = 92,                /*!< Device Interrupt */
    SOC_INT_NUM,
} irq_nmber_e;
// clang-format on
#endif /* IRQ_NUMBER_H */
