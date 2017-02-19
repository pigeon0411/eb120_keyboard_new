#include <board.h>
#include <rtthread.h>

#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */

#include "includes.h"
#include "oled.h"
#include "bmp.h"
#include "osd_menu.h"

#include "key_ctl.h"


extern u8 iris_motor_mode;

extern rt_mutex_t oled_disp_mut;

#define	IRIS_MSG_ITERMS_MAX		5

const u8 *iris_msg_string[IRIS_MSG_ITERMS_MAX]=
{
{"Auto"},
{"Man "},
{"    "},
};


const u8 *filter_string[]=
{
{" Day "},
{"Night"},
{"Fog1 "},
{"Fog2 "},
{"     "},
};


#define	OPT_MSG_ITERMS_MAX		25

const u8 *opt_msg_string[]=
{
{"CALL done!"},
{"PRESET done!"},
	{"Set mode!"},
	{"call mode!"},
	{"set iris mode"},
	{"set mode"},


{"Tele"},//6
{"Wide"},//7
{"Far "},
{"Near"},

	{"set id!"},

{" Day "},//11
{"Night"},
{"Fog1 "},
{"Fog2 "},
{"     "},


{"Auto"},//16
{"Manul "},
{"    "},



};


#if 1
const u8 *iris_msg_osd[]=
{
	{"DC-IRIS1       "}, //CALL 125
	{"P-IRIS2   "}, // SET 125
	{"P-IRIS3   "}, //130
	{"P-IRIS2   "}, // SET 125
	{"P-IRIS3   "}, //130

	{"-------- "},
};




void osd_clear_x_y(u8 x_start,u8 x_end,u8 y,u8 sizef)
{
	u8 ii;

	for(ii=x_start;ii<x_end;)
	{
		OLED_ShowString(ii,y," ",sizef);
		ii+=sizef;
	}
}



void osd_opt_message_disp(u8 type,u16 wait_s)
{
	if(type > OPT_MSG_ITERMS_MAX-1)
		return;

	osd_line1_val_disp_clear();	

	OLED_ShowString(0,OSD_VAL_START_ADDR_Y,(u8*)opt_msg_string[type],16);  


	u16 k=wait_s;
	while(k--)
	{
		key_from_wait = key_detect();
		if(key_from_wait)
		{
			
			break;
		}
		rt_thread_delay(50);

	}
	osd_line1_val_disp_clear();	

	
}

void osd_opt_message_disp_extend(u8 type)
{
	if(type > OPT_MSG_ITERMS_MAX-1)
		return;

	osd_line1_val_disp_clear();	

	OLED_ShowString(0,OSD_VAL_START_ADDR_Y,(u8*)opt_msg_string[type],16);  
	
}

void osd_opt_message_disp_iris(u8 val)
{

	osd_line1_val_disp_clear();	

	OLED_ShowString(0,OSD_VAL_START_ADDR_Y,(u8*)iris_msg_osd[val],16);  
	
}



const u8 *baudrate_string[]=
{
{"1200"},
{"2400"},
{"4800"},
{"9600"},
};




#define	OSD_LINE1_X_POS		0
#define	OSD_LINE4_Y_POS		6

#define	OSD_BAUDRATE_ID_Y_POS		OSD_LINE4_Y_POS

void osd_baudrate_disp(u16 data)
{
	if(data > 3)
		return;
	
	OLED_ShowString(OSD_LINE1_X_POS,OSD_BAUDRATE_ID_Y_POS,(u8 *)baudrate_string[data],16);
	OLED_ShowString(OSD_LINE1_X_POS+32,OSD_BAUDRATE_ID_Y_POS,"BPS",16);  


	
}

#define	OSD_ID_START_ADDR		80
#define	OSD_ID_NUM_START_ADDR	(OSD_ID_START_ADDR+24)

void osd_id_disp(u8 data)
{
	
	OLED_ShowString(OSD_ID_START_ADDR,OSD_BAUDRATE_ID_Y_POS,"ID:",16);  

	if(data<10)
	{
		OLED_ShowNum(OSD_ID_NUM_START_ADDR,OSD_BAUDRATE_ID_Y_POS,0,1,16);
		OLED_ShowNum(OSD_ID_NUM_START_ADDR+8,OSD_BAUDRATE_ID_Y_POS,0,1,16);
		OLED_ShowNum(OSD_ID_NUM_START_ADDR+8+8,OSD_BAUDRATE_ID_Y_POS,data,1,16);
	}
	else if(data < 100)
	{
		OLED_ShowNum(OSD_ID_NUM_START_ADDR,OSD_BAUDRATE_ID_Y_POS,0,1,16);
		OLED_ShowNum(OSD_ID_NUM_START_ADDR+8,OSD_BAUDRATE_ID_Y_POS,data,2,16);
	}
	else 
	{
		OLED_ShowNum(OSD_ID_NUM_START_ADDR,OSD_BAUDRATE_ID_Y_POS,data,3,16);
	}
}


