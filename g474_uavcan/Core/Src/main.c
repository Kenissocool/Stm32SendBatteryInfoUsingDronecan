/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "canard.h"
#include "canard_internals.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "uavcan.protocol.GetNodeInfo_res.h"
#include "uavcan.protocol.NodeStatus.h"
#include "uavcan.equipment.power.BatteryInfo.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CANARD_POOL_SIZE (1024U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
FDCAN_HandleTypeDef hfdcan1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
static uint8_t canard_pool[CANARD_POOL_SIZE];
static CanardInstance canard;
FDCAN_TxHeaderTypeDef txHeader;
uint8_t txData[8];
FDCAN_RxHeaderTypeDef rx_header;
uint8_t rx_data[8];
uint8_t count = 0;
uint8_t count1 = 0;
CanardCANFrame rx_frame;
static struct uavcan_protocol_NodeStatus node_status;
CanardCANFrame *frame;
//static CanardCANFrame* frame = NULL;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
//    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0)
//    {
		count++;
    HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &rx_header, rx_data);


//    canardHandleRxFrame(&canard, &rx_frame, HAL_GetTick() * 1000);

    rx_frame.id = rx_header.Identifier| CANARD_CAN_FRAME_EFF;
    rx_frame.data_len = rx_header.DataLength;
    memcpy(rx_frame.data, rx_data, rx_frame.data_len);

    canardHandleRxFrame(&canard, &rx_frame, HAL_GetTick() * 1000);
//    }
}

//void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
//{
//    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0)
//    {
//        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data);
//
//        // Optional: blink LED or set a flag
//    }
//}
static size_t my_strnlen(const char *s, size_t maxlen) {
    size_t i;
    for (i = 0; i < maxlen && s[i]; i++);
    return i;
}

static void handle_GetNodeInfo(CanardInstance *ins, CanardRxTransfer *transfer)
{
    uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE];
    struct uavcan_protocol_GetNodeInfoResponse pkt;

    memset(&pkt, 0, sizeof(pkt));
    pkt.status = node_status;

    pkt.software_version.major = 1;
    pkt.software_version.minor = 0;

    pkt.hardware_version.major = 1;
    pkt.hardware_version.minor = 0;
    for (int i = 0; i < 16; i++) pkt.hardware_version.unique_id[i] = i;

    const char *name = "kuybat";
    pkt.name.len = (uint8_t)my_strnlen(name, 80);
    memcpy(pkt.name.data, name, pkt.name.len);

    static uint8_t getnodeinfo_transfer_id = 0;
    uint16_t total_size = uavcan_protocol_GetNodeInfoResponse_encode(&pkt, buffer
    #if CANARD_ENABLE_TAO_OPTION
        , true
    #endif
    );

    canardRequestOrRespond(ins,
                           transfer->source_node_id,
                           UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_SIGNATURE,
                           1,
                           &getnodeinfo_transfer_id,
                           CANARD_TRANSFER_PRIORITY_LOW,
                           CanardResponse,
                           buffer,
                           total_size);
}

bool should_accept(const CanardInstance* ins, uint64_t* out_signature, uint16_t data_type_id,
                   CanardTransferType transfer_type, uint8_t source_node_id)
{
    if (transfer_type == CanardTransferTypeRequest && data_type_id == 1)
    {
        *out_signature = UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_SIGNATURE;
        return true;
    }
    return false;
}
void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer)
{
    if (transfer->transfer_type == CanardTransferTypeRequest) {
        switch (transfer->data_type_id) {
            case 1: // GetNodeInfo
								count1++;
                handle_GetNodeInfo(ins, transfer);
                break;
            //
        }
    }
    //
}

