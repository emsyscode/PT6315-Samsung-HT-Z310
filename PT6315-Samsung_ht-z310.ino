//Note: This is a not clean code, this is only a pratical test
//where we don't want use any library or dependence of other's code blocks!!!

//The idea is to make the code so basic that you can let it change and see what happens!

/* Port controls  (Platform dependent) */
//#define CS_LOW()  PORTB &= ~1     /* CS=low */
//#define CS_HIGH() PORTB |= 1      /* CS=high */
//#define LCD_Port PORTB

#define VFD_in 5// If 0 write LCD, if 1 read of LCD
#define VFD_clk 6 // if 0 is a command, if 1 is a data0
#define VFD_stb 7 // Must be pulsed to LCD fetch data of bus

#define delay_tcsh _delay_us(16)
#define AdjustPins    PORTC // before is C, but I'm use port C to VFC Controle signals

#define BUTTON_PIN 2 //Att check wich pins accept interrupts... Uno is 2 & 3
volatile byte buttonReleased = false;

unsigned char char0, char1, char2;
bool var = false;

unsigned char animChars[54] = {
  0B00000000, 0B00000000, 0B00000010, 
  0B00000000, 0B00000000, 0B00000001,
  0B00000000, 0B10000000, 0B00000000,
  0B00000000, 0B01000000, 0B00000000,
  0B00000000, 0B00100000, 0B00000000,
  0B00000000, 0B00010000, 0B00000000,
  0B00000000, 0B00001000, 0B00000000,
  0B00000000, 0B00000100, 0B00000000,
  0B00000000, 0B00000010, 0B00000000,
  0B00000000, 0B00000001, 0B00000000,
  0B10000000, 0B00000000, 0B00000000,
  0B01000000, 0B00000000, 0B00000000,
  0B00100000, 0B00000000, 0B00000000,
  0B00010000, 0B00000000, 0B00000000,
  0B00001000, 0B00000000, 0B00000000,
  0B00000100, 0B00000000, 0B00000000,
  0B00000010, 0B00000000, 0B00000000,
  0B00000001, 0B00000000, 0B00000000
};

void AD16315_send_cmd(unsigned char a) {
  // This send use the strob... good for send commands, not burst of data.
  unsigned char chr;
  chr = a;

  PORTD = (0 << VFD_clk);
  delayMicroseconds(1);//Delay
  PORTD = (0 << VFD_in);
  delayMicroseconds(1);//Delay
  PORTD = (0 << VFD_stb);
  delayMicroseconds(1);
        for (int i = 0; i < 8; i++) // 8 bit 0-7 // aqui inverti para ficar com 0x88 correccto
        {
          PIND = (1 << VFD_in);
          delayMicroseconds(1);
          PIND = (1 << VFD_clk); // O (^)XOR logo só muda se for diferente de "1 1" e "0 0"
          delayMicroseconds(1);
          PIND = (0 << VFD_clk);
          delayMicroseconds(1);
          PORTD = (0 << VFD_in);
          delayMicroseconds(1);
        }
  PORTD = (1 << VFD_stb) | (0 << VFD_clk) | (1 << VFD_in);
  delayMicroseconds(10);
}

void send_data(unsigned char a) {
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data = a;
  //This don't send the strobe signal, to be used in burst data send
  for (mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
    PIND = (0 << VFD_clk);
    if (data & mask) { // if bitwise AND resolves to true
      PORTD = (1 << VFD_in);
    }
    else { //if bitwise and resolves to false
      PORTD = (0 << VFD_in); // send 0
    }
    _delay_us(48); //delay
    PIND = (1 << VFD_clk);
    _delay_us(48); //delay
    PIND &= ~(0 << VFD_clk);
    _delay_us(48); //delay
  }
}

