const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const mqtt = require('mqtt');
const cors = require('cors');

const app = express();
app.use(cors());

const server = http.createServer(app);
const io = socketIo(server, {
    cors: {
        origin: [
            "https://smartpv.strubel.io",
            "http://localhost:4200"
        ],
    }
});

// MQTT Verbindung
const mqttClient = mqtt.connect(process.env.MQTT_IP, {
    username: process.env.MQTT_USER,
    password: process.env.MQTT_PASS
});

// Topic-Map um Nachrichten zu speichern
const topicsData: any = {};

mqttClient.on('connect', () => {
    console.log('Mit MQTT Broker verbunden');

    // Subscriben auf alle relevanten Topics
    mqttClient.subscribe('#', (err: any) => {
        if (err) {
            console.error('MQTT Subscription Fehler:', err);
        }
    });
});

mqttClient.on('message', (topic: any, message: any) => {
    const msg = message.toString();
    // console.log(`MQTT Nachricht auf ${topic}: ${msg}`);

    // Speichern der letzten Nachricht pro Topic
    topicsData[topic] = msg;

    // Nachricht an alle Clients, die auf dieses Topic hören
    io.to(topic).emit('mqtt', { topic, message: msg });
});

// Socket.IO Verbindung
io.on('connection', (socket: any) => {
    console.log('Neuer Client verbunden');

    // Client kann einem Topic beitreten
    socket.on('subscribe', (topic: any) => {
        console.log(`Client subscribed auf ${topic}`);
        socket.join(topic);

        // Letzte Nachricht direkt senden, falls vorhanden
        if (topicsData[topic]) {
            socket.emit('mqtt', { topic, message: topicsData[topic] });
        }
    });

    // Client kann ein Topic verlassen
    socket.on('unsubscribe', (topic: any) => {
        console.log(`Client unsubscribed von ${topic}`);
        socket.leave(topic);
    });

    socket.on('disconnect', () => {
        console.log('Client disconnected');
    });
});





export { server }