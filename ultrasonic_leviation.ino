#define PWMA 5        
#define PWMB 6

volatile uint8_t portD3_D4 = 8; // единица на D3 и ноль на D4

int fstart = 100;
int fstop = 255;
float heightChangeFreq = 13; //чем больше значение, тем быстрее меняется высота
int heightChangeRate = floor(40000 / heightChangeFreq);
volatile int fstep = 5;
volatile int forcea = fstart + floor((fstop-fstart)/2);
volatile int forceb = fstart + floor((fstop-fstart)/2);     
volatile int count = 0;

void setup() {
  Serial.begin(115200);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);

  forcea -= int(forcea % fstep);
  forceb -= int(forceb % fstep);
  
//  Serial.print("heightChangeRate = ");
//  Serial.println(heightChangeRate);
  Serial.print("forcea = ");
  Serial.println(forcea);
  Serial.print("forceb = ");
  Serial.println(forceb);
//  Serial.print("count = ");
//  Serial.println(count);
  
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
  
  if(count > heightChangeRate){
    count = 0;

    analogWrite(PWMA, forcea);
    analogWrite(PWMB, forceb);
  
    forcea = forcea + fstep;
    forceb = forceb - fstep;
  
    if (forcea <= fstart || forcea >= fstop || forceb <= fstart || forceb >= fstop) {
      fstep = -fstep;
    }   
  } else {
    count = count + 1;
  }
}
