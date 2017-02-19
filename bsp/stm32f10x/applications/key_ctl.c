#include <board.h>
#include <rtthread.h>

#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */

#include "includes.h"
#include "osd_menu.h"
#include "key_ctl.h"

#include <stdlib.h>

#ifndef BANK1_WRITE_START_ADDR
#define BANK1_WRITE_START_ADDR  ((uint32_t)0x0803c000)
#endif


#define	KEY_PORT1		GPIOA
#define	KEY_PORT2		GPIOB
#define	KEY_PORT3		GPIOC

enum key_type
{
	KEY_NONE,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0,

	KEY_ESC,
	KEY_CALL,
	
	KEY_PRESET,
	
	KEY_IRIS,//14

	KEY_MODE,//15
	KEY_FILTER,//
	
	KEY_F1,
	KEY_SETUP,//18
	
	KEY_F2,
	KEY_CAM,
	
	KEY_NO_CHANGE,

};



extern u8 iris_motor_mode;
extern u8 rs485_get_data_from_slave(void);
void key_value_all_clear(void);

u8 beep_enable = 1;//0,off; 1,on

u8 cam_para_mode=0;


u8 key_val_buffer_cnt = 0;
u16 key_val_buffer_func = 0;

u8 osd_mid_buff[30]={0};
u8 osd_mid_str_buff[10]={0};

u32 int_nu = 0;
u8 flag =0;

u8 ec11_power;      //火力0-100
s32 ec11_power_m=0;    //旋转编码器增量
u16 ec11_time=0;      //时间，分最大180分（3小时）
u16 ec11_time_m=100;          //旋转编码器增量


s32 BMQCounterTotal=0;
u8 RunRight = 0xff;

s32 BMQCounterTotal_zoom=0;
u8 RunRight_zoom = 0xff;

s32 BMQCounterTotal_focus=0;
u8 RunRight_focus = 0xff;

static struct rt_semaphore key_sem;

rt_mailbox_t joystick_mb;


void EXTI9_5_int(u8 mode)
{
	NVIC_InitTypeDef   NVIC_InitStructure;

  /* Enable and set EXTI9_5 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;

  if(mode)
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
else
	
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;

  NVIC_Init(&NVIC_InitStructure);

}


#if 1
void ec11_focus_key_interrupt(void)
{  
	static u8 Comparing = 0;
	static u16 PulAPol,PulBPol,PulState,PulLastState;
		
	if((EXTI_GetITStatus(EXTI_Line2) != RESET))
	{

			{                                                              
	    //TCNT3=0xfc41;                                               // 125uS
	    if(!Comparing) 
		{                                            
	       PulAPol = GPIO_ReadInputData(GPIOC)&0x000C;//PulAPol=PINE; 
		   PulLastState=PulAPol; 
		   Comparing++;   
	       PulAPol&=0x0004; 
		   PulLastState&=0x000C; 
		   PulAPol>>=2;    
		}                                                   

		if(Comparing)                                                         
		{                                                                    
			PulBPol=GPIO_ReadInputData(GPIOC)&0x000C;                                                     
			PulState=PulBPol;                                                
			PulState&=0x000C;                                                   
			PulBPol&=0x0008;                                                   
			PulBPol>>=3;                                                      
			if(PulState!=PulLastState) 
			{ 
				if(PulBPol==PulAPol) 
				{ 
					RunRight_focus=1; BMQCounterTotal_focus--;
				}
				else 
				{ 
					RunRight_focus=0; BMQCounterTotal_focus++; 
				} 
				Comparing=0;  
			}  
		}                                                                    
	    if(!Comparing) 
		{                                                    
			PulAPol=PulState; 
			PulLastState=PulAPol; 
			Comparing++; 
			PulAPol&=0x0004; 
			PulLastState&=0x000C; 
			PulAPol>>=2;      
		}                                                     
	}
		

	/* Clear the  EXTI line 8 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line2);
	}

}

#else
void ec11_focus_key_interrupt(void)
{  
	static u8 Comparing = 0;
	static u16 PulAPol,PulBPol,PulState,PulLastState;
		
	if((EXTI_GetITStatus(EXTI_Line2) != RESET))
	{

			{                                                              
	    //TCNT3=0xfc41;                                               // 125uS
	    if(!Comparing) 
		{                                            
	       PulAPol = GPIO_ReadInputData(GPIOC);//PulAPol=PINE; 
		   PulLastState=PulAPol; 
		   Comparing++;   
	       PulAPol&=0x0004; 
		   PulLastState&=0x000C; 
		   PulAPol>>=2;    
		}                                                   

		if(Comparing)                                                         
		{                                                                    
			PulBPol=GPIO_ReadInputData(GPIOC);                                                     
			PulState=PulBPol;                                                
			PulState&=0x000C;                                                   
			PulBPol&=0x0008;                                                   
			PulBPol>>=3;                                                      
			if(PulState!=PulLastState) 
			{ 
				if(PulBPol==PulAPol) 
				{ 
					RunRight_focus=1; BMQCounterTotal_focus--;
				}
				else 
				{ 
					RunRight_focus=0; BMQCounterTotal_focus++; 
				} 
				Comparing=0;  
			}  
		}                                                                    
	    if(!Comparing) 
		{                                                    
			PulAPol=PulState; 
			PulLastState=PulAPol; 
			Comparing++; 
			PulAPol&=0x0004; 
			PulLastState&=0x000C; 
			PulAPol>>=2;      
		}                                                     
	}
		

	/* Clear the  EXTI line 8 pending bit */
		if((EXTI_GetITStatus(EXTI_Line2) != RESET))
			EXTI_ClearITPendingBit(EXTI_Line2);
		else
			EXTI_ClearITPendingBit(EXTI_Line3);
	}

}
#endif


#if 1
void ec11_zoom_key_interrupt(void)
{  
	static u8 Comparing = 0;
	static u16 PulAPol,PulBPol,PulState,PulLastState;

	
	if((EXTI_GetITStatus(EXTI_Line14) != RESET))
	{

			{                                                              
	    //TCNT3=0xfc41;                                               // 125uS
	    if(!Comparing) 
		{                                            
	       PulAPol = ((uint16_t)GPIOC->IDR)&0xC000;//GPIO_ReadInputData(GPIOC)&0xC000;//PulAPol=PINE; 
		   PulLastState=PulAPol; 
		   Comparing++;   
	       PulAPol&=0x4000; 
		   PulLastState&=0xC000; 
		   PulAPol>>=14;    
		}                                                   

		if(Comparing)                                                         
		{                                                                    
			PulBPol=((uint16_t)GPIOC->IDR)&0xC000;                                                     
			PulState=PulBPol;                                                
			PulState&=0xC000;                                                   
			PulBPol&=0x8000;                                                   
			PulBPol>>=15;                                                      
			if(PulState!=PulLastState) 
			{ 
				if(PulBPol==PulAPol) 
				{ 
					RunRight_zoom=1; BMQCounterTotal_zoom--;
				}
				else 
				{ 
					RunRight_zoom=0; BMQCounterTotal_zoom++; 
				} 
				Comparing=0;  
			}  
		}                                                                    
	    if(!Comparing) 
		{                                                    
			PulAPol=PulState; 
			PulLastState=PulAPol; 
			Comparing++; 
			PulAPol&=0x4000; 
			PulLastState&=0xC000; 
			PulAPol>>=14;      
		}                                                     
	}
		

	/* Clear the  EXTI line 8 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line14);
	}

}


#else
void ec11_zoom_key_interrupt(void)
{  
	static u8 Comparing = 0;
	static u16 PulAPol,PulBPol,PulState,PulLastState;
		
	if((EXTI_GetITStatus(EXTI_Line14) != RESET))
	{

			{                                                              
	    //TCNT3=0xfc41;                                               // 125uS
	    if(!Comparing) 
		{                                            
	       PulAPol = GPIO_ReadInputData(GPIOC);//PulAPol=PINE; 
		   PulLastState=PulAPol; 
		   Comparing++;   
	       PulAPol&=0x4000; 
		   PulLastState&=0xC000; 
		   PulAPol>>=14;    
		}                                                   

		if(Comparing)                                                         
		{                                                                    
			PulBPol=GPIO_ReadInputData(GPIOC);                                                     
			PulState=PulBPol;                                                
			PulState&=0xC000;                                                   
			PulBPol&=0x8000;                                                   
			PulBPol>>=15;                                                      
			if(PulState!=PulLastState) 
			{ 
				if(PulBPol==PulAPol) 
				{ 
					RunRight_zoom=1; BMQCounterTotal_zoom--;
				}
				else 
				{ 
					RunRight_zoom=0; BMQCounterTotal_zoom++; 
				} 
				Comparing=0;  
			}  
		}                                                                    
	    if(!Comparing) 
		{                                                    
			PulAPol=PulState; 
			PulLastState=PulAPol; 
			Comparing++; 
			PulAPol&=0x4000; 
			PulLastState&=0xC000; 
			PulAPol>>=14;      
		}                                                     
	}
		

	/* Clear the  EXTI line 8 pending bit */
		if((EXTI_GetITStatus(EXTI_Line14) != RESET))
			EXTI_ClearITPendingBit(EXTI_Line14);
		else
			EXTI_ClearITPendingBit(EXTI_Line15);
	}

}
#endif