#define	OSD_CMD_START_ADDR		88
void osd_cmd_disp(u8 *data)
{
	if(strlen((const char*)data) > 4)
			return;
	
	OLED_ShowString(OSD_CMD_START_ADDR,0,data,16);  

}


void osd_val_disp(u8 *data,u8 cnt)
{
	//u8 i;

	//u8 buff[12];

	//if(cnt > 10)
	//	return;
	//
	//for(i=0;i<cnt;i++)
	//{
	//	buff[i]=data[i];
	//}
	//buff[i] = '\0';
	OLED_ShowString(OSD_VAL_START_ADDR_X,OSD_VAL_START_ADDR_Y,data,16);  

}

void osd_line2_val_disp_clear(void)
{

	//osd_clear_x_y(OSD_VAL_START_ADDR_X,OSD_VAL_START_ADDR_X+8*7,OSD_VAL_START_ADDR_Y,16);
	OLED_Clear_line(0,OSD_LINE2_Y_POS,8);
	OLED_Clear_line(0,OSD_LINE2_Y_POS+1,8);
}

void osd_line1_val_disp_clear(void)
{

	//osd_clear_x_y(OSD_VAL_START_ADDR_X,OSD_VAL_START_ADDR_X+8*7,OSD_VAL_START_ADDR_Y,16);
	OLED_Clear_line(0,0,16);
	OLED_Clear_line(0,0+1,16);
}

void osd_line2_disp(u8 x)
{
	osd_line2_val_disp_clear();
	osd_para_mode_disp(cam_para_mode);
	//osd_iris_mode_disp(iris_mode);
	osd_filter_mode_disp(cam_filter_mode);

}

void osd_set_para_disp_line_1(u8 item)
{
	if(item==1)
	{

			osd_para_mode_disp_xy(cam_para_mode,0,0);

	}

	else if(item ==2 )
	{
		osd_iris_mode_disp_xy(iris_mode,0,0);
	}
	else if(item == 3)
	{

		osd_filter_mode_disp_xy(cam_filter_mode,0,0);
	}

}


void osd_line_2_disp_item_clear(u8 select)
{
	if(select>3)
		return;

	u8 xstart;//,xend;
	
	//osd_line2_val_disp_clear();	

	switch(select)
	{
	case 1:

		break;
	case 2:
		osd_iris_mode_disp(0xff);
		break;
	case 3:// filter
		//osd_para_mode_disp(cam_para_mode);
		//osd_iris_mode_disp(iris_mode);
		osd_filter_mode_disp(0xff);
		break;
	default:break;

	}
	
}


extern u8 osd_mid_buff[30];
extern u8 key_val_buffer_cnt;
extern u8 osd_mid_str_buff[10];

void osd_line1_disp(u8 x)
{
	if(x)
		osd_val_disp(osd_mid_buff,key_val_buffer_cnt);

	OLED_ShowString(0,OSD_VAL_START_ADDR_Y,osd_mid_str_buff,16);  
	
}


void OLED_Clear_line(u8 x,u8 y,u8 sizef)  
{  
	u8 i,n;		
	rt_mutex_take(oled_disp_mut,RT_WAITING_FOREVER);

	i = y;
	
 
	OLED_WR_Byte (0xb0+i,OLED_CMD);    //¨¦¨¨??¨°3¦Ì??¡¤¡ê¡§0~7¡ê?
	OLED_WR_Byte (0x00,OLED_CMD);      //¨¦¨¨????¨º??????a¨¢D¦Ì¨ª¦Ì??¡¤
	OLED_WR_Byte (0x10,OLED_CMD);      //¨¦¨¨????¨º??????a¨¢D??¦Ì??¡¤   
	for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA); 

	rt_mutex_release(oled_disp_mut);
	
}



