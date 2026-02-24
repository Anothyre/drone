#include "tasks/imu_task.h"
#include "tasks.h"

int intialiseIMU()
{
    /*
    TODO:

    Init UART
       
        115200 bps, 8 data bits, no parity bit, one stop bit
        
            

    Set Accelerometer:
        range (2g,4g,8g,16g)
        Low-pass filter bandwidth (1kHz - <8Hz)

    Set Gyroscope:
        range (125°/s, 2000°/s)
        Low-pass filter bandwidth (523Hz - 12Hz)

    Set Magnetometer (if used):
        operation mode (Low power, Regular, Enhanced regular, High Accuracy)

    Set Sensor fusion mode

    Set Power mode (normal, low power, suspend) Default is normal mode
        Parameter       Value               [Reg Addr]: Reg Value
        Power Mode      Normal Mode         [PWR_MODE]: xxxxxx00b
                        Low Power Mode      [PWR_MODE]: xxxxxx01b
                        Suspend Mode        [PWR_MODE]: xxxxxx10b
                        Invalid             [PWR_MODE]: xxxxxx11b
        If needed Remap Axis

    Set units of output:
        Data            Units       [Reg Addr]: Register Value
        Acceleration    m/s2        [UNIT_SEL] : xxxxxxx0b
                        mg          [UNIT_SEL] : xxxxxxx1b
        Linear Acceleration,
        Gravity vector
                        m/s2        [UNIT_SEL] : xxxxxxx0b
        Magnetic Field 
        Strength 
                        Micro Tesla NA
        Angular Rate
                        Dps [UNIT_SEL] : xxxxxx0xb
                        Rps [UNIT_SEL] : xxxxxx1xb
        Euler Angles
                        Degrees [UNIT_SEL] : xxxxx0xxb
                        Radians [UNIT_SEL] : xxxxx1xxb
        Quaternion      Quaternion  units   NA
        Temperature °C [UNIT_SEL] : xxx0xxxxb
        °F [UNIT_SEL] : xxx1xxxxb

    Set Data output format:
        Parameter           Values      [Reg Addr]: Register value
        Fusion data output  Windows     [UNIT_SEL]: 0xxxxxxxb
        format              Android     [UNIT_SEL]: 1xxxxxxxb

    Set Operation mode (NDOF or IMU if large accelerations are missinterpreted as gravity, internal fusion is slow see if faster speed is needed)
        Parameter Value             [Reg Addr]: Reg Value
        CONFIG MODE CONFIGMODE      [OPR_MODE]: xxxx0000b
        Non-Fusion Modes:
        ACCONLY                     [OPR_MODE]: xxxx0001b
        MAGONLY                     [OPR_MODE]: xxxx0010b
        GYROONLY                    [OPR_MODE]: xxxx0011b
        ACCMAG                      [OPR_MODE]: xxxx0100b
        ACCGYRO                     [OPR_MODE]: xxxx0101b
        MAGGYRO                     [OPR_MODE]: xxxx0110b
        AMG                         [OPR_MODE]: xxxx0111b
        Fusion Modes:
        IMU                         [OPR_MODE]: xxxx1000b
        COMPASS                     [OPR_MODE]: xxxx1001b
        M4G                         [OPR_MODE]: xxxx1010b
        NDOF_FMC_OFF                [OPR_MODE]: xxxx1011b
        NDOF                        [OPR_MODE]: xxxx1100b
    WAIT 7ms AFTER SETTING MODE


    
    */
    return 0; // return 0 on success
}

void TaskIMU(void *pvParameters)
{
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);


        // read IMU DMA buffer
        // run AHRS (Altitude and Heading Reference System)
        // publish attitude (double buffer)
        xTaskNotifyGive(TaskControl_Handle);
    }
}

/*
TODO: 
    Expose calibration functions
    
    Write UART Functions:
    The maximum length support for read and write is 128 Byte.

        Register write
            Command:
            Byte 1      Byte 2      Byte 3      Byte 4      Byte 5      …..     Byte (n+4)
            Start Byte  Write       Reg addr    Length      Data 1      …..     Data n
            0xAA        0x00        <..>        <..>        <..>        …..     <..>

            Acknowledge Response:
            Byte 1              Byte 2
            Response Header     Status
            0xEE                0x01: WRITE_SUCCESSs
                                0x03: WRITE_FAIL
                                0x04: REGMAP_INVALID_ADDRESS
                                0x05: REGMAP_WRITE_DISABLED
                                0x06: WRONG_START_BYTE
                                0x07: BUS_OVER_RUN_ERROR
                                0X08: MAX_LENGTH_ERROR
                                0x09: MIN_LENGTH_ERROR
                                0x0A: RECEIVE_CHARACTER_TIMEOUT

        Register read
            Command:
            Byte 1      Byte 2      Byte 2      Byte 3
            Start Byte  Read        Reg addr    Length
            0xAA        0x01        <..>        <..>

        Read Success Response:
            Byte 1          Byte 2          Byte 3      …..         Byte (n+2)
            ResponseByte    length          Data 1      …..         Data n
            0xBB            <..>

        Read Failure or Acknowledge Response:
            Byte 1              Byte 2
            Response Header     Status
            0xEE                0x02: READ_FAIL
                                0x04: REGMAP_INVALID_ADDRESS
                                0x05: REGMAP_WRITE_DISABLED
                                0x06: WRONG_START_BYTE
                                0x07: BUS_OVER_RUN_ERROR
                                0X08: MAX_LENGTH_ERROR
                                0x09: MIN_LENGTH_ERROR
                                0x0A: RECEIVE_CHARACTER_TIMEOUT


        The command is rejected and no acknowledgement is sent when an invalid start byte
        is sent. The maximum character timeout is 30ms when receiving successive
        characters.
*/