#if 1

void ec11_key_interrupt(void)
{  
	static u8 Comparing = 0;
	static u16 PulAPol,PulBPol,PulState,PulLastState;
		
	if((EXTI_GetITStatus(EXTI_Line7) != RESET))
	{

			{															   
		//TCNT3=0xfc41; 											  // 125uS
		if(!Comparing) 
		{											 
		   PulAPol = GPIO_ReadInputData(GPIOB)&0x0180;//PulAPol=PINE; 
		   PulLastState=PulAPol; 
		   Comparing++;   
		   PulAPol&=0x0080; 
		   PulLastState&=0x0180; 
		   PulAPol>>=7;    
		}													

		if(Comparing)														  
		{																	 
			PulBPol=GPIO_ReadInputData(GPIOB)&0x0180;													   
			PulState=PulBPol;												 
			PulState&=0x0180;													
			PulBPol&=0x0100;												   
			PulBPol>>=8;													  
			if(PulState!=PulLastState) 
			{ 
				if(PulBPol==PulAPol) 
				{ 
					RunRight=1; BMQCounterTotal--;
				}
				else 
				{ 
					RunRight=0; BMQCounterTotal++; 
				} 
				Comparing=0;  
			}  
		}																	 
		if(!Comparing) 
		{													 
			PulAPol=PulState; 
			PulLastState=PulAPol; 
			Comparing++; 
			PulAPol&=0x0080; 
			PulLastState&=0x0180; 
			PulAPol>>=7;	  
		}													  
	}
		

	/* Clear the  EXTI line 8 pending bit */
		if((EXTI_GetITStatus(EXTI_Line8) != RESET))
			EXTI_ClearITPendingBit(EXTI_Line8);
		else
			EXTI_ClearITPendingBit(EXTI_Line7);
	}

}

#else
void ec11_key_interrupt(void)
{  
	static u8 Comparing = 0;
	static u16 PulAPol,PulBPol,PulState,PulLastState;
		
	if((EXTI_GetITStatus(EXTI_Line7) != RESET) || (EXTI_GetITStatus(EXTI_Line8) != RESET))
	{

			{															   
		//TCNT3=0xfc41; 											  // 125uS
		if(!Comparing) 
		{											 
		   PulAPol = GPIO_ReadInputData(GPIOB);//PulAPol=PINE; 
		   PulLastState=PulAPol; 
		   Comparing++;   
		   PulAPol&=0x0080; 
		   PulLastState&=0x0180; 
		   PulAPol>>=7;    
		}													

		if(Comparing)														  
		{																	 
			PulBPol=GPIO_ReadInputData(GPIOB);													   
			PulState=PulBPol;												 
			PulState&=0x0180;													
			PulBPol&=0x0100;												   
			PulBPol>>=8;													  
			if(PulState!=PulLastState) 
			{ 
				if(PulBPol==PulAPol) 
				{ 
					RunRight=1; BMQCounterTotal--;
				}
				else 
				{ 
					RunRight=0; BMQCounterTotal++; 
				} 
				Comparing=0;  
			}  
		}																	 
		if(!Comparing) 
		{													 
			PulAPol=PulState; 
			PulLastState=PulAPol; 
			Comparing++; 
			PulAPol&=0x0080; 
			PulLastState&=0x0180; 
			PulAPol>>=7;	  
		}													  
	}
		

	/* Clear the  EXTI line 8 pending bit */
		if((EXTI_GetITStatus(EXTI_Line8) != RESET))
			EXTI_ClearITPendingBit(EXTI_Line8);
		else
			EXTI_ClearITPendingBit(EXTI_Line7);
	}

}
#endif





/**
  * @brief  Configure PB.09 or PG.08 in interrupt mode
  * @param  None
  * @retval None
  */
void EXTI9_5_Config(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

  /* Enable GPIOB clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  /* Configure PB.09 pin as input floating */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

#if 1
  /* Enable AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  /* Connect EXTI9 Line to PB.09 pin */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource7);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);

  /* Configure EXTI9 line */
  EXTI_InitStructure.EXTI_Line = EXTI_Line7;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
 
 //EXTI_InitStructure.EXTI_Line = EXTI_Line8;
 // EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI9_5 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//DISABLE;//ENABLE;

  NVIC_Init(&NVIC_InitStructure);
#endif
}



//旋钮 1
void key_2_pin_init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	
	GPIO_Init(GPIOD, &GPIO_InitStructure);	

	EXTI9_5_Config();
}


//旋钮 2
void ec11_key_zoom_pin_init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);	




#if 1
	EXTI_InitTypeDef	 EXTI_InitStructure;
	NVIC_InitTypeDef	 NVIC_InitStructure;

	/* Enable AFIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	/* Connect EXTI9 Line to PB.09 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource14);

	/* Configure EXTI9 line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line14;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI9_5 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//DISABLE;//ENABLE;

	NVIC_Init(&NVIC_InitStructure);
#endif



}


//旋钮 3
void ec11_key_focus_pin_init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	



#if 1
	EXTI_InitTypeDef	 EXTI_InitStructure;
	NVIC_InitTypeDef	 NVIC_InitStructure;

	/* Enable AFIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	/* Connect EXTI9 Line to PB.09 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);

	/* Configure EXTI9 line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI9_5 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//DISABLE;//ENABLE;

	NVIC_Init(&NVIC_InitStructure);
#endif




}



u8 key_pb78_state = 0;


u8 key2_check_dir(void)
{
	
	if(key_pb78_state==1)
		return 1;// cw
	else if(key_pb78_state==2)
		return 2;//ccw
	else
		return 0;//no
}


//0,press; 1,no press
u8 key2_press_check(void)
{
	if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2) == 0)
	{
		rt_thread_delay(20);

			if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2) == 0)
				return 0;
	}

	return 1;
}


//0,press; 1,no press
u8 key2_focus_press_check(void)
{
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0)
	{
		rt_thread_delay(20);

			if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0)
				return 0;
	}

	return 1;
}


//0,press; 1,no press
u8 key2_zoom_press_check(void)
{
	if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 0)
	{
		rt_thread_delay(20);

			if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 0)
				return 0;
	}

	return 1;
}



#define ADC_CHN_M 2 //为2个通道 0,1
#define ADC_CHN_N (ADC_CHN_M*16) //每通道采16次

static vu16 Photoreg_ADC1_ConvertedValue[ADC_CHN_N];
static volatile u8 ADC_Ok=0;

static void Photoreg_ADC2_GPIO_INIT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0; //ADC1-light
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

}

static DMA_InitTypeDef DMA_InitStructure;

//DMA的配置
static void Photoreg_DMA_Configuration(void)
{
	
	NVIC_InitTypeDef NVIC_InitStructure;
	
	
	/* open the DMA1 intterrupt service */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;//ENABLE;
	NVIC_Init(&NVIC_InitStructure); 

	/* 允许 DMA1 */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	/* DMA通道1*/
	DMA_DeInit(DMA1_Channel1);
	//指定DMA外设基地址
	DMA_InitStructure.DMA_PeripheralBaseAddr =(u32)( &(ADC1->DR));		//ADC1数据寄存器
	//设定DMA内存基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Photoreg_ADC1_ConvertedValue;					//获取ADC的数组
	//外设作为数据传输的来源
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//片内外设作源头
	//指定DMA通道的DMA缓存大小
	DMA_InitStructure.DMA_BufferSize = ADC_CHN_N;								//每次DMA16个数据
	//外设地址不递增（不变）
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//外设地址不增加
	//内存地址不递增（不变）
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//内存地址增加
	//设定外设数据宽度为16位
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//半字
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//半字
	//设定DMA的工作模式普通模式，还有一种是循环模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//普通模式
	//设定DMA通道的软件优先级
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;							//高优先级
	//使能DMA内存到内存的传输，此处没有内存到内存的传输
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//非内存到内存
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	//DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);								//DMA通道1传输完成中断
	
	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

static void Photoreg_ADC_Configuration(void)
{
	ADC_InitTypeDef ADC_InitStructure;

	/* 允许ADC */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	/* ADC1 */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;						//独立模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;							//单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;						//连续扫描
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; 	//软件启动转换
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;					//数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 2; 								//1个通道
	ADC_Init(ADC1, &ADC_InitStructure);

	/* 配置通道1的采样速度,*/ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5);
	/* 配置通道0的采样速度,*/ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_239Cycles5);
	/* 配置通道0的采样速度,*/ 
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_239Cycles5);
//	/* 配置通道0的采样速度,*/ 
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_239Cycles5);


	/* 允许ADC1的DMA模式 */
	ADC_DMACmd(ADC1, ENABLE);

	/* 允许ADC1*/
	ADC_Cmd(ADC1, ENABLE);

	/*重置校准寄存器 */   
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));

	/*开始校准状态*/
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	   
	/* 人工打开ADC转换.*/ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);


}