void osd_para_mode_disp(u8 mode)
{
	u8 *mstr="Mode";
	u8 pp;

	if(mode > 8)
		return;
	OLED_ShowString(OSD_PARA_MODE_X_START,OSD_PARA_MODE_Y,mstr,16); 
	
	pp = mode+0x31;
	OLED_ShowString(OSD_PARA_MODE_X_START+32,OSD_PARA_MODE_Y,&pp,16); 
}

#define	OSD_SET_PARA_START_ADDR		8

void osd_para_mode_disp_xy(u8 mode,u8 x,u8 y)
{
	u8 *mstr="Mode";
	u8 pp;

	if(mode > 8)
		return;
	OLED_ShowString(OSD_PARA_MODE_X_START,OSD_PARA_MODE_Y,mstr,16); 
	
	pp = mode+0x31;
	OLED_ShowString(OSD_SET_PARA_START_ADDR,0,&pp,16); 
}

void osd_iris_mode_disp(u8 mode)
{
	if(mode == 0xff)
	{
		OLED_ShowString(OSD_IRIS_MODE_X_START,OSD_IRIS_MODE_Y,(u8*)iris_msg_string[IRIS_MSG_ITERMS_MAX-1],16); 

		return;
	}
	if(mode > IRIS_MSG_ITERMS_MAX-1)
		return;
	OLED_ShowString(OSD_IRIS_MODE_X_START,OSD_IRIS_MODE_Y,(u8*)iris_msg_string[mode],16); 

}



void osd_iris_mode_disp_xy(u8 mode,u8 x,u8 y)
{
	if(mode == 0xff)
	{
		OLED_ShowString(OSD_IRIS_MODE_X_START,OSD_IRIS_MODE_Y,(u8*)iris_msg_string[IRIS_MSG_ITERMS_MAX-1],16); 

		return;
	}
	if(mode > IRIS_MSG_ITERMS_MAX-1)
		return;
	OLED_ShowString(OSD_SET_PARA_START_ADDR,0,(u8*)iris_msg_string[mode],16); 

}

extern u8 cam_filter_mode;

void osd_filter_mode_disp(u8 mode)
{
	if(mode == 0xff)
	{
		OLED_ShowString(OSD_FILTER_MODE_X_START,OSD_FILTER_MODE_Y,(u8*)filter_string[4],16); 
		return;
	}

	
	if(mode > 3)
		return;
	OLED_ShowString(OSD_FILTER_MODE_X_START,OSD_FILTER_MODE_Y,(u8*)filter_string[mode],16); 

}

void osd_filter_mode_disp_xy(u8 mode,u8 x,u8 y)
{
	if(mode == 0xff)
	{
		OLED_ShowString(OSD_FILTER_MODE_X_START,OSD_FILTER_MODE_Y,(u8*)filter_string[4],16); 
		return;
	}

	
	if(mode > 3)
		return;
	OLED_ShowString(OSD_SET_PARA_START_ADDR,0,(u8*)filter_string[mode],16); 

}

const u8 *osd_shutter_string="S:";
const u8 *osd_iris_auto_string="P-IRIS     A";
const u8 *osd_iris_man_string="P-IRIS     M";





const u8 *iris_mode_osd[]=
{
		{"A "},
		{"M "}
};
#else
const u8 *iris_msg_osd[]=
{
	{"DC-IRIS1       "}, //CALL 125
	{"P-IRIS2   A    "}, // SET 125
	{"P-IRIS3   A    "}, //130
	{"P-IRIS2   M    "}, // SET 125
	{"P-IRIS3   M    "}, //130

	{"-------- "},
};
#endif


const u8 *auto_string="A";
const u8 *man_string="M";


#define	OSD_IRIS_X_START		0//80

extern u8 iris_motor_mode;
extern u8 iris_val;
extern u8 iris_mode;
extern u8 iris_val_osd_buf[10];


void num_to_string_ex(u16 data,u8 *dst,u8 num)
{

	dst[0] = data/100+0x30;
	dst[1] = data%100/10+0x30;
	dst[2] = data%10+0x30;
}


