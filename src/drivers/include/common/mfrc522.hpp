//
// Created by fatih on 5/25/19.
//

#pragma once

namespace tos
{

template <class SpiT>
class mfrc522
{
    using byte = uint8_t ;

public:
    using gpio_t = typename std::remove_pointer_t <SpiT>::gpio_type;
    using pin_t = typename gpio_t::pin_type;

    enum PCD_Register : uint8_t {
        // Page 0: Command and status
        //						  0x00			// reserved for future use
            CommandReg				= 0x01 << 1,	// starts and stops command execution
        ComIEnReg				= 0x02 << 1,	// enable and disable interrupt request control bits
        DivIEnReg				= 0x03 << 1,	// enable and disable interrupt request control bits
        ComIrqReg				= 0x04 << 1,	// interrupt request bits
        DivIrqReg				= 0x05 << 1,	// interrupt request bits
        ErrorReg				= 0x06 << 1,	// error bits showing the error status of the last command executed
        Status1Reg				= 0x07 << 1,	// communication status bits
        Status2Reg				= 0x08 << 1,	// receiver and transmitter status bits
        FIFODataReg				= 0x09 << 1,	// input and output of 64 byte FIFO buffer
        FIFOLevelReg			= 0x0A << 1,	// number of bytes stored in the FIFO buffer
        WaterLevelReg			= 0x0B << 1,	// level for FIFO underflow and overflow warning
        ControlReg				= 0x0C << 1,	// miscellaneous control registers
        BitFramingReg			= 0x0D << 1,	// adjustments for bit-oriented frames
        CollReg					= 0x0E << 1,	// bit position of the first bit-collision detected on the RF interface
        //						  0x0F			// reserved for future use

        // Page 1: Command
        // 						  0x10			// reserved for future use
            ModeReg					= 0x11 << 1,	// defines general modes for transmitting and receiving
        TxModeReg				= 0x12 << 1,	// defines transmission data rate and framing
        RxModeReg				= 0x13 << 1,	// defines reception data rate and framing
        TxControlReg			= 0x14 << 1,	// controls the logical behavior of the antenna driver pins TX1 and TX2
        TxASKReg				= 0x15 << 1,	// controls the setting of the transmission modulation
        TxSelReg				= 0x16 << 1,	// selects the internal sources for the antenna driver
        RxSelReg				= 0x17 << 1,	// selects internal receiver settings
        RxThresholdReg			= 0x18 << 1,	// selects thresholds for the bit decoder
        DemodReg				= 0x19 << 1,	// defines demodulator settings
        // 						  0x1A			// reserved for future use
        // 						  0x1B			// reserved for future use
            MfTxReg					= 0x1C << 1,	// controls some MIFARE communication transmit parameters
        MfRxReg					= 0x1D << 1,	// controls some MIFARE communication receive parameters
        // 						  0x1E			// reserved for future use
            SerialSpeedReg			= 0x1F << 1,	// selects the speed of the serial UART interface

        // Page 2: Configuration
        // 						  0x20			// reserved for future use
            CRCResultRegH			= 0x21 << 1,	// shows the MSB and LSB values of the CRC calculation
        CRCResultRegL			= 0x22 << 1,
        // 						  0x23			// reserved for future use
            ModWidthReg				= 0x24 << 1,	// controls the ModWidth setting?
        // 						  0x25			// reserved for future use
            RFCfgReg				= 0x26 << 1,	// configures the receiver gain
        GsNReg					= 0x27 << 1,	// selects the conductance of the antenna driver pins TX1 and TX2 for modulation
        CWGsPReg				= 0x28 << 1,	// defines the conductance of the p-driver output during periods of no modulation
        ModGsPReg				= 0x29 << 1,	// defines the conductance of the p-driver output during periods of modulation
        TModeReg				= 0x2A << 1,	// defines settings for the internal timer
        TPrescalerReg			= 0x2B << 1,	// the lower 8 bits of the TPrescaler value. The 4 high bits are in TModeReg.
        TReloadRegH				= 0x2C << 1,	// defines the 16-bit timer reload value
        TReloadRegL				= 0x2D << 1,
        TCounterValueRegH		= 0x2E << 1,	// shows the 16-bit timer value
        TCounterValueRegL		= 0x2F << 1,

