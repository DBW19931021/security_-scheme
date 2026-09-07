#ifndef IRQ_NUMBER_H
#define IRQ_NUMBER_H
typedef enum irq_nmber{
    SysTimerSW_IRQn                 = 3,                 /*!< System Timer SW interrupt */

    SOC_UART0_RX_IRQn               = 32,                /*!< Device Interrupt */
    SOC_UART0_TX_IRQn               = 33,                /*!< Device Interrupt */
    SOC_UART1_RX_IRQn               = 34,                /*!< Device Interrupt */
    SOC_UART1_TX_IRQn               = 35,                /*!< Device Interrupt */
    SOC_UART2_RX_IRQn               = 36,                /*!< Device Interrupt */
    SOC_UART2_TX_IRQn               = 37,                /*!< Device Interrupt */
    SOC_INT38_IRQn                  = 38,                /*!< Device Interrupt */
    SOC_INT39_IRQn                  = 39,                /*!< Device Interrupt */
    SOC_TIMER0_IRQn                 = 40,                /*!< Device Interrupt */
    SOC_SPI0_IRQn                   = 41,                /*!< Device Interrupt */
    SOC_DUALTIMER_IRQn              = 42,                /*!< Device Interrupt */
    SOC_INT43_IRQn                  = 43,                /*!< Device Interrupt */
    SOC_UART0_OW_IRQn               = 44,                /*!< Device Interrupt */
    SOC_UART1_OW_IRQn               = 45,                /*!< Device Interrupt */
    SOC_UART2_OW_IRQn               = 46,                /*!< Device Interrupt */
    SOC_INT47_IRQn                  = 47,                /*!< Device Interrupt */
    SOC_INT48_IRQn                  = 48,                /*!< Device Interrupt */
    SOC_INT49_IRQn                  = 49,                /*!< Device Interrupt */
    SOC_INT50_IRQn                  = 50,                /*!< Device Interrupt */
    SOC_INT51_IRQn                  = 51,                /*!< Device Interrupt */
    SOC_INT52_IRQn                  = 52,                /*!< Device Interrupt */
    SOC_INT53_IRQn                  = 53,                /*!< Device Interrupt */
    SOC_INT54_IRQn                  = 54,                /*!< Device Interrupt */
    SOC_INT55_IRQn                  = 55,                /*!< Device Interrupt */
    SOC_INT56_IRQn                  = 56,                /*!< Device Interrupt */
    SOC_INT57_IRQn                  = 57,                /*!< Device Interrupt */
    SOC_INT58_IRQn                  = 58,                /*!< Device Interrupt */
    SOC_INT59_IRQn                  = 59,                /*!< Device Interrupt */
    SOC_INT60_IRQn                  = 60,                /*!< Device Interrupt */
    SOC_INT61_IRQn                  = 61,                /*!< Device Interrupt */
    SOC_INT62_IRQn                  = 62,                /*!< Device Interrupt */
    SOC_INT63_IRQn                  = 63,                /*!< Device Interrupt */
    SOC_MAILBOX0_IRQn               = 64,                /*!< Device Interrupt */
    SOC_MAILBOX1_IRQn               = 65,                /*!< Device Interrupt */
    SOC_INT66_IRQn                  = 66,                /*!< Device Interrupt */
    SOC_INT67_IRQn                  = 67,                /*!< Device Interrupt */
    SOC_INT68_IRQn                  = 68,                /*!< Device Interrupt */
    SOC_INT69_IRQn                  = 69,                /*!< Device Interrupt */
    SOC_INT70_IRQn                  = 70,                /*!< Device Interrupt */
    SOC_INT71_IRQn                  = 71,                /*!< Device Interrupt */
    SOC_INT72_IRQn                  = 72,                /*!< Device Interrupt */
    SOC_INT73_IRQn                  = 73,                /*!< Device Interrupt */
    SOC_INT74_IRQn                  = 74,                /*!< Device Interrupt */
    SOC_INT75_IRQn                  = 75,                /*!< Device Interrupt */
    SOC_INT76_IRQn                  = 76,                /*!< Device Interrupt */
    SOC_INT77_IRQn                  = 77,                /*!< Device Interrupt */
    SOC_INT78_IRQn                  = 78,                /*!< Device Interrupt */
    SOC_INT79_IRQn                  = 79,                /*!< Device Interrupt */
    SOC_INT80_IRQn                  = 80,                /*!< Device Interrupt */
    SOC_INT81_IRQn                  = 81,                /*!< Device Interrupt */
    SOC_INT82_IRQn                  = 82,                /*!< Device Interrupt */
    SOC_INT83_IRQn                  = 83,                /*!< Device Interrupt */
    SOC_INT84_IRQn                  = 84,                /*!< Device Interrupt */
    SOC_INT85_IRQn                  = 85,                /*!< Device Interrupt */
    SOC_INT86_IRQn                  = 86,                /*!< Device Interrupt */
    SOC_INT87_IRQn                  = 87,                /*!< Device Interrupt */
    SOC_INT88_IRQn                  = 88,                /*!< Device Interrupt */
    SOC_INT89_IRQn                  = 89,                /*!< Device Interrupt */
    SOC_INT90_IRQn                  = 90,                /*!< Device Interrupt */
    SOC_INT91_IRQn	                = 91,                /*!< Device Interrupt */
    SOC_INT92_IRQn                  = 92,                /*!< Device Interrupt */
    SOC_INT_NUM,
} irq_nmber_e;
#endif /* IRQ_NUMBER_H */