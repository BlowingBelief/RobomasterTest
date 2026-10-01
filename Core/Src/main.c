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
#include "iwdg.h"
#include "tim.h"
#include "gpio.h"
//实现的效果：1.通过中断实现闪烁方式的变化 2.闪烁方式变化 3.看门狗
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* 音符枚举：12345671(高) */
typedef enum {
  NOTE_C4 = 0, // 1  (Do)  261.63 Hz
  NOTE_D4,     // 2  (Re)  293.66 Hz
  NOTE_E4,     // 3  (Mi)  329.63 Hz
  NOTE_F4,     // 4  (Fa)  349.23 Hz
  NOTE_G4,     // 5  (Sol) 392.00 Hz
  NOTE_A4,     // 6  (La)  440.00 Hz
  NOTE_B4,     // 7  (Si)  493.88 Hz
  NOTE_C5,     // 1' (高音Do) 523.25 Hz
  NOTE_REST    // 0  (休止符)
} Note_t;

/* 几分音符枚举 */
typedef enum {
  DUR_WHOLE = 0, // 全音符   (4拍)
  DUR_HALF,      // 二分音符 (2拍)
  DUR_QUARTER,   // 四分音符 (1拍)
  DUR_EIGHTH,    // 八分音符 (1/2拍)
  DUR_SIXTEENTH  // 十六分音符 (1/4拍)
} Duration_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t requested_mode = 0;

uint32_t tick = 0;
uint32_t current_count = 0;
uint32_t ccr3 = 0;
uint32_t cnt = 0;
uint32_t t = 0;
uint32_t dt = 0;
uint32_t RestartTime = 0;
int Light_Type = 1;
int m =0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* 频率表：对应 Note_t 枚举的值 */
const uint16_t NoteFreqTable[] = {
  262,  // NOTE_C4
  294,  // NOTE_D4
  330,  // NOTE_E4
  349,  // NOTE_F4
  392,  // NOTE_G4
  440,  // NOTE_A4
  494,  // NOTE_B4
  523   // NOTE_C5
};

/**
  * @brief  播放指定音符和节拍
  * @param  note: 音符枚举 (1-7,高音1，或休止符)
  * @param  duration: 节拍枚举 (全音符~十六分音符)
  * @retval 无
  */
void Buzzer_PlayNote(Note_t note, Duration_t duration)
{
  // 1. 计算该节拍对应的毫秒数。假设四分音符(1拍) = 500ms (即BPM=120)
  uint32_t ms = 500;
  switch (duration) {
  case DUR_WHOLE:     ms = 500 * 4; break; // 全音符 2000ms
  case DUR_HALF:      ms = 500 * 2; break; // 二分音符 1000ms
  case DUR_QUARTER:   ms = 500 * 1; break; // 四分音符 500ms
  case DUR_EIGHTH:    ms = 500 / 2; break; // 八分音符 250ms
  case DUR_SIXTEENTH: ms = 500 / 4; break; // 十六分音符 125ms
  default:            ms = 500;     break;
  }

  // 2. 如果是休止符，直接停止发声并延时
  if (note == NOTE_REST) {
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0); // 占空比设为0，停止发声
    HAL_Delay(ms);
    return;
  }

  // 3. 根据音符查表得到频率
  uint32_t freq = NoteFreqTable[note];

  // 4. 计算 ARR 值 (计数频率为 1MHz)
  // 公式: ARR = 1000000 / 频率 - 1
  uint32_t arr = (1000000 / freq) - 1;

  // 5. 设置 PWM 参数，发声
  __HAL_TIM_SET_AUTORELOAD(&htim4, arr);                // 改变音调
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, (arr+1)/2); // 50%占空比，声音最响

  // 6. 延时，持续发声
  HAL_Delay(ms);

  // 7. 停止发声（将占空比设为0），并加入20ms短暂静音，防止连音
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
  HAL_Delay(20); // 音符间隔，让耳朵能区分开来
}
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
  MX_TIM4_Init();
  MX_IWDG_Init();
  MX_TIM1_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
  //HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3);
  m=0;
  HAL_IWDG_Refresh(&hiwdg);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    cnt = __HAL_TIM_GET_COUNTER(&htim5);
    dt = HAL_GetTick() - t;
    t = HAL_GetTick();
    RestartTime += dt;

    if (Light_Type == 1)
    {
      current_count = __HAL_TIM_GET_COUNTER(&htim5);

      if (RestartTime <= 1000)
      {
        __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, (RestartTime/1000.f) * __HAL_TIM_GET_AUTORELOAD(&htim5));
      }
      else if (RestartTime <= 2000)
      {
        __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, (2000- RestartTime)/1000.f * __HAL_TIM_GET_AUTORELOAD(&htim5));
      }
      else
      {
        RestartTime = 0;
      }


    }
    else if (Light_Type == 0)
    {
      __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, __HAL_TIM_GET_AUTORELOAD(&htim5));
    }
    else if (Light_Type == 2)
    {
      if (RestartTime>500)
      {
        if (m == 0)
        {
          __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, 0);
          m = 1;
        }
        else
        {
          __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, __HAL_TIM_GET_AUTORELOAD(&htim5));
          m=0;
        }
        RestartTime = 0;
      }
    }




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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 6;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim_)
{

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

  if (GPIO_Pin == KEY_Pin)
  {
    static uint32_t last_key_tick = 0;
    static uint8_t has_last_key_tick = 0;
    uint32_t now = HAL_GetTick();

    if (!has_last_key_tick || (now - last_key_tick) >= 30U)
    {
      has_last_key_tick = 1;
      last_key_tick = now;
      requested_mode = (requested_mode + 1U) % 3U;
      Light_Type++;
      if (Light_Type > 3)
      {
        Light_Type = 0;
      }

      HAL_IWDG_Refresh(&hiwdg);

    }
  }
}
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
  while (1)
  {
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
