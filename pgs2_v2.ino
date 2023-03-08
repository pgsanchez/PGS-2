// Programa arduino para el funcionamiento del panel de control PGS-2
//
// Versión 2, con librería Joystick y temporizador con la función millis
//
// Solo para arduino Leonardo y Micro, que tiene soporte USB. En el 
// resto de placas arduino, no sé si habría que hacer algo más.
//
// By pgsanchez.net
// 2022
//--------------------------------------------------------------------

#include <Joystick.h>

// Pins de entrada de los interruptores y botones
#define PIN_CANOPY 2
#define PIN_FLAPS_UP 3
#define PIN_FLAPS_LND 4
#define PIN_PARACHUTE 5
#define PIN_NAV_LIGHT 6
#define PIN_JET_FUEL 7
#define PIN_APU 8
#define PIN_JET_WPNS 9
#define PIN_ENGINE 10
#define PIN_EJECT 11
#define PIN_CKPT_LIGHT 12  
#define PIN_AIRBRAKE 13
#define PIN_LNDGEAR A0


// Definición de los botones del joystick virtual
#define BTN_AIRBRAKE_ON 18
#define BTN_AIRBRAKE_OFF 19
#define BTN_PARACHUTE 3
#define BTN_LNDGEAR_DOWN 4
#define BTN_LNDGEAR_UP 5
#define BTN_FLAPS_UP 6
#define BTN_FLAPS_DOWN 7
#define BTN_FLAPS_LND 8
#define BTN_ENGINE_STOP 9
#define BTN_ENGINE_START 10
#define BTN_APU 11
#define BTN_CKPT_LIGHT 12
#define BTN_NAV_LIGHT 13
#define BTN_EJECT 14
#define BTN_JET_FUEL 15
#define BTN_JET_WPNS 16
#define BTN_CANOPY 17

// Create Joystick
Joystick_ Joystick;

// Pins de salida de los LEDS ¿se pueden poner como "defines"?
const int LED_LNDGEAR = A5;
const int LED_LNDRED = A4;
const int LED_FLAPS_LND = A3;
const int LED_FLAPS_DOWN = A2;
const int LED_AIRBRAKE = A1;

const int PRESS_TIME = 100;


void setup() {
  // Inicializar pins de interruptores
  pinMode(PIN_AIRBRAKE, INPUT_PULLUP);
  pinMode(PIN_PARACHUTE, INPUT); // el único que no es PULLUP
  pinMode(PIN_LNDGEAR, INPUT_PULLUP);
  pinMode(PIN_FLAPS_UP, INPUT_PULLUP);
  pinMode(PIN_FLAPS_LND, INPUT_PULLUP);
  pinMode(PIN_ENGINE, INPUT_PULLUP);
  pinMode(PIN_APU, INPUT_PULLUP);
  pinMode(PIN_CKPT_LIGHT, INPUT_PULLUP);
  pinMode(PIN_NAV_LIGHT, INPUT_PULLUP);
  pinMode(PIN_CANOPY, INPUT_PULLUP);
  pinMode(PIN_EJECT, INPUT_PULLUP);
  pinMode(PIN_JET_FUEL, INPUT_PULLUP);
  pinMode(PIN_JET_WPNS, INPUT_PULLUP);
  
  // Inicializar pins de LEDS
  pinMode(LED_LNDGEAR, OUTPUT);
  pinMode(LED_LNDRED, OUTPUT);
  pinMode(LED_FLAPS_DOWN, OUTPUT);
  pinMode(LED_FLAPS_LND, OUTPUT);
  pinMode(LED_AIRBRAKE, OUTPUT);
  
  
  // Initialize Joystick Library
  Joystick.begin();


  // Aquí hay que poner una inicialización de los leds de la pantalla, según como esté el aerofreno, flaps y tren de aterrizaje.
  if(digitalRead(PIN_AIRBRAKE)==LOW){
    digitalWrite(LED_AIRBRAKE, HIGH);
  }
  if(digitalRead(PIN_LNDGEAR)==HIGH){
    digitalWrite(LED_LNDGEAR, HIGH);
  }

  int initFlapsUpState = digitalRead(PIN_FLAPS_UP);
  int initFlapsLndState = digitalRead(PIN_FLAPS_LND);
  if ((initFlapsUpState == LOW)&&(initFlapsLndState == HIGH)){
      // Flaps arriba
      // Apagar todas las luces de flaps
      digitalWrite(LED_FLAPS_DOWN, LOW);
      digitalWrite(LED_FLAPS_LND, LOW);
    } else if((initFlapsUpState == HIGH)&&(initFlapsLndState == HIGH)){
      // Flaps abajo (posición media)
      // Encender luces de flaps abajo
      digitalWrite(LED_FLAPS_DOWN, HIGH);
      digitalWrite(LED_FLAPS_LND, LOW);
    } else{
      // Flaps Landing
      // encender luces de flaps abajo y flaps aterrizaje
      digitalWrite(LED_FLAPS_DOWN, HIGH);
      digitalWrite(LED_FLAPS_LND, HIGH);
    }
}



