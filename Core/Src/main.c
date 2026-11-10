/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "button.h"
#include "usbd_custom_hid_if.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MASTER_CTRL_RPT_ROW 6

#define FORWARD_1_SWITCH_MASK 0x80 // button 48
#define POS_2_SWITCH_MASK 0x04     // button 43
#define FORWARD_3_SWITCH_MASK 0x08 // button 44
#define BREAK_1_SWITCH_MASK 0x10   // button 45
#define BREAK_3_SWITCH_MASK 0x20   // button 46

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* Report layout: [report_id][7 button bytes][16-bit X axis, little-endian] */
static uint8_t s_report[10] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    uint8_t *button_state = ButtonMatrix_Scan();
    for (int i = 0; i < MATRIX_ROWS; i++) {
      s_report[i + 1] = button_state[i];
    }

    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
      Error_Handler();
    }
    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK) {
      Error_Handler();
    }
    uint16_t axis_value = (uint16_t)HAL_ADC_GetValue(&hadc1);
    if (HAL_ADC_Stop(&hadc1) != HAL_OK) {
      Error_Handler();
    }
    s_report[8] = (uint8_t)(axis_value & 0xFF);
    s_report[9] = (uint8_t)(axis_value >> 8);

    // Change master control report mappings
    // if position 2 switch is pressed, check for forward 1 and break 1 switches
    if (s_report[MASTER_CTRL_RPT_ROW] & POS_2_SWITCH_MASK) {

      // Forward branch if forward 1 switch is pressed
      if (s_report[MASTER_CTRL_RPT_ROW] & FORWARD_1_SWITCH_MASK) {
        if (s_report[MASTER_CTRL_RPT_ROW] & FORWARD_3_SWITCH_MASK) {
          // Forward 3 position: reset forward 1 and position 2 switches in HID report
          s_report[MASTER_CTRL_RPT_ROW] &= ~POS_2_SWITCH_MASK;
          s_report[MASTER_CTRL_RPT_ROW] &= ~FORWARD_1_SWITCH_MASK;
        } else {
          // Forward 2 position: reset forward 1 switch
          s_report[MASTER_CTRL_RPT_ROW] &= ~FORWARD_1_SWITCH_MASK;
        }
      }

      // Break branch if break 1 switch is pressed
      if (s_report[MASTER_CTRL_RPT_ROW] & BREAK_1_SWITCH_MASK) {
        if (s_report[MASTER_CTRL_RPT_ROW] & BREAK_3_SWITCH_MASK) {
          // Break 3 position: reset break 1 and position 2 switches in HID report
          s_report[MASTER_CTRL_RPT_ROW] &= ~POS_2_SWITCH_MASK;
          s_report[MASTER_CTRL_RPT_ROW] &= ~BREAK_1_SWITCH_MASK;
        } else {
          // Break 2 position:
          s_report[MASTER_CTRL_RPT_ROW] &= ~BREAK_1_SWITCH_MASK;
          // TODO: Add logic to set unused button
        }
      }
    }

    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, s_report, sizeof(s_report));

    HAL_Delay(10); /* 10ms delay, adjust as needed for your application */

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, BTN_R1_Pin|BTN_R2_Pin|BTN_R3_Pin|BTN_R4_Pin
                          |BTN_R5_Pin|BTN_R6_Pin|BTN_R7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BTN_C1_Pin BTN_C2_Pin BTN_C3_Pin BTN_C4_Pin */
  GPIO_InitStruct.Pin = BTN_C1_Pin|BTN_C2_Pin|BTN_C3_Pin|BTN_C4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_C5_Pin BTN_C6_Pin BTN_C7_Pin BTN_C8_Pin */
  GPIO_InitStruct.Pin = BTN_C5_Pin|BTN_C6_Pin|BTN_C7_Pin|BTN_C8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_R1_Pin BTN_R2_Pin BTN_R3_Pin BTN_R4_Pin
                           BTN_R5_Pin BTN_R6_Pin BTN_R7_Pin */
  GPIO_InitStruct.Pin = BTN_R1_Pin|BTN_R2_Pin|BTN_R3_Pin|BTN_R4_Pin
                          |BTN_R5_Pin|BTN_R6_Pin|BTN_R7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
