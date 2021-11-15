// Программа для ограничения времени работы ружья 10 секундами, затем 30 секунд остывания
/* 1) вычисляем, что при частоте 9,6МГц, и предделителе 1024 таймер будет увеличиваться на 1 ровно 9 375 в секунду (плюс минус пять процентов, т.к. скорость внутреннего генератора 9,6 МГц задана плюс-минус).
2) Подбираем число не больше 256, на которое 9375 делится нацело. Например, 125. То есть если прерывание будет вызываться каждый раз, когда таймер изменит значение 125 раз, то это будет 75 раз в секунду
3) заводим соответствующую переменную и настраиваем прерывание по output compare A
*/

#include <tiny13.h>
#define TIMER_TICKS_IN_ONE_SECOND 75
#define fire_time 10
#define cold_time 30

unsigned char count_to_second = 0;
unsigned int timer = 0; //переменная счётчика времени

interrupt [TIM0_COMPA] void timer0_compa_isr(void)
	{
	count_to_second++;
	if (count_to_second >= TIMER_TICKS_IN_ONE_SECOND) { // Отсчитываем прерывание 75 раз
		count_to_second = 0; // Сбрасываем счётчик
		timer++;  // пополняем счётчик секунд
		}
	}

#define ADC_VREF_TYPE 0x00

// Чтение результата АЦП
unsigned int read_adc(unsigned char adc_input){
	ADMUX=adc_input | (ADC_VREF_TYPE & 0xff);
	delay_us(10); // Delay needed for the stabilization of the ADC input voltage
	ADCSRA|=0x40; // Start the AD conversion
	while ((ADCSRA & 0x10)==0); // Wait for the AD conversion to complete
	ADCSRA|=0x10;
	return ADCW;
}

void main(){
/* 4) Настраиваем таймер для работы в режиме CTC. В этом режиме он считает до тех пор пока его значение не достигнет OCR0A, после этого происходит прерывание COMPA, таймер обнуляется и продолжает считать с нуля. */

	OCR0A = 124; // прерывание должно вызываться каждый 125 отсчёт таймера, поскольку таймер считает от нуля, а прерывание вызывается ПОСЛЕ того как он досчитает до OCR0A, то сюда нужно засунуть значение на единичку меньше расчётного
	TCCR0A = (1 << WGM01); // 010 в битах WGM02:WGM01:WGM00 задают режим CTC
	TCCR0B = (1 << CS02) | (1 << CS00); // 101 в битах CS02:CS01:CS00 задают предделитель 1024
	TIMSK0 = (1 << OCIE0A); // Разрешаем прерывание по переполнению таймера

	#asm("sei") // Включаем механизм прерываний

while(1) { //тут основная программа
	if (timer==5) {PORTB.3=1}//если 5 сек. "зажигаем" ПОРТ В.03
	if (timer==20) {PORTB.2=1} //если 20 сек. "зажигаем" ПОРТ В.02
	if (fire_flag == 1){
		if (timer < fire_time){ //считаем 10 секунд
			timer++;
			relay_pin=1;
			beep_pin=1;
				}
		else {
			fire_flag=0; //отстрелялся, запрещаем работу
			timer=0;
			beep_pin=0;
				}
		}
	else {
		if (timer < cold_time){ ///считаем 30 секунд
			timer++;
			relay_pin=0;
			}
		else {
			fire_flag=1; //разрешаем работу снова
			timer=0;
			for(int i=0; i++; i<100) {beep_pin=1}; //даём сигнал о готовности к стрельбе
			beep_pin=0;	
				}
		}
	} 
}