// Last state of the button
int lastAirBrkState = LOW;
int lastParachuteState = LOW;
int lastLndGearState = HIGH;
int lastFlapsUpState = HIGH;
int lastFlapsLndState = LOW;
int lastEngineState = HIGH;
int lastApuState = HIGH;
int lastCkptLightState = HIGH;
int lastNavLightState = HIGH;
int lastEjectState = HIGH;
int lastJetFuelState = HIGH;
int lastJetWpnsState = HIGH;
int lastCanopyState = HIGH;

// Variables globales
unsigned long millis_anteriores = 0UL;
unsigned long intervalo = 500UL; // Intervalo del parpadeo: 0.5seg
unsigned long tiempoInicioParpadeoLed = 0UL;
unsigned long tiempoParpadeoLed = 3000UL; // Tiempo que quiero que esté parpadeando el led: 3seg
int estado_led_rojo = LOW; // Estado del led rojo. Inicialmente apagado
int estado_tren = 1; // 1 = tren abajo; 0 = tren arriba


void loop() {
//---------------------------------- AIRBRAKE --------------------------------------------//
  // Posición abajo: interruptor abierto --> HIGH --> aerofreno cerrado --> L-CTRL + B
  // Posición arriba: interruptor cerrado --> LOW --> aerofreno abierto --> L-SHIFT + B
  // Se lee el switch del Airbrake
  int airBrkState = digitalRead(PIN_AIRBRAKE);
  // Se comprueba si ha cambiado de estado con respecto al que tenía
  if (airBrkState != lastAirBrkState){
    if(airBrkState == LOW){ // interruptor cerrado
      Joystick.pressButton(BTN_AIRBRAKE_ON);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_AIRBRAKE_ON);
      digitalWrite(LED_AIRBRAKE, HIGH);
    } else{
      Joystick.pressButton(BTN_AIRBRAKE_OFF);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_AIRBRAKE_OFF);
      digitalWrite(LED_AIRBRAKE, LOW);
    }
    
    lastAirBrkState = airBrkState;
  }
  // Fin AIRBRAKE
//----------------------------------------------------------------------------------------//
//---------------------------------- PARACAIDAS ------------------------------------------//
  // Posición abajo: interruptor abierto --> cortar paracaidas (2ª vez) --> P
  // Posición arriba: interruptor cerrado --> abrir paracaidas --> P
  // Se lee el switch del Paracaidas
  int parachuteState = digitalRead(PIN_PARACHUTE);
  // Se comprueba si ha cambiado de estado con respecto al que tenía
  if (parachuteState != lastParachuteState){
    if(parachuteState == HIGH){ // interruptor cerrado
      Joystick.pressButton(BTN_PARACHUTE);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_PARACHUTE);
    } else{
      Joystick.pressButton(BTN_PARACHUTE);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_PARACHUTE);
    }
    
    lastParachuteState = parachuteState;
  }
  // Fin PARACAIDAS
