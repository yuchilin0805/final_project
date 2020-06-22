#include "mbed.h"
#include "bbcar.h"

Thread thread1;
Thread xbeethread;
EventQueue xbeequeue(32 * EVENTS_EVENT_SIZE);
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

Serial uart(D1,D0); //tx,rx
RawSerial pc(USBTX, USBRX);
RawSerial xbee(D12, D11);
BBCar car(pin8, pin9, servo_ticker);
int nowdir=0; //0:+x 1:-x 2:+y 3:-y
int sendmode;
float angle=-1;
int image=-1;
int nowstatus=0; //0 straight 1 left 2 right  4 reverseparking 5 datamatrice 6 mission1
class position{
    public:
        position(float a,float b){
            x=a;
            y=b;
        }
        float x;
        float y;
};
position pos_now(25,25);
void straight(){
    car.goStraight(150);
    while(1){
        //pc.printf("%f\r\n",encoder1.get_cm());
        if(nowdir==0)
            pos_now.x+=encoder0.get_cm();
        else if(nowdir==1)
            pos_now.x-=encoder0.get_cm();
        else if(nowdir==2)
            pos_now.y+=encoder0.get_cm();
        else if(nowdir==3)
            pos_now.y-=encoder0.get_cm();
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
        if((float)ping1>25) led1 = 1;
        else if(encoder0.get_cm()*1.0667>100){
            led1 = 0;
            car.stop();
            break;
        }
        wait(0.05);

    } 
}
void right(){
    if(nowdir==0)
        nowdir=3;
    else if(nowdir==1)
        nowdir=2;
    else if(nowdir==2)
        nowdir=0;
    else if(nowdir==3)
        nowdir=1;
    car.turn(150,-0.3);
    wait(3);
    car.stop();
}
void left(){
    if(nowdir==0)
        nowdir=2;
    else if(nowdir==1)
        nowdir=3;
    else if(nowdir==2)
        nowdir=1;
    else if(nowdir==3)
        nowdir=0;
    
    car.turn(70,1);
    //wait(1.8);
    /*car.turn(110,0.8);
    wait(1.2);*/
    
    while(1){
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
    }
    car.stop();
}
void reverseleft(){
    car.turn(-80,0.3);
    wait(2);
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
    nowstatus=5;
    pc.printf("calib\r\n");
    while(1){
      sendmode=2;
      wait(0.01);
      if(uart.readable()){
            char recv = uart.getc();
            pc.putc(recv);
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
   }
   sendmode=0;
   wait(3);
   nowstatus=0;
}

void reverseparking(){
    car.goStraight(120);
    while(1){
        if((float)ping1>30) led1 = 1;
        else if(encoder0.get_cm()>80){
            led1 = 0;
            car.stop();
            break;
        }
        wait(.01);
    }
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
                pc.printf("buff %c",buff);
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
   nowstatus=0;
}
void sendpos(){
    
    if(nowstatus==0){
        pc.printf("x:%1.2f y:%1.2f\r\n",pos_now.x,pos_now.y);
        char xout[5],yout[5];
        sprintf(xout,"%1.2f",pos_now.x);
        sprintf(yout,"%1.2f",pos_now.y);
        xbee.printf("$(%s,%s)#",xout,yout);
        /*if(pos_now.x<100)
            xbee.printf("$(0%s,",xout);
        else 
            xbee.printf("$(%s,",xout);
        if(pos_now.y<100)
            xbee.printf("0%s)#",yout);
        else 
            xbee.printf("%s)#",yout);*/
    }
    else if(nowstatus==1){

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
            xbee.printf("$%s#",buff);
        }

    }
    else if(nowstatus==6){
        if(image!=-1){
            char buff[3];
            sprintf(buff,"$%d#",image);
            xbee.printf("%s",buff);
        }

    }
    
    
    
}
int main() {
    uart.baud(9600);
    xbee.baud(9600);
    pc.baud(9600);
    encoder0.reset();
    encoder1.reset();
    thread1.start(send_thread);
    xbeethread.start(callback(&xbeequeue, &EventQueue::dispatch_forever));
    xbeequeue.call_every(1000,sendpos);
    wait(5);
    //calib();
    mission1();
    //straight();
    encoder0.reset();
    wait(0.5);
    //left();
    encoder0.reset();
    wait(3);
   // reverseleft();    
    //reverseparking();

    car.stop();
}