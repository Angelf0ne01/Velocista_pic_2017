#include <18F2550.h>
#device ADC=10    //Configuro el ADC con una resolucion de 10bit. 
#include <math.h>  //Libreria para funciones matematicas.
#BYTE PORTA = 0xF80 //
#BYTE PORTB = 0xF81 //
#FUSES XT,MCLR,NOWDT,NOBROWNOUT, NOLVP, NOXINST //Fuses para la configuracion del pic.
#use delay(crystal=20000000)                     //Cristal de 20Mhz
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7)   //Configuracion de la comunicacion Serial.
///////////////////////////////////////////////////////////////////////////////
//Defino los nombres de los Sensores.
int16 RUIDO=100;

#define borde_izq 4
#define med_izq 5
#define cent_izq 3
#define cent_der 2
#define med_der 1
#define borde_der 0
///////////////////////////////////////////////////////////////////////////////
#define pulsador_start PIN_B0
///////////////////////////////////////////////////////////////////////////////
//Variables para: Salidas de pwm (CP1 y CP2).
int16 duty=226;
int16 pwm_m_izq;
int16 pwm_m_der;int16 curvas=0;
///////////////////////////////////////////////////////////////////////////////
//Variables necesarias para la comunicacion Serial.
//(MostrarINT32() y MostrarFLOAT())
int32 aux32,num,den;
int8 aux,temp;
char TablaNumeros[10]={'0','1','2','3','4','5','6','7','8','9'};
float auxfloat;
//////////////////////////////////////////////////////////////////////////////////
//Variables necesarias para la lectura de los sensores opticos.
int puntero;
float value, sensor[6];
float sensor_valor_min[6];
float sensor_valor_max[6];
/////////////////////////////////////////////////////////////////////////////////
//Variables para:  Control PID
float set_point=3.5; //Referencia.

float numerador;
float denominador;
float promedio;
float promedio_anterior;
float error;
float integral;
float derivativo;
float Pid;
float error_anterior;
int16 pwm;
int16 ganancia_pwm=50;

float valor_max=5;
float valor_min=2;
//////////////////////////////////////////////////////////////////////////////////
// Variable para definir el estado del Robot.
int1 en_carrera=0;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MostrarINT32 (int32 valor)
{
   aux32 = valor;
   temp=0;
   if(aux32 >= 100000)
   {
      while(aux32 >= 100000)
      {
         aux32 = aux32 - 100000;
         temp++;
      }
      if(temp>9) temp = 0;
      //centena = temp;       // Actualizo la centena a mostrar
      putc(TablaNumeros[temp]);
      temp = 0;
   }
   if(aux32 >= 10000)  // Valor entre 10000 y 65535 (maximo valor admitido por int16) 
   {
      while(aux32 >= 10000)
      {
         aux32 = aux32 - 10000;
         temp++;
      }
      if(temp>9) temp = 0;
      //centena = temp;       // Actualizo la centena a mostrar
      putc(TablaNumeros[temp]);
      temp = 0;
      while(aux32 >= 1000)
      {
         aux32 = aux32 - 1000;
         temp++;
      }
      if(temp>9) temp = 0;
      //centena = temp;       // Actualizo la centena a mostrar
      putc(TablaNumeros[temp]);
      temp = 0;
   }    
   else if (aux32 >= 1000)     // Valor entre 1000 y 9999
   {
      while(aux32 >= 1000)
      {
         aux32 = aux32 - 1000;
         temp++;
      }
      if(temp>9) temp = 0;
      //centena = temp;       // Actualizo la centena a mostrar
      putc(TablaNumeros[temp]);
      temp = 0;
   }
   
   while(aux32 >= 100)
   {
      aux32 = aux32 - 100;
      temp++;
   }
   if(temp>9) temp = 0;
   //centena = temp;       // Actualizo la centena a mostrar
   putc(TablaNumeros[temp]);
   temp = 0;
            
   while(aux32 >= 10)
   {
      aux32 = aux32-10;
      temp++;
   }
   if(temp>9) temp = 0;
   putc(TablaNumeros[temp]);
   temp = 0;
   while(aux32 >= 1)
   {
      aux32 = aux32-1;
      temp++;
   }
   if(temp>9) temp = 0;
   putc(TablaNumeros[temp]);
   temp = 0;
   return;
}