        // Page 3: Test Registers
        // 						  0x30			// reserved for future use
            TestSel1Reg				= 0x31 << 1,	// general test signal configuration
        TestSel2Reg				= 0x32 << 1,	// general test signal configuration
        TestPinEnReg			= 0x33 << 1,	// enables pin output driver on pins D1 to D7
        TestPinValueReg			= 0x34 << 1,	// defines the values for D1 to D7 when it is used as an I/O bus
        TestBusReg				= 0x35 << 1,	// shows the status of the internal test bus
        AutoTestReg				= 0x36 << 1,	// controls the digital self-test
        VersionReg				= 0x37 << 1,	// shows the software version
        AnalogTestReg			= 0x38 << 1,	// controls the pins AUX1 and AUX2
        TestDAC1Reg				= 0x39 << 1,	// defines the test value for TestDAC1
        TestDAC2Reg				= 0x3A << 1,	// defines the test value for TestDAC2
        TestADCReg				= 0x3B << 1		// shows the value of ADC I and Q channels
        // 						  0x3C			// reserved for production tests
        // 						  0x3D			// reserved for production tests
        // 						  0x3E			// reserved for production tests
        // 						  0x3F			// reserved for production tests
    };

    // MFRC522 commands. Described in chapter 10 of the datasheet.
    enum PCD_Command : uint8_t {
        PCD_Idle				= 0x00,		// no action, cancels current command execution
        PCD_Mem					= 0x01,		// stores 25 bytes into the internal buffer
        PCD_GenerateRandomID	= 0x02,		// generates a 10-byte random ID number
        PCD_CalcCRC				= 0x03,		// activates the CRC coprocessor or performs a self-test
        PCD_Transmit			= 0x04,		// transmits data from the FIFO buffer
        PCD_NoCmdChange			= 0x07,		// no command change, can be used to modify the CommandReg register bits without affecting the command, for example, the PowerDown bit
        PCD_Receive				= 0x08,		// activates the receiver circuits
        PCD_Transceive 			= 0x0C,		// transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission
        PCD_MFAuthent 			= 0x0E,		// performs the MIFARE standard authentication as a reader
        PCD_SoftReset			= 0x0F		// resets the MFRC522
    };

    // MFRC522 RxGain[2:0] masks, defines the receiver's signal voltage gain factor (on the PCD).
    // Described in 9.3.3.6 / table 98 of the datasheet at http://www.nxp.com/documents/data_sheet/MFRC522.pdf
    enum PCD_RxGain : uint8_t {
        RxGain_18dB				= 0x00 << 4,	// 000b - 18 dB, minimum
        RxGain_23dB				= 0x01 << 4,	// 001b - 23 dB
        RxGain_18dB_2			= 0x02 << 4,	// 010b - 18 dB, it seems 010b is a duplicate for 000b
        RxGain_23dB_2			= 0x03 << 4,	// 011b - 23 dB, it seems 011b is a duplicate for 001b
        RxGain_33dB				= 0x04 << 4,	// 100b - 33 dB, average, and typical default
        RxGain_38dB				= 0x05 << 4,	// 101b - 38 dB
        RxGain_43dB				= 0x06 << 4,	// 110b - 43 dB
        RxGain_48dB				= 0x07 << 4,	// 111b - 48 dB, maximum
        RxGain_min				= 0x00 << 4,	// 000b - 18 dB, minimum, convenience for RxGain_18dB
        RxGain_avg				= 0x04 << 4,	// 100b - 33 dB, average, convenience for RxGain_33dB
        RxGain_max				= 0x07 << 4		// 111b - 48 dB, maximum, convenience for RxGain_48dB
    };

