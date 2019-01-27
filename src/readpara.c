/*
 * readpara.c
 *
 *  Created on: 2016-4-8
 *      Author: vmuser
 */
#include "readpara.h"
#include "data.h"
#include <mysql.h>
#include "shmdata.h"

extern spiset 			spisets;
extern spidata 			spidatas;
extern i2cdata 			i2c;
extern flowR 			flowresult;
extern PVT_BlackOil		PVT_BlackOils;
extern PVT_Normal		PVT_Normals;
extern PFC_Three		PFC_Threes;
extern	corr			corrs;
extern	cpm				cpms;

extern int 			win_C[200][6];//计算用的二维数组
extern datetime 		m_time;
extern	MYSQL 			mysql_conn;
extern flag				flags;
extern unsigned int es[8192];
extern struct shared_use_st *shared;//指向shm
MYSQL_RES 				*res;												// 声明结果集
MYSQL_ROW 				row;												// 声明行操作符
FILE 					*F_fd;
time_t 					timep;
Adjust					ParaAdjust;

extern Flow_result		Flow_results;


int readpara(void)
{
	readtime();
	read_fpga_setpara();
	read_arm_para();
	read_flow_para();
	read_ad1115();
	read_adjust();
	return 0;
}
int readtime(void)
{
        int fd, retval;
        struct rtc_time rtc_tm;

        fd = open("/dev/rtc", O_RDONLY);
        if (fd == -1) {
                perror("/dev/rtc");
                exit(errno);
        }

        /* Read the RTC time/date */
        retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
        if (retval == -1) {
                perror("ioctl");
                exit(errno);
        }
 //
        m_time.timestamp=time(&timep)+8*60*60; /*当前time_t类型UTC时间戳*/
        //printf("time():%ld\n",m_time.timestamp);

 //       fprintf(stderr, "RTC date/time: %d/%d/%d %02d:%02d:%02d\n",
 //               rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
  //              rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
        m_time.fd_RTC	=fd;
        m_time.year 	= rtc_tm.tm_year + 1900;
        m_time.month 	= rtc_tm.tm_mon + 1;
        m_time.date		= rtc_tm.tm_mday;
        m_time.hour		= rtc_tm.tm_hour;
        m_time.minute	= rtc_tm.tm_min;
        m_time.second	= rtc_tm.tm_sec;
        close(fd);
        return 0;
}
/*
 * 从数据库里读取FPGA设置参数；
 * 生成FPGA参数的json文件，供网页读取；
 */