static void send_BatteryInfo(void)
{
    struct uavcan_equipment_power_BatteryInfo batt_info;
    memset(&batt_info, 0, sizeof(batt_info));


    batt_info.temperature = 383.5f;      // d? K
    batt_info.voltage = 25.2f;          // V
    batt_info.current = 2.1f;          // A
    batt_info.average_power_10sec = 200.0f; // W
    batt_info.remaining_capacity_wh = 40.0f;
    batt_info.full_charge_capacity_wh = 50.0f;
    batt_info.hours_to_full_charge = 0.5f;
    batt_info.status_flags = UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_IN_USE;
    batt_info.state_of_health_pct = 98;
    batt_info.state_of_charge_pct = 80;
    batt_info.state_of_charge_pct_stdev = 1;
    batt_info.battery_id = 1;
    batt_info.model_instance_id = 0x12345678;

    batt_info.model_name.len = 8;
    memcpy(batt_info.model_name.data, "LiPo4s", 8);

    // 2. Encode
    uint8_t buffer[UAVCAN_EQUIPMENT_POWER_BATTERYINFO_MAX_SIZE];
    uint32_t len = uavcan_equipment_power_BatteryInfo_encode(&batt_info, buffer
    #if CANARD_ENABLE_TAO_OPTION
        , true
    #endif
    );

    // 3. Truy?n lên DroneCAN b?ng canardBroadcast
    static uint8_t transfer_id = 0;
    canardBroadcast(&canard,
                    UAVCAN_EQUIPMENT_POWER_BATTERYINFO_SIGNATURE,
                    UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID,
                    &transfer_id,
                    CANARD_TRANSFER_PRIORITY_LOW,
                    buffer,
                    len);
}


//static uint8_t battery_info_transfer_id = 0;
//static void send_BatteryInfo(void)
//{
//    struct uavcan_equipment_power_BatteryInfo batt_info;
//    memset(&batt_info, 0, sizeof(batt_info));
//
//    batt_info.temperature = 30.5f;
//    batt_info.voltage = 15.8f;
//    batt_info.current = 12.1f;
//    batt_info.average_power_10sec = 180.0f;
//    batt_info.remaining_capacity_wh = 40.0f;
//    batt_info.full_charge_capacity_wh = 50.0f;
//    batt_info.hours_to_full_charge = 0.5f;
//    batt_info.status_flags = UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_IN_USE;
//    batt_info.state_of_health_pct = 98;
//    batt_info.state_of_charge_pct = 80;
//    batt_info.state_of_charge_pct_stdev = 1;
//    batt_info.battery_id = 1;
//    batt_info.model_instance_id = 0x12345678;
//    batt_info.model_name.len = 8;
//    memcpy(batt_info.model_name.data, "LiPo4s10", 8);
//
//    uint8_t buffer[UAVCAN_EQUIPMENT_POWER_BATTERYINFO_MAX_SIZE];
//    size_t len = uavcan_equipment_power_BatteryInfo_encode(&batt_info, buffer
//#if CANARD_ENABLE_TAO_OPTION
//        , true
//#endif
//    );
//
//    canardBroadcast(&canard,
//                    UAVCAN_EQUIPMENT_POWER_BATTERYINFO_SIGNATURE,
//                    UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID,
//                    &battery_info_transfer_id,
//                    CANARD_TRANSFER_PRIORITY_LOW,
//                    buffer,
//                    (uint16_t)len);
//}