    // Commands sent to the PICC.
    enum PICC_Command : uint8_t {
        // The commands used by the PCD to manage communication with several PICCs (ISO 14443-3, Type A, section 6.4)
            PICC_CMD_REQA			= 0x26,		// REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
        PICC_CMD_WUPA			= 0x52,		// Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
        PICC_CMD_CT				= 0x88,		// Cascade Tag. Not really a command, but used during anti collision.
        PICC_CMD_SEL_CL1		= 0x93,		// Anti collision/Select, Cascade Level 1
        PICC_CMD_SEL_CL2		= 0x95,		// Anti collision/Select, Cascade Level 2
        PICC_CMD_SEL_CL3		= 0x97,		// Anti collision/Select, Cascade Level 3
        PICC_CMD_HLTA			= 0x50,		// HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.
        PICC_CMD_RATS           = 0xE0,     // Request command for Answer To Reset.
        // The commands used for MIFARE Classic (from http://www.mouser.com/ds/2/302/MF1S503x-89574.pdf, Section 9)
        // Use PCD_MFAuthent to authenticate access to a sector, then use these commands to read/write/modify the blocks on the sector.
        // The read/write commands can also be used for MIFARE Ultralight.
            PICC_CMD_MF_AUTH_KEY_A	= 0x60,		// Perform authentication with Key A
        PICC_CMD_MF_AUTH_KEY_B	= 0x61,		// Perform authentication with Key B
        PICC_CMD_MF_READ		= 0x30,		// Reads one 16 byte block from the authenticated sector of the PICC. Also used for MIFARE Ultralight.
        PICC_CMD_MF_WRITE		= 0xA0,		// Writes one 16 byte block to the authenticated sector of the PICC. Called "COMPATIBILITY WRITE" for MIFARE Ultralight.
        PICC_CMD_MF_DECREMENT	= 0xC0,		// Decrements the contents of a block and stores the result in the internal data register.
        PICC_CMD_MF_INCREMENT	= 0xC1,		// Increments the contents of a block and stores the result in the internal data register.
        PICC_CMD_MF_RESTORE		= 0xC2,		// Reads the contents of a block into the internal data register.
        PICC_CMD_MF_TRANSFER	= 0xB0,		// Writes the contents of the internal data register to a block.
        // The commands used for MIFARE Ultralight (from http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf, Section 8.6)
        // The PICC_CMD_MF_READ and PICC_CMD_MF_WRITE can also be used for MIFARE Ultralight.
            PICC_CMD_UL_WRITE		= 0xA2		// Writes one 4 byte page to the PICC.
    };

    // MIFARE constants that does not fit anywhere else
    enum MIFARE_Misc {
        MF_ACK					= 0xA,		// The MIFARE Classic uses a 4 bit ACK/NAK. Any other value than 0xA is NAK.
        MF_KEY_SIZE				= 6			// A Mifare Crypto1 key is 6 bytes.
    };

    // PICC types we can detect. Remember to update PICC_GetTypeName() if you add more.
    // last value set to 0xff, then compiler uses less ram, it seems some optimisations are triggered
    enum PICC_Type : uint8_t {
        PICC_TYPE_UNKNOWN		,
        PICC_TYPE_ISO_14443_4	,	// PICC compliant with ISO/IEC 14443-4
        PICC_TYPE_ISO_18092		, 	// PICC compliant with ISO/IEC 18092 (NFC)
        PICC_TYPE_MIFARE_MINI	,	// MIFARE Classic protocol, 320 bytes
        PICC_TYPE_MIFARE_1K		,	// MIFARE Classic protocol, 1KB
        PICC_TYPE_MIFARE_4K		,	// MIFARE Classic protocol, 4KB
        PICC_TYPE_MIFARE_UL		,	// MIFARE Ultralight or Ultralight C
        PICC_TYPE_MIFARE_PLUS	,	// MIFARE Plus
        PICC_TYPE_MIFARE_DESFIRE,	// MIFARE DESFire
        PICC_TYPE_TNP3XXX		,	// Only mentioned in NXP AN 10833 MIFARE Type Identification Procedure
        PICC_TYPE_NOT_COMPLETE	= 0xff	// SAK indicates UID is not complete.
    };

    // Return codes from the functions in this class. Remember to update GetStatusCodeName() if you add more.
    // last value set to 0xff, then compiler uses less ram, it seems some optimisations are triggered
    enum StatusCode : uint8_t {
        STATUS_OK				,	// Success
        STATUS_ERROR			,	// Error in communication
        STATUS_COLLISION		,	// Collission detected
        STATUS_TIMEOUT			,	// Timeout in communication.
        STATUS_NO_ROOM			,	// A buffer is not big enough.
        STATUS_INTERNAL_ERROR	,	// Internal error in the code. Should not happen ;-)
        STATUS_INVALID			,	// Invalid argument.
        STATUS_CRC_WRONG		,	// The CRC_A does not match
        STATUS_MIFARE_NACK		= 0xff	// A MIFARE PICC responded with NAK.
    };