//static void DMAReConfig(void)
//{
//	DMA_DeInit(DMA1_Channel1);
//	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
//	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
//	DMA_Cmd(DMA1_Channel1, ENABLE);
//}



static void joystick_ADC_Init(void)
{

	Photoreg_ADC2_GPIO_INIT();
	Photoreg_DMA_Configuration();
	Photoreg_ADC_Configuration();

   
}



#define	ADC_SAMPLE_NUM	16	//ADC值抽样数	
#define	ADC_MIDDLE_START_NUM	(ADC_SAMPLE_NUM/5)
#define	ADC_MIDDLE_END_NUM		(ADC_SAMPLE_NUM - ADC_MIDDLE_START_NUM)
#define	ADC_SAMPLE_VALID_SIZE	(ADC_SAMPLE_NUM - ADC_MIDDLE_START_NUM * 2)	

/**************************************************************
** 函数名:DigitFilter
** 功能:软件滤波
** 注意事项:取NO 的2/5 作为头尾忽略值,注意N 要大于5,否则不会去头尾
***************************************************************/

static u16 DigitFilter(u16* buf,u8 no)
{
	u8 i,j;
	u32 tmp;
	u16 Pravite_ADC_buf[16];
	
	for(i=0;i<no;i++)
	{
		Pravite_ADC_buf[i] = buf[i];
	 	buf[i] = 0;
	}	
	//排序，将buf[0]到buf[no-1]从大到小排列
	for(i=0;i<no;i++)
	{
		for(j=0;j<no-i-1;j++)
		{
			if(Pravite_ADC_buf[j]>Pravite_ADC_buf[j+1])
			{
				tmp=Pravite_ADC_buf[j];
				Pravite_ADC_buf[j]=Pravite_ADC_buf[j+1];
				Pravite_ADC_buf[j+1]=tmp;
			}
		}
	}


	//平均
	tmp=0;
	//for(i=cut_no;i<no-cut_no;i++) //只取中间n-2*cut_no 个求平均
	for(i=ADC_MIDDLE_START_NUM;i<ADC_MIDDLE_END_NUM;i++) //只取中间n-2*cut_no 个求平均
		tmp+=Pravite_ADC_buf[i];
	return(tmp/ADC_SAMPLE_VALID_SIZE);
}

static void joystick_pin_init(void)
{

	joystick_ADC_Init();


	GPIO_InitTypeDef GPIOD_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_3; //SW22
    GPIOD_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIOD_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT2, &GPIOD_InitStructure);	

}


#define	JOYSTICK_UD_VOL_MID_MAX		1900
#define	JOYSTICK_UD_VOL_MID_MIN		1400

#define	JOYSTICK_UD_VOL_MID_MAX_2		2000



#define	JOYSTICK_LR_VOL_MID_MAX		1900
#define	JOYSTICK_LR_VOL_MID_MIN		1400

#define	JOYSTICK_LR_VOL_MID_MAX_2		2000


#define	JOYSTICK_VOL_PER_LEVEL_VOL		200

u16 joystick_lr_value,joystick_up_value;


enum JOYSTICK_DIR_TYPE
{
JOYSTICK_DIR_NONE,
JOYSTICK_DIR_LEFT,
JOYSTICK_DIR_RIGHT,
JOYSTICK_DIR_UP,
JOYSTICK_DIR_DOWN,
JOYSTICK_DIR_LU,
JOYSTICK_DIR_LD,
JOYSTICK_DIR_RU,
JOYSTICK_DIR_RD,

};

enum JOYSTICK_DIR_TYPE joystick_dir_state = JOYSTICK_DIR_NONE;
enum JOYSTICK_DIR_TYPE joystick_lr_dir_state = JOYSTICK_DIR_NONE;
enum JOYSTICK_DIR_TYPE joystick_ud_dir_state = JOYSTICK_DIR_NONE;

u8 joystick_lr_speed = 0,joystick_lr_speed_pre;
u8 joystick_ud_speed = 0,joystick_ud_speed_pre;

const u8 *joystick_msg[]=
{
	{"     "},
	{"Left      "},
	{"Right     "},
	{"Up        "},
	{"Down      "},
	{"LU        "},
	{"LD        "},
	{"RU        "},
	{"RD        "},
	{"OK        "},
	{"Left      "},
};

#define	BEEP_DELAY		200

static void joystick_handle(void)
{

	u8 lrudcmd = 0,lrudcmd2 = 0,lrspeed,udspeed;
	static u8 prestate = 0;

	if(joystick_ud_dir_state == JOYSTICK_DIR_DOWN)
	{
		lrudcmd = 0x10;
	}
	else if(joystick_ud_dir_state == JOYSTICK_DIR_UP)
	{
		
		lrudcmd = 0x08;
	}

	if(joystick_lr_dir_state == JOYSTICK_DIR_LEFT)
	{
		lrudcmd2 = 0x04;
	}
	else if(joystick_lr_dir_state == JOYSTICK_DIR_RIGHT)
	{

		lrudcmd2 = 0x02;
	}

	lrudcmd2 = lrudcmd2|lrudcmd;

	lrspeed = joystick_lr_speed;
	udspeed = joystick_ud_speed;


	if(joystick_lr_speed==0 && joystick_ud_speed==0  )
	{
		if(prestate==0)
		{
		prestate = 1;
		pelcod_stop_packet_send();
				memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
		strcat(osd_mid_str_buff,joystick_msg[0]);

		
		osd_line1_val_disp_clear();

		//key_val_buffer_cnt = 0xff;
		
		osd_line1_disp(0);
		
		rt_thread_delay(40);
		
		//key_value_all_clear();
		}
	}
	else
	{

	
		prestate = 0;
		pelcod_lrud_pre_packet_send(lrudcmd2,lrspeed, udspeed);

		u8 tmp;

		if(joystick_lr_dir_state!=JOYSTICK_DIR_NONE && joystick_ud_dir_state==JOYSTICK_DIR_NONE)
		{
			tmp = joystick_lr_dir_state;
		}
		else if(joystick_lr_dir_state==JOYSTICK_DIR_NONE && joystick_ud_dir_state!=JOYSTICK_DIR_NONE)
		{
			tmp = joystick_ud_dir_state;
		}
		else if(joystick_lr_dir_state!=JOYSTICK_DIR_NONE && joystick_ud_dir_state!=JOYSTICK_DIR_NONE)
		{
			tmp = joystick_lr_dir_state + joystick_ud_dir_state + joystick_lr_dir_state;
		}
		else
			tmp = 0;

		memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
		strcat(osd_mid_str_buff,joystick_msg[tmp]);

		
		osd_line1_val_disp_clear();
		osd_line1_disp(0);
		rt_thread_delay(40);
		//key_value_all_clear();
	}
	
}

#define	VOLATAGE_SPEED_VAL_BASE	 200


static u16  Get_joystick_Value(void)
{
	u8 i;
	u16 adc_value_tmp[16],adc_value2_tmp[16];
	u8 joystick_state=0;

	
    for (i = 0;i < 16;i++)  
    {  
        adc_value_tmp[i] = Photoreg_ADC1_ConvertedValue[i * ADC_CHN_M];   
    }  
	
    for (i = 0;i < 16;i++)  
    {  
        adc_value2_tmp[i] = Photoreg_ADC1_ConvertedValue[i * ADC_CHN_M+1];   
    } 	


	joystick_lr_value = DigitFilter((u16 *)adc_value_tmp,ADC_SAMPLE_NUM)*3300/0xFFF;
	joystick_up_value = DigitFilter((u16 *)adc_value2_tmp,ADC_SAMPLE_NUM)*3300/0xFFF;

	//DMAReConfig();

	if((JOYSTICK_LR_VOL_MID_MIN<joystick_lr_value) && (JOYSTICK_LR_VOL_MID_MAX>joystick_lr_value))
	{
		joystick_lr_dir_state = JOYSTICK_DIR_NONE;
		joystick_lr_speed = 0;
	}
	else if(joystick_lr_value < JOYSTICK_LR_VOL_MID_MIN)
	{
		joystick_lr_dir_state = JOYSTICK_DIR_LEFT;
		joystick_lr_speed = (JOYSTICK_LR_VOL_MID_MIN - joystick_lr_value)/VOLATAGE_SPEED_VAL_BASE + 1;

	}
	else if(joystick_lr_value > JOYSTICK_LR_VOL_MID_MAX_2)
	{
		joystick_lr_dir_state = JOYSTICK_DIR_RIGHT;
		joystick_lr_speed = (joystick_lr_value - JOYSTICK_LR_VOL_MID_MAX)/VOLATAGE_SPEED_VAL_BASE + 1;

	}

	if((JOYSTICK_UD_VOL_MID_MIN<joystick_up_value) && (JOYSTICK_UD_VOL_MID_MAX>joystick_up_value))
	{
		joystick_ud_dir_state = JOYSTICK_DIR_NONE;
		joystick_ud_speed = 0;
	}
	else if(joystick_up_value < JOYSTICK_UD_VOL_MID_MIN)
	{
		joystick_ud_dir_state = JOYSTICK_DIR_DOWN;
		joystick_ud_speed = (JOYSTICK_UD_VOL_MID_MIN - joystick_up_value)/VOLATAGE_SPEED_VAL_BASE + 1;

	}
	else if(joystick_up_value > JOYSTICK_UD_VOL_MID_MAX_2)
	{
		joystick_ud_dir_state = JOYSTICK_DIR_UP;
		joystick_ud_speed = (joystick_up_value - JOYSTICK_UD_VOL_MID_MAX)/VOLATAGE_SPEED_VAL_BASE + 1;

	}

	joystick_handle();

	if((joystick_lr_dir_state != JOYSTICK_DIR_NONE) || (joystick_ud_dir_state != JOYSTICK_DIR_NONE))
	{
		joystick_state = 1;
		
	}

	return 0;
}



