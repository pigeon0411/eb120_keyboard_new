#include <rthw.h>
#include "rs485_decode.h"
#include "includes.h"
#include <string.h>

#define target_id domeNo

extern rt_sem_t	uart1_sem;

extern rt_err_t rs485_send_data(u8* data,u16 len);
rt_err_t rs485_recieve_check(u8 val)
{

	
	if(rt_sem_take(uart1_sem, 200) == RT_EOK)
    {
		PELCO_D_P_protocol_analysis_2();
		
        if(command_byte == val)
	    {
	    	command_byte = 0xff;
			return RT_EOK;

   	    }

		command_byte = 0xff;
	}
	return RT_ERROR;

}



void rs485_recieve_test(void)
{
	while(1)
	{
		if(RT_EOK == rs485_recieve_check(0x99))
		{

			cam_filter_mode = Rocket_sec_data-1;

			if(cam_filter_mode>3)
			cam_filter_mode = 0;
			iris_mode = Rocket_thr_data&0x0f;
			//iris_motor_mode = (Rocket_thr_data>>4)&0x0f;

			osd_line3_disp(0);


		}
	}

	while(1)
	{
		if(rt_sem_take(uart1_sem, 200) == RT_EOK)
	    {
			rs485_send_data(keyboard_data_buffer, 7);


		}
	}




	
}



rt_sem_t rs485_return_sem;


void pelcod_call_pre_packet_send(u8 val)
{
	u8 cnt;
	
	u8 cmd_buff_private[7];
	cmd_buff_private[0] = 0xff;
	cmd_buff_private[1] = target_id;
	cmd_buff_private[2] = 0;
	cmd_buff_private[3] = 0x07;
	cmd_buff_private[4] = 0;
	cmd_buff_private[5] = val;
	
	cmd_buff_private[6] = cmd_buff_private[1] + cmd_buff_private[2] + cmd_buff_private[3] + cmd_buff_private[4] + cmd_buff_private[5];
	rs485_send_data(cmd_buff_private,7);

//	cnt=3;
//	while(cnt--)
//	{
//		if(RT_EOK == rs485_recieve_check(val))
//			break;
//		else
//			rs485_send_data(cmd_buff_private,7);
//	}
}


void pelcod_set_pre_packet_send(u8 val)
{
	u8 cnt;
	
	u8 cmd_buff_private[7];
	cmd_buff_private[0] = 0xff;
	cmd_buff_private[1] = target_id;
	cmd_buff_private[2] = 0;
	cmd_buff_private[3] = 0x03;
	cmd_buff_private[4] = 0;
	cmd_buff_private[5] = val;
	
	cmd_buff_private[6] = cmd_buff_private[1] + cmd_buff_private[2] + cmd_buff_private[3] + cmd_buff_private[4] + cmd_buff_private[5];
	rs485_send_data(cmd_buff_private,7);
}

void pelcod_set_pre_extend_packet_send(u8 val,u8 cmd)
{
	u8 cnt;
	
	u8 cmd_buff_private[7];
	cmd_buff_private[0] = 0xff;
	cmd_buff_private[1] = target_id;
	cmd_buff_private[2] = 0xff;
	cmd_buff_private[3] = cmd;
	cmd_buff_private[4] = val;
	cmd_buff_private[5] = 0;
	
	cmd_buff_private[6] = cmd_buff_private[1] + cmd_buff_private[2] + cmd_buff_private[3] + cmd_buff_private[4] + cmd_buff_private[5];
	rs485_send_data(cmd_buff_private,7);
}



//val: 0,open; 1,close
void pelcod_open_close_packet_send_exptend(u8 val,u8 speed,u8 data5)
{
	u8 cnt;
	
	u8 cmd_buff_private[7];
	cmd_buff_private[0] = 0xff;
	cmd_buff_private[1] = target_id;
	if(val)//close
		cmd_buff_private[2] = 0x04;
	else
		cmd_buff_private[2] = 0x02;
	cmd_buff_private[3] = 0;
	cmd_buff_private[4] = speed;
	cmd_buff_private[5] = data5;
	
	cmd_buff_private[6] = cmd_buff_private[1] + cmd_buff_private[2] + cmd_buff_private[3] + cmd_buff_private[4] + cmd_buff_private[5];
	rs485_send_data(cmd_buff_private,7);

//	cnt=3;
//	while(cnt--)
//	{
//		if(RT_EOK == rs485_recieve_check(val))
//			break;
//		else
//			rs485_send_data(cmd_buff_private,7);
//	}
}