    // A struct used for passing the UID of a PICC.
    struct Uid{
        uint8_t		size;			// Number of bytes in the UID. 4, 7 or 10.
        uint8_t		uidByte[10];
        uint8_t		sak;			// The SAK (Select acknowledge) byte returned from the PICC after successful selection.
    };

    // A struct used for passing a MIFARE Crypto1 key
    struct MIFARE_Key {
        uint8_t		keyByte[MF_KEY_SIZE];
    };


    mfrc522(SpiT spi, gpio_t* gpio, pin_t cs, pin_t rst) : m_cs{cs}, m_rst{rst}, m_gpio{gpio}, m_spi{std::move(spi)} {}

    template <class DelayT>
    void PCD_Reset(DelayT&& delay) {
        PCD_WriteRegister(CommandReg, PCD_SoftReset);	// Issue the SoftReset command.
        // The datasheet does not mention how long the SoftRest command takes to complete.
        // But the MFRC522 might have been in soft power-down mode (triggered by bit 4 of CommandReg)
        // Section 8.8.2 in the datasheet says the oscillator start-up time is the start up time of the crystal + 37,74μs. Let us be generous: 50ms.
        uint8_t count = 0;
        using namespace std::chrono_literals;
        do {
            // Wait for the PowerDown bit in CommandReg to be cleared (max 3x50ms)
            delay(50ms);
        } while ((PCD_ReadRegister(CommandReg) & (1 << 4)) && (++count) < 3);
    }

    template <class DelayT>
    void init(DelayT&& delay) {
        // Set the chipSelectPin as digital output, do not select the slave yet
        m_gpio->set_pin_mode(m_cs, tos::pin_mode::out);
        m_gpio->write(m_cs, tos::digital::high);

        m_gpio->set_pin_mode(m_rst, tos::pin_mode::in);

        using namespace std::chrono_literals;

        bool hard_reset = false;
        if (!m_gpio->read(m_rst))
        {
            // power down mode
            m_gpio->set_pin_mode(m_rst, tos::pin_mode::out);
            m_gpio->write(m_rst, tos::digital::low);
            delay(2us);
            m_gpio->write(m_rst, tos::digital::high);
            delay(50ms);
            hard_reset = true;
        }

        if (!hard_reset) { // Perform a soft reset if we haven't triggered a hard reset above.
            PCD_Reset(delay);
        }

        // Reset baud rates
        PCD_WriteRegister(TxModeReg, 0x00);
        PCD_WriteRegister(RxModeReg, 0x00);
        // Reset ModWidthReg
        PCD_WriteRegister(ModWidthReg, 0x26);

        // When communicating with a PICC we need a timeout if something goes wrong.
        // f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
        // TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
        PCD_WriteRegister(TModeReg, 0x80);			// TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
        PCD_WriteRegister(TPrescalerReg, 0xA9);		// TPreScaler = TModeReg[3..0]:TPrescalerReg, ie 0x0A9 = 169 => f_timer=40kHz, ie a timer period of 25μs.
        PCD_WriteRegister(TReloadRegH, 0x03);		// Reload timer with 0x3E8 = 1000, ie 25ms before timeout.
        PCD_WriteRegister(TReloadRegL, 0xE8);

        PCD_WriteRegister(TxASKReg, 0x40);		// Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
        PCD_WriteRegister(ModeReg, 0x3D);		// Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
        PCD_AntennaOn();						// Enable the antenna driver pins TX1 and TX2 (they were disabled by the reset)
    }

    void PCD_SetAntennaGain(uint8_t mask) {
        //if (PCD_GetAntennaGain() != mask)
        {						// only bother if there is a change
            PCD_ClearRegisterBitMask(RFCfgReg, (0x07<<4));		// clear needed to allow 000 pattern
            PCD_SetRegisterBitMask(RFCfgReg, mask & (0x07<<4));	// only set RxGain[2:0] bits
        }
    }

    uint8_t PCD_GetAntennaGain() const {
        return PCD_ReadRegister(RFCfgReg) & (0x07<<4);
    }

    void PCD_AntennaOn() {
        byte value = PCD_ReadRegister(TxControlReg);
        if ((value & 0x03) != 0x03) {
            PCD_WriteRegister(TxControlReg, value | 0x03);
        }
    }

    uint8_t get_fw_version() const {
        return PCD_ReadRegister(VersionReg);
    }