u16 zoom_key_val,focus_key_val;
#define	ZOOM_VOL_MID_MAX		1850
#define	ZOOM_VOL_MID_MIN		1500
#define	ZOOM_VOL_PER_LEVEL_VOL		100

#define	FOCUS_VOL_MID_MAX		1850
#define	FOCUS_VOL_MID_MIN		1500
#define	FOCUS_VOL_PER_LEVEL_VOL		100

s16 zoom_speed_dir = 0;
s16 focus_speed_dir = 0;

static u16  Get_zoom_focus_key_Value(void)
{
//	u32 sum = 0;
	u8 i;
	u16 adc_value_tmp[16],adc_value2_tmp[16];
	
    for (i = 0;i < 16;i++)  
    {  
        adc_value_tmp[i] = Photoreg_ADC1_ConvertedValue[i * 4+2];   
    }  
	
    for (i = 0;i < 16;i++)  
    {  
        adc_value2_tmp[i] = Photoreg_ADC1_ConvertedValue[i * 4+3];   
    } 	


	zoom_key_val = DigitFilter((u16 *)adc_value_tmp,ADC_SAMPLE_NUM)*3300/0xFFF;
	focus_key_val = DigitFilter((u16 *)adc_value2_tmp,ADC_SAMPLE_NUM)*3300/0xFFF;

	u16 ztmp,ftmp;
	
	ztmp = zoom_key_val;
	ftmp = focus_key_val;

	if(ztmp > ZOOM_VOL_MID_MIN && ztmp < ZOOM_VOL_MID_MAX)
	{
		zoom_speed_dir = 0;

	}
	else if(ztmp < ZOOM_VOL_MID_MIN)
	{
		zoom_speed_dir = -((ZOOM_VOL_MID_MIN - ztmp)/ZOOM_VOL_PER_LEVEL_VOL + 1);
	}
	else
	{
		zoom_speed_dir = ((ztmp-ZOOM_VOL_MID_MAX)/ZOOM_VOL_PER_LEVEL_VOL + 1);

	}

	
	if(ftmp > FOCUS_VOL_MID_MIN && ftmp < FOCUS_VOL_MID_MAX)
	{
		focus_speed_dir = 0;

	}
	else if(ftmp < FOCUS_VOL_MID_MIN)
	{
		focus_speed_dir = -((FOCUS_VOL_MID_MIN - ftmp)/FOCUS_VOL_PER_LEVEL_VOL + 1);
	}
	else
	{
		focus_speed_dir = ((ftmp-FOCUS_VOL_MID_MAX)/FOCUS_VOL_PER_LEVEL_VOL + 1);

	}

	return 0;
}




void zoomfocus_key_handle(void)
{
	static s8 zoom_speed_dir_pre = 0,focus_speed_dir_pre = 0;

	Get_zoom_focus_key_Value();

	if(zoom_speed_dir > 0 )
	{
		pelcod_zf_packet_send(1,zoom_speed_dir);
		osd_opt_message_disp(6,1);

	}
	else if(zoom_speed_dir < 0)
	{
		pelcod_zf_packet_send(2,abs(zoom_speed_dir));

		osd_opt_message_disp(7,1);
	}
	else if(zoom_speed_dir==0 && zoom_speed_dir_pre!=0)
	{
		pelcod_zf_packet_send(0,1);

	}
	zoom_speed_dir_pre = zoom_speed_dir;

	if(focus_speed_dir > 0 )
	{
				pelcod_zf_packet_send(3,focus_speed_dir);

		osd_opt_message_disp(8,1);

	}
	else if(focus_speed_dir < 0)
	{
				pelcod_zf_packet_send(4,abs(focus_speed_dir));

		osd_opt_message_disp(9,1);
	}
	else if(focus_speed_dir==0 && focus_speed_dir_pre!=0)
	{
				pelcod_zf_packet_send(0,1);

	}
	focus_speed_dir_pre = focus_speed_dir;

	
}

void key_pin_init(void)
{

	GPIO_InitTypeDef GPIOD_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;
    GPIOD_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIOD_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT1, &GPIOD_InitStructure);	

	GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_Init(KEY_PORT2, &GPIOD_InitStructure);	

	GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_12;
	GPIO_Init(KEY_PORT3, &GPIOD_InitStructure);	


	GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIOD_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIOD_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIOD_InitStructure);	

	GPIO_WriteBit(GPIOC,GPIO_Pin_4, Bit_RESET);
	
	key_2_pin_init();
	ec11_key_zoom_pin_init();
	ec11_key_focus_pin_init();
}

u16 key_pre = 0;


u32 key_merge(void)
{
	u32 data = 0,data2 = 0,data3 = 0;

	u32 key_tmp;
	
	//PA,PB,PC
	data = GPIO_ReadInputData(KEY_PORT1);
	data2 = GPIO_ReadInputData(KEY_PORT2);	
	data3 = GPIO_ReadInputData(KEY_PORT3);	
		
	key_tmp = (data2>>15)&0x0001;//0
	key_tmp |= (data3>>5)&0x0006;//1-2
	key_tmp |= (data2>>11)&0x0008;// 3
	key_tmp |= (data2>>9)&0x0010;// 4
	key_tmp |= (data2>>7)&0x0020;// 5
	key_tmp |= (data2>>4)&0x0040;// 6

	key_tmp |= (data2<<5)&0x0080;//7
	key_tmp |= (data2<<7)&0x0100;//8
	key_tmp |= (data<<3)&0x0200;//9

	key_tmp |= (data3<<2)&0x00000400;//10
	key_tmp |= (data3<<2)&0x00000800;//11

	key_tmp |= (data<<4)&0x1000;//12
	key_tmp |= (data>>2)&0x2000;//13//key_tmp |= (data<<2)&0x2000;//13
	key_tmp |= (data3<<2)&0x4000;//14//key_tmp |= (data<<2)&0x4000;//14

	key_tmp |= (data2<<4)&0x8000;//15
	key_tmp |= (data2<<16)&0x10000;//16

	key_tmp |= (data<<13)&0x20000;//17
	key_tmp |= (data<<13)&0x40000;//18
	key_tmp |= (data<<12)&0x80000;//19

	return key_tmp;
}


u8 long_key_state = 0;

#define		KEY_DELAY_CHECK_MS		40


//返回0为无按键，返回非0值，则为对应的按键号
static u32 key_ctl_check(void)
{
	u16 i;
	u32 key_tmp;
	static u32 long_press_cnt = 0;// 50ms
	
	key_tmp = key_merge();
	for(i=0;i<20;i++)
	{
		if(((key_tmp>>i)&0x0001)==0)
		{
			rt_thread_delay(KEY_DELAY_CHECK_MS);

			key_tmp = key_merge();

			if(((key_tmp>>i)&0x0001)==0)
			{
				if(key_pre == i+1)
				{
					if(long_press_cnt>20)
					{

						long_press_cnt=0;
						key_pre = (i+1)|0x9000;
						long_key_state = 1;
						return ((i+1)|0x9000);
					}

					if(long_key_state == 0)
						long_press_cnt++;
				}
				else if((key_pre&0x9000) == i+1)
				{

						//key_pre = (i+1)|0x9000;
						//long_key_state = 1;
						return (0);
				}
				key_pre = i+1;
				//return (i+1);
				break;
			}
		}
	}


	
	if((key_pre>0x9000)&& (key_pre&0x9000 == (i+1)) && i<20)
	{
		if(long_key_state)
		{
			long_key_state = 0;
		}

		key_pre = 0;
		return (i+1);

	}

	
	if((i+1)>=KEY_1 && (i+1)<=KEY_0)
	{
		return (i+1);

	}
	else
	{
		if((key_pre && key_pre!=(i+1))||(key_pre && i==20))
		{
			if(long_key_state)
			{
				long_key_state = 0;
				key_pre = 0;
				return 0;
			}
			
			i = key_pre|0x8000;
			key_pre = 0;
			return i;

		}
	}
	return 0;
}


