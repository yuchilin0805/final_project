#include "mbed.h"
#include "bbcar.h"

Thread thread1;
Thread xbeethread(osPriorityHigh);
Thread ledthread;
EventQueue ledqueue(32 * EVENTS_EVENT_SIZE);
EventQueue xbeequeue(32 * EVENTS_EVENT_SIZE);
EventQueue sendqueue(32 * EVENTS_EVENT_SIZE);
Ticker servo_ticker;
Ticker encoder_ticker;
Ticker encoder_ticker2;
Timer timer1;
PwmOut pin8(D8), pin9(D9);////////////////right D8 left D9
DigitalIn pin3(D3);
DigitalIn pin4(D4);
DigitalOut Leds(D7);
DigitalInOut pin10(D10);
parallax_encoder encoder0(pin3, encoder_ticker); //////left encoder
parallax_encoder encoder1(pin4, encoder_ticker2);//////right encoder                      
parallax_ping  ping1(pin10);
DigitalOut led1(LED1);

Serial uart(D1,D0); //tx,rx
RawSerial pc(USBTX, USBRX);
RawSerial xbee(D12, D11);
BBCar car(pin8, pin9, servo_ticker);
int nowdir=0; //0:+x 1:-x 2:+y 3:-y
int sendmode=0;
float angle=-1;
int image=-1;
int nowstatus=0; //0 straight 1 left 2 right  4 reverseparking 5 datamatrice 6 mission1 7 mission2 8 idle 9 end
int shape=0;
float last_encoder=0; 
int blinks=0;
/*class position{
    public:
        position(float a,float b){
            x=a;
            y=b;
        }
        float x;
        float y;
};
position pos_now(25,25);*/
void straight(){
    nowstatus=0;
    car.goStraight(150);
    last_encoder=0;
    while(1){
        //pc.printf("%f\r\n",pos_now.x);
        if(encoder0.get_cm()*1.066667>120){
            car.stop();
            //pc.printf("%f\r\n",encoder0.get_cm());
            //pc.printf("%f\r\n",encoder1.get_cm());
            break;
        }
        /*if(encoder1.get_cm()*1.067>90.5){
            car.stop();
            break;
        }*/
        //pc.printf("%f\r\n",(float)ping1);
        if((float)ping1>30) led1 = 1;
        else /*if(encoder0.get_cm()*1.0667>100)*/{
            led1 = 0;
            car.stop();
            break;
        }
        wait(0.05);

    } 
    /*nowstatus=9;
    wait(2);*/
}
void right(){
    encoder0.reset();
    encoder1.reset();
    nowstatus=2;
    car.turn(100,-0.2);
    while(encoder0.get_cm()<24){
        wait(0.01);
    }
    car.stop();
}
void left(){
    encoder0.reset();
    encoder1.reset();
    nowstatus=1;
    car.turn(80,0.2);
    while(encoder1.get_cm()<20){
        wait(0.01);
    }

    /*car.turn(110,0.8);
    wait(1.2);*/    

    
    /*while(1){
        pos_now.y+=encoder1.get_cm();
        if((float)ping1>90 &&(float)ping1<170){
            led1 = 0;
            car.stop();
            break;
        } 
        else{
           led1 = 1; 
        }
        wait(0.01);
    }*/
    car.stop();
}
void reverseright(){
    encoder0.reset();
    encoder1.reset();
    car.turn(-80,-0.2);
    while(encoder0.get_cm()<24){
        wait(0.01);
    }
    car.stop();
}
void reverseleft(){
    encoder0.reset();
    encoder1.reset();
    car.turn(-80,0.2);
    while(encoder1.get_cm()<24){
        wait(0.01);
    }
    /*car.turn(-120,0.23);
    wait(1.09);*/
    /*while(1){
        if((float)ping1>20) led1 = 1;
        else{
            led1 = 0;
            car.stop();
            break;
        }
        wait(.01);
    }*/
    car.stop();
}
void creepcw(){
    car.turn(100,-0.3);
    wait(0.3);
    car.stop();
}
void creepccw(){
    car.turn(100,0.3);
    wait(0.3);
    car.stop();
}
void calib(){
    char buff[30];
    int i=0;
    nowstatus=5;
    //pc.printf("calib\r\n");
    timer1.start();
    while(1){
      sendmode=2;
      wait(0.01);
      if(uart.readable()){
            char recv = uart.getc();
            //pc.putc(recv);
            buff[i]=recv;
            
            if(buff[i]=='\r'){
                uart.getc();
                //pc.putc('\r\n');
                //buff[i]='\0';
                char tmp[i-1];
                for(int j=0;j<i-1;j++){
                    tmp[j]=buff[j];
                }
                angle=atof(tmp);
                //pc.printf("%f",angle);
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
      if(timer1.read_ms()>3000){
          angle=2;
          break;
      }
   }
   sendmode=0;
   wait(2);
   nowstatus=0;
   timer1.reset();
}

void reverseparking(){
    nowstatus=0;
    car.goStraight(120);
    while(1){
        if((float)ping1>20) led1 = 1;
        else /*if(encoder0.get_cm()>80)*/{
            led1 = 0;
            car.stop();
            break;
        }
        wait(.01);
    }
    nowstatus=4;
    reverseleft();
    car.goStraight(-100);
    wait(0.8);
    while(1){
        if((float)ping1<41) led1 = 1;
        else{
            led1 = 0;
            car.stop();
            break;
        }
        wait(.01);
    }
    nowstatus=8;
}
void send_thread(){
   while(1){
      if( sendmode == 1){
         char s[21];
         sprintf(s,"image_classification");
         uart.puts(s);
         //pc.printf("send\r\n");
         wait(2);
      }
      else if(sendmode ==2){
        char s[4];
        sprintf(s,"data");
        uart.puts(s);
        //pc.printf("send\r\n");
        wait(2);
        //  break;
      }
   }
}
void mission1(){
    int flag=0;
    blinks=1;
    char buff;
    wait(1);
    nowstatus=6;
    while(1) {
        sendmode=1;
        wait(2);
      if(uart.readable()){
            char recv = uart.getc();
            if(!flag){
                buff=recv;
               // pc.printf("buff %c",buff);
                image=(int)buff-48;
                break;
            }
                
            //pc.putc(recv);
            /*if(recv!='o' &&recv!='\r'&& recv!='\n'){
                pc.printf("recv %c",recv);
                image=int(recv)-48;
                break;
            }   */             
      }
   }
   if(image>10)
        image=9;
   //pc.printf("%d",image);
   sendmode=0;
   wait(3);
   nowstatus=8;
   blinks=0;
}
void leaving_mission1(){
    nowstatus=0;
    car.goStraight(120);
    while(1){
        if((float)ping1>30) led1 = 1;
        else{
            led1 = 0;
            car.stop();
            break;
        }
        wait(.01);
    }
    wait(0.5);
    right();
    nowstatus=0;
    car.goStraight(120);
    while(1){
        if((float)ping1>51) led1 = 1;
        else{
            led1 = 0;
            car.stop();
            break;
        }
        wait(.01);
    }
    wait(0.5);
    right();
    nowstatus=0;
    car.goStraight(120);
    while(1){
        if((float)ping1>30) led1 = 1;
        else{
            led1 = 0;
            car.stop();
            break;
        }
        wait(.01);
    }
    right();
}
void moveright(){
    //pc.printf("right");
    creepcw();
    wait(2);
}
void moveleft(){
    //pc.printf("left");
    creepccw();
    wait(2);
}
void mission2(){

    blinks=1;
    car.goStraight(120);
    while(1){
        //pc.printf("%1.3f\r\n",(float)ping1);
        if((float)ping1>42) led1 = 1;
        else{
            led1 = 0;
            car.stop();
            break;
        }
        wait(.01);
    }
   // right();
    float firstcheck[10],rightcheck[10],leftcheck[10];
    for(int i=0;i<10;i++){
        firstcheck[i]=(float)ping1;
        wait(0.2);
    }
    moveright();
    for(int i=0;i<10;i++){
        rightcheck[i]=(float)ping1;
        wait(0.2);
    }
    moveleft();
    for(int i=0;i<10;i++){
        leftcheck[i]=(float)ping1;
        wait(0.2);
    }
    int square=0;
    int isosceles=0;
    int symmetry=0; //1 triangle  2 insidetriangle 3 isosceles
    for(int i=0;i<10;i++){
        if(firstcheck[i]-rightcheck[i]<1){
            square++;
        }
        if(rightcheck[i]-leftcheck[i]>3){
            symmetry=-1;
        }
    }
    for(int i=0;i<10;i++){
        if(firstcheck[i]-leftcheck[i]<1){
            square++;
        }
        /*if(rightcheck[i]-leftcheck[i]>5){
            isosceles++;
        }*/
        if(symmetry=-1){
            if(firstcheck[i]-rightcheck[i]<0&&firstcheck[i]-leftcheck[i]<0){
                symmetry=1;
            }
            else if(firstcheck[i]-rightcheck[i]>0&&firstcheck[i]-leftcheck[i]>0){
                symmetry=2;
            }
            else {
                symmetry=3;
            }
        }
        
    }
    int issquare=0;
    if(square>15){
        //pc.printf("square");
        issquare=1;
        shape=1;
    }
    if(!issquare){
        if(symmetry==1){
            //pc.printf("triangle");
            shape=0;
        }
        else if(symmetry==2){
           // pc.printf("insidetriangle");
            shape=3;
        }
        else{
            //pc.printf("isosceles");
            shape=2;
        } 
    }
    nowstatus=7;
    wait(2);
    nowstatus=8;
    blinks=0;
}
void to_mission1(){
    straight();
    encoder0.reset();
    wait(0.5);
    calib();
    encoder0.reset();
    encoder1.reset();
    left();
    encoder0.reset();
    wait(3);  
    reverseparking();
}
void leave_mission2(){
    reverseright();
    wait(2);
    car.goStraight(120);
    encoder0.reset();
    nowstatus=0;
    while(1){
        //pc.printf("%f\r\n",pos_now.x);
        if(encoder0.get_cm()*1.066667>40){
            car.stop();
            //pc.printf("%f\r\n",encoder0.get_cm());
            //pc.printf("%f\r\n",encoder1.get_cm());
            break;
        }
        /*if(encoder1.get_cm()*1.067>90.5){
            car.stop();
            break;
        }*/
        //pc.printf("%f\r\n",(float)ping1);
        if((float)ping1>30) led1 = 1;
        else if(encoder0.get_cm()*1.0667>20){
            led1 = 0;
            car.stop();
            break;
        }
        wait(0.05);
    } 
    car.stop();
    nowstatus=2;
    right();
    nowstatus=0;
    car.goStraight(120);
    while(1){
        if(encoder0.get_cm()>120){
            car.stop();
            break;
        }
    }
    car.stop();
    nowstatus=9;
}   
void sendpos(){
    
    if(nowstatus==0){
        xbee.printf("$straight#");
    }
    else if(nowstatus==1){
        xbee.printf("$left#");
    }
    else if(nowstatus==2){
        xbee.printf("$right#");
    }
    else if(nowstatus==4){
        xbee.printf("$reverse_parking#");
    }
    else if(nowstatus==5){
        char buff[5];
        if(angle>100)
            sprintf(buff,"%1.1f",angle);
        else if(angle>10)
            sprintf(buff,"0%1.1f",angle);
        else{
            sprintf(buff,"00%1.1f",angle);
        }
        if(angle!=-1){
            xbee.printf("$angle:%s#",buff);
        }

    }
    else if(nowstatus==6){
        if(image!=-1){
            char buff[3];
            sprintf(buff,"$image:%d#",image);
            xbee.printf("%s",buff);
        }

    }
    else if(nowstatus==7){

        xbee.printf("$shape:%d#",shape);
    }
    else if(nowstatus==9){
        xbee.printf("$end#");
    }
    
    
    
}
void ledstatus(){
    Leds=1;
    while(1){
        if(blinks==1){
            Leds=!Leds;
        }
        else{
            Leds=1;
        }
        wait(0.3);
    }
}
int main() {
    uart.baud(9600);
    xbee.baud(9600);
    pc.baud(9600);
    encoder0.reset();
    encoder1.reset();
    thread1.start(callback(&sendqueue, &EventQueue::dispatch_forever));
    sendqueue.call(send_thread);
    xbeethread.start(callback(&xbeequeue, &EventQueue::dispatch_forever));
    xbeequeue.call_every(1000,sendpos);
    ledthread.start(callback(&ledqueue, &EventQueue::dispatch_forever));
    ledqueue.call(ledstatus);
    //pc.printf("aaaaaaaaaaaaa");
    blinks=0;
    to_mission1();
    mission1();
    leaving_mission1();
    //wait(5);
    //straight();
    mission2();
    leave_mission2();
    car.stop();
    nowstatus=9;

}