/*! ----------------------------------------------------------------------------
 *  @file    ex_02f_rx_with_xtal_trim.c
 *  @brief   RX with Crystal Trim example code
 *
 *  This is a simple code example of a receiver that measures the clock offset of a remote transmitter
 *  and then uses the XTAL trimming function to modify the local clock to achieve a target clock offset.
 *  Note: To keep a system stable it is recommended to only adjust trimming at one end of a link.
 *
 *  @attention
 *
 *  Copyright 2017 (c) Decawave Ltd, Dublin, Ireland.
 *
 *  All rights reserved.
 *
 *  @author Decawave
 */
#ifdef EX_02F_DEF
#include "deca_device_api.h"
#include "deca_regs.h"
#include "lcd.h"
#include "deca_spi.h"
#include "port.h"

#include "stdlib.h"


/* Example application name and version to display on LCD screen. */
#define APP_NAME "RX TRIM v1.0"

/* Default communication configuration. We use here EVK1000's default mode (mode 3). */
static const dwt_config_t cfg = {
    .chan           = 2,               /* Channel number. */
    .prf            = DWT_PRF_64M,     /* Pulse repetition frequency. */
    .txPreambLength = DWT_PLEN_1024,   /* Preamble length. Used in TX only. */
    .rxPAC          = DWT_PAC32,       /* Preamble acquisition chunk size. Used in RX only. */
    .txCode         = 9,               /* TX preamble code. Used in TX only. */
    .rxCode         = 9,               /* RX preamble code. Used in RX only. */
    .nsSFD          = 1,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
    .dataRate       = DWT_BR_110K,     /* Data rate. */
    .phrMode        = DWT_PHRMODE_STD, /* PHY header mode. */
    .sfdTO          = (1025 + 64 - 32) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};

/* Default configuration for given UWB settings */
static float freqMultiplier = FREQ_OFFSET_MULTIPLIER_110KB;    /* Frequency Multiplier         : Depends on .dataRate in the communication configuration */
static float hzMultiplier   = HERTZ_TO_PPM_MULTIPLIER_CHAN_2;  /* Hz to PPM transfer Multiplier: Depends on .chan in the communication configuration */


/* Buffer to store received frame. See NOTE 1 below. */
#define FRAME_LEN_MAX 127

/* Holds a Current Crystal Trimming Value, so that it can be examined at a debug breakpoint. */
static uint8 uCurrentTrim_val;

/*
 * In this example, the crystal on the receiver will be trimmed to have a fixed offset with respect to the transmitter's crystal
 * not less than TARGET_XTAL_OFFSET_VALUE_PPM_MIN and not more than TARGET_XTAL_OFFSET_VALUE_PPM_MAX
 *
 * Note. For correct operation of the code, the min and max TARGET_XTAL_OFFSET_VALUE constants specified below should use positive numbers,
 * and the separation between min and max needs to be bigger than the trimming resolution, (which is approx 1.5 PPM).
 * We recommend that (max-min >= 2).
 *
 * */
# define TARGET_XTAL_OFFSET_VALUE_PPM_MIN    (2.0f)
# define TARGET_XTAL_OFFSET_VALUE_PPM_MAX    (4.0f)

/* The FS_XTALT_MAX_VAL defined the maximum value of the trimming value */
#define FS_XTALT_MAX_VAL                    (FS_XTALT_MASK)

/* The typical trimming range is ~48ppm over all steps, see chapter "5.14.2 Crystal Oscillator Trim" of DW1000 Datasheet */
#define AVG_TRIM_PER_PPM                    ((FS_XTALT_MAX_VAL+1)/48.0f)

/**
 * Application entry point.
 */