extern rt_sem_t	uart1_sem;


u8 iris_auto_manual_state = 0;// 默认0 为自动模式
void iris_auto_manual_switch(void)
{


}


void iris_auto_manual_set(u8 mode)
{

}


void num_to_string(u16 data,u8 *dst)
{

	dst[0] = data/1000+0x30;
	dst[1] = data/100%10+0x30;
	dst[2] = data%100/10+0x30;
	dst[3] = data%10+0x30;
}

u32 key_num_val = 0;

u8 osd_mid_buff_pointer=0;

#define	OSD_MSG_DISP_MAX_SECOND		40

#define	key_to_long(val)	(val|0x9000)
#define	key_to_release(val)	(val|0x8000)

u8 iris_mode=0;
u8 iris_mode_setup=0;

u8 cam_filter_mode = 1;


void key_value_all_clear(void)
{
	memset(osd_mid_buff,0,sizeof(osd_mid_buff));
	memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));

	key_num_val = 0;
	key_val_buffer_cnt = 0;
	osd_mid_buff_pointer=0;
	key_val_buffer_func = 0;
	osd_line1_val_disp_clear();
}


 enum key_type key_function_state = KEY_NONE;


extern uchar keyboard_data_buffer[20];
//return 0,failed; 1,success
u8 wait_device_reply(u8* srcdata,u8 len,u32 w_100ms)
{

return 0;
	
}


u8 cmd_buff[7];


u8 baudrate_to_num(u16 br)
{
	switch(br)
	{
	case 1200:

		return 0;
	case 2400:

		return 1;
	case 4800:

		return 2;
	case 9600:

		return 3;
	default:break;

	}
	return 1;
}


u16 num_to_baudrate(u8 numb)
{
	switch(numb)
	{
	case 0:

		return 1200;
	case 1:

		return 2400;
	case 2:

		return 4800;
	case 3:

		return 9600;
	default:break;

	}
	return 2400;
}


extern int flash_program_my(void);

void key_analyze(u16 val)
{

	switch(val)
	{
	case key_to_release(KEY_CALL):

		if(key_val_buffer_cnt)
		{
			pelcod_call_pre_packet_send((u8)key_num_val);

			osd_opt_message_disp(0,OSD_MSG_DISP_MAX_SECOND);

			
			if(key_num_val==125)
			{
				iris_motor_mode = 0;
			
				flash_program_my();
				
				osd_line3_disp(1);
				osd_opt_message_disp(16+iris_mode,OSD_MSG_DISP_MAX_SECOND);
				rs485_get_data_from_slave();


			}

			switch(key_num_val)
				{
			case 80:
			case 81:
			case 82:
			case 83:
				cam_filter_mode = key_num_val-80;
				break;
			default:
				break;
			}

		}
		key_value_all_clear();

		
		break;
	case key_to_release(KEY_PRESET):
		if(key_val_buffer_cnt)
		{
			pelcod_set_pre_packet_send((u8)key_num_val);
			osd_opt_message_disp(1,OSD_MSG_DISP_MAX_SECOND);

			if(key_num_val == 125 || key_num_val == 130)
			{

			
				if(key_num_val==125)
					iris_motor_mode = 1;
				else if(key_num_val==130)
						iris_motor_mode = 2;

				flash_program_my();
				
				osd_line3_disp(1);
				osd_opt_message_disp(16+iris_mode,OSD_MSG_DISP_MAX_SECOND);
				rs485_get_data_from_slave();

			}

		}
		key_value_all_clear();

		
		break;

	case key_to_release(KEY_CAM):

		if((key_val_buffer_cnt>0)&&(key_val_buffer_cnt<4))
		{if(key_num_val < 256)
		{
			domeNo = key_num_val;

			//osd_opt_message_disp(0,OSD_MSG_DISP_MAX_SECOND);

			osd_line_little_4_disp(1);

			key_value_all_clear();
		}
		}
		else if(key_val_buffer_cnt==0)
		{
			if(key_val_buffer_func == 0)
			{key_val_buffer_func = key_to_release(KEY_CAM);
			strcat(osd_mid_str_buff," ID+");
			osd_line1_disp(32);
			}

		}
		break;
	case key_to_release(KEY_IRIS):

		if(key_num_val==0)
		{
			//key_value_all_clear();
			
			{
				osd_line3_disp(1);
				//iris_mode = key_num_val-1;

				if(iris_motor_mode)
				{
					if(iris_mode == 1)
					{
						iris_mode = 0;

						pelcod_set_pre_packet_send(127);

						}
					else
					{
						iris_mode = 1;
						pelcod_call_pre_packet_send(126);

						}

					//rt_thread_delay(200);
					
					//osd_line2_disp(1);
					osd_line3_disp(1);
					
					osd_opt_message_disp_extend(16+iris_mode);
					rs485_get_data_from_slave();
					osd_opt_message_disp(16+iris_mode,20);
				}
				else
				{
					osd_opt_message_disp_iris(5);

				}
				
			key_value_all_clear();
			}
		}
		
	
		break;

	case key_to_release(KEY_MODE):

		if(key_num_val==0)
			key_value_all_clear();
		if(key_num_val > 0 && key_num_val < 9)
		{	
			pelcod_call_pre_packet_send((u8)key_num_val+200);
			cam_para_mode = key_num_val-1;
			osd_line2_disp(key_num_val);
			osd_opt_message_disp(3,OSD_MSG_DISP_MAX_SECOND);
			key_value_all_clear();

		}
		else if(key_val_buffer_cnt==0)
		{
			if(key_val_buffer_func == 0)
			{key_val_buffer_func = key_to_release(KEY_MODE);
			strcat(osd_mid_str_buff,"Mode+");
			osd_line1_disp(32);
			}

		}
		//osd_opt_message_disp(0,OSD_MSG_DISP_MAX_SECOND);
		break;
	case key_to_release(KEY_SETUP):
			if(key_num_val > 0 && key_num_val < 9)
			{
				if(key_val_buffer_func == key_to_release(KEY_MODE))
				{
					if(key_num_val <= 4)
					{
					pelcod_set_pre_packet_send(255);
					pelcod_set_pre_packet_send(253);
				pelcod_set_pre_packet_send((u8)key_num_val+200);
					pelcod_set_pre_packet_send(253);
					}
					else
					{
					pelcod_set_pre_packet_send(255);	
					pelcod_set_pre_packet_send((u8)key_num_val+200);
					}
				osd_line3_disp(1);
				osd_opt_message_disp(2,OSD_MSG_DISP_MAX_SECOND);
				
				if(wait_device_reply(cmd_buff,7,OSD_MSG_DISP_MAX_SECOND))
				{
					;//osd_opt_message_disp(2,OSD_MSG_DISP_MAX_SECOND);

				}
				else
					{
					
				}
				}

			}


			if(key_num_val == 888)
			{
				if(beep_enable)
				beep_enable = 0;
				else
					beep_enable=1;

				flash_program_my();
			}

			
			if(key_val_buffer_func == key_to_release(KEY_IRIS))
			{
				if(iris_mode==0)
					pelcod_set_pre_packet_send(128);
				else if(iris_mode==1)
					pelcod_call_pre_packet_send(128);
				else if(iris_mode==2)
					pelcod_call_pre_packet_send(127);
				else if(iris_mode==3)
					pelcod_call_pre_packet_send(126);
				osd_line2_disp(1);
				iris_set_ok = 1;
				//osd_opt_message_disp(4,OSD_MSG_DISP_MAX_SECOND);
			}

			if(key_val_buffer_func == key_to_release(KEY_FILTER))
			{
				pelcod_call_pre_packet_send(cam_filter_mode+80);
				osd_line2_disp(1);
				cam_filter_set_ok = 1;
			}

			if(key_val_buffer_func == key_to_release(KEY_CAM))
			{
				if(key_num_val<256)
				{
//					u8 dometmp;

//					dometmp = domeNo;
					domeNo = 255;
					pelcod_set_pre_packet_send(255);
					pelcod_set_pre_packet_send(255);
					pelcod_set_pre_packet_send(key_num_val);
					pelcod_set_pre_packet_send(255);
					osd_opt_message_disp(10,OSD_MSG_DISP_MAX_SECOND);

					domeNo = key_num_val;
					osd_line_little_4_disp(0);
				}
				osd_line2_disp(1);
				cam_filter_set_ok = 1;
			}

			
			key_value_all_clear();
			break;
			
	case key_to_release(KEY_ESC):
		key_value_all_clear();
		break;

	case key_to_release(KEY_FILTER):
		if(key_num_val==0)
			key_value_all_clear();
		if(key_num_val > 0 && key_num_val < 5)
		{
			cam_filter_mode = key_num_val-1;
			pelcod_call_pre_packet_send(cam_filter_mode+80);
				osd_line2_disp(1);
				cam_filter_set_ok = 1;

				osd_opt_message_disp(11+cam_filter_mode,OSD_MSG_DISP_MAX_SECOND);
			key_value_all_clear();
		}
		break;
	case key_to_long(KEY_SETUP):
		switch(key_num_val )
		{
		case 1200:
		case 2400:
		case 9600:
		case 4800:
			Baud_rate = baudrate_to_num(key_num_val);
			set_rs485_uart_baudrate();
			osd_line_little_4_disp(0);
			break;
		default:break;
		}

		key_value_all_clear();
		break;

	case key_to_long(KEY_IRIS):

		if(key_num_val >= 1 && key_num_val <= 3)
		{
			
			{
				iris_motor_mode = key_num_val-1;

				osd_line3_disp(1);

				switch(iris_motor_mode)
				{
				case 0:
					pelcod_call_pre_packet_send(125);
					flash_program_my();
					break;
				case 1:
					pelcod_set_pre_packet_send(125);
					flash_program_my();

					break;
				case 2:
					pelcod_set_pre_packet_send(130);
					flash_program_my();

					break;
				default:
					break;
				}

				rt_thread_delay(200);
				iris_mode = 0;
				pelcod_set_pre_packet_send(127);

					
				//osd_line2_disp(1);
				osd_line3_disp(1);
				osd_opt_message_disp_iris(iris_motor_mode);
				//osd_opt_message_disp(16+iris_mode,OSD_MSG_DISP_MAX_SECOND);
				rs485_get_data_from_slave();

				
			key_value_all_clear();
			}
		}


		break;
		
	default:
		break;
	}


}