int read_fpga_setpara(void)
{
	int t;
	char execsql[1024];
	int spi_set[20];
	mysql_select_db(&mysql_conn, "FPGA_SET");
	sprintf(execsql,"select * from FPGA_SET");								//设置将要操作的数据库
	t = mysql_real_query(&mysql_conn,execsql,strlen(execsql));			// 执行查询语句
	if (t)
		printf("查询数据库失败：%s\n", mysql_error(&mysql_conn));
	else {
		res = mysql_store_result(&mysql_conn);								// 获得查询结果
		while((row = mysql_fetch_row(res))) {								// 在结果集内步进
			for(t = 0; t < mysql_num_fields(res); t++){
	//			printf("%s ",row[t]);	// 输出每列的数据
				spi_set[t]=atoi(row[t]);
				printf("%d\n",spi_set[t]);

			}
			if(spi_set[0]== 1)                                              //这个位置以后可以设置多组数据用num替换1，判断用那一组参数
			{
				spisets.pch 	= spi_set[1];
				spisets.th 		= spi_set[2];
				spisets.winA1 	= spi_set[3];
				spisets.winA2 	= spi_set[4];
				spisets.winB1 	= spi_set[5];
				spisets.winB2	= spi_set[6];
				spisets.winC1	= spi_set[7];
				spisets.winC2 	= spi_set[8];
				spisets.winD1 	= spi_set[9];
				spisets.winD2	= spi_set[10];
				spisets.winE1	= spi_set[11];
				spisets.winE2	= spi_set[12];
				spisets.st 		= spi_set[13];
				spisets.ct 		= spi_set[14];
				spisets.pth 	= spi_set[15];
				spisets.pmin 	= spi_set[16];
				spisets.pmax 	= spi_set[17];
				spisets.bl		= spi_set[18];
			}
				printf("\n");
		}
	mysql_free_result(res);										// 释放查询结果
	}

	//FPGA to json
	F_fd = fopen("/usr/local/apache/htdocs/yangpai/tables/fpga-P.json", "w+b");
	sprintf(execsql,"[{\"para1\":\"WIN-AU\", \"value1\":\"%d\",\"para2\": \"WIN-AD\",\"value2\":\"%d\"},\n"
					 "{\"para1\":\"WIN-BU\", \"value1\":\"%d\",\"para2\": \"WIN-BD\",\"value2\":\"%d\"},\n"
					 "{\"para1\":\"WIN-CU\", \"value1\":\"%d\",\"para2\": \"WIN-CD\",\"value2\":\"%d\"},\n"
					 "{\"para1\":\"WIN-DU\", \"value1\":\"%d\",\"para2\": \"WIN-DD\",\"value2\":\"%d\"},\n"
					 "{\"para1\":\"WIN-EU\", \"value1\":\"%d\",\"para2\": \"WIN-ED\",\"value2\":\"%d\"},\n"
					 "{\"para1\":\"ES-T\", \"value1\":\"%d s\",\"para2\": \"CYC-T\",\"value2\":\"%d ms\"},\n"
					 "{\"para1\":\"ES-TH\", \"value1\":\"%d\",\"para2\": \"PUL-TH\",\"value2\":\"%d\"},\n"
					 "{\"para1\":\"PUL-U\", \"value1\":\"%d\",\"para2\": \"PUL-D\",\"value2\":\"%d\"},\n"
					 "{\"para1\":\"ES-BASE\", \"value1\":\"%d\",\"para2\": \"NULL\",\"value2\":\"NULL\"}]",
					spisets.winA1,spisets.winA2,spisets.winB1,spisets.winB2,spisets.winC1,spisets.winC2,
					spisets.winD1,spisets.winD2,spisets.winE1,spisets.winE2,spisets.ct,spisets.st,
					spisets.th, spisets.pth ,spisets.pmax,spisets.pmin,spisets.bl);
	fputs(execsql,F_fd);
	fclose(F_fd);

	return 0;
}
/*-------------------------------------------------------------------------------------------------------*/
int read_arm_para(void)
{
	int t;
	char execsql[1024];
	int spi_set[20];
	mysql_select_db(&mysql_conn, "ARM_SET");
	sprintf(execsql,"select * from ARM_SET");								//设置将要操作的数据库
	t = mysql_real_query(&mysql_conn,execsql,strlen(execsql));			// 执行查询语句
	if (t)
		printf("查询数据库失败：%s\n", mysql_error(&mysql_conn));
	else {
		res = mysql_store_result(&mysql_conn);								// 获得查询结果
		while((row = mysql_fetch_row(res))) {								// 在结果集内步进
			for(t = 0; t < mysql_num_fields(res); t++){
		//		printf("%s ",row[t]);	// 输出每列的数据
				spi_set[t]=atoi(row[t]);
				printf("%d\n",spi_set[t]);
			}
			if(spi_set[0]== 1)                                              //这个位置以后可以设置多组数据用num替换1，判断用那一组参数
			{
				flags.flag_modbus	=	spi_set[1]	&0x01;
				flags.flag_log		=	spi_set[1]	&0x02;
				flags.flag_es		=	spi_set[1]	&0x04;
				shared->HIV_en		=	spi_set[1]	&0x08;//改为是否自动调压 8为自动调整；
				flags.flag_5ms		=	spi_set[1]	&0x10;
				shared->F_WIN_en	=	spi_set[1]	&0x20;
				flags.flag_dog		=	spi_set[1]	&0x40;
				shared->OLED_en		=	spi_set[1]	&0x80;
				printf("test:%d,%d,%d,%d,%d,%d,%d\n",flags.flag_modbus,flags.flag_oled,flags.flag_es,flags.flag_pulse,flags.flag_5ms,flags.flag_MCA,flags.flag_dog);
				shared->S_HIV		=	spi_set[2];
				shared->S_VBB		=	spi_set[3];
				shared->S_HV_C		=	spi_set[4]/100.0;
		//		shared->CON_A1		=	spi_set[5];
			//	shared->CON_A2		=	spi_set[6];
			//	shared->S_PCH31		=	spi_set[7];
				shared->CON_B1		=	spi_set[8];
				shared->CON_B2		=	spi_set[9];
				shared->S_PCH31		=	spi_set[10];
				shared->CON_C1		=	spi_set[11];
				shared->CON_C2		=	spi_set[12];
				shared->S_PCH81		=	spi_set[13];
				//spi_set 11~16预留
				shared->CON_E1		=	spi_set[17];
				shared->CON_E2		=	spi_set[18];
				shared->S_PCH356	=	spi_set[19];
				shared->th_sum		=	spi_set[20];
			}
			printf("\n");
		}
	mysql_free_result(res);										// 释放查询结果
	}
	/*
	 * 数据处理
	 */
	//三点 改两点
/*	float ave_x		=	(31+81+356)/3.0;
	float ave_y		=	(shared->S_PCH31+shared->S_PCH81+shared->S_PCH356)/3.0;
	float ave_xy	=	(31*shared->S_PCH31+81*shared->S_PCH81+356*shared->S_PCH356)/3.0;
	float ave_xx	=	(31*31+81*81+356*356)/3.0;
	shared->a1		=	(ave_x*ave_y-ave_xy)/(ave_x*ave_x-ave_xx);
	shared->b1		=	ave_y-shared->a1*ave_x;*/


	shared->a1 = (shared->S_PCH81 - shared->S_PCH31)/(81.0 - 31.0);
	shared->b1 = shared->S_PCH31 - 31* shared->a1;
	printf("a1=%f,b1=%f\n",shared->a1,shared->b1);



	return 0;
}
int read_flow_para(void)
{
	int t;
	char execsql[1024];
	float flow_p[40];
	mysql_select_db(&mysql_conn, "FLOW_Para");
	sprintf(execsql,"select * from PVT_blockOil");								//设置将要操作的数据库
	t = mysql_real_query(&mysql_conn,execsql,strlen(execsql));			// 执行查询语句
	if (t)
		printf("查询数据库失败：%s\n", mysql_error(&mysql_conn));
	else {
		res = mysql_store_result(&mysql_conn);								// 获得查询结果
		while((row = mysql_fetch_row(res))) {								// 在结果集内步进
			for(t = 0; t < mysql_num_fields(res); t++){
		//		printf("%s ",row[t]);	// 输出每列的数据
				flow_p[t]=atof(row[t]);
				printf("%f\n",flow_p[t]);
			}
			if(flow_p[0]== 1)                                              //这个位置以后可以设置多组数据用num替换1，判断用那一组参数
			{
				PVT_BlackOils.oilExpansionFactor	=	flow_p[1];
				PVT_BlackOils.waterExpansionFactor	=	flow_p[2];
				PVT_BlackOils.localPressure			=	flow_p[3];
				PVT_BlackOils.TempertureSC			=	flow_p[4];
				PVT_BlackOils.oilDensitySC			=	flow_p[5];
				PVT_BlackOils.waterDensitySC		=	flow_p[6];
				PVT_BlackOils.gasDensitySC			=	flow_p[7];
				PVT_BlackOils.Gr					=	flow_p[8];
				PVT_BlackOils.nMC					=	flow_p[9];
				PVT_BlackOils.nMn					=	flow_p[10];
				PVT_BlackOils.nMC1					=	flow_p[11];
				PVT_BlackOils.nZn					=	flow_p[12];
				PVT_BlackOils.nPb					=	flow_p[13];
				PVT_BlackOils.referenceTempertaureSC	=	flow_p[14];

			}
			printf("\n");
		}
	mysql_free_result(res);										// 释放查询结果
	}

	sprintf(execsql,"select * from PFC_ThreeEnergy");								//设置将要操作的数据库
	t = mysql_real_query(&mysql_conn,execsql,strlen(execsql));			// 执行查询语句
	if (t)
		printf("查询数据库失败：%s\n", mysql_error(&mysql_conn));
	else {
		res = mysql_store_result(&mysql_conn);								// 获得查询结果
		while((row = mysql_fetch_row(res))) {								// 在结果集内步进
			for(t = 0; t < mysql_num_fields(res); t++){
		//		printf("%s ",row[t]);	// 输出每列的数据
				flow_p[t]=atof(row[t]);
				printf("%f\n",flow_p[t]);
			}
			if(flow_p[0]== 1)                                              //这个位置以后可以设置多组数据用num替换1，判断用那一组参数
			{
				PFC_Threes.Energy1_Empty	=	flow_p[1];
				PFC_Threes.Energy1_Oil		=	flow_p[2];
				PFC_Threes.Energy1_Gas		=	flow_p[3];
				PFC_Threes.Energy1_Water	=	flow_p[4];
				PFC_Threes.Energy2_Empty	=	flow_p[5];
				PFC_Threes.Energy2_Oil		=	flow_p[6];
				PFC_Threes.Energy2_Gas		=	flow_p[7];
				PFC_Threes.Energy2_Water	=	flow_p[8];
				PFC_Threes.Energy5_Empty	=	flow_p[9];
				PFC_Threes.Energy5_Oil		=	flow_p[10];
				PFC_Threes.Energy5_Gas		=	flow_p[11];
				PFC_Threes.Energy5_Water	=	flow_p[12];
				PFC_Threes.f1				=	flow_p[13];
				PFC_Threes.f2				=	flow_p[14];
				PFC_Threes.C				=	flow_p[15];
				PFC_Threes.epsilon			=	flow_p[16];
				PFC_Threes.Diameter			=	flow_p[17];
				PFC_Threes.beta				=	flow_p[18];
			}
			printf("\n");
		}
	mysql_free_result(res);										// 释放查询结果
	}

	/*
	 *
*/
	sprintf(execsql,"select * from PVT_normal");								//设置将要操作的数据库
		t = mysql_real_query(&mysql_conn,execsql,strlen(execsql));			// 执行查询语句
		if (t)
			printf("查询数据库失败：%s\n", mysql_error(&mysql_conn));
		else {
			res = mysql_store_result(&mysql_conn);								// 获得查询结果
			while((row = mysql_fetch_row(res))) {								// 在结果集内步进
				for(t = 0; t < mysql_num_fields(res); t++){
			//		printf("%s ",row[t]);	// 输出每列的数据
					flow_p[t]=atof(row[t]);
					printf("%f\n",flow_p[t]);
				}
				if(flow_p[0]== 1)                                              //这个位置以后可以设置多组数据用num替换1，判断用那一组参数
				{
					PVT_Normals.beatO			=	flow_p[1];
					PVT_Normals.beatW			=	flow_p[2];
					PVT_Normals.P0				=	flow_p[3];
					PVT_Normals.T0				=	flow_p[4];
					PVT_Normals.oilDensitySC	=	flow_p[5];
					PVT_Normals.gasDensitySC	=	flow_p[6];
					PVT_Normals.waterDensitySC	=	flow_p[7];
					PVT_Normals.ntime			=	flow_p[8];

				}
				printf("\n");
			}
		mysql_free_result(res);										// 释放查询结果
		}
/*
*
*/
	sprintf(execsql,"select * from FlowMinute");								//设置将要操作的数据库
	t = mysql_real_query(&mysql_conn,execsql,strlen(execsql));			// 执行查询语句
	if (t)
		printf("查询数据库失败：%s\n", mysql_error(&mysql_conn));
	else {
		res = mysql_store_result(&mysql_conn);								// 获得查询结果
		while((row = mysql_fetch_row(res))) {								// 在结果集内步进
			for(t = 0; t < mysql_num_fields(res); t++){
//				printf("%s ",row[t]);	// 输出每列的数据
				flow_p[t]=atof(row[t]);
				printf("%f\n",flow_p[t]);
			}
			if(flow_p[0]== 1)                                              //这个位置以后可以设置多组数据用num替换1，判断用那一组参数
			{
				cpms.P_CPM_A			=	flow_p[1];
				cpms.P_CPM_B			=	flow_p[2];
				cpms.DP_CPM_A			=	flow_p[3];
				cpms.DP_CPM_B			=	flow_p[4];
				cpms.DP_CPM_C			=	flow_p[5];
				cpms.DP_CPM_D			=	flow_p[6];
				cpms.TF_PCPM_SC			=	flow_p[7];
				cpms.P_CPM_SC			=	flow_p[8];
				cpms.F_CPM				=	flow_p[9];
				cpms.D_CPM				=	flow_p[10];
				cpms.G31_CPM_A			=	flow_p[11];
				cpms.G31_CPM_B			=	flow_p[12];
				cpms.W31_CPM			=	flow_p[13];
				cpms.G356_CPM_A			=	flow_p[14];
				cpms.G356_CPM_B			=	flow_p[15];
				cpms.W356_CPM			=	flow_p[16];
				cpms.WD_CPM				=	flow_p[17];
				cpms.GD_CPM_SC			=	flow_p[18];
				cpms.b_CPM				=	flow_p[19];
				cpms.C_CPM				=	flow_p[20];
				cpms.E_CPM				=	flow_p[21];

			}
			printf("\n");
		}
		mysql_free_result(res);										// 释放查询结果
	}

/*
*
*/
	sprintf(execsql,"select * from Correction");								//设置将要操作的数据库
	t = mysql_real_query(&mysql_conn,execsql,strlen(execsql));			// 执行查询语句
	if (t)
		printf("查询数据库失败：%s\n", mysql_error(&mysql_conn));
	else {
		res = mysql_store_result(&mysql_conn);								// 获得查询结果
		while((row = mysql_fetch_row(res))) {								// 在结果集内步进
			for(t = 0; t < mysql_num_fields(res); t++){
	//			printf("%s ",row[t]);	// 输出每列的数据
				flow_p[t]=atof(row[t]);
				printf("%f\n",flow_p[t]);
			}
			if(flow_p[0]== 1)                                              //这个位置以后可以设置多组数据用num替换1，判断用那一组参数
			{
				corrs.OVF_CPM_A			=	flow_p[1];
				corrs.OVF_CPM_B			=	flow_p[2];
				corrs.OMF_CPM_A			=	flow_p[3];
				corrs.OMF_CPM_B			=	flow_p[4];
				corrs.Qv_CPM_A			=	flow_p[5];
				corrs.Qv_CPM_B			=	flow_p[6];
				corrs.Qm_CPM_A			=	flow_p[7];
				corrs.Qm_CPM_B			=	flow_p[8];
				corrs.K31_Empty			=	flow_p[9];
				corrs.CMP_ave			=	flow_p[10];
				corrs.DP_th				=	flow_p[11];
			}
			printf("\n");
		}
		mysql_free_result(res);										// 释放查询结果
	}

	/*
	*
	*/
		sprintf(execsql,"select * from Flow_Result");								//设置将要操作的数据库
		t = mysql_real_query(&mysql_conn,execsql,strlen(execsql));			// 执行查询语句
		if (t)
			printf("查询数据库失败：%s\n", mysql_error(&mysql_conn));
		else {
			res = mysql_store_result(&mysql_conn);								// 获得查询结果
			while((row = mysql_fetch_row(res))) {								// 在结果集内步进
				for(t = 0; t < mysql_num_fields(res); t++){
		//			printf("%s ",row[t]);	// 输出每列的数据
					flow_p[t]=atof(row[t]);
					printf("%f\n",flow_p[t]);
				}
				if(flow_p[0]== 1)                                              //这个位置以后可以设置多组数据用num替换1，判断用那一组参数
				{
					shared->Qmg_CPM			=	flow_p[1];
					shared->Qml_CPM			=	flow_p[2];
				}
				printf("\n");
			}
			mysql_free_result(res);										// 释放查询结果
		}

	return 0;
}
int read_ad1115(void)
{
	int t;
	char execsql[1024];
	float flow_p[20];
	mysql_select_db(&mysql_conn, "ARM_SET");
	sprintf(execsql,"select * from AD1115");								//设置将要操作的数据库
	t = mysql_real_query(&mysql_conn,execsql,strlen(execsql));			// 执行查询语句
	if (t)
		printf("查询数据库失败：%s\n", mysql_error(&mysql_conn));
	else {
		res = mysql_store_result(&mysql_conn);								// 获得查询结果
		while((row = mysql_fetch_row(res))) {								// 在结果集内步进
			for(t = 0; t < mysql_num_fields(res); t++){
		//		printf("%s ",row[t]);	// 输出每列的数据
				flow_p[t]=atof(row[t]);
				printf("%f\n",flow_p[t]);
			}
			if(flow_p[0]== 1)                                              //这个位置以后可以设置多组数据用num替换1，判断用那一组参数
			{
				shared->Pre_a		=	flow_p[1];
				shared->Pre_b		=	flow_p[2];
				shared->DP_a		=	flow_p[3];
				shared->DP_b		=	flow_p[4];
				shared->PreT_a		=	flow_p[5];
				shared->PreT_b		=	flow_p[6];
				}
			printf("\n");
		}
	mysql_free_result(res);										// 释放查询结果
	}
	return 0;

}
int read_adjust(void)
{
	int t;
	char execsql[1024];
	float flow_p[20];
	mysql_select_db(&mysql_conn, "ARM_SET");
	sprintf(execsql,"select * from Adjust");								//设置将要操作的数据库
	t = mysql_real_query(&mysql_conn,execsql,strlen(execsql));			// 执行查询语句
	if (t)
		printf("查询数据库失败：%s\n", mysql_error(&mysql_conn));
	else {
		res = mysql_store_result(&mysql_conn);								// 获得查询结果
		while((row = mysql_fetch_row(res))) {								// 在结果集内步进
			for(t = 0; t < mysql_num_fields(res); t++){
		//		printf("%s ",row[t]);	// 输出每列的数据
				flow_p[t]=atof(row[t]);
				printf("%f\n",flow_p[t]);
			}
			if(flow_p[0]== 1)                                              //这个位置以后可以设置多组数据用num替换1，判断用那一组参数
			{
				ParaAdjust.Adjust_WinB_a		=	flow_p[1];
				ParaAdjust.Adjust_WinB_b		=	flow_p[2];
				ParaAdjust.Adjust_WinC_a		=	flow_p[3];
				ParaAdjust.Adjust_WinC_b		=	flow_p[4];
				ParaAdjust.Adjust_WinE_a		=	flow_p[5];
				ParaAdjust.Adjust_WinE_b		=	flow_p[6];
				}
			printf("\n");
		}
	mysql_free_result(res);										// 释放查询结果
	}
	return 0;

}
int write_5ms_data(void)
{
	int i;
	static int num_5ms;
	char execsql[2048];
	char time_5ms[20];
	static int mysql_year_5ms;
	static int mysql_month_5ms;
	static int mysql_date_5ms;
	static int mysql_hour_5ms;
	float ad_data[200][3];
	for(i=0;i<200;i++)
	{
		ad_data[i][0]=shared->ad1115[i][0];
		ad_data[i][1]=shared->ad1115[i][1];
		ad_data[i][2]=shared->ad1115[i][2];

		//printf("%f\n",shared->ad1115[i][2]);
	}

	sprintf(time_5ms,"%.2d:%.2d",m_time.minute,m_time.second);
	//printf("%s\n",time_5ms);
	//创建文件夹     判断年月ri
	if((mysql_year_5ms != m_time.year)||(mysql_month_5ms != m_time.month)||(mysql_date_5ms != m_time.date))
	{
		sprintf(execsql,"create database %4d_%.2d_%.2d_5ms",m_time.year,m_time.month,m_time.date);
//		printf("%s \n", execsql);
		mysql_real_query(&mysql_conn, execsql, strlen(execsql));//执行查询语句：创建数据库
		i = mysql_affected_rows(&mysql_conn);
		if (i<=0){
			printf("Can not create database\n");
		}
	}
	//创建数据表格   判断日期
	if((mysql_year_5ms != m_time.year)||(mysql_month_5ms != m_time.month)||(mysql_date_5ms != m_time.date)||(mysql_hour_5ms != m_time.hour))
	{
		sprintf(execsql,"%4d_%.2d_%.2d_5ms",m_time.year,m_time.month,m_time.date);
//		printf("%s \n", execsql);
		mysql_select_db(&mysql_conn, execsql);//设置要操作的数据库
		sprintf(execsql, "create table %4d_%.2d_%.2d_%.2d(num char(30),time char(20), win1 int(16),"
				"win2 int(16),win3 int(16),win4 int(16),win5 int(16),ad1 float(32),ad2 float(32),ad3 float(32),"
				"oilDensityLC float(32),gasDensityLC float(32),waterDensityLC float(32),"
				"Qm_Oils float(32),Qm_Gass float(32),Qm_Waters float(32))",
				m_time.year,m_time.month,m_time.date,m_time.hour);//以年月日为名字
//		printf("%s \n", execsql);
		mysql_real_query(&mysql_conn, execsql, strlen(execsql));
		num_5ms = 0;
	}
	//数据填充
	for(i=0;i<200;i++)
	{
		/*sprintf(execsql,"insert into %4d_%.2d_%.2d_%.2d values('%d','%s',%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f)",
				m_time.year,m_time.month,m_time.date,m_time.hour,num_5ms,time_5ms,win_C[i][0],win_C[i][1],win_C[i][2],
				win_C[i][3],win_C[i][4],ad_data[i][0],ad_data[i][1],ad_data[i][2],1.0,1.0,
				1.0,1.0,1.0,1.0);*/
		sprintf(execsql,"insert into %4d_%.2d_%.2d_%.2d values('%d','%s',%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f)",
						m_time.year,m_time.month,m_time.date,m_time.hour,num_5ms,time_5ms,win_C[i][0],win_C[i][1],win_C[i][2],
						win_C[i][3],win_C[i][4],ad_data[i][0],ad_data[i][1],ad_data[i][2],Flow_results.oilDensityLC,Flow_results.gasDensityLC,
						Flow_results.waterDensityLC,Flow_results.Qm_Oils[i],Flow_results.Qm_Gass[i],Flow_results.Qm_Waters[i]);
		mysql_query(&mysql_conn, execsql);
		num_5ms++;
	}
//	sprintf(execsql,"insert into %4d_%.2d_%.2d values('%d', %f)",m_time.year,m_time.month,m_time.date,num , 1.1);
//	printf("%s \n", execsql);

	mysql_year_5ms 	= m_time.year;
	mysql_month_5ms	=m_time.month;
	mysql_date_5ms	=m_time.date;
	mysql_hour_5ms	=m_time.hour;

	return 0;
}
int write_mysql_data(void)
{
	int i;
	char *names[3];
	names[0] = "haha";
	names[1] = "xixi";
	names[2] = "wuwu";

	char execsql[1024];
	sprintf(execsql, "create database one_db");//名称是时间
				mysql_real_query(&mysql_conn, execsql, strlen(execsql));//执行查询语句：创建数据库
				i = mysql_affected_rows(&mysql_conn);
				if (i<=0){
					printf("Can not create database one_db \n");
				}

				mysql_select_db(&mysql_conn, "one_db");//设置即将操作的数据库

				sprintf(execsql, "create table boys(name char(10), age int(8))");//以年月日为名字
				mysql_real_query(&mysql_conn, execsql, strlen(execsql));

				mysql_select_db(&mysql_conn, "one_db");
				for(i=0; i<3; i++){
					sprintf(execsql,"insert into boys values('%s', %d)", names[i], 19+i);
					printf("%s \n", execsql);
					mysql_query(&mysql_conn, execsql);
				}
	return 0;
}