#if 1
void osd_iris_val_disp(u8 irisv)
{
	if(iris_mode>1)
		iris_mode = 1;

	u8 tmp;
	u8 no_val_str[]={"---"};



	if(iris_motor_mode > 4)
		{
		OLED_ShowString(OSD_IRIS_X_START,OSD_LINE3_Y_POS,(u8*)iris_msg_osd[5],16); 
	}
	else
		{
		OLED_ShowString(OSD_IRIS_X_START,OSD_LINE3_Y_POS,(u8*)iris_msg_osd[iris_motor_mode],16); 

		}

#if 0
	iris_motor_mode = 1;
	iris_mode = 1;
	iris_val = 50;
#endif

	if(0 == iris_motor_mode)
	{

		memset(iris_val_osd_buf,0,10);
//		strcat(iris_val_osd_buf,no_val_str);
//
//		OLED_ShowString(88,OSD_LINE3_Y_POS,iris_val_osd_buf,16); 
			
	}
	else
	{
			
		memset(iris_val_osd_buf,0,10);
		strcat(iris_val_osd_buf,iris_mode_osd[iris_mode]);
		
		if(iris_mode)
		{
			if(iris_val == 0xff)
			{
				strcat(iris_val_osd_buf,no_val_str);
			}
			else
			{
				num_to_string_ex((u16)iris_val,iris_val_osd_buf+1,3);
			}
			
		}
		OLED_ShowString(88,OSD_LINE3_Y_POS,iris_val_osd_buf,16); 

	}
	


}


#else
void osd_iris_val_disp(u8 irisv)
{
	if(iris_mode>1)
		iris_mode = 1;

	u8 tmp;
	
	if(iris_mode == 1)//man
	{
		switch(iris_motor_mode)
		{//// 0,call 125; 1,set 125; 2 ,set 130
		case 1:
			tmp = 3;
			break;
			case 2:
				tmp = 4;
				break;
		default:
			tmp = 3;
			break;

		}
	}
	else//auto
	{
		switch(iris_motor_mode)
		{//// 0,call 125; 1,set 125; 2 ,set 130
		case 1:
			tmp = 1;
			break;
		case 2:
			tmp = 2;
			break;
		case 0:
			tmp = 0;
			break;

		default:
			tmp = 1;
			break;

		}

	}

	if(iris_motor_mode == 0xff)
		{
		OLED_ShowString(OSD_IRIS_X_START,OSD_LINE3_Y_POS,(u8*)iris_msg_osd[5],16); 
	}
	else
		{
		OLED_ShowString(OSD_IRIS_X_START,OSD_LINE3_Y_POS,(u8*)iris_msg_osd[tmp],16); 

		}

	u8 no_val_str[]={"---"};
	
	if(iris_mode)
	{
		if(iris_val == 0xff)
		{
			memset(iris_val_osd_buf,0,10);
			strcat(iris_val_osd_buf,no_val_str);
		}
		else
		{
			num_to_string_ex((u16)iris_val,iris_val_osd_buf,3);
		}
		OLED_ShowString(88,OSD_LINE3_Y_POS,iris_val_osd_buf,16); 
		
	}

}
#endif

const u8 *shutter_val_string[]=
{
{"1/10"},
{"1/100"},
{"1/500"},
{"1/800"},
{"1/1000"},
{"1/1200"},
{"1/1500"},
{"1/10"},
{"1/10"},
{"1/10"},
{"1/10"},
{"1/10"},
{"1/10"},
{"1/10"},
{"1/10"},


};

void osd_shutter_val_disp(u8 sv)
{
	if(sv)
	{
	OLED_ShowString(0,OSD_LINE3_Y_POS,(u8*)osd_shutter_string,16); 

	}
	else
	{
	OLED_ShowString(0,OSD_LINE3_Y_POS,(u8*)osd_shutter_string,16); 
	OLED_ShowString(16,OSD_LINE3_Y_POS,(u8*)auto_string,16); 

	}
}

u8 iris_val = 0xff;
u8 shutter_val = 5;

void osd_line3_disp(u8 select)
{
	
	OLED_Clear_line(0,OSD_LINE3_Y_POS,16);
	OLED_Clear_line(0,OSD_LINE3_Y_POS+1,16);

	
	osd_iris_val_disp(iris_mode);
	//osd_shutter_val_disp(svtmp);


}
void osd_line3_disp_clear_line(void)
{
	
	OLED_Clear_line(0,OSD_LINE3_Y_POS,16);
	OLED_Clear_line(0,OSD_LINE3_Y_POS+1,16);

}


#define	OSD_LINE_4_LITTLE_START_X	0
#define	OSD_LINE_4_LITTLE_START_Y	(6)

#define	OSD_LINE_4_FONT_PIXEL_X		8