//----------------------------------------------------------------------------------------//
//-------------------------------- TREN DE ATERRIZAJE ------------------------------------//
  // Interruptor en INPUT_PULLUP
  // Posición abajo: interruptor abierto --> tren abajo --> L-SHIFT + G     (HIGH)
  // Posición arriba: interruptor cerrado --> tren arriba --> L-CTRL + G   (LOW)
  // Se lee el switch del Tren de Aterrizaje
  int lndGearState = digitalRead(PIN_LNDGEAR);
  //digitalWrite(LED_LNDRED, lndGearState);
  // Se comprueba si ha cambiado de estado con respecto al que tenía
  if (lndGearState != lastLndGearState){
    if(lndGearState == HIGH){ // interruptor ABIERTO
      Joystick.pressButton(BTN_LNDGEAR_DOWN);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_LNDGEAR_DOWN);
      //tres segundos de parpadeo
      tiempoInicioParpadeoLed = millis();
      estado_tren = 1;
    } else{
      Joystick.pressButton(BTN_LNDGEAR_UP);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_LNDGEAR_UP);
      //tres segundos de parpadeo
      tiempoInicioParpadeoLed = millis();
      estado_tren = 0;
    }
    
    lastLndGearState = lndGearState;
    
  }
  activarParpadeoLed();
  // Fin TREN DE ATERRIZAJE
//----------------------------------------------------------------------------------------//

//------------------------------------------------------------------------------//
//----------------------------------- FLAPS ------------------------------------//
/* Interruptor de 3 posiciones:
   *  Posición arriba: flaps arriba -> L-Ctrl + F
   *  Posición media: flaps abajo -> F
   *  Posición abajo: flaps aterrizaje -> L-Shift + F
   *  Se utilizan 2 entradas para Arduino más la posición de OFF del interruptor, 
   *  lo que hacen 3 botones virtuales.
   */
   int flapsUpState = digitalRead(PIN_FLAPS_UP);
   int flapsLndState = digitalRead(PIN_FLAPS_LND);
   
  // Se comprueba si ha cambiado de estado con respecto al que tenía
  if ((flapsUpState != lastFlapsUpState)||(flapsLndState != lastFlapsLndState)){
    if ((flapsUpState == LOW)&&(flapsLndState == HIGH)){
      // Flaps arriba
      Joystick.pressButton(BTN_FLAPS_UP);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_FLAPS_UP);
      // Apagar todas las luces de flaps
      digitalWrite(LED_FLAPS_DOWN, LOW);
      digitalWrite(LED_FLAPS_LND, LOW);
    } else if((flapsUpState == HIGH)&&(flapsLndState == HIGH)){
      // Flaps abajo (posición media)
      Joystick.pressButton(BTN_FLAPS_DOWN);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_FLAPS_DOWN);
      // Encender luces de flaps abajo
      digitalWrite(LED_FLAPS_DOWN, HIGH);
      digitalWrite(LED_FLAPS_LND, LOW);
    } else{
      // Flaps Landing
      Joystick.pressButton(BTN_FLAPS_LND);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_FLAPS_LND);
      // encender luces de flaps abajo y flaps aterrizaje
      digitalWrite(LED_FLAPS_DOWN, HIGH);
      digitalWrite(LED_FLAPS_LND, HIGH);
    }
    lastFlapsUpState = flapsUpState;
    lastFlapsLndState = flapsLndState;
  }
// FIN FLAPS
//------------------------------------------------------------------------------//
//--------------------------------- MOTORES ------------------------------------//
  /*
  * Posición arriba: interruptor cerrado (LOW) --> Encender motores --> R-Shift + Inicio
  * Posición abajo: interruptor abierto (HIGH) --> Apagar motores --> R-Shift + Fin
  */
  // Se lee el switch del motor
  int engineState = digitalRead(PIN_ENGINE);
  // Se comprueba si ha cambiado de estado con respecto al que tenía
  if (engineState != lastEngineState){
    if(engineState == HIGH){ // interruptor ABIERTO
      Joystick.pressButton(BTN_ENGINE_STOP);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_ENGINE_STOP);
    } else{
      Joystick.pressButton(BTN_ENGINE_START);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_ENGINE_START);
    }
    
    lastEngineState = engineState;
  }
// FIN MOTORES
//------------------------------------------------------------------------------//
//----------------------------------- APU --------------------------------------//
  
  // Se lee el switch de la APU
  int apuState = digitalRead(PIN_APU);
  // Se comprueba si ha cambiado de estado con respecto al que tenía
  if (apuState != lastApuState){
    if (apuState == LOW){
      Joystick.pressButton(BTN_APU);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_APU);
    }

    lastApuState = apuState;
  }