//。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
int pulse_data_w(void)
{
	//printf("pulse-json working\n");
	int k = 0;
	int c = 0;
	char data_time1[20] = {};
	char spi_en[8192] = {};
	char p_log1[4096] = {};//add data
	FILE *fd_pulse;
	fd_pulse = fopen("/usr/local/apache/htdocs/yangpai/pulse_data.json", "w+b");
	for(k=0; k<500;k++)
	{
//		c = pt->pulse_data[2*k]*256+pt->pulse_data[2*k+1];
		c = spidatas.dpul[k+10];//横坐标用纳秒表示，
		if(c < 0){
		c = 0;
		}
		sprintf(data_time1,"[%d,%d],",k*10,c);
		strcat(spi_en,data_time1);
	}

	sprintf(p_log1,"%s%s%s%d%s","{\"label\":\"脉冲曲线 x:t/ns y:v/mv\",\"data\":[",spi_en,"[5000,",spidatas.dpul[500+10],"]]}");
//	fwrite(p_log1,strlen(p_log1),1,fd_en );
	fputs(p_log1,fd_pulse);

	fclose(fd_pulse);
	return 0;


}
/*能普数据写入json*/
int spectral_data_w(void)
{
//	printf("####\n");
	int k = 0;
	int i = 0;
	char data_time1[20] = {};
	char spi_en[132000] = {};
	char p_log1[66666] = {};//add data
	unsigned int es512[512];
	FILE *fd_en;
	fd_en = fopen("/usr/local/apache/htdocs/yangpai/data-eu-gdp-growth.json", "w+b");
	for(k=0; k<8192;k++)
	{
		sprintf(data_time1,"[%d,%d],",k,es[k]);
		strcat(spi_en,data_time1);
	}

	sprintf(p_log1,"%s%s%s","{\"label\":\"能谱曲线\",\"data\":[",spi_en,"[8192,0]]}");
//	fwrite(p_log1,strlen(p_log1),1,fd_en );
	fputs(p_log1,fd_en);

	fclose(fd_en);

	return 0;
}
int write_es_data(void)
{
//	printf("++1/n");
	int i;
	static int esnum;
	char execsql[2048];
	static int esmysql_year;
	static int esmysql_month;
	static int esmysql_date;
	static int esmysql_hour;
	static int esmysql_minute;


	//创建文件夹     判断年月
	if((esmysql_year != m_time.year)||(esmysql_month != m_time.month))
	{
	//	printf("++2/n");
		sprintf(execsql,"create database es%4d_%.2d",m_time.year,m_time.month);
	//	printf("%s \n", execsql);
		mysql_real_query(&mysql_conn, execsql, strlen(execsql));//执行查询语句：创建数据库
		i = mysql_affected_rows(&mysql_conn);
		if (i<=0){
			printf("Can not create database\n");
		}
	}
	//创建数据表格   判断日期
	if((esmysql_year != m_time.year)||(esmysql_month != m_time.month)||(esmysql_date != m_time.date)||(esmysql_hour != m_time.hour)||(esmysql_minute != m_time.minute))
	{
	//	printf("++3/n");

		sprintf(execsql,"es%4d_%.2d",m_time.year,m_time.month);
//		printf("%s \n", execsql);
		mysql_select_db(&mysql_conn, execsql);//设置要操作的数据库
		sprintf(execsql, "create table es%4d_%.2d_%.2d_%.2d_%.2d(num char(30), es int(16))",m_time.year,m_time.month,m_time.date,m_time.hour,m_time.minute);//以年月日为名字
//		printf("%s \n", execsql);
		mysql_real_query(&mysql_conn, execsql, strlen(execsql));
		esnum = 0;
//		sprintf(execsql,"%4d_%.2d",m_time.year,m_time.month);
	//	printf("%s \n", execsql);
//		mysql_select_db(&mysql_conn, execsql);//设置要操作的数据库
//		sprintf(execsql, "create table %4d_%.2d_%.2d_%.2d:%.2d(num char(30), es int(16))",m_time.year,m_time.month,m_time.date,m_time.hour,m_time.minute);//以年月日为名字
	//	printf("%s \n", execsql);
//		mysql_real_query(&mysql_conn, execsql, strlen(execsql));
//		esnum = 0;
	}
	//数据填充
	for(i=0;i<8192;i++)
	{
	//	printf("++4/n");
		sprintf(execsql,"insert into es%4d_%.2d_%.2d_%.2d_%.2d values('%d',%d)",m_time.year,m_time.month,m_time.date,m_time.hour,m_time.minute,i ,spidatas.des[i]);
		mysql_query(&mysql_conn, execsql);
		esnum++;
	}
	//	sprintf(execsql,"insert into %4d_%.2d_%.2d values('%d', %f)",m_time.year,m_time.month,m_time.date,num , 1.1);
	//	printf("%s \n", execsql);

	esmysql_year 	= m_time.year;
	esmysql_month	=m_time.month;
	esmysql_date	=m_time.date;
	esmysql_hour	=m_time.hour;
	esmysql_minute	=m_time.minute;

	return 0;
}
int flow_data_w(void)
{
	int i = 0;
	static int flowc =0;
	char data_time1[132000] = {};
	char spi_en[132000] = {};
	char p_log1[132000] = {};//add data
	F_fd = fopen("/usr/local/apache/htdocs/yangpai/tables/flow.json", "w+b");//{"label":"脉冲曲线 x:t/ns y:v/mv","data":[[1196463600000,70],[1196463660000,100],[1196463720000,97]]}
	for(i=0;i<599;i++)
	{
		flowresult.foils[599-i] = flowresult.foils[598-i];
		m_time.timestamps[599-i] = m_time.timestamps[598-i];
	}
	flowresult.foils[0]=flowresult.foil;
	m_time.timestamps[0]=m_time.timestamp;
	//printf("%f === %ld\n",flowresult.foil,m_time.timestamps[0]);

	if(flowc == 0)
	{
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[flowc]*1000,flowresult.foils[flowc]);
		strcat(spi_en,data_time1);
		flowc++;
	}
	else if (flowc < 600)
	{
		for(i=0;i<flowc;i++)
		{
			sprintf(data_time1,"[%.0f,%f],",(double)m_time.timestamps[i]*1000,flowresult.foils[i]);
			strcat(spi_en,data_time1);
		}
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[flowc]*1000,flowresult.foils[flowc]);
		strcat(spi_en,data_time1);
		flowc++;
	}
	else
	{
		for(i=0; i<599;i++)
		{
			sprintf(data_time1,"[%.0f,%f],",(double)m_time.timestamps[i]*1000,flowresult.foils[i]);
			strcat(spi_en,data_time1);
		}
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[599]*1000,flowresult.foils[599]);
		strcat(spi_en,data_time1);
	}



	sprintf(p_log1,"%s%s%s","{\"label\":\"oil-flow\",\"data\":[",spi_en,"]}");
