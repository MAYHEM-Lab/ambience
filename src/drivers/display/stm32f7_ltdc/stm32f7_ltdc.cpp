#include <stm32_hal/rcc.hpp>
#include <stm32_hal/rcc_ex.hpp>
#include <tos/debug/panic.hpp>
#include <tos/periph/stm32f7_ltdc.hpp>

/**
 * @brief  RK043FN48H Size
 */
#define RK043FN48H_WIDTH ((uint16_t)480)  /* LCD PIXEL WIDTH            */
#define RK043FN48H_HEIGHT ((uint16_t)272) /* LCD PIXEL HEIGHT           */

/**
 * @brief  RK043FN48H Timing
 */
#define RK043FN48H_HSYNC ((uint16_t)41) /* Horizontal synchronization */
#define RK043FN48H_HBP ((uint16_t)13)   /* Horizontal back porch      */
#define RK043FN48H_HFP ((uint16_t)32)   /* Horizontal front porch     */
#define RK043FN48H_VSYNC ((uint16_t)10) /* Vertical synchronization   */
#define RK043FN48H_VBP ((uint16_t)2)    /* Vertical back porch        */
#define RK043FN48H_VFP ((uint16_t)2)    /* Vertical front porch       */

/**
 * @brief  RK043FN48H frequency divider
 */
#define RK043FN48H_FREQUENCY_DIVIDER 5 /* LCD Frequency divider      */

namespace tos::stm32::f7 {
ltdc::ltdc(const ltdc_layer& layer) {
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
    PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
    PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    /* LTDC Initialization -------------------------------------------------------*/

    /* Polarity configuration */
    /* Initialize the horizontal synchronization polarity as active low */
    m_ltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    /* Initialize the vertical synchronization polarity as active low */
    m_ltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    /* Initialize the data enable polarity as active low */
    m_ltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    /* Initialize the pixel clock polarity as input pixel clock */
    m_ltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

    /* The RK043FN48H LCD 480x272 is selected */
    /* Timing Configuration */
    m_ltdc.Init.HorizontalSync = (RK043FN48H_HSYNC - 1);
    m_ltdc.Init.VerticalSync = (RK043FN48H_VSYNC - 1);
    m_ltdc.Init.AccumulatedHBP = (RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
    m_ltdc.Init.AccumulatedVBP = (RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
    m_ltdc.Init.AccumulatedActiveH =
        (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
    m_ltdc.Init.AccumulatedActiveW =
        (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
    m_ltdc.Init.TotalHeigh =
        (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP + RK043FN48H_VFP - 1);
    m_ltdc.Init.TotalWidth =
        (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP + RK043FN48H_HFP - 1);

    /* Configure R,G,B component values for LCD background color : all black background */
    m_ltdc.Init.Backcolor.Blue = 0;
    m_ltdc.Init.Backcolor.Green = 0;
    m_ltdc.Init.Backcolor.Red = 0;

    m_ltdc.Instance = LTDC;

    gpio_config();

    /* Configure the LTDC */
    if (HAL_LTDC_Init(&m_ltdc) != HAL_OK) {
        debug::panic("Could not initialize ltdc");
    }

    /* Configure the Layer*/
    if (HAL_LTDC_ConfigLayer(&m_ltdc,
                             const_cast<LTDC_LayerCfgTypeDef*>(layer.native_handle()),
                             1) != HAL_OK) {
        debug::panic("Could not initialize layer");
    }
}

void ltdc::config() {
}


} // namespace tos::stm32::f7
void tos::stm32::f7::ltdc::gpio_config() {
    GPIO_InitTypeDef GPIO_Init_Structure;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable the LTDC Clock */
    __HAL_RCC_LTDC_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /******************** LTDC Pins configuration *************************/
    /* Enable GPIOs clock */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOJ_CLK_ENABLE();
    __HAL_RCC_GPIOK_CLK_ENABLE();

    /*** LTDC Pins configuration ***/
    /* GPIOE configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_4;
    GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Pull = GPIO_NOPULL;
    GPIO_Init_Structure.Speed = GPIO_SPEED_FAST;
    GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

    /* GPIOG configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_12;
    GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Alternate = GPIO_AF9_LTDC;
    HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);

    /* GPIOI LTDC alternate configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOI, &GPIO_Init_Structure);

    /* GPIOJ configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                              GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
                              GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
                              GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOJ, &GPIO_Init_Structure);

    /* GPIOK configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 |
                              GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOK, &GPIO_Init_Structure);

    /* LCD_DISP GPIO configuration */
    GPIO_Init_Structure.Pin =
        GPIO_PIN_12; /* LCD_DISP pin has to be manually controlled */
    GPIO_Init_Structure.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOI, &GPIO_Init_Structure);

    /* LCD_BL_CTRL GPIO configuration */
    GPIO_Init_Structure.Pin =
        GPIO_PIN_3; /* LCD_BL_CTRL pin has to be manually controlled */
    GPIO_Init_Structure.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOK, &GPIO_Init_Structure);
/**
 * @brief LCD special pins
 */
/* Display enable pin */
#define LCD_DISP_PIN GPIO_PIN_12
#define LCD_DISP_GPIO_PORT GPIOI
#define LCD_DISP_GPIO_CLK_ENABLE() __HAL_RCC_GPIOI_CLK_ENABLE()
#define LCD_DISP_GPIO_CLK_DISABLE() __HAL_RCC_GPIOI_CLK_DISABLE()

/* Backlight control pin */
#define LCD_BL_CTRL_PIN GPIO_PIN_3
#define LCD_BL_CTRL_GPIO_PORT GPIOK
#define LCD_BL_CTRL_GPIO_CLK_ENABLE() __HAL_RCC_GPIOK_CLK_ENABLE()
#define LCD_BL_CTRL_GPIO_CLK_DISABLE() __HAL_RCC_GPIOK_CLK_DISABLE()

    /* Assert display enable LCD_DISP pin */
    HAL_GPIO_WritePin(LCD_DISP_GPIO_PORT, LCD_DISP_PIN, GPIO_PIN_SET);

    /* Assert backlight LCD_BL_CTRL pin */
    HAL_GPIO_WritePin(LCD_BL_CTRL_GPIO_PORT, LCD_BL_CTRL_PIN, GPIO_PIN_SET);
}
