#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

void setup()
{
SerialBT.begin("ESP32_Health");    //lazem el esm yb2a kda
}

void loop()
{
int temp_int = (int)temperature; 
int temp_dec = (int)((temperature - temp_int) * 100);       //bluetooth cant send float data so send the integar part then decimal(for temprature)

//if(all sensors detect the patient mateb2a4 fel hawa){
sendBT_func(beatAvg, temp_int , temp_dec , sp02Avg);          //lw el data ymkn t5rg 3n el range 0:255  write: sendBT_func( constrain(beatAvg,0,255), constrain(temp_int,0,255) , temp_dec , constrain(sp02Avg,0,100) ); 
// }

}

void sendBT_func(int hr, int temp_i , int temp_d, int spo2) 
{
SerialBT.write(255);
SerialBT.write(hr);
SerialBT.write(temp_i);
SerialBT.write(temp_d);
SerialBT.write(spo2);
SerialBT.write(255);
}