//	fwrite(p_log1,strlen(p_log1),1,fd_en );
	fputs(p_log1,F_fd);

	fclose(F_fd);

	return 0;
}
int flow1_data_w(void)
{
	int i = 0;
	static int flowc1 =0;
	char data_time1[132000] = {};
	char spi_en[132000] = {};
	char p_log1[132000] = {};//add data
	F_fd = fopen("/usr/local/apache/htdocs/yangpai/tables/flow1.json", "w+b");//{"label":"脉冲曲线 x:t/ns y:v/mv","data":[[1196463600000,70],[1196463660000,100],[1196463720000,97]]}
	for(i=0;i<599;i++)
	{
		flowresult.fgass[599-i] = flowresult.fgass[598-i];
	}
	flowresult.fgass[0]=flowresult.fgas;
	//printf("%f === %ld\n",flowresult.fgas,m_time.timestamp);

	if(flowc1 == 0)
	{
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[flowc1]*1000,flowresult.fgass[flowc1]);
		strcat(spi_en,data_time1);
		flowc1++;
	}
	else if (flowc1 < 600)
	{
		for(i=0;i<flowc1;i++)
		{
			sprintf(data_time1,"[%.0f,%f],",(double)m_time.timestamps[i]*1000,flowresult.fgass[i]);
			strcat(spi_en,data_time1);
		}
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[flowc1]*1000,flowresult.fgass[flowc1]);
		strcat(spi_en,data_time1);
		flowc1++;
	}
	else
	{
		for(i=0; i<599;i++)
		{
			sprintf(data_time1,"[%.0f,%f],",(double)m_time.timestamps[i]*1000,flowresult.fgass[i]);
			strcat(spi_en,data_time1);
		}
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[599]*1000,flowresult.fgass[599]);
		strcat(spi_en,data_time1);
	}



	sprintf(p_log1,"%s%s%s","{\"label\":\"gas-flow\",\"data\":[",spi_en,"]}");
