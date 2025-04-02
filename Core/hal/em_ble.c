#include "em_ble.h"
#include "em_printer.h"
#include "em_config.h"

bool bleConnected = true;
uint32_t packcount = 0;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;

// ����һ���ַ���
char *ble_in_at_mode = "+++";
char *ble_set_status = "AT+STATUS=0\r\n"; //�ر��豸״̬��ʾ
char *ble_query_status = "AT+STATUS?\r\n"; //�ر��豸״̬��ʾ
char *ble_query_name = "AT+NAME?\r\n"; //AT+NAME=RF-CRAZY\r\nOK
char *ble_set_name = "AT+NAME=Mini-Printer\r\n"; //OK ��д��
//char *ble_set_name = "AT+NAME=RF-CRAZY\r\n"; //OK ��д��
char *ble_out_at_mode = "AT+EXIT\r\n";

typedef enum{
        BLE_INIT_START = 0,
        BLE_IN_AT_MODE,
        BLE_IN_AT_MODE_SUCCESS,
        BLE_CLOSE_STATUS,
        BLE_CLOSE_STATUS_SUCCESS,
				BLE_QUERY_STATUS,
				BLE_QUERY_STATUS0_SUCCESS,
        BLE_QUERY_NAME,
        BLE_NEED_SET_NAME,
        BLE_NONEED_SET_NAME,
        BLE_SET_NAME,
        BLE_SET_NAME_SUCCESS,
        BLE_OUT_AT_MODE,
        BLE_INIT_FINISH,
				BLE_RESET,
}e_ble_init_step;

e_ble_init_step g_ble_init_step = BLE_INIT_START;
bool need_reboot_ble = false;


void clean_blepack_count(){
    packcount = 0;
}


uint32_t get_blepack_count(){
    return packcount;
}

bool get_ble_connect(){
    return bleConnected;
}

void ble_report(){
    if (get_ble_connect()){
        device_state_t *pdevice = get_device_state();
        uint8_t status[4];
        status[0] = pdevice->battery;
        status[1] = pdevice->temperature;
        status[2] = pdevice->paper_state;
        status[3] = pdevice->printer_state;
			  HAL_UART_Transmit(&huart2,(uint8_t*)&status,sizeof(status),0xffff);
    }
}

int cmd_index = 0;
uint8_t cmd_buffer[100];
bool need_clean_ble_status = false;

void uart_cmd_handle(uint8_t data){
        cmd_buffer[cmd_index++] = data;
        char *ptr_char = (char*)cmd_buffer;
        if(g_ble_init_step == BLE_INIT_FINISH){
												if(strstr(ptr_char, "CONNECTED") != NULL){
													need_clean_ble_status = true;
													run_led(LED_CONNECT);
												}
												if(strstr(ptr_char, "DISCONNECTED") != NULL){
													need_clean_ble_status = true;
													run_led(LED_DISCONNECT);
												}
												if(strstr(ptr_char, "DEVICE ERROR") != NULL){
													need_clean_ble_status = true;
													run_led(LED_CONNECT);
												}
                        if(cmd_index == 5){
                                 if(cmd_buffer[0] == 0xA5 && cmd_buffer[1] == 0xA5 && cmd_buffer[2] == 0xA5 && cmd_buffer[3] == 0xA5){
                                                 if(cmd_buffer[4] == 1){
                                                                 set_heat_density(30);
                                                 }else if(cmd_buffer[4] == 2){
                                                                 set_heat_density(60); 
                                                 }else{
                                                                 set_heat_density(100);
                                                 }
                                                 cmd_index = 0;
																								 memset(cmd_buffer,0,sizeof(cmd_buffer));
                                                 return;
                                        }
                                 if(cmd_buffer[0] == 0xA6 && cmd_buffer[1] == 0xA6 && cmd_buffer[2] == 0xA6 && cmd_buffer[3] == 0xA6){
                                                set_read_ble_finish(true);
                                                LOG_INFO("---->read finish 1 = %d\n",packcount);
                                                cmd_index = 0;
																								memset(cmd_buffer,0,sizeof(cmd_buffer));
                                                return;
                                 }
                        }
												if(cmd_index >= 5){
													if(cmd_buffer[cmd_index-1] == 0x01){
														if(cmd_buffer[cmd_index-2] == 0xA6 && cmd_buffer[cmd_index-3] == 0xA6 && cmd_buffer[cmd_index-4] == 0xA6 && cmd_buffer[cmd_index-5] == 0xA6){
															LOG_INFO("---->read finish 2 = %d\n",packcount);
															cmd_index = 0;
															memset(cmd_buffer,0,sizeof(cmd_buffer));
															set_read_ble_finish(true);
															return;
														}
													}
												}
													
                        if(cmd_index >= 48){
                                packcount++;
                                write_to_printbuffer(cmd_buffer,cmd_index);
                                cmd_index = 0;
                                memset(cmd_buffer,0,sizeof(cmd_buffer));
																//LOG_INFO("packcount = %d\n",packcount);
													
                        }
        }else{
								
                if(strstr(ptr_char, "OK\r\n") != NULL){
									if(g_ble_init_step == BLE_IN_AT_MODE){
													g_ble_init_step = BLE_IN_AT_MODE_SUCCESS;
									}else if(g_ble_init_step == BLE_CLOSE_STATUS){
													g_ble_init_step = BLE_CLOSE_STATUS_SUCCESS;
									}else if(g_ble_init_step == BLE_QUERY_NAME){
													if(strstr(ptr_char,"RF-CRAZY") != NULL){
													//if(strstr(ptr_char,"Mini-Printer") != NULL){
																	g_ble_init_step = BLE_NEED_SET_NAME;
													}else{
																	g_ble_init_step = BLE_NONEED_SET_NAME;
													}
									}else if(g_ble_init_step == BLE_SET_NAME){
													g_ble_init_step = BLE_SET_NAME_SUCCESS;
									}else if(g_ble_init_step == BLE_OUT_AT_MODE){
													g_ble_init_step = BLE_INIT_FINISH;
									}else if(g_ble_init_step == BLE_RESET){
													g_ble_init_step = BLE_INIT_START;
									}else if(g_ble_init_step == BLE_QUERY_STATUS){
													if(strstr(ptr_char,"AT+STATUS=0") != NULL){
																	g_ble_init_step = BLE_QUERY_STATUS0_SUCCESS;
													}else{
																	g_ble_init_step = BLE_CLOSE_STATUS;
													}
									}
									cmd_index = 0;
									memset(cmd_buffer,0,sizeof(cmd_buffer));
									return;
                }
								if(strstr(ptr_char, "ERROR\r\n") != NULL){
									g_ble_init_step = BLE_RESET;
								}
								if(cmd_index >= sizeof(cmd_buffer)){
										cmd_index = 0;
								}
        }
}



