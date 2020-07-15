#include <stm32_hal/gpio.hpp>
#include <stm32_hal/hal.hpp>
#include <stm32_hal/rcc.hpp>
#include <stm32_hal/rcc_ex.hpp>
#include <tos/debug/panic.hpp>
#include <tos/periph/stm32f7_sdram.hpp>

/* #define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_8  */
#define SDRAM_MEMORY_WIDTH FMC_SDRAM_MEM_BUS_WIDTH_16

#define SDCLOCK_PERIOD FMC_SDRAM_CLOCK_PERIOD_2
/* #define SDCLOCK_PERIOD                FMC_SDRAM_CLOCK_PERIOD_3 */

#define SDRAM_TIMEOUT ((uint32_t)0xFFFF)

#define SDRAM_MODEREG_BURST_LENGTH_1 ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2 ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4 ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8 ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2 ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3 ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE ((uint16_t)0x0200)

namespace tos::stm32::f7 {
namespace {
void SystemInit_ExtMemCtl(void) {
    uint32_t tmpreg = 0, timeout = 0xFFFF;
    __IO uint32_t index;

    /* Enable GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and GPIOH interface
    clock */
    RCC->AHB1ENR |= 0x000000FC;

    /* Connect PCx pins to FMC Alternate function */
    GPIOC->AFR[0] = 0x0000C000;
    GPIOC->AFR[1] = 0x00000000;
    /* Configure PCx pins in Alternate function mode */
    GPIOC->MODER = 0x00000080;
    /* Configure PCx pins speed to 50 MHz */
    GPIOC->OSPEEDR = 0x00000080;
    /* Configure PCx pins Output type to push-pull */
    GPIOC->OTYPER = 0x00000000;
    /* No pull-up, pull-down for PCx pins */
    GPIOC->PUPDR = 0x00000040;

    /* Connect PDx pins to FMC Alternate function */
    GPIOD->AFR[0] = 0x000000CC;
    GPIOD->AFR[1] = 0xCC000CCC;
    /* Configure PDx pins in Alternate function mode */
    GPIOD->MODER = 0xA02A000A;
    /* Configure PDx pins speed to 50 MHz */
    GPIOD->OSPEEDR = 0xA02A000A;
    /* Configure PDx pins Output type to push-pull */
    GPIOD->OTYPER = 0x00000000;
    /* No pull-up, pull-down for PDx pins */
    GPIOD->PUPDR = 0x50150005;

    /* Connect PEx pins to FMC Alternate function */
    GPIOE->AFR[0] = 0xC00000CC;
    GPIOE->AFR[1] = 0xCCCCCCCC;
    /* Configure PEx pins in Alternate function mode */
    GPIOE->MODER = 0xAAAA800A;
    /* Configure PEx pins speed to 50 MHz */
    GPIOE->OSPEEDR = 0xAAAA800A;
    /* Configure PEx pins Output type to push-pull */
    GPIOE->OTYPER = 0x00000000;
    /* No pull-up, pull-down for PEx pins */
    GPIOE->PUPDR = 0x55554005;

    /* Connect PFx pins to FMC Alternate function */
    GPIOF->AFR[0] = 0x00CCCCCC;
    GPIOF->AFR[1] = 0xCCCCC000;
    /* Configure PFx pins in Alternate function mode */
    GPIOF->MODER = 0xAA800AAA;
    /* Configure PFx pins speed to 50 MHz */
    GPIOF->OSPEEDR = 0xAA800AAA;
    /* Configure PFx pins Output type to push-pull */
    GPIOF->OTYPER = 0x00000000;
    /* No pull-up, pull-down for PFx pins */
    GPIOF->PUPDR = 0x55400555;

    /* Connect PGx pins to FMC Alternate function */
    GPIOG->AFR[0] = 0x00CC00CC;
    GPIOG->AFR[1] = 0xC000000C;
    /* Configure PGx pins in Alternate function mode */
    GPIOG->MODER = 0x80020A0A;
    /* Configure PGx pins speed to 50 MHz */
    GPIOG->OSPEEDR = 0x80020A0A;
    /* Configure PGx pins Output type to push-pull */
    GPIOG->OTYPER = 0x00000000;
    /* No pull-up, pull-down for PGx pins */
    GPIOG->PUPDR = 0x40010505;

    /* Connect PHx pins to FMC Alternate function */
    GPIOH->AFR[0] = 0x00C0C000;
    GPIOH->AFR[1] = 0x00000000;
    /* Configure PHx pins in Alternate function mode */
    GPIOH->MODER = 0x00000880;
    /* Configure PHx pins speed to 50 MHz */
    GPIOH->OSPEEDR = 0x00000880;
    /* Configure PHx pins Output type to push-pull */
    GPIOH->OTYPER = 0x00000000;
    /* No pull-up, pull-down for PHx pins */
    GPIOH->PUPDR = 0x00000440;

    /* Enable the FMC interface clock */
    RCC->AHB3ENR |= 0x00000001;

    /* Configure and enable SDRAM bank1 */
    FMC_Bank5_6->SDCR[0] = 0x00001954;
    FMC_Bank5_6->SDTR[0] = 0x01115351;

    /* SDRAM initialization sequence */
    /* Clock enable command */
    FMC_Bank5_6->SDCMR = 0x00000011;
    tmpreg = FMC_Bank5_6->SDSR & 0x00000020;
    while ((tmpreg != 0) && (timeout-- > 0)) {
        tmpreg = FMC_Bank5_6->SDSR & 0x00000020;
    }

    /* Delay */
    for (index = 0; index < 1000; index++)
        ;

    /* PALL command */
    FMC_Bank5_6->SDCMR = 0x00000012;
    timeout = 0xFFFF;
    while ((tmpreg != 0) && (timeout-- > 0)) {
        tmpreg = FMC_Bank5_6->SDSR & 0x00000020;
    }

    /* Auto refresh command */
    FMC_Bank5_6->SDCMR = 0x000000F3;
    timeout = 0xFFFF;
    while ((tmpreg != 0) && (timeout-- > 0)) {
        tmpreg = FMC_Bank5_6->SDSR & 0x00000020;
    }

    /* MRD register program */
    FMC_Bank5_6->SDCMR = 0x00044014;
    timeout = 0xFFFF;
    while ((tmpreg != 0) && (timeout-- > 0)) {
        tmpreg = FMC_Bank5_6->SDSR & 0x00000020;
    }

    /* Set refresh count */
    tmpreg = FMC_Bank5_6->SDRTR;
    FMC_Bank5_6->SDRTR = (tmpreg | (0x0000050C << 1));

    /* Disable write protection */
    tmpreg = FMC_Bank5_6->SDCR[0];
    FMC_Bank5_6->SDCR[0] = (tmpreg & 0xFFFFFDFF);
}
} // namespace
sdram::sdram() {
    SystemInit_ExtMemCtl();

    m_sdram.Instance = FMC_SDRAM_DEVICE;

    m_timing.LoadToActiveDelay = 2;
    m_timing.ExitSelfRefreshDelay = 6;
    m_timing.SelfRefreshTime = 4;
    m_timing.RowCycleDelay = 6;
    m_timing.WriteRecoveryTime = 2;
    m_timing.RPDelay = 2;
    m_timing.RCDDelay = 2;

    m_sdram.Init.SDBank = FMC_SDRAM_BANK1;
    m_sdram.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
    m_sdram.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
    m_sdram.Init.MemoryDataWidth = SDRAM_MEMORY_WIDTH;
    m_sdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    m_sdram.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_2;
    m_sdram.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    m_sdram.Init.SDClockPeriod = SDCLOCK_PERIOD;
    m_sdram.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
    m_sdram.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;

    setup_gpio();

    /* Initialize the SDRAM controller */
    if (HAL_SDRAM_Init(&m_sdram, &m_timing) != HAL_OK) {
        debug::panic("Cannot initialize SDRAM");
    }

    init_sequence();
}

void* sdram::bank_base() const {
    return reinterpret_cast<void*>(0xC0000000);
}

void sdram::init_sequence() {
    /* Step 3:  Configure a clock configuration enable command */
    m_command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    m_command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    m_command.AutoRefreshNumber = 1;
    m_command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&m_sdram, &m_command, SDRAM_TIMEOUT);

