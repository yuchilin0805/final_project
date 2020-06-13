#include "mbed.h"
#include "bbcar.h"

Thread thread1;
Ticker servo_ticker;
Ticker encoder_ticker;
Ticker encoder_ticker2;
PwmOut pin8(D8), pin9(D9);////////////////right D8 left D9
DigitalIn pin3(D3);
DigitalIn pin4(D4);
DigitalInOut pin10(D10);
parallax_encoder encoder0(pin3, encoder_ticker); //////left encoder
parallax_encoder encoder1(pin4, encoder_ticker2);//////right encoder                      
parallax_ping  ping1(pin10);
DigitalOut led1(LED1);
Serial pc(USBTX,USBRX); //tx,rx
Serial uart(D1,D0); //tx,rx

BBCar car(pin8, pin9, servo_ticker);
int sendmode;
float angle=0;
int image=0;
struct pos{
    float x=0;
    float y=0;
};

void straight(){
    car.goStraight(150);
    while(1){
        //pc.printf("%f\r\n",encoder1.get_cm());
        if(encoder0.get_cm()*1.066667>90.5){
            car.stop();
            pc.printf("%f\r\n",encoder0.get_cm());
            pc.printf("%f\r\n",encoder1.get_cm());
            break;
        }
        /*if(encoder1.get_cm()*1.067>90.5){
            car.stop();
            break;
        }*/
        if((float)ping1>20) led1 = 1;
        else{
            led1 = 0;
            car.stop();
            break;
        }
        wait(.01);
     
    } 
}
void right(){
    car.turn(150,-0.3);
    wait(3);
    car.stop();
}
void left(){
    car.turn(150,0.3);
    wait(3);
    car.stop();
}
void reverseleft(){
    car.turn(-150,0.3);
    wait(3);
    car.stop();
}
void creepcw(){
    car.turn(100,-0.5);
    wait(0.1);
    car.stop();
}
void creepccw(){
    car.turn(100,0.5);
    wait(0.1);
    car.stop();
}
void calib(){
    char buff[30];
    int i=0;
    while(1){
      sendmode=2;
      wait(0.01);
      if(uart.readable()){
            char recv = uart.getc();
            //pc.putc(recv);
            buff[i]=recv;
            
            if(buff[i]=='\r'){
                uart.getc();
                pc.putc('\r\n');
                //buff[i]='\0';
                char tmp[i-1];
                for(int j=0;j<i-1;j++){
                    tmp[j]=buff[j];
                }
                angle=atof(tmp);
                pc.printf("%f",angle);
                if(angle<=3 || angle>=357){
                    break;
                }
                else{
                    i=0;
                    if(angle<180)
                        creepcw();
                    else
                        creepccw();
                    continue;
                }
                break;
            }
            i++;
      }
   }
   sendmode=0;
}

void reverseparking(){
    car.goStraight(150);
    while(1){
        if((float)ping1>20) led1 = 1;
        else{
            led1 = 0;
            car.stop();
            break;
        }
        wait(.01);
    }
    reverseleft();
    car.goStraight(-150);
    while(1){
        if((float)ping1>51) led1 = 1;
        else{
            led1 = 0;
            car.stop();
            break;
        }
        wait(.01);
    }
}
void send_thread(){
   while(1){
      if( sendmode == 1){
         char s[21];
         sprintf(s,"image_classification");
         uart.puts(s);
         pc.printf("send\r\n");
         wait(1);
      }
      else if(sendmode ==2){
        char s[4];
        sprintf(s,"data");
        uart.puts(s);
        pc.printf("send\r\n");
        wait(2);
        //  break;
      }
   }
}
void mission1(){
    int flag=0;
    char buff;
    wait(1);
    while(1) {
        sendmode=1;
        wait(2);
      if(uart.readable()){
            char recv = uart.getc();
            if(!flag){
                buff=recv;
                pc.printf("buff %c",buff);
                image=(int)buff-48;
                break;
            }
                
            pc.putc(recv);
            /*if(recv!='o' &&recv!='\r'&& recv!='\n'){
                pc.printf("recv %c",recv);
                image=int(recv)-48;
                break;
            }   */             
      }
   }
   if(image>10)
        image=-1;
   pc.printf("%d",image);
   sendmode=0;
}
int main() {
    uart.baud(9600);
    encoder0.reset();
    encoder1.reset();
    thread1.start(send_thread);
    calib();
    mission1();
    //straight();
    //wait(5);
    //left();    


    car.stop();
}