const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const dgram = require('dgram');
const path = require('path');

const app = express();
const server = http.createServer(app);
const io = new Server(server);
const udpClient = dgram.createSocket('udp4');

const WEB_PORT = 3002;
const UDP_PORT = 5000;

// Serve static files from 'public' directory
app.use(express.static(path.join(__dirname, 'public')));

// UDP Server Logic
udpClient.on('error', (err) => {
    console.error(`UDP Server error:\n${err.stack}`);
    udpClient.close();
});

udpClient.on('message', (msg, rinfo) => {
    try {
        const payloadStr = msg.toString();
        // USBArmyKnife sends JSON payload
        const payload = JSON.parse(payloadStr);
        console.log(`[UDP] Received from ${rinfo.address}:${rinfo.port} ->`, payload);
        
        // Broadcast the data to all connected web clients
        io.emit('telemetry', payload);
    } catch (e) {
        console.error(`[UDP] Failed to parse message: ${msg.toString()}`);
    }
});

udpClient.on('listening', () => {
    const address = udpClient.address();
    console.log(`UDP Telemetry Listener running on port ${address.port}`);
});

// Bind UDP port
udpClient.bind(UDP_PORT);

// Socket.io Web Client Logic
io.on('connection', (socket) => {
    console.log('Web Client Connected:', socket.id);
    socket.on('disconnect', () => {
        console.log('Web Client Disconnected:', socket.id);
    });
});

// Start Web Server
server.listen(WEB_PORT, () => {
    console.log(`Blue Team C2 Dashboard running at http://localhost:${WEB_PORT}`);
});