int dw_main(void)
{
    uint8    rx_buffer[FRAME_LEN_MAX];
    uint16   frame_len;
    uint32   status_reg;


    /* Display application name on LCD. */
    lcd_display_str(APP_NAME);

    /* Reset and initialise DW1000. See NOTE 2 below.
     * For initialisation, DW1000 clocks must be temporarily set to crystal speed. After initialisation SPI rate can be increased for optimum
     * performance. */
    reset_DW1000(); /* Target specific drive of RSTn line into DW1000 low for a period. */

    port_set_dw1000_slowrate();

    if (dwt_initialise(DWT_LOADNONE) == DWT_ERROR)
    {
        lcd_display_str("INIT FAILED");
        while (1)
        { };
    }

    port_set_dw1000_fastrate();

    /* Configure DW1000. */
    dwt_configure(&cfg);

    /* Read the initial crystal trimming value. This needs to be done after dwt_initialise(), which sets up initial trimming code.*/
    uCurrentTrim_val = dwt_getinitxtaltrim();

    /* Loop forever receiving frames. */
    while (1)
    {
        int     i;

        /* Clear local RX buffer to avoid having leftovers from previous receptions  This is not necessary but is included here to aid reading
         * the RX buffer.
         * This is a good place to put a breakpoint. Here (after first time through the loop) the local status register will be set for last event
         * and if a good receive has happened the data buffer will have the data in it, and frame_len will be set to the length of the RX frame. */
        for (i = 0 ; i < FRAME_LEN_MAX; i++ )
        {
            rx_buffer[i] = 0;
        }

        /* Activate reception immediately. See NOTE 3 below. */
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        /* Poll until a frame is properly received or an error/timeout occurs. See NOTE 4 below.
         * STATUS register is 5 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
         * function to access it.
         * */
        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)))
        { };

        if (status_reg & SYS_STATUS_RXFCG)
        {
            /* Followed code block is the example of reading received frame to the rx_buffer.
             * While this is not necessary to show the clock offset adjustment algorithm, in a real implementation it is obviously important
             * to read the RX frame and validate it is from the expected source node whose crystal we want to track.
             * */
            {
                /* A frame has been received, copy it to our local buffer. */
                frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
                if (frame_len <= FRAME_LEN_MAX)
                {
                    dwt_readrxdata(rx_buffer, frame_len, 0);
                }
            }

            /* Followed code block is the example of Crystal Trimming using carrier integrator value.
             * In a real application it would be important to check that the message is from correct sender
             * before we trim our crystal to follow its clock.
             * */
            {
                int32    tmp;
                float     xtalOffset_ppm;

                /* Now we read the carrier integrator value, which reflects the frequency offset of the remote transmitter.
                 * A positive value means the local RX clock is running faster than the remote transmitter's clock.
                 * For a valid result the carrier integrator should before the receiver is re-enabled.
                 */
                tmp = dwt_readcarrierintegrator();

                /* Calculate the crystal offset in Parts Per Million units (ppm).
                 * Note, the calculation code below uses a standard library (stdlib), which makes the target application ~1.5 KBytes bigger.
                 * The math below is for reference and can be optimised further, if required.
                 * */
                xtalOffset_ppm = tmp * (freqMultiplier * hzMultiplier);

                /* TESTING BREAKPOINT LOCATION #1 */

                /* Example of crystal trimming to be in the range
                 * (TARGET_XTAL_OFFSET_VALUE_PPM_MIN..TARGET_XTAL_OFFSET_VALUE_PPM_MAX) out of the transmitter's crystal frequency.
                 * This may be used in application, which require small offset to be present between ranging sides.
                 * */
                if(fabs(xtalOffset_ppm) > TARGET_XTAL_OFFSET_VALUE_PPM_MAX ||
                   fabs(xtalOffset_ppm) < TARGET_XTAL_OFFSET_VALUE_PPM_MIN)
                {
                    uCurrentTrim_val -= ((TARGET_XTAL_OFFSET_VALUE_PPM_MAX + TARGET_XTAL_OFFSET_VALUE_PPM_MIN)/2 + xtalOffset_ppm) * AVG_TRIM_PER_PPM;
                    uCurrentTrim_val &= FS_XTALT_MASK;

                    /* Configure new Crystal Offset value */
                    dwt_setxtaltrim(uCurrentTrim_val);
                }
            }

            /* Clear good RX frame event in the DW1000 status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);
        }
        else
        {
            /* Clear RX error events in the DW1000 status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        }
    }
}

#endif
/*****************************************************************************************************************************************************
 * NOTES:
 *
 * 1. In this example, maximum frame length is set to 127 bytes which is 802.15.4 UWB standard maximum frame length. DW1000 supports an extended
 *    frame length (up to 1023 bytes long) mode which is not used in this example.
 * 2. In this example, LDE microcode is not loaded upon calling dwt_initialise(). This will prevent the IC from generating an RX timestamp. If
 *    time-stamping is required, DWT_LOADUCODE parameter should be used. See two-way ranging examples (e.g. examples 5a/5b).
 * 3. Manual reception activation is performed here but DW1000 offers several features that can be used to handle more complex scenarios or to
 *    optimise system's overall performance (e.g. timeout after a given time, etc.).
 * 4. We use polled mode of operation here to keep the example as simple as possible but RXFCG and error/timeout status events can be used to generate
 *    interrupts. Please refer to DW1000 User Manual for more details on "interrupts".
 * 5. The user is referred to DecaRanging ARM application (distributed with EVK1000 product) for additional practical example of usage, and to the
 *    DW1000 API Guide for more details on the DW1000 driver functions.
 ****************************************************************************************************************************************************/
