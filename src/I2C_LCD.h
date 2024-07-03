volatile uint16_t *TIMER1_OCRA = (uint16_t*)0x88;
volatile uint16_t *TIMER1_TCNT = (uint16_t*)0x84;
volatile uint8_t *TIMER1_TCCRA = (uint8_t*)0x80;
volatile uint8_t *TIMER1_TCCRB = (uint8_t*)0x81;
volatile uint8_t *TIMER1_TIMSK = (uint8_t*)0x6F;
volatile uint8_t *DC= (uint8_t*)0x30, *OUTC = (uint8_t*)0x31, *INC = (uint8_t*)0x2F;
volatile char m;
bool backlight=0;
void delayMicro(uint32_t);
void delayMilli(uint32_t);
bool readSDA();
bool readSCL();
void clockstretch();
void clock();
void setSDAlow();
void setSDAhigh();
void setSCLhigh();
void setSCLlow();
void start();
void stop();
void writebit(bool);
bool writebyte(uint8_t);
bool beginTransmission(uint8_t, bool);
void i2cinit();
void lcd_data(uint8_t data);
void lcd_command(uint8_t command);

void lcd_clear(){ // Clears Display
  lcd_command(1);
}
void lcd_returnHome(){  //Returns cursor to address 0
  lcd_command(2);
}
void lcd_entryModeSet(bool ID, bool S){  //ID=1 increment ID=0 decrement (cursor), S=1 with display shift S=0 no display shift 
  lcd_command(4|((ID<<1)+ S));
}
void lcd_displayControl(bool D, bool C, bool B){  //D=1 Display On, C=1 Cursor on, B=1 cursor position blinking on
  lcd_command(8|((D<<2)+((C<<1)+B)));
}
void lcd_cursor_display_shift(bool SC, bool RL){  //SC=1 Display shift SC=0 cursor shift, RL=1 right RL=0 left
  lcd_command(16|((SC<<3)+ (RL<<2)));
}
void lcd_init(uint8_t address, bool N, bool F){ //N=1 2line N=0 1 line, F=1 5x10 font F=0 5x8 font
  i2cinit();
  beginTransmission(address,0);// address, read=1 or write=0
  writebyte(0b00101100);      //Function set from 8bit to 4bit
  writebyte(0b00101000);

  lcd_command(48&((N<<3)+(F<<2)));
}
void lcd_close_i2c(){ //Close the i2c connection using i2c stop bit
  stop();
}
void lcd_print(char *s){
  for(int i=0;s[i]!=0;i++) lcd_data(s[i]);
}
void lcd_backlight(bool state){
  backlight = state;
  writebyte(backlight<<3);

}

void delayMicro(uint32_t micro){
  *TIMER1_TIMSK = 2;// Interrupt enable for OCRA
  *TIMER1_TCCRA = 0;
  *TIMER1_TCCRB = 0;
  *TIMER1_OCRA =16;
  *TIMER1_TCCRB |= 1 << 3;

  for(;micro>0;micro--){
    m=0;
    *TIMER1_TCNT = 0;
    *TIMER1_TCCRB &= ~(110);*TIMER1_TCCRB |= 1;  //set no prescaling
      while(1){
        if(m!=0) break;
      }
  }
  *TIMER1_TCCRB &= ~(111); // stop clock
}
void delayMilli(uint32_t milli){
  *TIMER1_TIMSK = 2;// Interrupt enable for OCRA
  *TIMER1_TCCRA = 0;
  *TIMER1_TCCRB = 0;
  *TIMER1_OCRA =16000;
  *TIMER1_TCCRB |= 1 << 3;

  for(;milli>0;milli--){
    m=0;
    *TIMER1_TCNT = 0;
    *TIMER1_TCCRB &= ~(110);*TIMER1_TCCRB |= 1;  //set no prescaling
      while(1){
        if(m!=0) break;
      }
  }
  *TIMER1_TCCRB &= ~(111); // stop clock
}

ISR(TIMER1_COMPA_vect){
  m=1;
}

//void delayMicro(uint32_t micro){delayMicroseconds(micro);}
bool readSDA(){
  if((*INC & 0b00000010)>0) return 1;
  else return 0;
}
bool readSCL(){
  if((*INC & 0b00000100)>0) return 1;
  else return 0;
}
void clockstretch(){
  while(1){
    if(readSCL()==1) break;
  }
}
void clock(){
  delayMicro(5);
  setSCLhigh();
  clockstretch();
  delayMicro(10);
  setSCLlow();
}

void setSDAlow(){
  *DC |= 0x02;
  *OUTC &= 0xFD;
}
void setSCLlow(){
  *DC |= 0x04;
  *OUTC &= 0xFB;
}
void setSDAhigh(){
*DC &= 0xFD;
}
void setSCLhigh(){
  *DC &= 0xFB;
}
void start(){
  setSDAhigh();
  setSCLhigh();
  delayMicro(10);
  setSDAlow();
  delayMicro(10);
}
void stop(){
  setSDAlow();
  setSCLhigh();
  delayMicro(10);
  setSDAhigh();
  delayMicro(10);
}
void writebit(bool b){
  setSCLlow();
  if(b==0) setSDAlow();
  if(b==1) setSDAhigh();
  delayMicro(10);
  clock();

}
bool writebyte(uint8_t value){
  bool e;
  for(int i=7;i>=0;i--){
    writebit(value & (1<<i));
  }
  setSDAhigh();
  if(readSDA()==0) e = 0;
  else e=1;
  clock();
  return e;
}
bool beginTransmission(uint8_t address, bool rw){
  start();
  return writebyte(address<<1 + rw);
}
void i2cinit(){
  setSDAhigh();
  setSCLhigh();
}
void lcd_command(uint8_t command){
  uint8_t temp = backlight<<3 | 0b111;
  writebyte((command|temp)&(~0b11));
  writebyte((command|temp)&(~0b111));
  writebyte(((command<<4 )|temp) &(~0b11));
  writebyte(((command<<4 )|temp) &(~0b111));
}
void lcd_data(uint8_t data){
  uint8_t temp = backlight<<3 | 0b111;
  writebyte((data|temp)&(~0b10));
  writebyte((data|temp)&(~0b110));
  writebyte(((data<<4 )|temp) &(~0b10));
  writebyte(((data<<4 )|temp) &(~0b110));
}
