#define AIN2 3
#define BIN1 4
volatile uint8_t portD3_D4 = 8; // единица на D3 и ноль на D4
void setup() {
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);

  // Инициализируем Timer1
  TCNT1 = 0;
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 199; // Установить регистр сравнения 16 МГц / 80 кГц = 200
  TCCR1B = (1 << WGM12)|(1 << CS10); //Устанавливаем режим CTC,  без делителя
  TIMSK1 |= (1 << OCIE1A); // Включить прерывания таймера
}

void loop() {
  

}
ISR (TIMER1_COMPA_vect) // Обработчик прерывания по таймеру
{
  PORTD = portD3_D4; // Отправляем значения в порт
  portD3_D4 = 255-portD3_D4;// Инвертируем значения для следующей отправки в порт
}