// FIN APU
//------------------------------------------------------------------------------//
//------------------------------ COCKPIT LIGHT ---------------------------------//
 
  // Se lee el switch del COCKPIT LIGHT
  int ckptLightState = digitalRead(PIN_CKPT_LIGHT);
  // Se comprueba si ha cambiado de estado con respecto al que tenía
  if (ckptLightState != lastCkptLightState){
    if (ckptLightState == LOW){
      Joystick.pressButton(BTN_CKPT_LIGHT);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_CKPT_LIGHT);
    }

    lastCkptLightState = ckptLightState;
  }
// FIN COCKPIT LIGHT
//------------------------------------------------------------------------------//
//-------------------------------- NAV LIGHT -----------------------------------//
  
  // Se lee el switch del NAV LIGHT
  int navLightState = digitalRead(PIN_NAV_LIGHT);
  // Se comprueba si ha cambiado de estado con respecto al que tenía
  if (navLightState != lastNavLightState){
    if (navLightState == LOW){
      Joystick.pressButton(BTN_NAV_LIGHT);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_NAV_LIGHT);
    }

    lastNavLightState = navLightState;
  }
// FIN NAV LIGHT
//------------------------------------------------------------------------------//
//--------------------------------- CANOPY -------------------------------------//
  
  // Se lee el switch del CANOPY
  int canopyState = digitalRead(PIN_CANOPY);
  // Se comprueba si ha cambiado de estado con respecto al que tenía
  if (canopyState != lastCanopyState){
    if (canopyState == LOW){
      Joystick.pressButton(BTN_CANOPY);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_CANOPY);
    }

    lastCanopyState = canopyState;
  }
// FIN CANOPY
//------------------------------------------------------------------------------//
//---------------------------------- EJECT -------------------------------------//
  
  // Se lee el switch del EJECT
  int ejectState = digitalRead(PIN_EJECT);
  // Se comprueba si ha cambiado de estado con respecto al que tenía
  if (ejectState != lastEjectState){
    if (ejectState == LOW){
      Joystick.pressButton(BTN_EJECT);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_EJECT);
      delay(PRESS_TIME);

      Joystick.pressButton(BTN_EJECT);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_EJECT);
      delay(PRESS_TIME);

      Joystick.pressButton(BTN_EJECT);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_EJECT);
    }

    lastEjectState = ejectState;
  }
// FIN EJECT
//------------------------------------------------------------------------------//
//----------------------------- JETTISON FUEL ----------------------------------//
  
  // Se lee el switch del JET FUEL
  int jetFuelState = digitalRead(PIN_JET_FUEL);
  // Se comprueba si ha cambiado de estado con respecto al que tenía
  if (jetFuelState != lastJetFuelState){
    if (jetFuelState == LOW){
      Joystick.pressButton(BTN_JET_FUEL);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_JET_FUEL);
    }

    lastJetFuelState = jetFuelState;
  }
// FIN JETTISON FUEL
//------------------------------------------------------------------------------//
//----------------------------- JETTISON WEAPONS ----------------------------------//
 
  // Se lee el switch del JET WPNS
  int jetWpnsState = digitalRead(PIN_JET_WPNS);
  // Se comprueba si ha cambiado de estado con respecto al que tenía
  if (jetWpnsState != lastJetWpnsState){
    if (jetWpnsState == LOW){
      Joystick.pressButton(BTN_JET_WPNS);
      delay(PRESS_TIME);
      Joystick.releaseButton(BTN_JET_WPNS);
    }

    lastJetWpnsState = jetWpnsState;
  }
// FIN JETTISON WEAPONS
//------------------------------------------------------------------------------//
  delay(50);
}

void activarParpadeoLed(){
  unsigned long millis_actuales = millis();

  if ((millis_actuales - tiempoInicioParpadeoLed) < tiempoParpadeoLed){
    if((millis_actuales - millis_anteriores) > intervalo) {
      // almacena la ultima vez que parpadeo el LED
      millis_anteriores = millis_actuales;  
    
      // si el LED esta apagado, entonces se enciende o viceversa.
      if (estado_led_rojo == LOW) {
        estado_led_rojo = HIGH;
      } else {
        estado_led_rojo = LOW;
      }
      // configura el LED con la variable estado_led
      digitalWrite(LED_LNDRED, estado_led_rojo);
    }
  }else{
    // Ya han pasado los 3s de parpadeo. Apagamos el led rojo y ponemos las luces
    // del tren de aterrizaje a lo que corresponda
    digitalWrite(LED_LNDRED, LOW);
    digitalWrite(LED_LNDGEAR, estado_tren);
  }
}
