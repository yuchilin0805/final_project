#include "mbed.h"
#include "bbcar.h"

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
float angle=0;
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
void reverseparking(){
    car.turn(-150,0.3);
    wait(3);
    car.stop();
}
void calib(){
    char buff[100];
    int i=0;
    while(1){
      if(uart.readable()){
            char recv = uart.getc();
            pc.putc(recv);
            buff[i]=recv;
            if(buff[i]=='\r'){
                buff[i]='\0';
               // angle=(float)buff;
                break;
            }
            i++;
      }
   }
}
void mission1(){

}
int main() {
    uart.baud(9600);
    encoder0.reset();
    encoder1.reset();
    straight();
    wait(5);
    left();    


    car.stop();
}