//	fwrite(p_log1,strlen(p_log1),1,fd_en );
	fputs(p_log1,F_fd);

	fclose(F_fd);

	return 0;
}
int flow2_data_w(void)
{
	int i = 0;
	static int flowc2 =0;
	char data_time1[132000] = {};
	char spi_en[132000] = {};
	char p_log1[132000] = {};//add data
	F_fd = fopen("/usr/local/apache/htdocs/yangpai/tables/flow2.json", "w+b");//{"label":"脉冲曲线 x:t/ns y:v/mv","data":[[1196463600000,70],[1196463660000,100],[1196463720000,97]]}
	for(i=0;i<599;i++)
	{
		flowresult.fwaters[599-i] = flowresult.fwaters[598-i];
	}
	flowresult.fwaters[0]=flowresult.fwater;
	//printf("%f === %ld\n",flowresult.fgas,m_time.timestamp);

	if(flowc2 == 0)
	{
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[flowc2]*1000,flowresult.fwaters[flowc2]);
		strcat(spi_en,data_time1);
		flowc2++;
	}
	else if (flowc2 < 600)
	{
		for(i=0;i<flowc2;i++)
		{
			sprintf(data_time1,"[%.0f,%f],",(double)m_time.timestamps[i]*1000,flowresult.fwaters[i]);
			strcat(spi_en,data_time1);
		}
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[flowc2]*1000,flowresult.fwaters[flowc2]);
		strcat(spi_en,data_time1);
		flowc2++;
	}
	else
	{
		for(i=0; i<599;i++)
		{
			sprintf(data_time1,"[%.0f,%f],",(double)m_time.timestamps[i]*1000,flowresult.fwaters[i]);
			strcat(spi_en,data_time1);
		}
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[599]*1000,flowresult.fwaters[599]);
		strcat(spi_en,data_time1);
	}



	sprintf(p_log1,"%s%s%s","{\"label\":\"water-flow\",\"data\":[",spi_en,"]}");