int retry_count = 0;

void init_ble()
{
                while(1){
												retry_count ++;
                        vTaskDelay(50);
                        if(g_ble_init_step == BLE_INIT_START || g_ble_init_step == BLE_IN_AT_MODE){
																LOG_INFO("BLE:������ATģʽ\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_in_at_mode, strlen(ble_in_at_mode), 0xffff);
                                g_ble_init_step = BLE_IN_AT_MODE;
                        }else if(g_ble_init_step == BLE_IN_AT_MODE_SUCCESS || g_ble_init_step == BLE_CLOSE_STATUS){
																LOG_INFO("BLE:������statusΪ0 �ر�״̬��ʾ\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_set_status, strlen(ble_set_status), 0xffff);
                                g_ble_init_step = BLE_CLOSE_STATUS;
                        }else if(g_ble_init_step == BLE_CLOSE_STATUS_SUCCESS || g_ble_init_step == BLE_QUERY_STATUS){
																LOG_INFO("BLE:����ѯ״̬�Ƿ�Ϊ0\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_query_status, strlen(ble_query_status), 0xffff);
                                g_ble_init_step = BLE_QUERY_STATUS;
                        }else if(g_ble_init_step == BLE_QUERY_STATUS0_SUCCESS || g_ble_init_step == BLE_QUERY_NAME){
																LOG_INFO("BLE:����ѯ�豸����\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_query_name, strlen(ble_query_name), 0xffff);
                                g_ble_init_step = BLE_QUERY_NAME;
                        }
												else if(g_ble_init_step == BLE_NEED_SET_NAME || g_ble_init_step == BLE_SET_NAME){
																LOG_INFO("BLE:�������豸����\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_set_name, strlen(ble_set_name), 0xffff);
                                g_ble_init_step = BLE_SET_NAME;
                                need_reboot_ble = true;
                        }else if(g_ble_init_step == BLE_SET_NAME_SUCCESS || g_ble_init_step == BLE_NONEED_SET_NAME || g_ble_init_step == BLE_OUT_AT_MODE){
																LOG_INFO("BLE:���˳�ATģʽ\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_out_at_mode, strlen(ble_out_at_mode), 0xffff);
                                g_ble_init_step = BLE_OUT_AT_MODE;
                        }else if(g_ble_init_step == BLE_INIT_FINISH){
                                break;
                        }else if(g_ble_init_step == BLE_RESET){
																LOG_INFO("BLE:BLE RESET �˳�ATģʽ\n");
																HAL_UART_Transmit(&huart2, (uint8_t*)ble_out_at_mode, strlen(ble_out_at_mode), 0xffff);
                                
												}
												LOG_INFO("g_ble_init_step = %d\n",g_ble_init_step);
												run_led(LED_BLE_INIT);
                }
                if(need_reboot_ble){
                        LOG_INFO("�������-�������豸ʹ��\n");
                }else{
                        LOG_INFO("�������-��������ʹ��\n");
												
                }
								vTaskDelay(1000);
								cmd_index = 0;
								memset(cmd_buffer,0,sizeof(cmd_buffer));
}

//�ⲽ��������Ϊ���ҵ�����ģ�飬����statusֻ����busy��connect timeout��device start��wake up
//������Ҫ��CONNECTED DISCONNECTED DEVICE ERROR��Щҵ���޹��������
void ble_status_data_clean(){
	if(need_clean_ble_status){
		vTaskDelay(200);
		LOG_INFO("clean --->%s\n",cmd_buffer);
		cmd_index = 0;
		memset(cmd_buffer,0,sizeof(cmd_buffer));
		need_clean_ble_status = false;
	}
	
}