static void send_NodeStatus(void)
{
    uint8_t buffer[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE];

    node_status.uptime_sec = HAL_GetTick() / 1000;
    node_status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
    node_status.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
    node_status.sub_mode = 0;
    node_status.vendor_specific_status_code = 1234;

    uint32_t len = uavcan_protocol_NodeStatus_encode(&node_status, buffer
#if CANARD_ENABLE_TAO_OPTION
        , true
#endif
    );

    static uint8_t transfer_id = 0;

    canardBroadcast(&canard,
                    UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
                    UAVCAN_PROTOCOL_NODESTATUS_ID,
                    &transfer_id,
                    CANARD_TRANSFER_PRIORITY_LOW,
                    buffer,
                    len);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3) //10hz
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
		send_BatteryInfo();
	}
	if(htim->Instance == TIM2) //1hz
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
			send_NodeStatus();
	}
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
  MX_FDCAN1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

	HAL_FDCAN_Start(&hfdcan1);
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0);
	canardInit(&canard, canard_pool, CANARD_POOL_SIZE, onTransferReceived, should_accept, NULL);
	canardSetLocalNodeID(&canard, 11);
	FDCAN_FilterTypeDef sFilterConfig;
	sFilterConfig.IdType = FDCAN_EXTENDED_ID;            // 29-bit ID
	sFilterConfig.FilterIndex = 0;                       // Use filter slot 0 (0–127 for FDCAN1)
	sFilterConfig.FilterType = FDCAN_FILTER_MASK;        // Use mask-based filtering
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;// Send matching messages to RX FIFO 0
	sFilterConfig.FilterID1 =  0x00000000;                // ID to match
	sFilterConfig.FilterID2 =  0x00000000;                // Full mask to match all 29 bits
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig);
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start_IT(&htim2);
	uint32_t last_node_status = 0;
	uint32_t last_battinfo = 0;
	uint32_t node_status_period = 1000; // ms
	uint32_t battinfo_period = 100;
	bool sent_any = false;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//	  HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txHeader, txData);

//		while (canard.tx_queue = !NULL)
//		{

				frame = canardPeekTxQueue(&canard);
				if (frame)
				{
//						HAL_Delay(1);
						memset(&txHeader, 0, sizeof(txHeader));
						memset(txData, 0, sizeof(txData));
//						HAL_Delay(1);
						if (frame->id & CANARD_CAN_FRAME_EFF) {
								txHeader.Identifier = frame->id & CANARD_CAN_EXT_ID_MASK;
								txHeader.IdType = FDCAN_EXTENDED_ID;

						} else {
								txHeader.Identifier = frame->id;
								txHeader.IdType = FDCAN_STANDARD_ID;
						}
//						txHeader.RTR = CAN_RTR_DATA;
						txHeader.DataLength = frame->data_len;

//						txHeader.TransmitGlobalTime = DISABLE;
//						HAL_Delay(1);
						for (uint8_t i = 0; i < 8; i++) {
								txData[i] = (i < frame->data_len) ? frame->data[i] : 0;
						}
//						uint32_t txMailbox;

//						HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txHeader, txData);
//						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,1);  // Assuming onboard LED
//						HAL_Delay(1000);  // So you can see the blinking
//						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,0);  // Assuming onboard LED
//						HAL_Delay(1000);  // So you can see the blinking
//						if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) > 0)
//						{
//						    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txHeader, txData) == HAL_OK)
//						    {
////						        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
////						        HAL_Delay(1000);
////						        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
////						        HAL_Delay(1000);
//						    }
//						}
//						HAL_Delay(1);
						HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txHeader, txData);
//				}
				canardPopTxQueue(&canard);
		}
//		HAL_Delay(1);
		// Clean stale transfers (gi?ng process1HzTasks trong battery_node)
		canardCleanupStaleTransfers(&canard, HAL_GetTick() * 1000);

		HAL_Delay(1);
//		send_BatteryInfo();



//      const CanardCANFrame* frame = canardPeekTxQueue(&canard);
//      while (frame)
//      {
//          FDCAN_TxHeaderTypeDef txHeader = {0};
//          uint8_t txData[8] = {0};
//
//          if (frame->id & CANARD_CAN_FRAME_EFF)
//          {
//              txHeader.Identifier = frame->id & CANARD_CAN_EXT_ID_MASK;
//              txHeader.IdType = FDCAN_EXTENDED_ID;
//          }
//          else
//          {
//              txHeader.Identifier = frame->id;
//              txHeader.IdType = FDCAN_STANDARD_ID;
//          }
//          txHeader.DataLength = frame->data_len;
//
//          for (uint8_t i = 0; i < frame->data_len; i++)
//          {
//              txData[i] = frame->data[i];
//          }
//
//          HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txHeader, txData);
//          canardPopTxQueue(&canard);
//          frame = canardPeekTxQueue(&canard);
//      }
//
//      canardCleanupStaleTransfers(&canard, HAL_GetTick() * 1000);
//      HAL_Delay(1);
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 10;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 14;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 170;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1700;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

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