//	fwrite(p_log1,strlen(p_log1),1,fd_en );
	fputs(p_log1,F_fd);

	fclose(F_fd);

	return 0;
}
int flow3_data_w(void)
{
	int i = 0;
	static int flowc3 =0;
	char data_time1[132000] = {};
	char spi_en[132000] = {};
	char p_log1[132000] = {};//add data
	F_fd = fopen("/usr/local/apache/htdocs/yangpai/tables/flow3.json", "w+b");//{"label":"脉冲曲线 x:t/ns y:v/mv","data":[[1196463600000,70],[1196463660000,100],[1196463720000,97]]}
	for(i=0;i<599;i++)
	{
		flowresult.tem1s[599-i] = flowresult.tem1s[598-i];
	}
	flowresult.tem1s[0]=flowresult.OMF;
	//printf("%f === %ld\n",flowresult.foil,m_time.timestamp);

	if(flowc3 == 0)
	{
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[flowc3]*1000,flowresult.tem1s[flowc3]);
		strcat(spi_en,data_time1);
		flowc3++;
	}
	else if (flowc3 < 600)
	{
		for(i=0;i<flowc3;i++)
		{
			sprintf(data_time1,"[%.0f,%f],",(double)m_time.timestamps[i]*1000,flowresult.tem1s[i]);
			strcat(spi_en,data_time1);
		}
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[flowc3]*1000,flowresult.tem1s[flowc3]);
		strcat(spi_en,data_time1);
		flowc3++;
	}
	else
	{
		for(i=0; i<599;i++)
		{
			sprintf(data_time1,"[%.0f,%f],",(double)m_time.timestamps[i]*1000,flowresult.tem1s[i]);
			strcat(spi_en,data_time1);
		}
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[599]*1000,flowresult.tem1s[599]);
		strcat(spi_en,data_time1);
	}



	sprintf(p_log1,"%s%s%s","{\"label\":\"temputer1\",\"data\":[",spi_en,"]}");
//	fwrite(p_log1,strlen(p_log1),1,fd_en );
	fputs(p_log1,F_fd);

	fclose(F_fd);

	return 0;
}
int flow4_data_w(void)
{
	int i = 0;
	static int flowc4 =0;
	char data_time1[132000] = {};
	char spi_en[132000] = {};
	char p_log1[132000] = {};//add data
	F_fd = fopen("/usr/local/apache/htdocs/yangpai/tables/flow4.json", "w+b");//{"label":"脉冲曲线 x:t/ns y:v/mv","data":[[1196463600000,70],[1196463660000,100],[1196463720000,97]]}
	for(i=0;i<599;i++)
	{
		flowresult.tem2s[599-i] = flowresult.tem2s[598-i];
	}
	flowresult.tem2s[0]=flowresult.GMF;
	//printf("%f === %ld\n",flowresult.fgas,m_time.timestamp);

	if(flowc4 == 0)
	{
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[flowc4]*1000,flowresult.tem2s[flowc4]);
		strcat(spi_en,data_time1);
		flowc4++;
	}
	else if (flowc4 < 600)
	{
		for(i=0;i<flowc4;i++)
		{
			sprintf(data_time1,"[%.0f,%f],",(double)m_time.timestamps[i]*1000,flowresult.tem2s[i]);
			strcat(spi_en,data_time1);
		}
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[flowc4]*1000,flowresult.tem2s[flowc4]);
		strcat(spi_en,data_time1);
		flowc4++;
	}
	else
	{
		for(i=0; i<599;i++)
		{
			sprintf(data_time1,"[%.0f,%f],",(double)m_time.timestamps[i]*1000,flowresult.tem2s[i]);
			strcat(spi_en,data_time1);
		}
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[599]*1000,flowresult.tem2s[599]);
		strcat(spi_en,data_time1);
	}



	sprintf(p_log1,"%s%s%s","{\"label\":\"temputer2\",\"data\":[",spi_en,"]}");
