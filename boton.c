#define BUTTON_PIN 13  
#define DEBOUNCE_TIME 50  
#define RESET_COUNT 5 
#define MAX_TIME_BETWEEN_PRESSES 1000  // Max 1 s entre pulsaciones
#define TOTAL_RESET_TIME 3000          // Max 3 s para completar la secuencia
int lastSteadyState = LOW;
int lastFlickerableState = LOW;
int currentState;
unsigned long lastDebounceTime = 0;
int contador = 0;
unsigned long ultimaPulsacion = 0;
bool botonPresionado = false;
void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println("=== CONTADOR PULSACIONES ===");
  Serial.println("Cada pulsacion incrementa el marcador del contador");
  Serial.println("5 pulsaciones rapidas reinician el contador");
  Serial.println("===============================");
  mostrarContador();
}

// Funcion  detectar pulsaciones con debounce
bool deteccionPulsacion() {
  currentState = digitalRead(BUTTON_PIN);
  bool pulsacionDetectada = false;

  if (currentState != lastFlickerableState) {
    lastDebounceTime = millis();
    lastFlickerableState = currentState;
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    if (lastSteadyState != currentState) {
      lastSteadyState = currentState;
      // boton presionado
      if (currentState == LOW) {
        pulsacionDetectada = true;
        botonPresionado = true;
        ultimaPulsacion = millis();
      } else {
        botonPresionado = false;
      }
    }
  }
  
  return pulsacionDetectada;
}

// Función incrementar contador
void incrementarContador() {
  contador++;
  Serial.print("¡Pulsación detectada! Contador: ");
  Serial.println(contador);
}

// Funcion resetear contador
void resetearContador() {
  contador = 0;
  Serial.println("*** CONTADOR RESETEADO ***");
  mostrarContador();
}
// Funcion mostrar  contador actual
void mostrarContador() {
  Serial.print("Contador actual: ");
  Serial.println(contador);
}

// Funcion detectar reset = 5 pulsaciones rapidas
void verificarReset(bool pulsacionDetectada) {
  static unsigned long tiempoPrimeraPulsacion = 0;
  static unsigned long tiempoUltimaPulsacion = 0;
  static int contadorRapido = 0;
  static bool secuenciaActiva = false;
  if (pulsacionDetectada) {
    unsigned long tiempoActual = millis();
    if (!secuenciaActiva) {
      // Verificar pulsación y la anterior
      if (tiempoUltimaPulsacion > 0 && (tiempoActual - tiempoUltimaPulsacion) <= MAX_TIME_BETWEEN_PRESSES) {
        // ¡Iniciar secuencia rapida
        secuenciaActiva = true;
        tiempoPrimeraPulsacion = tiempoUltimaPulsacion; // Empezar desde la primera pulsación rápida
        contadorRapido = 2; // Ya tenemos 2 pulsaciones rápidas
        Serial.println("--- ¡Secuencia rápida detectada! ---");
        Serial.print("Pulsación rápida 2/");
        Serial.print(RESET_COUNT);
        Serial.println(" - ¡Continúa pulsando rápido!");
      }
      // Guardar tiempo de esta pulsación para la próxima verificación
      tiempoUltimaPulsacion = tiempoActual;
    } 
    else {
      // Secuencia ya activa - continuar contando
      if ((tiempoActual - tiempoUltimaPulsacion) <= MAX_TIME_BETWEEN_PRESSES) {
        contadorRapido++;
        Serial.print("Pulsación rápida ");
        Serial.print(contadorRapido);
        Serial.print("/");
        Serial.print(RESET_COUNT);
        Serial.print(" - Tiempo desde última: ");
        Serial.print(tiempoActual - tiempoUltimaPulsacion);
        Serial.println(" ms");
        
        // Verificar si se completó la secuencia
        if (contadorRapido >= RESET_COUNT) {
          if ((tiempoActual - tiempoPrimeraPulsacion) <= TOTAL_RESET_TIME) {
            resetearContador();
            Serial.println("¡Reseteo exitoso! (5 pulsaciones rápidas)");
          } else {
            Serial.println("¡Demasiado lento en total! No se resetea el contador");
          }
          // Reiniciar secuencia
          secuenciaActiva = false;
          contadorRapido = 0;
          tiempoPrimeraPulsacion = 0;
        }
      } else {
        // Demasiado lento entre pulsaciones - cancelar secuencia
        Serial.println("--- Demasiado lento, secuencia cancelada ---");
        secuenciaActiva = false;
        contadorRapido = 0;
        tiempoPrimeraPulsacion = 0;
      }
      tiempoUltimaPulsacion = tiempoActual;
    }
  }
  
  // Limpiar secuencia si ha pasado demasiado tiempo total
  if (secuenciaActiva && (millis() - tiempoPrimeraPulsacion) > TOTAL_RESET_TIME) {
    Serial.println("--- Tiempo total agotado, secuencia cancelada ---");
    secuenciaActiva = false;
    contadorRapido = 0;
    tiempoPrimeraPulsacion = 0;
  }
}

void loop() {
  // Detectar pulsación UNA SOLA VEZ por ciclo
  bool pulsacion = deteccionPulsacion();
  
  if (pulsacion) {
    incrementarContador();
  }
  
  // Verificar si hay pulsaciones rápidas para reset
  verificarReset(pulsacion);
  
  // Mostrar estado cada 10 segundos
  static unsigned long ultimoReporte = 0;
  if (millis() - ultimoReporte > 10000) {
    Serial.print("Estado - Contador: ");
    Serial.println(contador);
    ultimoReporte = millis();
  }
  
  delay(10);
}