extern u16 num_to_baudrate(u8 numb);

void osd_line_little_4_disp(u8 chn)
{
	OLED_Clear_line(0,6,16);
	OLED_Clear_line(0,7,16);
	
	osd_id_disp(domeNo);
	osd_baudrate_disp(Baud_rate);
}

void osd_line_little_4_disp_item_clear(u8 select)
{
//	u8 i,k;

	if(select>3)
		return;


	u8 xstart;//,xend;
	
	//if(select)
	{
		if(select==0)
		{
			xstart = OSD_LINE_4_LITTLE_START_X;
			//xend = xstart + strlen((const char *)iris_msg_string[select])*8;

		}
		else
		{
			
			xstart = OSD_LINE_4_FONT_PIXEL_X*((select) + strlen((const char*)iris_msg_string[select]));		
			//xend = xstart + strlen((const char*)iris_msg_string[select]);
		}
		
		//osd_clear_x_y(xstart,xend,OSD_LINE_4_LITTLE_START_Y,8); 
		//osd_clear_x_y(xstart,xend,OSD_LINE_4_LITTLE_START_Y+1,8); 

		OLED_Clear_line(xstart,OSD_LINE_4_LITTLE_START_Y,8);
		OLED_Clear_line(xstart,OSD_LINE_4_LITTLE_START_Y+1,8);
	}
}


void osd_line_1to4_all_disp(void)
{
	osd_line1_disp(1);
	
	osd_line2_disp(1);
	osd_line3_disp(2);
	
	osd_line_little_4_disp(0);
}

void osd_init(void)
{
	OLED_Init();			//3?¨º??¡¥OLED  
	OLED_Clear()  	; 
	osd_line_1to4_all_disp();

	
	
	
}

void osd_test(void)
{	u8 t;
		OLED_Init();			//3?¨º??¡¥OLED  
		OLED_Clear()  	; 
	
		t=' ';
		OLED_ShowCHinese(0,0,0);//?D
				OLED_ShowCHinese(18,0,1);//?¡ã
		OLED_ShowCHinese(36,0,2);//?¡ã
		OLED_ShowCHinese(54,0,3);//¦Ì?
		OLED_ShowCHinese(72,0,4);//¡Á¨®
		OLED_ShowCHinese(90,0,5);//??
		OLED_ShowCHinese(108,0,6);//??
	while(1) 
	{		
		OLED_Clear();
		OLED_ShowCHinese(0,0,0);//?D
		OLED_ShowCHinese(18,0,1);//?¡ã
		OLED_ShowCHinese(36,0,2);//?¡ã
		OLED_ShowCHinese(54,0,3);//¦Ì?
		OLED_ShowCHinese(72,0,4);//¡Á¨®
		OLED_ShowCHinese(90,0,5);//??
		OLED_ShowCHinese(108,0,6);//??
		OLED_ShowString(6,3,"0.96' OLED TEST",16);

				OLED_ShowString(6,4,"0.96' OLED TEST",16);
		OLED_ShowString(0,5,"0.96' OLED TEST",16);

		OLED_ShowString(0,6,"0.96' OLED TEST",16);
		OLED_ShowString(1,7,"0.96' OLED TEST",16);

		OLED_ShowString(6,3,"0.96' OLED TEST",16);

		//OLED_ShowString(8,2,"ZHONGJINGYUAN");  
	 //	OLED_ShowString(20,4,"2014/05/01");  
		OLED_ShowString(0,6,"ASCII:",16);  
		OLED_ShowString(63,6,"CODE:",16);  
		OLED_ShowChar(48,6,t,16);//??¨º?ASCII¡Á?¡¤?	   
		t++;
		if(t>'~')t=' ';
		OLED_ShowNum(103,6,t,3,16);//??¨º?ASCII¡Á?¡¤?¦Ì????¦Ì 	
		rt_thread_delay(1000*3);


		OLED_DrawBMP(0,0,128,8,BMP1);  //¨ª?????¨º?(¨ª?????¨º?¨¦¡Â¨®?¡ê?¨¦¨²3¨¦¦Ì?¡Á?¡À¨ª??¡ä¨®¡ê??¨¢??¨®????¨¤????¡ê?FLASH????8K¨°???¨¦¡Â¨®?)
		rt_thread_delay(1000*3);

		OLED_DrawBMP(0,0,128,8,BMP1);
		rt_thread_delay(1000*3);


	}	
	
}