//	fwrite(p_log1,strlen(p_log1),1,fd_en );
	fputs(p_log1,F_fd);

	fclose(F_fd);

	return 0;
}
int flow5_data_w(void)
{
	int i = 0;
	static int flowc5 =0;
	char data_time1[132000] = {};
	char spi_en[132000] = {};
	char p_log1[132000] = {};//add data
	F_fd = fopen("/usr/local/apache/htdocs/yangpai/tables/flow5.json", "w+b");//{"label":"脉冲曲线 x:t/ns y:v/mv","data":[[1196463600000,70],[1196463660000,100],[1196463720000,97]]}
	for(i=0;i<599;i++)
	{
		flowresult.pres[599-i] = flowresult.pres[598-i];
	}
	flowresult.pres[0]=flowresult.WMF;
	//printf("%f === %ld\n",flowresult.fgas,m_time.timestamp);

	if(flowc5 == 0)
	{
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[flowc5]*1000,flowresult.pres[flowc5]);
		strcat(spi_en,data_time1);
		flowc5++;
	}
	else if (flowc5 < 600)
	{
		for(i=0;i<flowc5;i++)
		{
			sprintf(data_time1,"[%.0f,%f],",(double)m_time.timestamps[i]*1000,flowresult.pres[i]);
			strcat(spi_en,data_time1);
		}
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[flowc5]*1000,flowresult.pres[flowc5]);
		strcat(spi_en,data_time1);
		flowc5++;
	}
	else
	{
		for(i=0; i<599;i++)
		{
			sprintf(data_time1,"[%.0f,%f],",(double)m_time.timestamps[i]*1000,flowresult.pres[i]);
			strcat(spi_en,data_time1);
		}
		sprintf(data_time1,"[%.0f,%f]",(double)m_time.timestamps[599]*1000,flowresult.pres[599]);
		strcat(spi_en,data_time1);
	}



	sprintf(p_log1,"%s%s%s","{\"label\":\"pressure\",\"data\":[",spi_en,"]}");