    auto PCD_CommunicateWithPICC(	byte command,		///< The command to execute. One of the PCD_Command enums.
                                     byte waitIRq,		///< The bits in the ComIrqReg register that signals successful completion of the command.
                                     byte *sendData,		///< Pointer to the data to transfer to the FIFO.
                                     byte sendLen,		///< Number of bytes to transfer to the FIFO.
                                     byte *backData,		///< nullptr or pointer to buffer if data should be read back after executing the command.
                                     byte *backLen,		///< In: Max number of bytes to write to *backData. Out: The number of bytes returned.
                                     byte *validBits,	///< In/Out: The number of valid bits in the last byte. 0 for 8 valid bits.
                                     byte rxAlign,		///< In: Defines the bit position in backData[0] for the first bit received. Default 0.
                                     bool checkCRC		///< In: True => The last two bytes of the response is assumed to be a CRC_A that must be validated.
    ) {
        // Prepare values for BitFramingReg
        byte txLastBits = validBits ? *validBits : 0;
        byte bitFraming = (rxAlign << 4) + txLastBits;		// RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]

        PCD_WriteRegister(CommandReg, PCD_Idle);			// Stop any active command.
        PCD_WriteRegister(ComIrqReg, 0x7F);					// Clear all seven interrupt request bits
        PCD_WriteRegister(FIFOLevelReg, 0x80);				// FlushBuffer = 1, FIFO initialization
        PCD_WriteRegister(FIFODataReg, sendLen, sendData);	// Write sendData to the FIFO
        PCD_WriteRegister(BitFramingReg, bitFraming);		// Bit adjustments
        PCD_WriteRegister(CommandReg, command);				// Execute the command
        if (command == PCD_Transceive) {
            PCD_SetRegisterBitMask(BitFramingReg, 0x80);	// StartSend=1, transmission of data starts
        }

        // Wait for the command to complete.
        // In PCD_Init() we set the TAuto flag in TModeReg. This means the timer automatically starts when the PCD stops transmitting.
        // Each iteration of the do-while-loop takes 17.86μs.
        // TODO check/modify for other architectures than Arduino Uno 16bit
        uint16_t i;
        for (i = 2000; i > 0; i--) {
            byte n = PCD_ReadRegister(ComIrqReg);	// ComIrqReg[7..0] bits are: Set1 TxIRq RxIRq IdleIRq HiAlertIRq LoAlertIRq ErrIRq TimerIRq
            if (n & waitIRq) {					// One of the interrupts that signal success has been set.
                break;
            }
            if (n & 0x01) {						// Timer interrupt - nothing received in 25ms
                return STATUS_TIMEOUT;
            }
        }
        // 35.7ms and nothing happend. Communication with the MFRC522 might be down.
        if (i == 0) {
            return STATUS_TIMEOUT;
        }

        // Stop now if any errors except collisions were detected.
        byte errorRegValue = PCD_ReadRegister(ErrorReg); // ErrorReg[7..0] bits are: WrErr TempErr reserved BufferOvfl CollErr CRCErr ParityErr ProtocolErr
        if (errorRegValue & 0x13) {	 // BufferOvfl ParityErr ProtocolErr
            return STATUS_ERROR;
        }

        byte _validBits = 0;

        // If the caller wants data back, get it from the MFRC522.
        if (backData && backLen) {
            byte n = PCD_ReadRegister(FIFOLevelReg);	// Number of bytes in the FIFO
            if (n > *backLen) {
                return STATUS_NO_ROOM;
            }
            *backLen = n;											// Number of bytes returned
            PCD_ReadRegister(FIFODataReg, n, backData, rxAlign);	// Get received data from FIFO
            _validBits = PCD_ReadRegister(ControlReg) & 0x07;		// RxLastBits[2:0] indicates the number of valid bits in the last received byte. If this value is 000b, the whole byte is valid.
            if (validBits) {
                *validBits = _validBits;
            }
        }

        // Tell about collisions
        if (errorRegValue & 0x08) {		// CollErr
            return STATUS_COLLISION;
        }

        // Perform CRC_A validation if requested.
        if (backData && backLen && checkCRC) {
            // In this case a MIFARE Classic NAK is not OK.
            if (*backLen == 1 && _validBits == 4) {
                return STATUS_MIFARE_NACK;
            }
            // We need at least the CRC_A value and all 8 bits of the last byte must be received.
            if (*backLen < 2 || _validBits != 0) {
                return STATUS_CRC_WRONG;
            }
            // Verify CRC_A - do our own calculation and store the control in controlBuffer.
            byte controlBuffer[2];
            StatusCode status = PCD_CalculateCRC(&backData[0], *backLen - 2, &controlBuffer[0]);
            if (status != STATUS_OK) {
                return status;
            }
            if ((backData[*backLen - 2] != controlBuffer[0]) || (backData[*backLen - 1] != controlBuffer[1])) {
                return STATUS_CRC_WRONG;
            }
        }

        return STATUS_OK;
    }