//val: 0,open; 1,close
void pelcod_open_close_packet_send(u8 val)
{
	u8 cnt;
	
	u8 cmd_buff_private[7];
	cmd_buff_private[0] = 0xff;
	cmd_buff_private[1] = target_id;
	if(val)//close
		cmd_buff_private[2] = 0x04;
	else
		cmd_buff_private[2] = 0x02;
	cmd_buff_private[3] = 0;
	cmd_buff_private[4] = 0;
	cmd_buff_private[5] = 0;
	
	cmd_buff_private[6] = cmd_buff_private[1] + cmd_buff_private[2] + cmd_buff_private[3] + cmd_buff_private[4] + cmd_buff_private[5];
	rs485_send_data(cmd_buff_private,7);

//	cnt=3;
//	while(cnt--)
//	{
//		if(RT_EOK == rs485_recieve_check(val))
//			break;
//		else
//			rs485_send_data(cmd_buff_private,7);
//	}
}

void pelcod_lrud_pre_packet_send(u8 lrudcmd,u8 lrspeed,u8 udspeed)
{
	u8 cnt;
	
	u8 cmd_buff_private[7];
	cmd_buff_private[0] = 0xff;
	cmd_buff_private[1] = target_id;
	cmd_buff_private[2] = 0;
	cmd_buff_private[3] = lrudcmd;
	cmd_buff_private[4] = lrspeed;
	cmd_buff_private[5] = udspeed;
	
	cmd_buff_private[6] = cmd_buff_private[1] + cmd_buff_private[2] + cmd_buff_private[3] + cmd_buff_private[4] + cmd_buff_private[5];
	rs485_send_data(cmd_buff_private,7);
}

//cmd,0,stop; 1,tele,2wide; 3,far,4,near
void pelcod_zf_packet_send(u8 cmd,u8 zfspeed)
{
	u8 cnt;
	
	u8 cmd_buff_private[7];
	cmd_buff_private[0] = 0xff;
	cmd_buff_private[1] = target_id;


//	if(cmd==1||cmd==2||cmd==3||cmd==4)
//	{
//		if(zfspeed>8)
//			zfspeed = 8;
//		pelcod_call_pre_packet_send(100+zfspeed);
//	}
//	
	switch(cmd)
	{
	case 1:
		cmd_buff_private[3] = 0x20;
		cmd_buff_private[2] = 0;
		
		cmd_buff_private[4] = zfspeed;
		break;
	case 2:
		cmd_buff_private[3] = 0x40;
		cmd_buff_private[2] = 0;
		cmd_buff_private[4] = zfspeed;
		break;
	case 3:
		cmd_buff_private[3] = 0x00;
		cmd_buff_private[2] = 0x01;//
		
		cmd_buff_private[4] = zfspeed;
		break;
	case 4:
		cmd_buff_private[3] = 0x80;
		cmd_buff_private[2] = 0;
		
		cmd_buff_private[4] = zfspeed;
		break;
	case 0:
		cmd_buff_private[3] = 0x00;
		cmd_buff_private[2] = 0;
		break;
	}
	
	//cmd_buff_private[4] = 0;
	cmd_buff_private[5] = 0;
	
	cmd_buff_private[6] = cmd_buff_private[1] + cmd_buff_private[2] + cmd_buff_private[3] + cmd_buff_private[4] + cmd_buff_private[5];
	rs485_send_data(cmd_buff_private,7);
}