void sendVFD(unsigned char a) {
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask

  data = a;
  PIND &= ~(1 << VFD_stb);
  _delay_us(48);
  for (mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
    PIND = (0 << VFD_clk);
    if (data & mask) { // if bitwise AND resolves to true
      PORTD = (1 << VFD_in);
    }
    else { //if bitwise and resolves to false
      PORTD = (0 << VFD_in); // send 0
    }
    _delay_us(48); //delay
    PIND = (1 << VFD_clk);
    _delay_us(48); //delay
    PIND &= ~(0 << VFD_clk);
    _delay_us(48); //delay
  }
  PIND = (1 << VFD_stb);
  _delay_us(48);
}

void pt6315_init(void) {
  delayMicroseconds(200); //power_up delay
  // Configure VFD display (grids)
  sendVFD(0b00000111);//     cmd2 11 grids 16 segm
  delay_tcsh; // 
  // Write to memory display, increment address, normal operation
  sendVFD(0b01000000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
  delay_tcsh;
  // Address 00H - 15H ( total of 11*2Bytes=176 Bits)
  sendVFD(0b11000000);//(BIN(01100110)); //(BIN(01100110)));
  delay_tcsh;
  // set DIMM/PWM to value
  sendVFD((0b10001000) | 7);//0 min - 7 max  )(0b01010000)
  delay_tcsh;
}

void msg(){
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(0B00000000);  send_data(0B00000000);  send_data(0B00000000);  // 17-24;   9-16;    1-8;
send_data(0B00000000);  send_data(0B00000000);  send_data(0B00000000);  // 41-48;  33-40;   25-32;
send_data(0B10001000);  send_data(0B10001111);  send_data(0B00000000);  // 65-72;  57-64;   49-56; //H
send_data(0B00100100);  send_data(0B00100010);  send_data(0B00000001);  // 89-96;  81-88;   73-80; //I
send_data(0B00000000);  send_data(0B00000000);  send_data(0B00000000);  //113-120; 105-112; 97-104; //Space
send_data(0B00001100);  send_data(0B00001111);  send_data(0B00000000);  //137-144; 129-136; 121-128; //F
send_data(0B10001100);  send_data(0B10001000);  send_data(0B00000001);  //161-168; 153-160; 145-152; //O
send_data(0B00001000);  send_data(0B00001000);  send_data(0B00000001);  //185-192; 177-184; 169-176; //L
send_data(0B01001000);  send_data(0B01001011);  send_data(0B00000000);  //209-216; 201-208; 193-200; //K
send_data(0B00001100);  send_data(0B10000111);  send_data(0B00000001);  //233-240; 225-232; 217-224; //S
send_data(0B00000000);  send_data(0B00000000);  send_data(0B00000000);  
send_data(0B00000000);  send_data(0B00000000);  send_data(0B00000000);  

 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}

void anim7(){
  char0 = (animChars[0] | char0);
  char1 = (animChars[1] | char1);
  char2 = (animChars[2] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[0] | char0);  send_data(animChars[1] | char1);  send_data(animChars[2] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim8(){
  char0 = (animChars[3] | char0);
  char1 = (animChars[4] | char1);
  char2 = (animChars[5] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[3] | char0);  send_data(animChars[4] | char1);  send_data(animChars[5] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim9(){
  char0 = (animChars[6] | char0);
  char1 = (animChars[7] | char1);
  char2 = (animChars[8] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[6] | char0);  send_data(animChars[7] | char1);  send_data(animChars[8] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim10(){
  char0 = (animChars[9] | char0);
  char1 = (animChars[10] | char1);
  char2 = (animChars[11] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[9] | char0);  send_data(animChars[10] | char1);  send_data(animChars[11] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim11(){
char0 = (animChars[12] | char0);
char1 = (animChars[13] | char1);
char2 = (animChars[14] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[12] | char0);  send_data(animChars[13] | char1);  send_data(animChars[14] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim12(){
  char0 = (animChars[15] | char0);
  char1 = (animChars[16] | char1);
  char2 = (animChars[17] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[15] | char0);  send_data(animChars[16] | char1);  send_data(animChars[17] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim13(){
  char0 = (animChars[18] | char0);
  char1 = (animChars[19] | char1);
  char2 = (animChars[20] | char2);
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[18] | char0);  send_data(animChars[19] | char1);  send_data(animChars[20] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim14(){
  char0 = (animChars[21] | char0);
  char1 = (animChars[22] | char1);
  char2 = (animChars[23] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[21] | char0);  send_data(animChars[22] | char1);  send_data(animChars[23] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim15(){
  char0 = (animChars[24] | char0);
  char1 = (animChars[25] | char1);
  char2 = (animChars[26] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[24] | char0);  send_data(animChars[25] | char1);  send_data(animChars[26] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim16(){
  char0 = (animChars[27] | char0);
  char1 = (animChars[28] | char1);
  char2 = (animChars[29] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[27] | char0);  send_data(animChars[28] | char1);  send_data(animChars[29] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim17(){
  char0 = (animChars[30] | char0);
  char1 = (animChars[31] | char1);
  char2 = (animChars[32] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[30] | char0);  send_data(animChars[31] | char1);  send_data(animChars[32] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim18(){
  char0 = (animChars[33] | char0);
  char1 = (animChars[34] | char1);
  char2 = (animChars[35] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[33] | char0);  send_data(animChars[34] | char1);  send_data(animChars[35] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim19(){
  char0 = (animChars[36] | char0);
  char1 = (animChars[37] | char1);
  char2 = (animChars[38] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[36] | char0);  send_data(animChars[37] | char1);  send_data(animChars[38] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim20(){
  char0 = (animChars[39] | char0);
  char1 = (animChars[40] | char1);
  char2 = (animChars[41] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[39] | char0);  send_data(animChars[40] | char1);  send_data(animChars[41] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim21(){
  char0 = (animChars[42] | char0);
  char1 = (animChars[43] | char1);
  char2 = (animChars[44] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[42] | char0);  send_data(animChars[43] | char1);  send_data(animChars[44] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim22(){
  char0 = (animChars[45] | char0);
  char1 = (animChars[46] | char1);
  char2 = (animChars[47] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[45] | char0);  send_data(animChars[46] | char1);  send_data(animChars[47] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim23(){
  char0 = (animChars[48] | char0);
  char1 = (animChars[49] | char1);
  char2 = (animChars[50] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[48] | char0);  send_data(animChars[49] | char1);  send_data(animChars[50] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}
void anim24(){
  char0 = (animChars[51] | char0);
  char1 = (animChars[52] | char1);
  char2 = (animChars[53] | char2);
  
sendVFD(0b01000000); // cmd 2
PIND &= ~(1 << VFD_stb);
delayMicroseconds(4);
send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
//
send_data(animChars[51] | char0);  send_data(animChars[52] | char1);  send_data(animChars[53] | char2);  // 17-24;   9-16;    1-8;
 PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
}

void clearVFD(void) 
{
  /*
    Here I clean all registers
    Could be done only on the number of grid
    to be more fast. The 12 * 3 bytes = 36 registers
  */
  sendVFD(0b01000000); // cmd 2
  PIND &= ~(1 << VFD_stb);
  delayMicroseconds(4);
  send_data(0b11000000); //cmd 3

  for (int i = 0; i < 36; i++) {
    send_data(0b00000000); // data
  }

  PIND = (1 << VFD_stb);
  delayMicroseconds(4);
}
void allOn(void) 
{
  /*
    Here I clean all registers
    Could be done only on the number of grid
    to be more fast. The 12 * 3 bytes = 36 registers
  */
  sendVFD(0b01000000); // cmd 2: Normal operation, incremente, write to display.
  PIND &= ~(1 << VFD_stb);
  delayMicroseconds(4);
  send_data(0b11000000); //cmd 3: Address RAM to start fill data.

  for (int i = 0; i < 36; i++) {
    send_data(0b11111111); // data
  }

  PIND = (1 << VFD_stb);
  delayMicroseconds(4);
}
void allOff(void) 
{
  /*
    Here I clean all registers
    Could be done only on the number of grid
    to be more fast. The 12 * 3 bytes = 36 registers
  */
  sendVFD(0b01000000); // cmd 2
  PIND &= ~(1 << VFD_stb);
  delayMicroseconds(4);
  send_data(0b11000000); //cmd 3

  for (int i = 0; i < 36; i++) {
    send_data(0b00000000); // data
  }

  PIND = (1 << VFD_stb);
  delayMicroseconds(4);
}
//**************ZONE Functions to write HI FOLKS***********************//
void msgHiFolks(void){
  for(int i=4; i> 0; i--){
  writeCharH();
  writeCharI();
  writeCharSpace();
  writeCharF();
  writeCharO();
  writeCharL();
  writeCharK();
  writeCharS();
  delayMicroseconds(1000);
  clearVFD();
  }
}
//
void writeCharH(void){
    sendVFD(0b01000000); // cmd 2
    PIND &= ~(1 << VFD_stb);
    delayMicroseconds(4);
    send_data((0b11000000) | 6); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b11110000); // data
    send_data(0b00011000); // data
    send_data(0b00000000); // data
          
    PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00001000); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharI(void){
    sendVFD(0b01000000); // cmd 2
    PIND &= ~(1 << VFD_stb);
    delayMicroseconds(4);
    send_data((0b11000000) | 9); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b00000101); // data
    send_data(0b00100010); // data
    send_data(0b00000000); // data
          
    PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00001000); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharSpace(void){
    sendVFD(0b01000000); // cmd 2
    PIND &= ~(1 << VFD_stb);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x0C); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b00000000); // data
    send_data(0b00000000); // data
    send_data(0b00000000); // data
          
    PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00001000); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharF(void){
    sendVFD(0b01000000); // cmd 2
    PIND &= ~(1 << VFD_stb);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x0F); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b11010000); // data
    send_data(0b00101000); // data
    send_data(0b00000000); // data
          
    PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00001000); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharO(void){
    sendVFD(0b01000000); // cmd 2
    PIND &= ~(1 << VFD_stb);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x12); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b00110001); // data
    send_data(0b00111000); // data
    send_data(0b00000000); // data
          
    PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00001000); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharL(void){
    sendVFD(0b01000000); // cmd 2
    PIND &= ~(1 << VFD_stb);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x15); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b00010001); // data
    send_data(0b00001000); // data
    send_data(0b00000000); // data
          
    PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00001000); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharK(void){
    sendVFD(0b01000000); // cmd 2
    PIND &= ~(1 << VFD_stb);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x18); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b10010010); // data
    send_data(0b00001001); // data
    send_data(0b00000000); // data
          
    PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00001000); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharS(void){
    sendVFD(0b01000000); // cmd 2
    PIND &= ~(1 << VFD_stb);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x1B); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b11100001); // data
    send_data(0b00101000); // data
    send_data(0b00000000); // data
          
    PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00001000); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
//************END ZONE Functions to write HI FOLKS********************//

void findSegments(void) {
  byte AA = 0x00;
  byte AB = 0x00;
  byte AC = 0x00;
  unsigned long n = 0b100000000000000000000000;
  unsigned int numberOfSeg = 0;
  
  for (int i = 0; i < 36; i = i + 3) {  // 12 Grids * 3 bytes
    for(int m = 0; m < 24; m++) {

//This start the control of button to allow continue teste! 
                      while(1){
                            if(!buttonReleased){
                              delay(200);
                            }
                            else{
                              delay(15);
                               buttonReleased = false;
                               break;
                               }
                         }
      
    sendVFD(0b01000000); // cmd 2
    PIND &= ~(1 << VFD_stb);
    delayMicroseconds(4);
    send_data((0b11000000) | i); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

   AA= (n & 0x000000ffUL) >>  0;
   AB= (n & 0x0000ff00UL) >>  8;
   AC= (n & 0x00ff0000UL) >> 16;
         
    send_data(AA); // data
    send_data(AB); // data
    send_data(AC); // data
    
        
      if (n > 8388608; n >>= 1) { //iterate through bit mask  //(8388608) ( 16777216)
        numberOfSeg++;   
      }
      else{
        n = 0b100000000000000000000000;// Length of 3 bytes (24 bit)
        numberOfSeg++; 
       clearVFD();  //Is important this clear here to keep the VFD clean after skip over the group to 3 bytes!
      }
Serial.println(numberOfSeg, DEC);Serial.print("  :     ");
      Serial.print(AC, HEX); Serial.print(", "); Serial.print(AB, HEX); Serial.print(", "); Serial.print(AA, HEX); Serial.print(", nº: "); Serial.println(m, HEX);

    PIND = (1 << VFD_stb);
    delayMicroseconds(4);

    sendVFD(0b00000111); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0b10001000) | 7); //cmd 4
    delayMicroseconds(4);
    
        for(int s = 0; s<1000; s++){
          delayMicroseconds(100);
        }
    }
  }
}

void animation(){
  unsigned int t0=80;
  unsigned int randNumber;
  unsigned int randTime;
  unsigned int cycle = 0x00;
  unsigned int m = 0x00;
 //
      for (unsigned int n = 0; n < 25; n++){
            char0 = 0x00;
            char1 = 0x00;
            char2 = 0x00;
            Serial.println("--------------------------------------------------------");
            randNumber = random(7, 24);
     
            for (randNumber; ((randNumber > 6) && (randNumber < 25)); randNumber++){
           
              t0=random(80-200);
              Serial.println(randNumber, DEC);
                  switch(randNumber){
                    case 7:anim7(); delay(t0); break;
                    case 8:anim8(); delay(t0); break;
                    case 9:anim9(); delay(t0); break;
                    case 10:anim10(); delay(t0); break;
                    case 11:anim11(); delay(t0); break;
                    case 12:anim12(); delay(t0); break;
                    case 13:anim13(); delay(t0); break;
                    case 14:anim14(); delay(t0); break;
                    case 15:anim15(); delay(t0); break;
                    case 16:anim16(); delay(t0); break;
                    case 17:anim17(); delay(t0); break;
                    case 18:anim18(); delay(t0); break;
                    case 19:anim19(); delay(t0); break;
                    case 20:anim20(); delay(t0); break;
                    case 21:anim21(); delay(t0); break;
                    case 22:anim22(); delay(t0); break;
                    case 23:anim23(); delay(t0); break;
                    case 24:anim24(); delay(t0); break;
                  }
            }
        }
 
}

void setup() {
  cli();
  // put your setup code here, to run once:
  pinMode( VFD_stb, OUTPUT);// Must be pulsed to LCD fetch data of bus
  pinMode( VFD_in, OUTPUT);//  If 0 write LCD, if 1 read of LCD
  pinMode( VFD_clk, OUTPUT);// if 0 is a command, if 1 is a data0


  
  pinMode( 13, OUTPUT); //Pin of pinMode(LED_BUILTIN, OUTPUT);

pinMode(BUTTON_PIN, INPUT);

 attachInterrupt(digitalPinToInterrupt(BUTTON_PIN),
                  buttonReleasedInterrupt,
                  FALLING);

  Serial.begin(115200);
  unsigned char a, b;

/*
  TCCR1B = (1 << CS12 | 1 << WGM12);
  OCR1A = 32768 - 1; //32768-1;
  TIMSK1 = 1 << OCIE1A;
*/
//set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  a = 0x33;
  b = 0x01;
  //You write a 1 to make a pin an output and a 0 to make it an input.
  CLKPR = (0x80);
  //Set PORT
  DDRD = 0xFB;
  PORTD = 0x00;
//  DDRB = 0xFF;
//  PORTB = 0x00;
  pt6315_init();
  clearVFD();
  
 sei();
}

void loop() {
  for(unsigned int n = 0; n< 5; n++){
     allOff();
     delay(500);
     allOn();
     delay(500);
  }
 
 msg();
 animation();

allOff();
//   findSegments();
}

void buttonReleasedInterrupt() {
  buttonReleased = true; // This is the line of interrupt button to advance one step on the search of segments!
}

ISR(TIMER1_COMPA_vect) {  //Interrupt Trigger
  var = digitalRead(13);
  digitalWrite(13, !var);
  //actSSMMHH();
}
