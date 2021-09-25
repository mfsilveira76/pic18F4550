/*
 * File:   newmain.c
 * Author: Marcelo Silveira
 *
 * Created on 5 de Julho de 2021, 03:23
 */

#include <xc.h>
#include <stdio.h>
#include <xc8debug.h>
#include <pic18f4550.h>
/*
 * FUSES
 */
#pragma config FOSC = HS        // Cristal de alta velocidade
#pragma config WDT = OFF        // Habilitação do Watchdog Timer
#pragma config MCLRE = ON       // Utilização do pino 1 (RE3) para Master Reset

/**
 *  Definições fixas 
 * 
 */

#define _XTAL_FREQ 8000000

/* Definições para o LCD */
#define RS      PORTEbits.RE2   // Configura recepção de dado ou instrução
#define ENABLE  PORTEbits.RE1   // Habilitação do LCD
#define LCD     PORTD           // Barramento de dados do LCD

/* Definições para os Botões de Entrada */
#define btnUp       PORTBbits.RB2
#define btnDown     PORTBbits.RB0
#define btnEnter    PORTBbits.RB1

/* Definições para os displays 7seg */
#define Disp1 PORTAbits.RA2
#define Disp2 PORTAbits.RA3
#define Disp3 PORTAbits.RA4
#define Disp4 PORTAbits.RA5





/**
 * Debouncing;. 
 * @param INST
 */

/** 
 *  MIDI 
 *  Primeira Oitava
 *  
 *      Nota|Freq 
 */
#define C   523
#define CS  554
#define D   587
#define DS  622
#define E   659
#define F   698
#define FS  740
#define G   784
#define GS  830
#define A   880
#define AS  932
#define B   987

#define C2  C*2
#define C2S CS*2
#define D2  D*2
#define D2S DS *2
#define E2  E*2
#define F2  F*2
#define F2S FS *2
#define G2  G*2
#define G2S GS *2
#define A2  A*2
#define A2S AS *2
#define B2  B*2

/* SEM SOM */
#define v   125000

#define DEBOUNCE_ENTER  while(btnEnter == 0 );
#define DEBOUNCE_DOWN   while(btnDown == 0 );
#define DEBOUNCE_UP     while(btnUp == 0 );

void Delay_ms(unsigned int milisegundos);

void ConfiguraLCD4(unsigned char INST);
void InicializaLCD4(void);
void EscreveLCD4(unsigned char DADO);
void EscreveLinhaLCD4(unsigned char LINHA, char *FRASE);
void Escreve7Seg(unsigned char DISPLAY, unsigned char DADO, unsigned char PONTO);

short capturaUpDown(short opt);

char fimTimer( void ) {
    return INTCON &= 1 << 2; // Testa o bit 2
 }

void aguardaTimer( void ) {
    while ( ! ( INTCON & 1 << 2 ) );
 }
void resetaTimer( unsigned int tempo ) {
    unsigned ciclos = tempo * 2 ;   // para 8MHz 1ms = 2 ciclos          
    ciclos = 65535-ciclos;          // overflow     
    ciclos -= 14;                   // Subtrai tempo do overhead    
    TMR0H = ( ciclos >> 8 ) ;       // Salva  parte alta    
    TMR0L = ( ciclos & 0x00FF ) ;   // Salva parte baixa
    INTCON &= ~(1 << 2);            // Limpar flat de overflow.
 }
void inicializaTimer( void ) {
    T0CON = 0b00001000 ;               // configura o preescaler 
    T0CON |= 1<<7 ;           // liga o timer         
 }
void setaPWM1(unsigned char porcento){

    // Formaula do duty cicle:  y c l e :
    // DC porcento = V/((PR2+1)*4;                      
    // V=DC/100 * (PR2+1) * 4 = DC * (PR2+1) /25

    unsigned int val = ( ( unsigned int ) porcento )*( PR2+1) ;
    val = val / 25;

    // garante que tem a apenas 10 bits                                        
    val &= 0x03ff ;

    // os 8 primeiros btis são colocados no CCPR1L 
    CCPR1L = val >> 2 ;

    // os últimos dois são colocados na pocisão 5 e 4 do ccp1con 
    CCP1CON |= ( val & 0x0003 ) << 4 ;
 }
