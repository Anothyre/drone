
const WebSocket = require('ws');
const dgram = require('dgram');

// Configuration
const CONFIG = {
    wsPort: 8080,              // WebSocket server port
    udpControlPort: 14550,     // UDP control port (to drone)
    udpTelemetryPort: 14551,   // UDP telemetry port (from drone)
    droneIP: '192.168.4.1',    // Default drone IP (ESP32 SoftAP)
};

// Create UDP sockets
const udpControlSocket = dgram.createSocket('udp4');
const udpTelemetrySocket = dgram.createSocket('udp4');

// Create WebSocket server
const wss = new WebSocket.Server({ port: CONFIG.wsPort });

console.log(`🚀 UDP Bridge Server started`);
console.log(`   WebSocket: ws://localhost:${CONFIG.wsPort}`);
console.log(`   UDP Control: ${CONFIG.droneIP}:${CONFIG.udpControlPort}`);
console.log(`   UDP Telemetry: *:${CONFIG.udpTelemetryPort}`);

// Track connected WebSocket clients
const clients = new Set();

// WebSocket connection handler
wss.on('connection', (ws) => {
    console.log('✅ WebSocket client connected');
    clients.add(ws);

    // Receive binary data from WebSocket → send as UDP
    ws.on('message', (data) => {
        if (Buffer.isBuffer(data)) {
            udpControlSocket.send(data, CONFIG.udpControlPort, CONFIG.droneIP, (err) => {
                if (err) console.error('❌ UDP send error:', err);
            });
        }
    });

    ws.on('close', () => {
        console.log('❌ WebSocket client disconnected');
        clients.delete(ws);
    });

    ws.on('error', (err) => {
        console.error('WebSocket error:', err);
        clients.delete(ws);
    });
});

// UDP Telemetry listener (drone → WebSocket clients)
udpTelemetrySocket.bind(CONFIG.udpTelemetryPort);

udpTelemetrySocket.on('message', (msg, rinfo) => {
    console.log(`📡 Received telemetry from ${rinfo.address}:${rinfo.port} (${msg.length} bytes)`);
    
    // Forward to all connected WebSocket clients
    clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(msg);
        }
    });
});

udpTelemetrySocket.on('listening', () => {
    const address = udpTelemetrySocket.address();
    console.log(`📡 UDP Telemetry listening on ${address.address}:${address.port}`);
});

// Error handlers
udpControlSocket.on('error', (err) => {
    console.error('UDP Control socket error:', err);
});

udpTelemetrySocket.on('error', (err) => {
    console.error('UDP Telemetry socket error:', err);
});

// Graceful shutdown
process.on('SIGINT', () => {
    console.log('\n🛑 Shutting down...');
    udpControlSocket.close();
    udpTelemetrySocket.close();
    wss.close();
    process.exit(0);
});