void key_handle(u16 val)
{
	u16 tmp;

	if(!val)
		return;
	
	if(val<=10)
	{
		tmp = val%10;

		if(key_val_buffer_cnt>4)
		{
			key_val_buffer_cnt=0;
		}

		if(key_val_buffer_cnt == 0)
			key_num_val = tmp;
		else if(key_val_buffer_cnt == 1)
			key_num_val = key_num_val*10 + tmp;
		else if(key_val_buffer_cnt == 2)
			key_num_val = key_num_val*10 + tmp;
		else
			key_num_val = key_num_val*10+tmp;

		
		if(key_num_val > 255)
		{
			if(key_val_buffer_cnt == 4)
			{
				if((key_num_val != 1200)&&(key_num_val != 2400) &&(key_num_val != 4800)&&(key_num_val != 9600))
				{
					key_val_buffer_cnt = 0;
					//osd_clear_x_y(OSD_VAL_START_ADDR_X,4*8,OSD_VAL_START_ADDR_Y,16);
					OLED_Clear_line(0,OSD_VAL_START_ADDR_Y,8);
					OLED_Clear_line(0,OSD_VAL_START_ADDR_Y+1,8);

					key_val_buffer_cnt = 1;
					key_num_val = tmp;
					
					//return;
				}
			}
			else
			{
				key_val_buffer_cnt++;

			}
		}
		else
		{
			key_val_buffer_cnt++;

		}


LABEL_KHANDLE:
		if(key_val_buffer_cnt > 4 )
		{
			key_value_all_clear();
		}

		if((key_val_buffer_func == key_to_release(KEY_MODE)))
		{
			if(key_num_val>8)
				key_value_all_clear();
		}
		num_to_string(key_num_val,osd_mid_buff);
		
		osd_line1_disp(32);

		//osd_val_disp(osd_mid_buff,key_val_buffer_cnt);	
		goto KH_LABEL_EXIT;
		
	}


	//key_val_buffer_cnt = 0;
	
	key_analyze(val);

	osd_line1_disp(32);
	//osd_line_little_4_disp(iris_mode);
	//osd_line_1to4_all_disp();

KH_LABEL_EXIT	:
	return;
}


#if 1
u16 key_detect(void)
{
	u32 tmp;
	static u8 key_release_flag=0,key_pre_tmp=0;
	
	tmp = key_ctl_check();
	if((tmp>=(KEY_1)) && (tmp<=(KEY_0)))
	{
		if(key_release_flag==0)
		{
			key_release_flag = 1;
			key_pre_tmp = tmp;
			return tmp;
		}
		else
		{
			if(tmp == key_pre_tmp)
				return KEY_NONE;
			

		}
	}

	key_release_flag = 0;
	
	return(tmp);

}
#else
u16 key_detect(void)
{
	u32 tmp;

	tmp = key_ctl_check();
	if((tmp>=key_to_release(KEY_1)) && (tmp<=key_to_release(KEY_0)))
		tmp = tmp&(~0x9000);
	return(tmp);

}
#endif

u16 key_from_wait = 0;



//1,press; 0,no press
u8 key_sw22_check(void)
{
	static u8 key_sw22_pre=0;
	
	if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == 0)
	{
		rt_thread_delay(KEY_DELAY_CHECK_MS);

			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == 0)
			{
				if(key_sw22_pre==1)
				{
					return 0;
				}
				key_sw22_pre = 1;
				return key_sw22_pre;

			}
	}
	else
	{
		if(key_sw22_pre == 1)
		{
			key_sw22_pre = 0x10;
			return key_sw22_pre;
		}
		else
			{
			key_sw22_pre = 0;

		}
	}
	

	
	return 0;
}



/* 邮箱控制块*/
static struct rt_mailbox key_mb;
/* 用于放邮件的内存池*/
static char key_mb_pool[40];


void rt_key_thread_entry(void* parameter)
{

	u16 k;
	
    while(1)
	{
		if(key_from_wait)
		{
			key_handle(key_from_wait);
			key_from_wait = 0;
		}
			
		k = key_detect();
		if(k)
		{
			if(k<0x8000)
				rt_sem_release(&key_sem);	
			
			rt_mb_send(&key_mb, (rt_uint32_t)k);

			
		}
		
		rt_thread_delay(40);

    }
}


#define	EC11_XN_NUMS_VAL	90



#if 1
void ec11_check_handle(void)
{  
		
	static rt_uint8_t pulse_state_bak = 0;
	
	while(1)
	{
		
		if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7) == 1))   //第一次中断，并且A相是下降沿
		{
			if(pulse_state_bak == 0)
				pulse_state_bak = 7;
			else if(pulse_state_bak == 0x08)
				flag = 1;
			else
			{
				pulse_state_bak = 0;
				flag = 0;
			}

			if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) == 0)) 
			{
				if(pulse_state_bak == 0)
							pulse_state_bak = 0x08;
				else if(pulse_state_bak == 0x07)
							flag = 2;
				else
				{
					pulse_state_bak = 0;
					flag = 0;
				}
			}
			else if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) == 1)) 
			{
				if(pulse_state_bak == 0)
						pulse_state_bak = 0x88;
				else if(pulse_state_bak == 0x07)
						flag = 1;
				else
				{
					pulse_state_bak = 0;
					flag = 0;
				}


			}
			
		}
		
			
		if(flag==1)
		{
			--ec11_power_m;
			flag = 0;
			int_nu = 0;

		}
		else if(flag==2)
		{

			++ec11_power_m;
			flag = 0;
			int_nu = 0;

		}

		rt_thread_delay(RT_TICK_PER_SECOND/100);
	}

}

#endif

u8 ec11_mid_key_state = 0xff;////0,press; 1,no press


void rt_ec11_mid_key_thread_entry(void* parameter)
{

//	u16 k;
	
    while(1)
	{
		ec11_mid_key_state = key2_press_check();
		if(ec11_mid_key_state==1)
		{
			rt_sem_release(&key_sem);	//beep
		}
		rt_thread_delay(40);
    }
}


#define	CTL_STEPS_BASE		2

#define	PD_CMD_DELAY_MS		200


const u8* focus_msg[]=
{
{"     "},
{"Far  "},
{"Near "}
};



#define	OSD_DISP_DELAY		300