void pelcod_stop_packet_send(void)
{
	u8 cnt;
	
	u8 cmd_buff_private[7];
	cmd_buff_private[0] = 0xff;
	cmd_buff_private[1] = target_id;
		cmd_buff_private[2] = 0;
	cmd_buff_private[3] = 0;
	cmd_buff_private[4] = 0;
	cmd_buff_private[5] = 0;
	
	cmd_buff_private[6] = cmd_buff_private[1] + cmd_buff_private[2] + cmd_buff_private[3] + cmd_buff_private[4] + cmd_buff_private[5];
	rs485_send_data(cmd_buff_private,7);
}

extern u8 wait_device_reply(u8* srcdata,u8 len,u32 w_100ms);

u8 is_mode=0;
u8 is_value = 0;


extern u8 cam_filter_mode;
extern u8 iris_mode;
extern u8 iris_val;

extern void num_to_string(u16 data,u8 *dst);

u8 iris_val_osd_buf[10];

u8 iris_motor_mode = 0;




u8 rs485_get_data_from_slave_thread(void)
{
	
	static u8 cmd_buff_private[7];

	u8 cam_filter_mode_bak = 0xff,iris_mode_bak=0xff,iris_val_bak=0;
	
	cmd_buff_private[0] = 0xff;
	cmd_buff_private[1] = target_id;
		cmd_buff_private[2] = 0;
	cmd_buff_private[3] = 0x88;
	cmd_buff_private[4] = 0;
	cmd_buff_private[5] = 0;
	
	cmd_buff_private[6] = cmd_buff_private[1] + cmd_buff_private[2] + cmd_buff_private[3] + cmd_buff_private[4] + cmd_buff_private[5];


	while(1)
	{
		rt_thread_delay(800);

		rs485_send_data(cmd_buff_private,7);
	
	
		if(RT_EOK == rs485_recieve_check(0x99))
		{

			cam_filter_mode = Rocket_sec_data-1;

			if(cam_filter_mode>3)
			cam_filter_mode = 0;
			
			iris_mode = Rocket_thr_data&0x0f;
			//iris_motor_mode = (Rocket_thr_data>>4)&0x0f;
			iris_val = Rocket_fou_data;
			if(iris_val>100)
				iris_val=100;

			if(iris_val_bak != iris_val || cam_filter_mode_bak!=cam_filter_mode || iris_mode_bak!= iris_mode)
			{
				iris_val_bak = iris_val;
				cam_filter_mode_bak = cam_filter_mode;
				iris_mode_bak = iris_mode;

				
				osd_line3_disp(0);
				osd_line2_disp(0);
			}

		}
		else
		{
			iris_val = 0xff;
			//iris_motor_mode = 0xff;
		}
		
	}

	//osd_line3_disp(0);
	return 0;
}




u8 rs485_get_data_from_slave(void)
#if 1

{}
#else
{
	u8 cnt;
	
	u8 cmd_buff_private[7];

	rt_thread_delay(100);
	
	cmd_buff_private[0] = 0xff;
	cmd_buff_private[1] = target_id;
		cmd_buff_private[2] = 0;
	cmd_buff_private[3] = 0x88;
	cmd_buff_private[4] = 0;
	cmd_buff_private[5] = 0;
	
	cmd_buff_private[6] = cmd_buff_private[1] + cmd_buff_private[2] + cmd_buff_private[3] + cmd_buff_private[4] + cmd_buff_private[5];
	rs485_send_data(cmd_buff_private,7);
	
	cnt=2;
	while(cnt--)
	{
		if(RT_EOK == rs485_recieve_check(0x99))
			{

			cam_filter_mode = Rocket_sec_data;
			iris_mode = Rocket_thr_data&0x0f;
			//iris_motor_mode = (Rocket_thr_data>>4)&0x0f;
			iris_val = Rocket_fou_data;
			if(iris_val>100)
				iris_val=100;
			osd_line3_disp(0);
			break;

			}
		else
		{
			iris_val = 0xff;
			//iris_motor_mode = 0xff;
		}
		//else
		//	rs485_send_data(cmd_buff_private,7);


		
		rs485_send_data(cmd_buff_private,7);
		
		rt_thread_delay(50);
	}

	//osd_line3_disp(0);
	return 0;
}
#endif

