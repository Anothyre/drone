/**
 * Drone Control Packet Encoder
 * 
 * Binary Layout (Little Endian, packed):
 * - uint32 magic        (0x44524F4E = "DRON")
 * - uint16 seq          (sequence number)
 * - uint32 timestamp_ms (current time in ms)
 * - float  x            (roll/vx)
 * - float  y            (pitch/vy)
 * - float  z            (throttle/vz)
 * - uint8  mode         (flight mode)
 * - uint16 crc          (CRC-16-IBM)
 * 
 * Total: 23 bytes
 */

// CRC-16-IBM constants
const CRC_INIT = 0xFFFF;
const CRC_POLY = 0xA001;

/**
 * Calculate CRC-16-IBM checksum
 * @param {Uint8Array} data - Data to checksum
 * @returns {number} - 16-bit CRC value
 */
export function calculateCRC16IBM(data) {
    let crc = CRC_INIT;
    
    for (let i = 0; i < data.length; i++) {
        crc ^= data[i];
        for (let j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ CRC_POLY;
            } else {
                crc = crc >> 1;
            }
        }
    }
    
    return crc & 0xFFFF;
}

/**
 * Encode a control packet for the drone
 * @param {Object} params - Control parameters
 * @param {number} params.seq - Sequence number (uint16)
 * @param {number} params.x - X control value (float, -1 to 1)
 * @param {number} params.y - Y control value (float, -1 to 1)
 * @param {number} params.z - Z control value (float, 0 to 1 for throttle)
 * @param {number} params.mode - Flight mode (uint8)
 * @returns {Uint8Array} - Encoded packet with CRC
 */
export function encodeControlPacket({ seq, x, y, z, mode }) {
    const MAGIC = 0x44524F4E; // "DRON"
    const PACKET_SIZE = 23;
    
    const buffer = new ArrayBuffer(PACKET_SIZE);
    const view = new DataView(buffer);
    const timestamp = Date.now() & 0xFFFFFFFF; // Wrap to uint32
    
    let offset = 0;
    
    // Write fields (Little Endian)
    view.setUint32(offset, MAGIC, true);        offset += 4;  // magic
    view.setUint16(offset, seq & 0xFFFF, true); offset += 2;  // seq
    view.setUint32(offset, timestamp, true);    offset += 4;  // timestamp_ms
    view.setFloat32(offset, x, true);           offset += 4;  // x
    view.setFloat32(offset, y, true);           offset += 4;  // y
    view.setFloat32(offset, z, true);           offset += 4;  // z
    view.setUint8(offset, mode & 0xFF);         offset += 1;  // mode
    view.setUint16(offset, 0, true);            // crc placeholder (0 for calculation)
    
    // Calculate CRC over entire packet with crc field = 0
    const packetBytes = new Uint8Array(buffer);
    const crc = calculateCRC16IBM(packetBytes);
    
    // Write actual CRC
    view.setUint16(PACKET_SIZE - 2, crc, true);
    
    return new Uint8Array(buffer);
}

/**
 * Decode a packet for debugging
 * @param {Uint8Array} packet - Raw packet bytes
 * @returns {Object} - Decoded fields
 */
export function decodePacket(packet) {
    if (packet.length !== 23) {
        return { error: 'Invalid packet size' };
    }
    
    const view = new DataView(packet.buffer, packet.byteOffset, packet.byteLength);
    
    return {
        magic: '0x' + view.getUint32(0, true).toString(16).toUpperCase(),
        seq: view.getUint16(4, true),
        timestamp: view.getUint32(6, true),
        x: view.getFloat32(10, true).toFixed(4),
        y: view.getFloat32(14, true).toFixed(4),
        z: view.getFloat32(18, true).toFixed(4),
        mode: view.getUint8(22),
        crc: '0x' + view.getUint16(21, true).toString(16).toUpperCase().padStart(4, '0')
    };
}

/**
 * Verify packet CRC
 * @param {Uint8Array} packet - Packet to verify
 * @returns {boolean} - True if CRC is valid
 */
export function verifyPacketCRC(packet) {
    if (packet.length !== 23) return false;
    
    // Copy packet and zero out CRC field
    const testPacket = new Uint8Array(packet);
    testPacket[21] = 0;
    testPacket[22] = 0;
    
    const calculatedCRC = calculateCRC16IBM(testPacket);
    const view = new DataView(packet.buffer, packet.byteOffset, packet.byteLength);
    const packetCRC = view.getUint16(21, true);
    
    return calculatedCRC === packetCRC;
}