void MostrarFloat (float valor)
{
   float decimales, unidades;
   if(valor < 0)  // Valor entre 10000 y 65535 (maximo valor admitido por int16)
   {
      putc('-');  // Muestro el signo si es negativo
      valor = -(valor);    // cambio el signo para trabajar con numeros positivos 
                                 // el signo ya lo mostre.
   }
   decimales=modf(valor,&unidades);
   aux32 = unidades;
   // comienzo a mostrar la parte entera del float
   temp=0; 
   if(aux32>10000)
   {
      while(aux32 >= 10000)
      {
         aux32 = aux32 - 10000;
         temp++;
      }
      if(temp>9) temp = 0;
      putc(TablaNumeros[temp]);
      temp = 0;
      while(aux32 >= 1000)
      {
         aux32 = aux32 - 1000;
         temp++;
      }
      if(temp>9) temp = 0;
      putc(TablaNumeros[temp]);
      temp = 0;
   }    
   else if (aux32 >= 1000)     // Valor entre 1000 y 9999
   {
      while(aux32 >= 1000)
      {
         aux32 = aux32 - 1000;
         temp++;
      }
      if(temp>9) temp = 0;
      //centena = temp;       // Actualizo la centena a mostrar
      putc(TablaNumeros[temp]);
      temp = 0;
   }
   
   while(aux32 >= 100)
   {
      aux32 = aux32 - 100;
      temp++;
   }
   if(temp>9) temp = 0;
   //centena = temp;       // Actualizo la centena a mostrar
   putc(TablaNumeros[temp]);
   temp = 0;
            
   while(aux32 >= 10)
   {
      aux32 = aux32-10;
      temp++;
   }
   if(temp>9) temp = 0;
   putc(TablaNumeros[temp]);
   temp = 0;
   while(aux32 >= 1)
   {
      aux32 = aux32-1;
      temp++;
   }
   if(temp>9) temp = 0;
   putc(TablaNumeros[temp]);
   temp = 0;
   
   // ahora muestro los decimales
   putc('.');
   while(decimales >= 0.1)
   {
      decimales = decimales-0.1;
      temp++;
   }
   if(temp>9) temp = 0;
   putc(TablaNumeros[temp]);
   temp = 0;
   while(decimales >= 0.01)
   {
      decimales = decimales-0.01;
      temp++;
   }
   if(temp>9) temp = 0;
   putc(TablaNumeros[temp]);
   temp = 0;
   return;
}
void lectura_de_sensores(){
//4-5-3-2-1-0   
   for(puntero=0;puntero<5;puntero++)
   {
      //Sensores activos bajos.
      set_adc_channel(puntero);  //Selecciono el puerto analogico que se va leer.
      //El sensor disminuye su valor al detectar color Blanco
      value=1024-read_adc(); //Lectura de adc. Ajusto los valores del sensor para que varie entre 0 y 1024 (0=linea negro, 1024= Linea lanco). 
      Sensor[puntero]=value*1000.0/1024.0; //Adapto el rango del sensor a un valor entre 0 y 1000.
      
      if(sensor[puntero]<sensor_valor_min[puntero]) sensor_valor_min[puntero]=sensor[puntero];  //Indico el valor minimo leido por el sensor
      if(sensor[puntero]>sensor_valor_max[puntero]) sensor_valor_max[puntero]=sensor[puntero];  //Indico el valor maximo leido por el sensor
      
      //Ajusto los valores maximos y minimos que puede tener el robot.
      int32 num=(sensor[puntero]-sensor_valor_min[puntero]);
      num*=1000.0;
      int32 den=(sensor_valor_max[puntero]-sensor_valor_min[puntero]);
      float aux_sensor_float=(float)num/den;
      sensor[puntero]=(int16)aux_sensor_float;   
      if (sensor[puntero]<RUIDO) sensor[puntero]=0;
   }

      //Lectura directa del puerto AN8
      //Se hace esto para no leer dos puertos inexistente y ahorrar tiempo en lectura de los sensores.
      //Puertos inexistentes AN6 y AN7.
      set_adc_channel(8);  
      value=1024-read_adc(); 
      Sensor[5]=value*1000.0/1024.0; //
     
      if(sensor[5]<sensor_valor_min[5]) sensor_valor_min[5]=sensor[5];
      if(sensor[5]>sensor_valor_max[5]) sensor_valor_max[5]=sensor[5];
      
      int32 num=(sensor[5]-sensor_valor_min[5]);
      num*=1000.0;
      int32 den=(sensor_valor_max[5]-sensor_valor_min[5]);
      float aux_sensor_float=(float)num/den;
      sensor[5]=(int16)aux_sensor_float;   
      if (sensor[5]<ruido) sensor[5]=0;

}