    /* Step 4: Insert 100 us minimum delay */
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    HAL_Delay(1);

    /* Step 5: Configure a PALL (precharge all) command */
    m_command.CommandMode = FMC_SDRAM_CMD_PALL;
    m_command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    m_command.AutoRefreshNumber = 1;
    m_command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&m_sdram, &m_command, SDRAM_TIMEOUT);

    /* Step 6 : Configure a Auto-Refresh command */
    m_command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    m_command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    m_command.AutoRefreshNumber = 8;
    m_command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&m_sdram, &m_command, SDRAM_TIMEOUT);

    /* Step 7: Program the external memory mode register */
    auto tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1 |
                  SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL | SDRAM_MODEREG_CAS_LATENCY_2 |
                  SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                  SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    m_command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    m_command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    m_command.AutoRefreshNumber = 1;
    m_command.ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(&m_sdram, &m_command, SDRAM_TIMEOUT);

    /* Step 8: Set the refresh rate counter */
    /* (15.62 us x Freq) - 20 */
    /* Set the device refresh counter */
    m_sdram.Instance->SDRTR |= ((uint32_t)((1292) << 1));
}
void sdram::setup_gpio() {
    GPIO_InitTypeDef GPIO_Init_Structure;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO clocks */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    /* Enable FMC clock */
    __HAL_RCC_FMC_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Pull = GPIO_PULLUP;
    GPIO_Init_Structure.Speed = GPIO_SPEED_FAST;
    GPIO_Init_Structure.Alternate = GPIO_AF12_FMC;

    /* GPIOC configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_3;
    HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);

    /* GPIOD configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 |
                              GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);

    /* GPIOE configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 |
                              GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
                              GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

    /* GPIOF configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                              GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 |
                              GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);

    /* GPIOG configuration */
    GPIO_Init_Structure.Pin =
        GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);

    /* GPIOH configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_3 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOH, &GPIO_Init_Structure);
}
} // namespace tos::stm32::f7