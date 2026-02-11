# Pinbelegung

## 1. Übersicht GPIO-Belegung

  
  GPIO         Funktion                     Beschreibung / Schnittstelle
  ------------ ---------------------------- --------------------------------
  GPIO0        Bootmode-Taster              Nicht verwenden

  GPIO1--4     Motor 1--4                   50 Hz PWM Ausgang

  GPIO5        Batterie-Monitor             ADC Eingang

  GPIO6        Strommessung (Current Sense) ADC Eingang

  GPIO7        IMU Reset                    Digitaler Ausgang, Active Low

  GPIO8--14    Status-LEDs                  Digitaler Ausgang

  GPIO15--16   ---                          NC (Not Connected)

  GPIO17--18   GPS                          UART

  GPIO19--20   Programmierung               USB

  GPIO21       Buzzer                       Digitaler Ausgang

  GPIO22--25   Intern                       NC

  GPIO26       ---                          NC

  GPIO27--32   Intern                       NC

  GPIO33       ---                          NC

  GPIO34--37   Drucksensor                  SPI

  GPIO38--42   ---                          NC

  GPIO43--44   IMU                          UART

  GPIO45--48   ---                          NC
 


## 2. Elektronik-Grundbegriffe


  Abkürzung                           Bedeutung
  ----------------------------------- -----------------------------------
  **GPIO**                            General Purpose Input/Output

  **PWM**                             Pulse Width Modulation

  **ADC**                             Analog-Digital Converter

  **IMU**                             Inertial Measurement Unit

  **NC**                              Not Connected

  **UART**                            Universal Asynchronous Receiver
                                      Transmitter

  **SPI**                             Serial Peripheral Interface

  **Active Low**                      Die Funktion wird ausgelöst, wenn
                                      das Signal logisch LOW ist
  