void Control_PID(float Kp,float Kd)//,float ki)
{
  numerador=((sensor[borde_izq])+(sensor[med_izq]*2)+(sensor[cent_izq]*3)+(sensor[cent_der]*4)+(sensor[med_der]*5)+(sensor[borde_der]*6));//Valor maximo 21000
   denominador=(sensor[borde_izq]+sensor[med_izq]+sensor[cent_izq]+sensor[cent_der]+sensor[med_der]+sensor[borde_der]); //Valor maximo 6000. 
   //Promedio=21000/6000 = 3.5
   
   promedio=(numerador/denominador); //Promedio ponderador
  
   //Mientras los sensores est� dentro de la linea blanca, el robot va guardar el valor del promedio.
   //Una vez que se sale completamente de la "pista" el robot recuerda el ultimo estado en el que se encontraba.
   //NOTA: Falta ajustar los valores de 1.2 y 5.8 en la vida real
   if((promedio<valor_min)||(promedio>valor_max)) // Dentro de la linea blanca.
   {   
    promedio=promedio_anterior;      //Recuerda el ultimo valor de linea blanca
   }else{
      promedio_anterior=promedio; //Guarda el valor de linea blanca
      curvas=0;
   }
   
   error=(promedio-set_point);   //Calculo el error 

   integral+=error_anterior;  //
   
   derivativo=(error-error_anterior); //Disminuyo las oscilaciones 
   
   pid=((Kp*error)+(kd*derivativo));//+(Ki*integral)); //
   
   error_anterior=error;   //Estado anteropr  
}



void Actualizo_motores()
{
//Correccion  de los PWM de los motores.
   if(pid>0)   //Si el valor del PID es mayor a 0 corrijo el duty del motor Izq. 
   {
      pwm=pid*ganancia_pwm; 
      pwm_m_izq=(duty-pwm);   
      set_pwm2_duty((INT16)pwm_m_izq); 
      set_pwm1_duty((INT16)duty);  
   }
   else if(pid<0)   //Si el valor del PID es menor a 0 corrijo el duty del motor Der. 
   {
      pid = ((-1)*pid);        // Cambio el signo para ajustar el rango
      pwm = (pid*ganancia_pwm);
      pwm_m_der = (duty-pwm);
      set_pwm1_duty((INT16)pwm_m_der);
      set_pwm2_duty((INT16)duty);
   }
}

void mostrar_pantalla()
{
   printf("SBI= "); //Sensor Borde Derecho
   mostrarINT32(Sensor[borde_izq]);
   putc(' ');
   printf("SMI= "); //Sensor Medio Derecho
   mostrarINT32(Sensor[med_izq]);
   putc(' ');
   printf("SCI= "); //Sensor Centro Derecho 
   mostrarINT32(Sensor[cent_izq]);
   putc(' ');
   printf("SCD= "); //Sensor Centro Izquierdo
   mostrarINT32(Sensor[cent_der]);
   putc(' ');
   printf("SMD= "); //Sesor Medio Izquierdo
   mostrarINT32(Sensor[med_der]);
   putc(' ');
   printf("SBD= "); //Sensor Borde Izquierdo
   mostrarINT32(Sensor[borde_der]); 
   putc(' ');
   printf("izq= ");
   mostrarINT32(pwm_m_izq);
   putc(' ');
   printf("der= ");
   mostrarINT32(pwm_m_der);
    putc(' ');
   printf("error= ");
   mostrarFLOAT(error);
   putc(' ');
   printf("prom");
   mostrarFLOAT(promedio);
   putc(13);
   putc(10);   
}

Void main()
{
     
     setup_uart(9600);   
     setup_adc_ports(AN0_TO_AN8);
     setup_adc(ADC_CLOCK_DIV_2|ADC_TAD_MUL_0);

     setup_timer_2(T2_DIV_BY_16,255,1);      //819 us overflow, 819 us interrupt
     setup_ccp1(CCP_PWM);
     setup_ccp2(CCP_PWM);
     set_pwm1_duty((int16)0); 
     set_pwm2_duty((int16)0); 
     setup_timer_0(RTCC_INTERNAL|RTCC_DIV_32|RTCC_8_bit);  //1,6 ms overflow
     enable_interrupts(INT_TIMER0);  //Habilito las interrupcion por TMR0
     enable_interrupts(GLOBAL);  //Habilito las interrupciones globales
     output_high(PIN_B4);
     for(puntero=0;puntero<6;puntero++)
     {
      Sensor_valor_max[puntero]=500;  //
      Sensor_valor_min[puntero]=500;
     }
  while(true)
  {
   lectura_de_sensores();  //Lectura de los sensores Opticos
   if(input(pulsador_start)==0b0)
   {
      en_carrera=1; //
      output_low(PIN_B4);     
   }
   while(en_carrera) //Bucle infinitow
   {
      lectura_de_sensores();//Lectura de los sensores Opticos.
      control_pid(2,1); //kp,kd,ki
      actualizo_motores();  //actualizo las salidas de los motores.
      mostrar_pantalla();  
   }
  }
}