    auto PCD_TransceiveData(	byte *sendData,		///< Pointer to the data to transfer to the FIFO.
    byte sendLen,		///< Number of bytes to transfer to the FIFO.
        byte *backData,		///< nullptr or pointer to buffer if data should be read back after executing the command.
    byte *backLen,		///< In: Max number of bytes to write to *backData. Out: The number of bytes returned.
        byte *validBits=nullptr, byte rxAlign = 0, bool checkCRC = false) {
        byte waitIRq = 0x30;		// RxIRq and IdleIRq
        return PCD_CommunicateWithPICC(PCD_Transceive, waitIRq, sendData, sendLen, backData, backLen, validBits, rxAlign, checkCRC);
    }

    auto PICC_REQA_or_WUPA(	byte command, 		///< The command to send - PICC_CMD_REQA or PICC_CMD_WUPA
    byte *bufferATQA,	///< The buffer to store the ATQA (Answer to request) in
        byte *bufferSize	///< Buffer size, at least two bytes. Also number of bytes returned if STATUS_OK.
    ) {
        byte validBits;
        StatusCode status;

        if (bufferATQA == nullptr || *bufferSize < 2) {	// The ATQA response is 2 bytes long.
            return STATUS_NO_ROOM;
        }
        PCD_ClearRegisterBitMask(CollReg, 0x80);		// ValuesAfterColl=1 => Bits received after collision are cleared.
        validBits = 7;									// For REQA and WUPA we need the short frame format - transmit only 7 bits of the last (and only) byte. TxLastBits = BitFramingReg[2..0]
        status = PCD_TransceiveData(&command, 1, bufferATQA, bufferSize, &validBits);
        if (status != STATUS_OK) {
            return status;
        }
        if (*bufferSize != 2 || validBits != 0) {		// ATQA must be exactly 16 bits.
            return STATUS_ERROR;
        }
        return STATUS_OK;
    }

    auto PICC_RequestA(	byte *bufferATQA,	///< The buffer to store the ATQA (Answer to request) in
    byte *bufferSize	///< Buffer size, at least two bytes. Also number of bytes returned if STATUS_OK.
    ) {
        return PICC_REQA_or_WUPA(PICC_CMD_REQA, bufferATQA, bufferSize);
    }

    auto PICC_IsNewCardPresent() {
        byte bufferATQA[2];
        byte bufferSize = sizeof(bufferATQA);

        // Reset baud rates
        PCD_WriteRegister(TxModeReg, 0x00);
        PCD_WriteRegister(RxModeReg, 0x00);
        // Reset ModWidthReg
        PCD_WriteRegister(ModWidthReg, 0x26);

        StatusCode result = PICC_RequestA(bufferATQA, &bufferSize);
        return (result == STATUS_OK || result == STATUS_COLLISION);
    }

private:
    StatusCode PCD_CalculateCRC(	byte *data,		///< In: Pointer to the data to transfer to the FIFO for CRC calculation.
                                    byte length,	///< In: The number of bytes to transfer.
                                    byte *result	///< Out: Pointer to result buffer. Result is written to result[0..1], low byte first.
    ) {
        PCD_WriteRegister(CommandReg, PCD_Idle);		// Stop any active command.
        PCD_WriteRegister(DivIrqReg, 0x04);				// Clear the CRCIRq interrupt request bit
        PCD_WriteRegister(FIFOLevelReg, 0x80);			// FlushBuffer = 1, FIFO initialization
        PCD_WriteRegister(FIFODataReg, length, data);	// Write data to the FIFO
        PCD_WriteRegister(CommandReg, PCD_CalcCRC);		// Start the calculation

        // Wait for the CRC calculation to complete. Each iteration of the while-loop takes 17.73μs.
        // TODO check/modify for other architectures than Arduino Uno 16bit

        // Wait for the CRC calculation to complete. Each iteration of the while-loop takes 17.73us.
        for (uint16_t i = 5000; i > 0; i--) {
            // DivIrqReg[7..0] bits are: Set2 reserved reserved MfinActIRq reserved CRCIRq reserved reserved
            byte n = PCD_ReadRegister(DivIrqReg);
            if (n & 0x04) {									// CRCIRq bit set - calculation done
                PCD_WriteRegister(CommandReg, PCD_Idle);	// Stop calculating CRC for new content in the FIFO.
                // Transfer the result from the registers to the result buffer
                result[0] = PCD_ReadRegister(CRCResultRegL);
                result[1] = PCD_ReadRegister(CRCResultRegH);
                return STATUS_OK;
            }
        }
        // 89ms passed and nothing happend. Communication with the MFRC522 might be down.
        return STATUS_TIMEOUT;
    }