void setaPWM2(unsigned char porcento){
    unsigned int val = porcento * PR2;      
    val /= 25;
    val &= 0x03ff ;
    CCPR2L = val >> 2 ;
    CCP2CON |= ( val & 0x0003 ) << 4 ;
  }
void setaFreqPWM (unsigned int freq){
    //PR2 = fosc /( fpwm*4* preescaler)?1 = ( 8000000/ ( freq*4*16 )) ? 1
    PR2 = (125000/(freq))-1;
}
void inicializaPWM(void){
    TRISCbits.RC1 = 0; // &= ~(1 << 1); // Configura os pinos como saída
    TRISCbits.RC2 = 0; // &= ~(1 << 1); // Configura os pinos como saída
    TRISCbits.RC6 = 0; // &= ~(1 << 1); // Configura os pinos como saída
  
    
    //TRISC &= ~(1 << 4);
    T2CON   |= 0b00000011 ;       // Configura os preescaler do timer2 para 1:16 

    T2CON   |=  1<< 2;            // Liga o timer 2        
    CCP1CON |= 0b00001100 ;     // Configura CCP1 como um PWM        
    CCP2CON |= 0b00001100 ;     // Configura CCP2 como um PWM   
}
void testarDisplay1(void){
    
    unsigned char idx = 0;
  
    unsigned char displays[5] = {
                                0b00000000, 
                                0b00000100, 
                                0b00001000,
                                0b00010000, 
                                0b00100000
                                };
    
    EscreveLinhaLCD4(1, "Teste 8888      ");
    EscreveLinhaLCD4(2, "Aguarde...      ");
    __delay_ms(1000);
    
    TRISA  = 0b00000000;
    
    PORTD = 0b01111111; // 8 (oito)
    
    Disp1 = 1;
    Disp2 = 1;
    Disp3 = 1;
    Disp4 = 1;
    
    __delay_ms(3000);
    
    Disp1 = 0;
    Disp2 = 0;
    Disp3 = 0;
    Disp4 = 0;
    
    EscreveLinhaLCD4(1, "Teste 8888      ");
    EscreveLinhaLCD4(2, "Pronto....      ");
    __delay_ms(1000);
    
    
}
void testarDisplay2(void){
    
    /* Mensagem ao Usuario */
    
    EscreveLinhaLCD4(1, "Display em teste");
    EscreveLinhaLCD4(2, "RB1 para voltar ");
    __delay_ms(1000);
    
   
    /* TODO: Salvar os estados atuais das portas */
    
    /* Configurando as saidas */
    PORTA  = 0b00000000;    // Limpa PORTA
    PORTD  = 0b00000000;    // Limpa PORTD
    TRISA  = 0b00000000;    // Entradas: Nenhum     Sa?das: RA0 a RA6
    TRISD  = 0b00000000;    // Entradas: Nenhum     Sa?das: RD0 a RD7
    
    ADCON1 = 0b00001111;    // Desabilita m?dulo AD: necess?rio para usar pinos
                            // anal?gicos como digitais
    const char sconta[16]; 
    unsigned int conta,aux;
    unsigned char unidade, dezena, centena, milhar;

    unsigned int t1,t2,ct;
    
    
    conta =9999;
    
    do{
        
       
        milhar =   conta/1000;
        aux = conta%1000;
        centena =  (aux)/100;
        aux = aux%100;
        dezena =   (aux)/10;
        unidade =  conta%10;
         
        // deve atrasar muito. 
        sprintf(sconta, "De %0000d a  0    ",conta);
        
        //Escreve7Seg(5,unidade,0); //desliga displays;
        EscreveLinhaLCD4(2,sconta);
        ct = 0;
      
        //Delay_ms(5);
       // while (ct < 1)
        {
            Escreve7Seg(1, milhar, 0);   __delay_ms(18);
            Escreve7Seg(2, centena,0);   __delay_ms(18);
            Escreve7Seg(3, dezena ,0);   __delay_ms(18);       
            Escreve7Seg(4, unidade,0);   __delay_ms(18);   
            Disp4 = 0; // desligar senao fica muito tempo
            
            ct++;
        }     
        // __delay_ms(50);
        conta--;
        if (conta == 0) conta = 9999;

    } while( btnEnter != 0 );
    DEBOUNCE_ENTER;
}
void testarRelays(unsigned char relay){
    //rc0 e re0
    
    const char msg1[16];
    const char msg2[16];
    
    TRISC = 0x00;
    TRISE = 0x00;
    
    PORTC = 0x00;
    PORTE = 0x00;
    
    sprintf(msg2,"RB2: ON RB0: OFF");
            
    while(btnEnter != 0){
        switch(relay){
            case 1:
                if(btnUp == 0){
                    PORTCbits.RC0 = 1;
                    DEBOUNCE_UP
                    sprintf(msg1,"Relay %d ON ",relay);
                }else if (btnDown == 0){
                    PORTCbits.RC0 = 0;
                    DEBOUNCE_DOWN
                    sprintf(msg1,"Relay %d OFF",relay);
                }break;
            case 2:
                if(btnUp == 0){
                    PORTEbits.RE0 = 1;
                    DEBOUNCE_UP
                    sprintf(msg1,"Relay %d ON ",relay);
                }else if (btnDown == 0){
                    PORTEbits.RE0 = 0;
                    DEBOUNCE_DOWN
                    sprintf(msg1,"Relay %d OFF",relay);
                }break;
        }
        EscreveLinhaLCD4(1,msg1);
        EscreveLinhaLCD4(2,msg2);   
    }
    DEBOUNCE_ENTER
}
void testarLCD(void){
    unsigned char idx = 0;
    
    EscreveLinhaLCD4(1,"Teste do LCD    ");
    EscreveLinhaLCD4(2,"RB1 para voltar ");
    
    
    //while (idx < 255 )
    // TODO: melhorar esse teste
    {
        ConfiguraLCD4(0x80); // linha 1; 
        for(int l = 0; l <16; l++){
            EscreveLCD4(0xFF);  
        }
        ConfiguraLCD4(0xC0); // linha 2; 
        for(int l = 0; l <16; l++){
            EscreveLCD4(0xFF);  
        }
    }
    __delay_ms(2000);
} 
void testarBuzzerPWM(unsigned char pwm){
    char msg[16];
    inicializaPWM();
    inicializaTimer();
    setaFreqPWM(5000);
    setaPWM2(pwm);
    sprintf(msg,">PWM %d%%  ON    ",pwm);
    EscreveLinhaLCD4(1,msg);
    while(btnEnter!=0);
    //if (btnEnter == 0 ){
        //sprintf(msg,">PWM %d%%  OFF  ");
        //EscreveLinhaLCD4(1,msg);
        setaPWM2(0);
        DEBOUNCE_ENTER
    //}
    
    
}
void testarCoolerPot(unsigned char pot){
       
    char msg[16];
    unsigned short vTemp = 0; 
    unsigned short vaux  = 0;
    
    unsigned char DD1, DD2, DD3, DD4;
    unsigned short t, opcao;
      
    sprintf(msg,"Potenc.  %d      ",pot+1);
    EscreveLinhaLCD4(1,msg);
    EscreveLinhaLCD4(2,"RB1 p/VOLTAR     ");
    
    
    opcao = pot; // seleciona AD.
    
    PORTA = 0b00000000; // Limpa PORTA
    PORTD = 0b00000000; // Limpa PORTB
    
    TRISA = 0b00000011; // Entradas: RA0 a RA1
    TRISD = 0b00000000; // Entradas: Nenhuma
    
    // Inicialização do módulo A/D
    ADCON2 = 0b10000001; // Resultado justificado à direita, 0 Tad, Fosc/8
    ADCON1 = 0b00001011; // Tensões de referência padrão, 4 entradas analógicas
    ADCON0 = 0b00000001; // Seleciona o canal 0 e ativa o módulo A/D
    
    inicializaTimer();
    inicializaPWM();
    setaFreqPWM(1000);
    
    __delay_ms(10);
    while(1){
        ADCON0bits.CHS1 = 0;        // Seleciona o canal AN0
        ADCON0bits.CHS0 = opcao;
        ADCON0bits.GO_DONE = 1;     // Inicia a conversão
        
        while(ADCON0bits.GO_DONE);      // Aguarda término da conversão
      
        //__delay_ms(10);{ necessário ? }  
        vTemp = ADRES;             // Lê resultado da conversão AD
        vTemp = (vTemp*0.09765625); /* 100/1023 */
               
        setaPWM1((unsigned char) vTemp);
        
        DD1 =   (vTemp)/100;
        vaux =  vTemp%100;
        DD2 =   (vaux)/10;
        DD3 =   vTemp%10;
        DD4 =   253;
        
        t = 0;
        while(t < 2){
            Escreve7Seg(4, DD4, 0); __delay_ms(4); // Apresentação do dígito da unidade
            Escreve7Seg(3, DD3, 0); __delay_ms(4); // Apresentação do dígito da centena
            Escreve7Seg(2, DD2, 0); __delay_ms(4); // Apresentação do dígito da dezena
            Escreve7Seg(1, DD1, 0); __delay_ms(4); // Apresentação do dígito da unidade
            t = t+1;
        }
        if (btnEnter == 0){
            DEBOUNCE_ENTER;
            break;
        }
    }; 
}
void testarCoolerPotUpDown(void){
       
    char msg[16];
    
    unsigned short vaux  = 0;
    
    unsigned char DD1, DD2, DD3, DD4;
    unsigned short t, opcao;
    short vlrUpDown = 1;
    
    
    PORTA = 0b00000000; // Limpa PORTA
    PORTD = 0b00000000; // Limpa PORTB

    TRISA = 0b00000000; //ntradas: RA0 a RA1
    TRISD = 0b00000000; // Entradas: Nenhuma
    
    inicializaTimer();
    inicializaPWM();
    setaFreqPWM(1000);
    
    __delay_ms(10);
    while(1){
        
        
          sprintf(msg,"Potenc.  %d%%    ",vlrUpDown);
          EscreveLinhaLCD4(1,msg);
          EscreveLinhaLCD4(2,"RB1 p/VOLTAR     ");
          Escreve7Seg(5, 0, 0);
               
        setaPWM1((unsigned char) vlrUpDown);
        
        DD1 =   (vlrUpDown)/100;
        vaux =  vlrUpDown%100;
        DD2 =   (vaux)/10;
        DD3 =   vlrUpDown%10;
        DD4 =   253;
        
        //__delay_ms(10);
        t = 0;
        while(t < 2){
            Escreve7Seg(4, DD4, 1); __delay_ms(8); // Apresentação do dígito da unidade
            Escreve7Seg(3, DD3, 0); __delay_ms(8); // Apresentação do dígito da centena
            Escreve7Seg(2, DD2, 0); __delay_ms(8); // Apresentação do dígito da dezena
            Escreve7Seg(1, DD1, 0); __delay_ms(8); // Apresentação do dígito da unidade
            Escreve7Seg(7, 0, 0);// força o apagamento do display
            t = t+1;
        }
        if (vlrUpDown >0 && btnDown ==0 ){
            DEBOUNCE_DOWN
            vlrUpDown--;
        }else if (vlrUpDown <100 && btnUp == 0){
            DEBOUNCE_UP
            vlrUpDown++;
        }
        if (btnEnter == 0){
            DEBOUNCE_ENTER;
            break;
        }
    }
    setaPWM1(0);
}
void miniMidi(void){
    /* Inicialização */
    unsigned char cont=0;
    unsigned char pos =0;
    EscreveLinhaLCD4(1,"MIDI on          ");
    EscreveLinhaLCD4(2,"RB1 p/VOLTAR     ");
    // Imperial March (Guerra nas Estrelas Episodio V)
    unsigned char tempo [] ={
                            50,10,50,10,50,10,50,5,25,5,50,5,50,5,25,5,50,50,50,
                            10,50,10,50,10,50,5,25,5,50,5,50,5,25,5,50,50,100,5,
                            25,5,25,10,100,5,50,5,25,2,10,2,10,2,100,250
                            };
 
    unsigned int  notas [] ={
                            G,v,G,v,G,v,E,v,B,v,G,v,E,v,B,v,G,v,D2S,v,D2S,v,
                            D2S,v,E2,v,B,v,FS,v,E,v,B,v,G,v,G2S,v,G,v,G,v,G2S,v,
                            G2,v,F2S,v,F2,v,E2,v,F2S,v
                            };  
 
    
    // configura o timer 
    T0CON = 0b00001000 ; 
    T0CON |= 1 << 7;        // seta o bit 
 
    inicializaPWM();
    inicializaTimer();
    
    setaFreqPWM(notas[0]);
    setaPWM2(50);
    
    while(btnEnter != 0){
        aguardaTimer ( ) ;
        resetaTimer (10000) ;
        cont++;
        if( cont >= tempo [ pos ] ){
            if ( pos <54){
                pos++;
            }
        setaFreqPWM ( notas [ pos ] ) ;
        setaPWM2 ( 50 ) ;
        cont  =0;
        }
    }
    /**
     * fazer o debouncing senao ele acaba 
     * reentrando na funcao
     */
    DEBOUNCE_ENTER
    setaFreqPWM(v);
    setaPWM2(0);
  
    
}
short capturaUpDown(short opt){
    
    if(btnDown == 0 ){
        opt++;  DEBOUNCE_DOWN
    } else  if(btnUp == 0){
       opt--;   DEBOUNCE_UP
    }  
      
   return opt;
}
short subMenuDisplay7Seg(void){
    int vopc = 1;
    while (1){
        vopc = capturaUpDown(vopc);
        switch(vopc){
            case 1:
                EscreveLinhaLCD4(1, ">Teste 8888     "); // Escreve texto no LCD usando função de linha inteira
                EscreveLinhaLCD4(2, " Contar 9999->0 ");
                if (btnEnter == 0){
                    DEBOUNCE_ENTER
                    testarDisplay1();
                }
                break;
            case 2:
                 EscreveLinhaLCD4(1, ">Contar 9999->0 ");
                 EscreveLinhaLCD4(2, " Voltar         "); // Escreve texto no LCD usando função de linha inteira
                 if (btnEnter == 0 ) {                         
                    testarDisplay2();
                    DEBOUNCE_ENTER
                 }
                 break;
            case 3:
                EscreveLinhaLCD4(1, ">Voltar         ");
                EscreveLinhaLCD4(2, "________________");
                if (btnEnter == 0 ){
                    
                    DEBOUNCE_ENTER
                    return 1;
                }
                break;
             default :
                 vopc = 1;
                
        }
    }
}
short subMenuRelays(void){
    int vopc = 1;
    while (1){
        vopc = capturaUpDown(vopc);
        switch(vopc){
            case 1:
                EscreveLinhaLCD4(1, ">Relay 1        "); // Escreve texto no LCD usando função de linha inteira
                EscreveLinhaLCD4(2, " Relay 2        ");
                if (btnEnter == 0){
                    DEBOUNCE_ENTER
                    testarRelays(1);
                }
                break;
            case 2:
                 EscreveLinhaLCD4(1, ">Relay 2        ");
                 EscreveLinhaLCD4(2, " Voltar         "); // Escreve texto no LCD usando função de linha inteira
                  if (btnEnter == 0 ){
                     DEBOUNCE_ENTER
                      testarRelays(2); 
                  }
                 break;
            case 3:
                EscreveLinhaLCD4(1, ">Voltar         ");
                EscreveLinhaLCD4(2, "________________");
                if (btnEnter == 0 ){
                   
                    DEBOUNCE_ENTER
                    return 1;
                }
                break;
             default :
                 vopc = 1;
                
        }
    }
    
}
short subMenuBuzzer(void){
    int vopc = 1;
    while (1){
        vopc = capturaUpDown(vopc);
        switch(vopc){
            case 1:
                EscreveLinhaLCD4(1, ">Mini MIDI      "); // Escreve texto no LCD usando função de linha inteira
                EscreveLinhaLCD4(2, " PWM 25%        ");
                if (btnEnter == 0 ){
                    DEBOUNCE_ENTER
                    miniMidi();
                }
                break;
            case 2:
                 EscreveLinhaLCD4(1, ">PWM 25%  Off   ");
                 EscreveLinhaLCD4(2, " PWM 50%  Off   "); // Escreve texto no LCD usando função de linha inteira
                 if (btnEnter == 0 ){
                     DEBOUNCE_ENTER
                     testarBuzzerPWM(25);
                 }
                 break;
            case 3:
                 EscreveLinhaLCD4(1, ">PWM 50%  Off   ");
                 EscreveLinhaLCD4(2, " PWM 75%  Off   "); // Escreve texto no LCD usando função de linha inteira
                  if (btnEnter == 0 ){
                     DEBOUNCE_ENTER
                     testarBuzzerPWM(50);
                 }
                 break;
            case 4:
                 EscreveLinhaLCD4(1, ">PWM 75%  Off   ");
                 EscreveLinhaLCD4(2, " PWM 100% Off   "); // Escreve texto no LCD usando função de linha inteira
                  if (btnEnter == 0 ){
                     DEBOUNCE_ENTER
                     testarBuzzerPWM(75);
                 }
                 break;
            case 5:
                 EscreveLinhaLCD4(1, ">PWM 100% Off   ");
                 EscreveLinhaLCD4(2, " Voltar         "); // Escreve texto no LCD usando função de linha inteira
                  if (btnEnter == 0 ){
                     DEBOUNCE_ENTER
                     testarBuzzerPWM(100);
                 }
                 break;
            case 6:
                EscreveLinhaLCD4(1, ">Voltar         ");
                EscreveLinhaLCD4(2, "________________");
                if (btnEnter == 0 ){
                    
                    DEBOUNCE_ENTER
                    return 1;
                }
                break;
             default :
                 vopc = 1;
                
        }
    }
}
short subMenuLCD(void){
    int vopc = 1;
    while (1){
        vopc = capturaUpDown(vopc);
        switch(vopc){
            case 1:
                EscreveLinhaLCD4(1, ">Iniciar Teste  "); //D usando função de linha inteira
                EscreveLinhaLCD4(2, " Voltar         ");
                if (btnEnter == 0){
                    DEBOUNCE_ENTER
                   testarLCD();
                }
                break;
            case 2:
                EscreveLinhaLCD4(1, ">Voltar         ");
                EscreveLinhaLCD4(2, "________________");
                if (btnEnter == 0 ){
                    
                    DEBOUNCE_ENTER
                    return 1;
                }
                break;
             default :
                 vopc = 1;
                
        }
    }
}
short subMenuCooler (void){
    int vopc = 1;
    while (1){
        vopc = capturaUpDown(vopc);
        switch(vopc){
            case 1:
                EscreveLinhaLCD4(1, ">Potenciometro 1"); // Escreve texto no LCD usando função de linha inteira
                EscreveLinhaLCD4(2, " Potenciometro 2");
                if (btnEnter == 0 ){
                    DEBOUNCE_ENTER
                    testarCoolerPot(0);
                }
                break;
            case 2:
                 EscreveLinhaLCD4(1, ">Potenciometro 2");
                 EscreveLinhaLCD4(2, " RB2+ RB0-      "); // Escreve texto no LCD usando função de linha inteira
                 if (btnEnter == 0){
                     DEBOUNCE_ENTER
                     testarCoolerPot(1);
                 }
                 break;
            case 3:
                 EscreveLinhaLCD4(1, ">RB2- RB0+      ");
                 EscreveLinhaLCD4(2, " Voltar         "); // Escreve texto no LCD usando função de linha inteira
                 if(btnEnter==0){
                     DEBOUNCE_ENTER
                     testarCoolerPotUpDown();
                 }
                 break;
            case 4:
                EscreveLinhaLCD4(1, ">Voltar          ");
                EscreveLinhaLCD4(2, "_________________");
                if (btnEnter == 0 ){
                    
                    DEBOUNCE_ENTER
                    return 1;
                }
                break;
             default :
                 vopc = 1;
                
        }
    }
    
}
void showSobre(void ){
    
    char sobre[7][17] ={{"IHM Versao 1.00 \0"},
                        {"* 05/07/2021 *  \0"},
                        {"Desenvolvedor:  \0"},
                        {"Marcelo Silveira\0"},
                        {"IFRS - Tecnico  \0"},
                        {"Eletronica      \0"},
                        {"Turma 2019/1    \0"},
                        {"                \0"}
                     };
    unsigned short idx = 0;
    while(idx < 6){
        
        //EscreveLinhaLCD4(1,"                ");
        EscreveLinhaLCD4(1,&sobre[idx][0]);
        //EscreveLinhaLCD4(2,"                ");
        EscreveLinhaLCD4(2,&sobre[idx+1][0]);
        Delay_ms(3000);
        //while(btnEnter!=0);
        if (++idx == 7){
            idx = 0;
        } 
        if (btnEnter == 0){
            idx = 6  ; DEBOUNCE_ENTER
        }   
    }  
}
void InicializaPIC(void){
    
    PORTB  = 0b00000000;    // Limpa PORTB
    PORTD  = 0b00000000;    // Limpa PORTD
    PORTE  = 0b00000000;    // Limpa PORTE
    TRISB  = 0b00011111;    // Entradas: RB0 a RB4  Saídas: RB5 a RB7
    TRISD  = 0b00000000;    // Entradas: Nenhum     Saídas: RD0 a RD7
    TRISE  = 0b00000000;    // Entradas: Nenhum     Saídas: RE0 a RE3
    
    ADCON1 = 0b00001111;    // Desabilita módulo A/D
    
    InicializaLCD4();            // Inicialização do LCD
}
void main(void) {
    InicializaPIC();
    
    short opcao =1 ;
    
    EscreveLinhaLCD4(1, "   ELETRONICA   "); // Escreve texto no LCD usando função de linha inteira
    EscreveLinhaLCD4(2, "   IFRS         ");
    Delay_ms(700);
    
    EscreveLinhaLCD4(1, " BEM VINDO AO    "); // Escreve texto no LCD usando função de linha inteira
    EscreveLinhaLCD4(2, " SISTEMA         ");
    Delay_ms(700);
    
    while(1){
    
        if(btnDown == 0 ){
            opcao++;
            while(btnDown==0);
        } else  if(btnUp == 0){
            opcao--;
            while(btnUp==0);
        }
        
        switch(opcao){
            case 1:        
                EscreveLinhaLCD4(1, ">Display 7 Seg  ");
                EscreveLinhaLCD4(2, " LCD            ");        
                if (btnEnter==0 ){            
                    DEBOUNCE_ENTER
                    subMenuDisplay7Seg();
                }
                
                break;
            case 2: 
                EscreveLinhaLCD4(1, ">LCD            ");
                EscreveLinhaLCD4(2, " Buzzer         ");
                if (btnEnter==0 ){
                    DEBOUNCE_ENTER
                    subMenuLCD();
                }
               
                break;
             case 3:
                EscreveLinhaLCD4(1, ">Buzzer         ");
                EscreveLinhaLCD4(2, " Cooler         ");
                if (btnEnter==0 ){                   
                    DEBOUNCE_ENTER
                    subMenuBuzzer();
                }
                break;    
             case 4:
                EscreveLinhaLCD4(1, ">Cooler         ");
                EscreveLinhaLCD4(2, " Relays         ");
                 if (btnEnter==0 ){
                    DEBOUNCE_ENTER
                    subMenuCooler();
                }
                break;
             case 5:
                EscreveLinhaLCD4(1, ">Relays         ");
                EscreveLinhaLCD4(2, " Sobre          ");
                 if (btnEnter==0 ){ 
                    DEBOUNCE_ENTER
                    subMenuRelays();
                }
                break;
            case  6:
                EscreveLinhaLCD4(1, ">Sobre          ");
                EscreveLinhaLCD4(2, " Display 7 Seg  "); 
                 if (btnEnter==0 ){ 
                   DEBOUNCE_ENTER
                   showSobre();
                }
                
                break;           
            case  7:
                opcao = 1; break;
                
            case 0:
                opcao = 6; break;
        }
    }        
    return ;
}
// Função para realizar a inicialização do LCD
void InicializaLCD4(void){
    //__delay_ms(15);         // aguarda pelo menos 15ms
    __delay_ms(5);
    ConfiguraLCD4(0x30);    // instrução de inicialização
    __delay_ms(5);          // aguarda pelo menos 4.1ms
    ConfiguraLCD4(0x30);    // instrução de inicialização
    __delay_us(100);        // aguarda pelo menos 100us
    ConfiguraLCD4(0x30);    // instrução de inicialização
    ConfiguraLCD4(0x02);	// define interface de 4 bits
    ConfiguraLCD4(0x28);	// define interface de 4 bits e display de 2 linhas
    ConfiguraLCD4(0x06);    // modo de entrada: desloca cursor para direita sem deslocar a mensagem
    ConfiguraLCD4(0x0C);    // desliga cursor
    ConfiguraLCD4(0x01);    // limpa display
}
// Função para enviar instruções de configuração para o LCD
void ConfiguraLCD4(unsigned char INST){
    RS = 0;             // seleciona o envio de uma instrução
    LCD = (LCD & 0x0f)|(0xF0 & INST); // coloca parte alta da instrução no barramento de dados
    ENABLE = 1;         // habilita o display
    __delay_us(10);     // espera 10us
    ENABLE = 0;         // desabilita o display
        __delay_ms(5);      // espera 5ms
    LCD = (LCD & 0x0f)|(INST<<4);    // coloca parte baixa da instrução no barramento de dados
    ENABLE = 1;         // habilita o display
    __delay_us(10);     // espera 10us
    ENABLE = 0;         // desabilita o display
    __delay_ms(5);      // espera 5ms
}
// Função para escrever um caractere no LCD
void EscreveLCD4(unsigned char DADO){
    RS = 1;             // seleciona o envio de um caractere
    LCD = (LCD & 0x0f)|(0xF0 & DADO);  // coloca parte alta do caractere no barramento de dados
    ENABLE = 1;         // habilita o display
    __delay_us(10);     // espera 10us
    ENABLE = 0;         // desabilita o display
    __delay_us(100);     // espera 100us
    LCD = (LCD & 0x0f)|(DADO<<4);      // coloca parte baixa do caractere no barramento de dados
    ENABLE = 1;         // habilita o display
    __delay_us(10);     // espera 10us
    ENABLE = 0;         // desabilita o display
    __delay_us(100);     // espera 100us
}
// Função para escrever uma linha completa no LCD
void EscreveLinhaLCD4(unsigned char LINHA, char *FRASE){
    
    if(LINHA == 1) ConfiguraLCD4(0x80);
    if(LINHA == 2) ConfiguraLCD4(0xC0);
    
    // Escreve todos os caracteres até encontrar o final do ponteiro
    while(*FRASE != '\0'){
        EscreveLCD4(*FRASE);
        ++FRASE;
    }
}
void Escreve7Seg(unsigned char DISPLAY, unsigned char DADO, unsigned char PONTO){
    // Display selection
    switch(DISPLAY){
        case 1: Disp1 = 1; Disp2 = 0; Disp3 = 0; Disp4 = 0; break;
        case 2: Disp1 = 0; Disp2 = 1; Disp3 = 0; Disp4 = 0; break;
        case 3: Disp1 = 0; Disp2 = 0; Disp3 = 1; Disp4 = 0; break;
        case 4: Disp1 = 0; Disp2 = 0; Disp3 = 0; Disp4 = 1; break;
        
        default: Disp1 = 0; Disp2 = 0; Disp3 = 0; Disp4 = 0; 
    }
    // Data display
    switch(DADO){
        case 0: PORTD = 0b00111111; break;
        case 1: PORTD = 0b00000110; break;
        case 2: PORTD = 0b01011011; break;
        case 3: PORTD = 0b01001111; break;
        case 4: PORTD = 0b01100110; break;
        case 5: PORTD = 0b01101101; break;
        case 6: PORTD = 0b01111100; break;
        case 7: PORTD = 0b00000111; break;
        case 8: PORTD = 0b01111111; break;
        case 9: PORTD = 0b01100111; break;
        case 253:PORTD = 0b01010010; break;
        case 254:PORTD = 0b00111001; break; /*C */
        case 255:PORTD = 0b01100011; break; /*Bolinha */ 
        default: PORTD = 0b00000000; break;
    }
    // Dot segment display
    if(PONTO == 1) PORTDbits.RD7 = 1;
}
void Delay_ms(unsigned int milisegundos){
   while(milisegundos > 0){
       __delay_ms(1);   // n?o aceita vari?veis
      milisegundos--;
    }
 }