#if 1
void rt_ec11_focus_thread_entry(void* parameter)
{

	u16 k;
//	s32 ec11_counter_pre = 0,ec11_counter_bak = 0;

	static u8 key_press_state_tmp = 0;
	s32 result;

	while(1)
	{
		k = key2_focus_press_check();
		if(k)
		{
			if(key_press_state_tmp)
			{
				//rt_sem_release(&key_sem);	//beep
				
				pelcod_zf_packet_send(PD_ZOOM_FOCUS_STOP,0);
				
				memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
				strcat(osd_mid_str_buff,focus_msg[0]);
				osd_line1_disp(0);
				rs485_get_data_from_slave();

				key_press_state_tmp = 0;
				continue;
			}
			
			if(BMQCounterTotal_focus != 0)  //
			{
//				ec11_counter_pre = BMQCounterTotal_focus;
				
				rt_thread_delay(PD_CMD_DELAY_MS);

				
				result = BMQCounterTotal_focus;
				BMQCounterTotal_focus = 0;


			//	rt_sem_release(&key_sem);	//beep


				if(result > 0)
				{
					pelcod_zf_packet_send(PD_FOCUS_FAR_CMD,abs(result));
					
					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
					strcat(osd_mid_str_buff,focus_msg[1]);
					
					osd_line1_disp(0);
					}
				else if(result < 0)
				{

				pelcod_zf_packet_send(PD_FOCUS_NEAR_CMD,abs(result));
				
				memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
				strcat(osd_mid_str_buff,focus_msg[2]);
				osd_line1_disp(0);


					}
				
				rt_thread_delay(OSD_DISP_DELAY);
				pelcod_zf_packet_send(PD_ZOOM_FOCUS_STOP,0);
				memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
				strcat(osd_mid_str_buff,focus_msg[0]);
				
				osd_line1_disp(0);

				
				rs485_get_data_from_slave();
			}

		}
		else
		{
			
			if(BMQCounterTotal_focus != 0)  //
			{
//				ec11_counter_pre = BMQCounterTotal_focus;
				
				rt_thread_delay(PD_CMD_DELAY_MS);

#if 0
				if(result != BMQCounterTotal_focus)
					rt_sem_release(&key_sem);	//beep
#endif

				result = BMQCounterTotal_focus;
				//BMQCounterTotal_focus = 0;

				
				

				if(result > 0)
				{	pelcod_zf_packet_send(PD_FOCUS_FAR_CMD,abs(result));

	
					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));

				strcat(osd_mid_str_buff,focus_msg[1]);
				osd_line1_disp(0);

				}
				else if(result < 0)
				{
					pelcod_zf_packet_send(PD_FOCUS_NEAR_CMD,abs(result));
					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));

					strcat(osd_mid_str_buff,focus_msg[2]);
					osd_line1_disp(0);
					}
				
				rt_thread_delay(60);

				key_press_state_tmp = 1;
				
			}
				
		}

		rt_thread_delay(40);
	}
}

#endif

const u8* zoom_msg[]=
{
{"     "},
{"Tele "},
{"Wide "}
};


#if 1
void rt_ec11_zoom_thread_entry(void* parameter)
{

	u16 k;
//	s32 ec11_counter_pre = 0,ec11_counter_bak = 0;

	static u8 key_press_state_tmp = 0;
	s32 result;
	
	while(1)
	{
		k = key2_zoom_press_check();
		if(k)
		{
		

			if(key_press_state_tmp)
			{
				//rt_sem_release(&key_sem);	//beep
				
				pelcod_zf_packet_send(PD_ZOOM_FOCUS_STOP,0);
				memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
				strcat(osd_mid_str_buff,zoom_msg[0]);
				osd_line1_disp(0);

				
				rs485_get_data_from_slave();
				key_press_state_tmp = 0;
				continue;
			}
			
			if(BMQCounterTotal_zoom != 0)  //
			{
//				ec11_counter_pre = BMQCounterTotal_zoom;
				
				rt_thread_delay(PD_CMD_DELAY_MS);

				
				
				result = BMQCounterTotal_zoom;
				BMQCounterTotal_zoom = 0;

#if 0				
				if(result != 0)
				rt_sem_release(&key_sem);	//beep
#endif
				
				if(result > 0)
				{
					pelcod_zf_packet_send(1,abs(result));
					
					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
					strcat(osd_mid_str_buff,zoom_msg[1]);
					osd_line1_disp(0);

					}
				else if(result < 0)
				{
					pelcod_zf_packet_send(2,abs(result));
					
					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
					strcat(osd_mid_str_buff,zoom_msg[2]);
					osd_line1_disp(0);

					}

				
				rt_thread_delay(OSD_DISP_DELAY);
				pelcod_zf_packet_send(PD_ZOOM_FOCUS_STOP,0);
				
				memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
				strcat(osd_mid_str_buff,zoom_msg[0]);
				osd_line1_disp(0);


				rs485_get_data_from_slave();
			}

		}
		else
		{
			
			if(BMQCounterTotal_zoom != 0)  //
			{
//				ec11_counter_pre = BMQCounterTotal_zoom;
				
				rt_thread_delay(PD_CMD_DELAY_MS);

#if 0
				if(result != BMQCounterTotal_zoom)
					rt_sem_release(&key_sem);	//beep
#endif

				result = BMQCounterTotal_zoom;

				if(result > 0)
				{
					pelcod_zf_packet_send(1,abs(result));
					
					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
					strcat(osd_mid_str_buff,zoom_msg[1]);
					osd_line1_disp(0);

				}
				else if(result < 0)
				{
					pelcod_zf_packet_send(2,abs(result));
					
					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
					strcat(osd_mid_str_buff,zoom_msg[2]);
					osd_line1_disp(0);

				}

				
				rt_thread_delay(60);

				key_press_state_tmp = 1;
				
			}
				
		}

		rt_thread_delay(40);
	}
}


#endif

//iris 

const u8* iris_msg[]=
{
{"     "},
{"Open "},
{"Close"}
};


extern void rs485_recieve_test(void);


void rt_ec11_thread_entry(void* parameter)
{

	u16 k;
//	s32 ec11_counter_pre = 0,ec11_counter_bak = 0;

	static u8 key_press_state_tmp = 0;

	s32 result;

    while(1)
	{
		k = key2_press_check();
		if(k)
		{

			if(key_press_state_tmp)
			{
			//	rt_sem_release(&key_sem);	//beep;
				
				memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
				strcat(osd_mid_str_buff,iris_msg[0]);
				
				osd_line1_disp(0);

				
				rs485_get_data_from_slave();
				key_press_state_tmp = 0;
				continue;
			}
			
			if(BMQCounterTotal != 0)  //
			{
//				ec11_counter_pre = BMQCounterTotal;
				
				rt_thread_delay(PD_CMD_DELAY_MS);
				
								
				result = BMQCounterTotal;
				BMQCounterTotal = 0;

#if 0
				if(result!=0)
					rt_sem_release(&key_sem);	//beep;
#endif
					
				if(result > 0)
				{

					
					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
					strcat(osd_mid_str_buff,iris_msg[1]);
					osd_line1_disp(0);

					pelcod_open_close_packet_send_exptend(0,abs(result)&(~0x80),0x80);

				}
				else if(result < 0)
				{

					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
					strcat(osd_mid_str_buff,iris_msg[2]);
					osd_line1_disp(0);

					pelcod_open_close_packet_send_exptend(1,abs(result)&(~0x80),0x80);

				}

				rt_thread_delay(OSD_DISP_DELAY);
				//pelcod_stop_packet_send();

				memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
				
				strcat(osd_mid_str_buff,iris_msg[0]);
				osd_line1_disp(0);

				
				rs485_get_data_from_slave();
			}

		}
		else
		{
			
			if(BMQCounterTotal != 0)  //
			{
//				ec11_counter_pre = BMQCounterTotal;
				
				rt_thread_delay(PD_CMD_DELAY_MS);

				s32 result;

#if 0
				if(result != BMQCounterTotal)
					rt_sem_release(&key_sem);	//beep
#endif

				result = BMQCounterTotal;

							

				if(result > 0)
				{
					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
					strcat(osd_mid_str_buff,iris_msg[1]);
					osd_line1_disp(0);
					pelcod_open_close_packet_send_exptend(0,abs(result)&(~0x80),0x80);


				}
				else if(result < 0)
				{
					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
					strcat(osd_mid_str_buff,iris_msg[2]);
					osd_line1_disp(0);
					pelcod_open_close_packet_send_exptend(1,abs(result)&(~0x80),0x80);

				}


				rt_thread_delay(OSD_DISP_DELAY);
				
//				memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
//				strcat(osd_mid_str_buff,iris_msg[0]);
//				osd_line1_disp(0);
//
				key_press_state_tmp = 1;
				
			}
				
		}

		rt_thread_delay(40);
    }
}


u8 iris_set_ok=1;
u8 cam_filter_set_ok = 1;

void rt_blink_thread_entry(void* parameter)
{
	static  u8 state_pre,cam_filter_pre;
	
	while(1)
	{
		cam_filter_pre = cam_filter_set_ok;
		state_pre = iris_set_ok;

		if(!iris_set_ok)
		{
			osd_line2_disp(1);
			rt_thread_delay(600);
			osd_line_2_disp_item_clear(2);
		}

		if(!state_pre && iris_set_ok)
			osd_line2_disp(1);
		
		if(!cam_filter_set_ok)
		{
			osd_line2_disp(1);
			rt_thread_delay(600);
			osd_line_2_disp_item_clear(3);
		}

		if(!cam_filter_pre && cam_filter_set_ok)
			osd_line2_disp(1);
		rt_thread_delay(600);
	}

}