    void PCD_ClearRegisterBitMask(	PCD_Register reg,	///< The register to update. One of the PCD_Register enums.
    byte mask			///< The bits to clear.
    ) {
        byte tmp;
        tmp = PCD_ReadRegister(reg);
        PCD_WriteRegister(reg, tmp & (~mask));		// clear bit mask
    }

    void PCD_SetRegisterBitMask(	PCD_Register reg,	///< The register to update. One of the PCD_Register enums.
    byte mask			///< The bits to set.
    ) {
        byte tmp;
        tmp = PCD_ReadRegister(reg);
        PCD_WriteRegister(reg, tmp | mask);			// set bit mask
    }

    void PCD_WriteRegister(PCD_Register reg, uint8_t value) {
        m_gpio->write(m_cs, tos::digital::low);
        m_spi->write({ reinterpret_cast<uint8_t*>(&reg), 1 });
        m_spi->write({ &value, 1 });
        m_gpio->write(m_cs, tos::digital::high);
    }

    void PCD_WriteRegister(PCD_Register reg, uint8_t count, uint8_t *values) {
        m_gpio->write(m_cs, tos::digital::low);
        m_spi->write({ reinterpret_cast<uint8_t*>(&reg), 1 });
        m_spi->write({ values, count});
        m_gpio->write(m_cs, tos::digital::high);
    }

    uint8_t PCD_ReadRegister(PCD_Register reg) const {
        m_gpio->write(m_cs, tos::digital::low);
        uint8_t req = 0x80 | reg;
        m_spi->write({ &req, 1 });
        auto value = m_spi->exchange(0);
        m_gpio->write(m_cs, tos::digital::high);
        return value;
    }

    void PCD_ReadRegister(	PCD_Register reg,	///< The register to read from. One of the PCD_Register enums.
    byte count,			///< The number of bytes to read
        byte *values,		///< Byte array to store the values in.
    byte rxAlign		///< Only bit positions rxAlign..7 in values[0] are updated.
    ) {
        if (count == 0) {
            return;
        }
        //Serial.print(F("Reading ")); 	Serial.print(count); Serial.println(F(" bytes from register."));
        byte address = 0x80 | reg;				// MSB == 1 is for reading. LSB is not used in address. Datasheet section 8.1.2.3.
        byte index = 0;							// Index in values array.
        m_gpio->write(m_cs, tos::digital::low);
        count--;								// One read is performed outside of the loop
        m_spi->write({&address, 1});
        if (rxAlign) {		// Only update bit positions rxAlign..7 in values[0]
            // Create bit mask for bit positions rxAlign..7
            byte mask = (0xFF << rxAlign) & 0xFF;
            // Read value and tell that we want to read the same address again.
            byte value = m_spi->exchange(address);
            // Apply mask to both current value of values[0] and the new data in value.
            values[0] = (values[0] & ~mask) | (value & mask);
            index++;
        }
        while (index < count) {
            values[index] = m_spi->exchange(address);	// Read value and tell that we want to read the same address again.
            index++;
        }
        values[index] = m_spi->exchange(0);			// Read the final byte. Send 0 to stop reading.
        m_gpio->write(m_cs, tos::digital::high);
    }

    pin_t m_cs, m_rst;
    gpio_t *m_gpio;
    SpiT m_spi;
};
}