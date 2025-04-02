#include "em_config.h"
#include "main.h"
#include "math.h"

extern ADC_HandleTypeDef hadc1;

#define ADC_READ_TIME 10

uint32_t ADC_Value[2];

void adc_init() {
//  pinMode(PIN_ADC_EN, OUTPUT);
//  analogReadResolution(BATTERY_ADC_BIT);
//  digitalWrite(PIN_ADC_EN, LOW);
	HAL_ADC_Start(&hadc1);	
	HAL_ADC_PollForConversion(&hadc1,1000);	
}


uint32_t adc_alg_handle(uint32_t *adc, int size) {
    uint32_t sum = 0;
    uint32_t min_val = adc[0];
    uint32_t max_val = adc[0];

    for (int i = 0; i < size; i++) {
        if (adc[i] < min_val) {
            min_val = adc[i];
        }
        else if (adc[i] > max_val) {
            max_val = adc[i];
        }
        sum += adc[i];
    }
    sum = sum - (min_val + max_val);
    uint32_t avg_val = sum / (size - 2);
    return avg_val;
}


/**
 * @brief ��ȡADC���ŵ�ѹֵ���ɸ�����Ҫ�����˲��㷨
 * 
 * @return int 
 */
int get_adc_volts()
{
  uint32_t data = 0;
  uint32_t adc1[ADC_READ_TIME];
  uint32_t adc2[ADC_READ_TIME];

  for (int sample_ptr = 0; sample_ptr < ADC_READ_TIME; sample_ptr++)
  {
		HAL_ADC_Start(&hadc1);
		if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
		{
			 adc1[sample_ptr] = HAL_ADC_GetValue(&hadc1);
		}
		HAL_ADC_Start(&hadc1);
		if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
		{
			 adc2[sample_ptr] = HAL_ADC_GetValue(&hadc1);
		}
		HAL_ADC_Stop(&hadc1);
  }
  ADC_Value[0] = adc_alg_handle(adc1, ADC_READ_TIME);
  ADC_Value[1] = adc_alg_handle(adc2, ADC_READ_TIME);
	
  LOG_INFO(" ADC channel0 end value = ->%1.3fV \r\n", ADC_Value[0] * 3.3f / 4096);
  LOG_INFO(" ADC channel1 end value = ->%1.3fV \r\n", ADC_Value[1] * 3.3f / 4096);
  data = ADC_Value[0] * 3.3f / 4096;

  HAL_ADC_Stop(&hadc1);
  return data;
}

/**
 ��ʽ����
NTC ���������¶ȼ��㹫ʽ��Rt = Rp *EXP(B*(1/T1-1/T2))
���Եõ��¶�T1�����Rt�Ĺ�ϵT1=1/��log��Rt/Rp��/B+1/T2��

T1��T2ָ����K�ȣ����������¶�,K��=273.15(�����¶�)+���϶ȡ�
T2=(273.15+25)
Rt ������������T1�¶��µ���ֵ��
Rp ������������T2�����µı����ֵ��100K����������25���ֵΪ100K����R=100K����T2=(273.15+25)
Bֵ�������������Ҫ����  B25/50= 3950K ��1%
���ն�Ӧ�������¶�t=T1-273.15,ͬʱ+0.5����������

�������
ADC������8λ ��������R1=10K R2=�������� ��ѹ3.3V
��֪
Rt = R*(3.3-VR)/VR
VR = 3.3*ADC_Value/1024
�ó�
ADC_Value = VR*4096/3.3 =3.3*R/(Rt+R)*4096/3.3 = R/(Rt+R)*1024
ADC_Value=3.3/(C5+10)*10/3.3*1023
�����Ҫ�õ�С���������¶ȣ���ȷ�ķ�����ʹ�ù�ʽRT��R0*exp(B (1/T-1/T0))��excel�м���õ�����ֵΪ0.1����¶ȱ�
Rt=100*exp(3950*(1/(273.15+T1)-1/(273.15+25)))
Rt=100*EXP(3990*(1/(273.15+T1)-1/(273.15+25)))
*/

/**
 * @brief ��ֵת���¶�
 *
 * @param Rt ����������ֵ
 * @return float �������϶��¶�
 */
float em_temp_calculate(float Rt)
{
    float Rp = 30000; // 30k
    float T2 = 273.15 + 25;
    float Bx = 3950; // Bֵ
    float Ka = 273.15;
    float temp = 0.0f;

    temp = 1 / (log(Rt / Rp) / Bx + 1 / T2) - Ka + 0.5;
    return temp;
}

/**
 * @brief Get the adc temperatrue object
 * 
 * @return float 
 */
float get_adc_temperatrue(){
  float temp = 0.0f;
  float Rt=0;
  float vol=0;
  //ADCת��Ϊ��ѹ vol=AD/4096*VCC
  vol=(float)ADC_Value[1]*3.3f/4096;
	LOG_INFO("ADC temperatrue analog value = %f\n",vol);
  //��ѹת��Ϊ��ֵ ԭ��ͼΪ10k 1%����  vol/VCC=Rt/(R+Rt)  vol/3.3=Rt/(10000+Rt)
  Rt=(vol*10000)/(3.3-vol);
	LOG_INFO("ADC temperatrue Rt = %f\n",Rt);
  temp = em_temp_calculate(Rt);
  return temp;
}

void em_adc_test(){
  float Rt=0;
  float vol=3.0f;
  Rt=(vol*10000)/(3.3-vol);
  LOG_INFO("Rt = %f\n",Rt);
  
  Rt = 60000; //60k 10��C
  float temp = 0.0f;
  temp = em_temp_calculate(Rt);
  LOG_INFO("temp = %f\n",temp);

  if(temp >= 1e-7){
      LOG_INFO("��\n");
  }else{
      LOG_INFO("��\n");
  }
}