void rt_beep_thread_entry(void* parameter)
{
//	static  u8 state_pre,cam_filter_pre;
	
	while(1)
	{
		rt_sem_take(&key_sem, RT_WAITING_FOREVER);	

		if(beep_enable)	
		{
			GPIO_WriteBit(GPIOC,GPIO_Pin_4, Bit_SET);
			rt_thread_delay(BEEP_DELAY);
			GPIO_WriteBit(GPIOC,GPIO_Pin_4, Bit_RESET);
			//rt_thread_delay(300);
		}
		else
		{
			rt_thread_delay(BEEP_DELAY);
		}

	}

}

void rt_key_handle_thread_entry(void* parameter)
{
//	static  u8 state_pre,cam_filter_pre;
	u16 k;
	
	while(1)
	{
		//rt_sem_take(&key_handle_sem, RT_WAITING_FOREVER);	

				/* 从邮箱中收取邮件*/
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&k, RT_WAITING_FOREVER) == RT_EOK)
		{
		
			if(k)
			{
				key_handle(k);
	
			}

			rt_thread_delay(10);
		}
		

		

    }

}


void rt_joystick_check_thread_entry(void* parameter)
{
//	static  u8 state_pre,cam_filter_pre;
	u16 k;
	
	while(1)
	{

		k = Get_joystick_Value();
		if(k)
		{
			rt_mb_send(joystick_mb, (rt_uint32_t)k);	
		}
		
		rt_thread_delay(40);
    }

}



void rt_joystick_handle_thread_entry(void* parameter)
{
//	static  u8 state_pre,cam_filter_pre;
	u16 k;
	
	while(1)
	{
		if (rt_mb_recv(joystick_mb, (rt_uint32_t*)&k, RT_WAITING_FOREVER) == RT_EOK)
		{
			rt_sem_release(&key_sem);	//beep

			if(k)
			{
				key_handle(k);

				if(k==1)
				{
					pelcod_open_close_packet_send(0);
					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
					strcat(osd_mid_str_buff,joystick_msg[9]);
					osd_line1_disp(0);
				}
				else if(k==0x10)
				{
		
					pelcod_stop_packet_send();
					memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
					strcat(osd_mid_str_buff,joystick_msg[0]);
					osd_line1_disp(0);
		
				}

			}
				
			rt_thread_delay(20);
	    }

	}
}

rt_mailbox_t key_sw22_mb;

void rt_key_sw22_check_thread_entry(void* parameter)
{
	u8 k;
	
    while(1)
	{
		k = key_sw22_check();
		if(k)
		{
			if(k==1)
				rt_sem_release(&key_sem);	//beep
			rt_mb_send(key_sw22_mb, (rt_uint32_t)k);
			
		}
		rt_thread_delay(40);
    }
}

void rt_key_sw22_handle_thread_entry(void* parameter)
{

	u8 k;
	
    while(1)
	{

		if (rt_mb_recv(key_sw22_mb, (rt_uint32_t*)&k, RT_WAITING_FOREVER) == RT_EOK)
		if(k)
		{			
			if(k==1)
			{
				
				memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
				strcat(osd_mid_str_buff,joystick_msg[9]);
				osd_line1_disp(0);
								pelcod_open_close_packet_send(0);

			}
			else if(k==0x10)
			{

				memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
				strcat(osd_mid_str_buff,joystick_msg[0]);
				osd_line1_disp(0);
				pelcod_stop_packet_send();

			}
			
			rt_thread_delay(200);
    	}
		rt_thread_delay(40);

    }
}



void rt_key_old_thread_entry(void* parameter)
{

	u16 k;

	
    while(1)
	{
		
		
		//rs485_send_data(wifi_enter_at_mode,strlen(wifi_enter_at_mode));
		
		
		if(key_from_wait)
		{
			key_handle(key_from_wait);
			key_from_wait = 0;
		}
		
		k = key_detect();
		if(k)
		{

			GPIO_WriteBit(GPIOC,GPIO_Pin_4, Bit_SET);

			rt_thread_delay(BEEP_DELAY);
			GPIO_WriteBit(GPIOC,GPIO_Pin_4, Bit_RESET);

			key_handle(k);

			//osd_line3_disp(1);

			
			//rt_thread_delay(50);
		}

		k = Get_joystick_Value();
		
		//osd_line_1to4_all_disp();

		


		k = key_sw22_check();
		if(k==1)
		{
		
		GPIO_WriteBit(GPIOC,GPIO_Pin_4, Bit_SET);
		
		rt_thread_delay(BEEP_DELAY);
		GPIO_WriteBit(GPIOC,GPIO_Pin_4, Bit_RESET);

			pelcod_open_close_packet_send(0);
			memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
			strcat(osd_mid_str_buff,joystick_msg[9]);
			osd_line1_disp(0);
		}
		else if(k==0x10)
		{
			GPIO_WriteBit(GPIOC,GPIO_Pin_4, Bit_SET);

			rt_thread_delay(BEEP_DELAY);
			GPIO_WriteBit(GPIOC,GPIO_Pin_4, Bit_RESET);

			pelcod_stop_packet_send();
			memset(osd_mid_str_buff,0,sizeof(osd_mid_str_buff));
			strcat(osd_mid_str_buff,joystick_msg[0]);
			osd_line1_disp(0);

		}


		wait_device_reply(cmd_buff,7,OSD_MSG_DISP_MAX_SECOND);

		
		rt_thread_delay(20);
    }
}

//static rt_timer_t timer1;  
//static rt_uint8_t count;  
//////////22222222222222

//void ec11_ISR(void)                                       
//{

//}


//static void timeout1(void* parameter)  
//{  

//	ec11_ISR();

//} 

u8 rs485_get_data_from_slave_thread_entry(void* parameter)
{

	rs485_get_data_from_slave_thread();



}



int rt_key_ctl_init(void)
{

	
    rt_thread_t init_thread;

	rt_sem_init(&key_sem, "sem",0, RT_IPC_FLAG_FIFO);  

	joystick_mb = rt_mb_create("joymb",20,RT_IPC_FLAG_FIFO);
	key_sw22_mb = rt_mb_create("sw22mb",20,RT_IPC_FLAG_FIFO);

	/* 初始化一个mailbox */
	rt_mb_init(&key_mb,
	"mbt", /* 名称是mbt */
	&key_mb_pool[0], /* 邮箱用到的内存池是mb_pool */
	sizeof(key_mb_pool)/4, /* 大小是mb_pool/4，因为每封邮件的大小是4字节*/
	RT_IPC_FLAG_FIFO); /* 采用FIFO方式进行线程等待*/


		
	key_pin_init();
	joystick_pin_init();
	
	rt_thread_delay(200);


#if 1
    init_thread = rt_thread_create("key",
                                   rt_key_thread_entry, RT_NULL,
                                   256, 5, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

	init_thread = rt_thread_create("ec11",
                                   rt_ec11_thread_entry, RT_NULL,
                                   256, 5, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);


	init_thread = rt_thread_create("ec11z",
                                   rt_ec11_zoom_thread_entry, RT_NULL,
                                   256, 5, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);


	init_thread = rt_thread_create("ec11f",
                                   rt_ec11_focus_thread_entry, RT_NULL,
                                   512, 5, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);


	init_thread = rt_thread_create("get",
                                   rs485_get_data_from_slave_thread_entry, RT_NULL,
                                   300, 5, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

#endif
	//init_thread = rt_thread_create("ec11key",
 //                                  rt_ec11_mid_key_thread_entry, RT_NULL,
 //                                  256, 10, 5);
 //   if (init_thread != RT_NULL)
 //       rt_thread_startup(init_thread);
	
    //init_thread = rt_thread_create("blink",
    //                               rt_blink_thread_entry, RT_NULL,
    //                               1024, 10, 5);
    //if (init_thread != RT_NULL)
    //    rt_thread_startup(init_thread);


	//timer1 = rt_timer_create("timer1",  
 //       timeout1,  
 //       RT_NULL,  
 //       5,  
 //       RT_TIMER_FLAG_PERIODIC);  
 //   if (timer1 != RT_NULL)  
 //       rt_timer_start(timer1); 

    init_thread = rt_thread_create("beep",
                                   rt_beep_thread_entry, RT_NULL,
                                   400, 6, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

#if 1
    init_thread = rt_thread_create("key_handle",
                                   rt_key_handle_thread_entry, RT_NULL,
                                   1024, 5, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    init_thread = rt_thread_create("joy_handle",
                                   rt_joystick_handle_thread_entry, RT_NULL,
                                   800, 5, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    init_thread = rt_thread_create("joy_check",
                                   rt_joystick_check_thread_entry, RT_NULL,
                                   500, 5, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);
#endif

    init_thread = rt_thread_create("sw22_check",
                                   rt_key_sw22_check_thread_entry, RT_NULL,
                                   500, 5, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    init_thread = rt_thread_create("sw22_han",
                                   rt_key_sw22_handle_thread_entry, RT_NULL,
                                   500, 5, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

	
    return 0;
}