//	fwrite(p_log1,strlen(p_log1),1,fd_en );
	fputs(p_log1,F_fd);

	fclose(F_fd);

	return 0;
}
int flow_table_w(void)
{
	char ftable[8192];
	static int flowT =0;
	if(flowT<4)
	{
		flowT++;
		return 0;
	}
	F_fd = fopen("/usr/local/apache/htdocs/yangpai/tables/flow-data.json", "w+b");
	sprintf(ftable,"[{\"z1\":\"%.2ld:%.2ld\",\"z2\":\"%f\",\"z3\":\"%f\",\"z4\":\"%f\",\"z5\":\"%f\",\"z6\":\"%f\","
					 "\"z7\":\"%.2ld:%.2ld\",\"z8\":\"%f\",\"z9\":\"%f\",\"z10\":\"%f\",\"z11\":\"%f\",\"z12\":\"%f\","
					 "\"z13\":\"%.2ld:%.2ld\",\"z14\":\"%f\",\"z15\":\"%f\",\"z16\":\"%f\",\"z17\":\"%f\",\"z18\":\"%f\","
					 "\"z19\":\"%.2ld:%.2ld\",\"z20\":\"%f\",\"z21\":\"%f\",\"z22\":\"%f\",\"z23\":\"%f\",\"z24\":\"%f\"}]",
					(((m_time.timestamps[3]/60)/60)%24),((m_time.timestamps[3]/60)%60),flowresult.fgass[3],flowresult.foils[3],flowresult.fwaters[3],flowresult.pres[3],flowresult.tem2s[3],
					(((m_time.timestamps[2]/60)/60)%24),((m_time.timestamps[2]/60)%60),flowresult.fgass[2],flowresult.foils[2],flowresult.fwaters[2],flowresult.pres[2],flowresult.tem2s[2],
					(((m_time.timestamps[1]/60)/60)%24),((m_time.timestamps[1]/60)%60),flowresult.fgass[1],flowresult.foils[1],flowresult.fwaters[1],flowresult.pres[1],flowresult.tem2s[1],
					(((m_time.timestamps[0]/60)/60)%24),((m_time.timestamps[0]/60)%60),flowresult.fgass[0],flowresult.foils[0],flowresult.fwaters[0],flowresult.pres[0],flowresult.tem2s[0]);
	fputs(ftable,F_fd);
	fclose(F_fd);
	return 0;
}
int arm_P(void)
{
	char afP[8192];
	int j;
	static int arm_P_C=0;
	static double S_winA=0;
	static double S_winB=0;
	static double S_winC=0;
	static double S_winD=0;
	static double S_winE=0;
	double winA=0.0;
	double winB=0.0;
	double winC=0.0;
	double winD=0.0;
	double winE=0.0;
	float P;
	float DP;


	for(j=0;j<200;j++)
	{
		winA=winA+win_C[j][0];
		winB=winB+win_C[j][1];
		winC=winC+win_C[j][2];
		winD=winD+win_C[j][3];
		winE=winE+win_C[j][4];
	}

	if(arm_P_C<60)
	{
		S_winA	=	S_winA+winA;
		S_winB	=	S_winB+winB;
		S_winC	=	S_winC+winC;
		S_winD	=	S_winD+winD;
		S_winE	=	S_winE+winE;
	}
	/*数据线性拟合*/
	S_winB = S_winB*((shared->S_PCH31 - shared->S_F31)*ParaAdjust.Adjust_WinB_a + ParaAdjust.Adjust_WinB_b);
	S_winC = S_winC*((shared->S_PCH81 - shared->S_F81)*ParaAdjust.Adjust_WinC_a + ParaAdjust.Adjust_WinC_b);
	S_winE = S_winE*((shared->S_PCH356 - shared->S_F356)*ParaAdjust.Adjust_WinE_a + ParaAdjust.Adjust_WinE_b);
//	printf("__________________________________________%d\n",arm_P);
	if(arm_P_C==59)
	{
		//数据准备好了可以存储
		//数据整理好后存储
		DP =	cpms.DP_CPM_A * shared->ad1115[0][2] + cpms.DP_CPM_B;
		P =		cpms.P_CPM_A * shared->ad1115[0][1] + cpms.P_CPM_B;
		F_fd = fopen("/usr/local/apache/htdocs/yangpai/tables/ARM-P.json", "w+b");
		sprintf(afP,"[{\"ac\":\"%.0lf\",\"bc\":\"%.0lf\",\"cc\":\"%.0lf\",\"dc\":\"%.0lf\","
					"\"ec\":\"%.0lf\",\"hv\":\"%d\",\"vbb\":\"%d\",\"t1\":\"%.2f C\","
					"\"t2\":\"%.2f C\",\"a_pre\":\"%.2fKPa\",\"d_pre\":\"%.2fPa\","
					"\"t3\":\"%.2f C\",\"PAS\":\"%d\",\"PA\":\"%d\",\"PBS\":\"%d\","
					"\"PB\":\"%d\",\"PCS\":\"%d\",\"PC\":\"%d\",\"PDS\":\"%d\",\"PD\":\"%d\","
					"\"PES\":\"%d\",\"PE\":\"%d\"}]",
					S_winA,S_winB,S_winC,S_winD,S_winE,shared->S_HIV,shared->S_VBB,
					shared->DEV_T1,shared->ad1115[200][0],P,
					DP,shared->DEV_T2,shared->S_PCH31,shared->S_F31,
					shared->S_PCH81,shared->S_F81,0,0,0,0,shared->S_PCH356,shared->S_F356);
		fputs(afP,F_fd);
		fclose(F_fd);
		flowresult.winA=S_winA;
		flowresult.winB=S_winB;
		flowresult.winC=S_winC;
		flowresult.winD=S_winD;
		flowresult.winE=S_winE;
		//printf("B=%d\n C=%d\n",flowresult.winB,flowresult.winC);

		arm_P_C	=	-1;
		S_winA	=	0;
		S_winB	=	0;
		S_winC	=	0;
		S_winD	=	0;
		S_winE	=	0;
	}
	arm_P_C++;
	return 0;
}
void W_logfile(void)
{
	int i;
	static int num_log;
	char execsql[2048];
	char time_5ms[20];
	static int mysql_year_log;
	static int mysql_month_log;
	static int mysql_date_log;
	float P;
	float DP;
	sprintf(time_5ms,"%.2d:%.2d",m_time.hour,m_time.minute);
	//printf("%s\n",time_5ms);
	//创建文件夹     判断年月ri
	if((mysql_year_log != m_time.year)||(mysql_month_log != m_time.month))
	{
		sprintf(execsql,"create database %4d_%.2d_log",m_time.year,m_time.month);
	//		printf("%s \n", execsql);
		mysql_real_query(&mysql_conn, execsql, strlen(execsql));//执行查询语句：创建数据库
		i = mysql_affected_rows(&mysql_conn);
		if (i<=0){
			printf("Can not create log database\n");
		}
	}

	//创建数据表格   判断日期
	if((mysql_year_log != m_time.year)||(mysql_month_log != m_time.month)||(mysql_date_log != m_time.date))
	{
		num_log=0;
		sprintf(execsql,"%4d_%.2d_log",m_time.year,m_time.month);
	//	printf("%s \n", execsql);
		mysql_select_db(&mysql_conn, execsql);//设置要操作的数据库
		sprintf(execsql, "create table %4d_%.2d_%.2d(num char(30),time char(20),flow_oil float(32),flow_gas float(32),"
				"flow_water float(32),OMF float(32),GMF float(32),WMF float(32),WINA int(16),WINB int(16),WINC int(16),"
				"WIND int(16),WINE int(16),Temperture float(32),Pressure float(32),DPressure float(32),PEAK_A int(16),"
				"PEAK_B int(16),PEAK_E int(16),Tem_IN float(32),Tem2 float(32),HV int(16),BU int(16),BD int(16),"
				"CU int(16),CD int(16),EU int(16),ED int(16),OVF1 float(32),GVF1 float(32),Qv float(32),"
				"OMF1 float(32),GMF1 float(32),Qm float(32))",
				m_time.year,m_time.month,m_time.date);//以年月日为名字
	//	printf("%s \n", execsql);
		mysql_real_query(&mysql_conn, execsql, strlen(execsql));
	}
	else
	{
		sprintf(execsql,"%4d_%.2d_log",mysql_year_log,mysql_month_log);
		mysql_select_db(&mysql_conn, execsql);//设置要操作的数据库
	}
	//数据填充
	num_log++;
	/*sprintf(execsql,"insert into %4d_%.2d_%.2d values('%d','%s',%f,%f,%f,%f,%f,%f,%d,%d,%d,%d,%d,%f,%f,%f,%d,%f,%f)",m_time.year,
			m_time.month,m_time.date,num_log,time_5ms,1.0,1.0,1.0,
			0.5,0.5,0.5,flowresult.winA,flowresult.winB,flowresult.winC,flowresult.winD,
			flowresult.winE,shared->ad1115[0][0],shared->ad1115[0][1],shared->ad1115[0][2],shared->S_F356,shared->DEV_T1,shared->DEV_T2);*/

	DP =	cpms.DP_CPM_A * shared->ad1115[0][2] + cpms.DP_CPM_B;
	P =		cpms.P_CPM_A * shared->ad1115[0][1] + cpms.P_CPM_B;
	sprintf(execsql,"insert into %4d_%.2d_%.2d values('%d','%s',%f,%f,%f,%f,%f,%f,%d,%d,%d,%d,%d,%f,%f,%f,%d,%d,%d,%f,%f,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f)",m_time.year,
				m_time.month,m_time.date,num_log,time_5ms,flowresult.foil,flowresult.fgas,flowresult.fwater,
				flowresult.OMF,flowresult.GMF,flowresult.WMF,flowresult.winA,flowresult.winB,flowresult.winC,flowresult.winD,
				flowresult.winE,shared->ad1115[0][0],P,DP,shared->S_F31,shared->S_F81,
				shared->S_F356,shared->DEV_T1,shared->DEV_T2,shared->S_HIV,spisets.A1,spisets.A2,spisets.B1,spisets.B2,spisets.E1,
				spisets.E2,cpms.OVF_CPM,cpms.GVF_CPM,cpms.Qv_CPM,cpms.OMF_CPM,cpms.GMF_CPM,cpms.Qm_CPM);
	mysql_query(&mysql_conn, execsql);

	//	sprintf(execsql,"insert into %4d_%.2d_%.2d values('%d', %f)",m_time.year,m_time.month,m_time.date,num , 1.1);
	//	printf("%s \n", execsql);

		mysql_year_log 	= m_time.year;
		mysql_month_log	=m_time.month;
		mysql_date_log	=m_time.date;

}
void W_FlowData(void)
{
	char execsql[1024];
	mysql_select_db(&mysql_conn, "FLOW_Para");

	sprintf(execsql," update Flow_Result set Qmg='%f' where id=1",shared->Qmg_CPM);
	mysql_query(&mysql_conn, execsql);

	sprintf(execsql," update Flow_Result set Qml='%f' where id=1",shared->Qml_CPM);
	mysql_query(&mysql_conn, execsql);
}




