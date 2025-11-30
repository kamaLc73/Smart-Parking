import React, { useState, useEffect } from 'react';
import { Wifi, WifiOff, Car, AlertCircle, Activity } from 'lucide-react';

const ParkingDashboard = () => {
  const [parkingData, setParkingData] = useState(null);
  const [connected, setConnected] = useState(false);
  const [lastUpdate, setLastUpdate] = useState(null);
  const [client, setClient] = useState(null);
  const [messages, setMessages] = useState([]);

  // MQTT Configuration - replace with your credentials
  const MQTT_CONFIG = {
    broker: 'wss://43b843e091064d22bd63b1edb91a947a.s1.eu.hivemq.cloud:8884/mqtt',
    username: 'smart_parking',
    password: '0123456789Aze_',
    topic: 'smartparking/status'
  };

  useEffect(() => {
    // Load Paho MQTT library via CDN if not present
    if (!window.Paho) {
      const script = document.createElement('script');
      script.src = 'https://cdnjs.cloudflare.com/ajax/libs/paho-mqtt/1.0.1/mqttws31.min.js';
      script.async = true;
      script.onload = () => {
        connectToMQTT();
      };
      document.body.appendChild(script);
    } else {
      connectToMQTT();
    }

    return () => {
      if (client && client.disconnect) {
        client.disconnect();
      }
    };
  }, []);

  const connectToMQTT = () => {
    try {
      const clientId = 'dashboard_' + Math.random().toString(16).substr(2, 8);
      const mqttClient = new window.Paho.MQTT.Client(
        '43b843e091064d22bd63b1edb91a947a.s1.eu.hivemq.cloud',
        8884,
        '/mqtt',
        clientId
      );

      mqttClient.onConnectionLost = (responseObject) => {
        if (responseObject.errorCode !== 0) {
          console.log('Connection lost:', responseObject.errorMessage);
          setConnected(false);
          setTimeout(connectToMQTT, 5000);
        }
      };

      mqttClient.onMessageArrived = (message) => {
        console.log('Message received:', message.payloadString);
        try {
          const data = JSON.parse(message.payloadString);
          setParkingData(data);
          setLastUpdate(new Date());
          setMessages(prev => [{
            time: new Date().toLocaleTimeString(),
            data: data
          }, ...prev.slice(0, 9)]);
        } catch (e) {
          console.error('Error parsing message:', e);
        }
      };

      const connectOptions = {
        useSSL: true,
        userName: MQTT_CONFIG.username,
        password: MQTT_CONFIG.password,
        onSuccess: () => {
          console.log('Connected to MQTT broker');
          setConnected(true);
          mqttClient.subscribe(MQTT_CONFIG.topic);
          console.log('Subscribed to:', MQTT_CONFIG.topic);
        },
        onFailure: (error) => {
          console.error('Connection failed:', error);
          setConnected(false);
          setTimeout(connectToMQTT, 5000);
        }
      };

      mqttClient.connect(connectOptions);
      setClient(mqttClient);
    } catch (error) {
      console.error('Error setting up MQTT:', error);
    }
  };

  const getStatusColor = (occupied) => {
    return occupied ? 'bg-red-500' : 'bg-green-500';
  };

  const getStatusText = (occupied) => {
    return occupied ? 'OCCUPIED' : 'FREE';
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-blue-900 to-slate-900 text-white p-8">
      <div className="max-w-7xl mx-auto mb-8">
        <div className="flex items-center justify-between">
          <div>
            <h1 className="text-4xl font-bold mb-2 flex items-center gap-3">
              <Car className="w-10 h-10" />
              Smart Parking System
            </h1>
            <p className="text-blue-300">Real-time Monitoring Dashboard</p>
          </div>
          <div className="text-right">
            <div className={`flex items-center gap-2 ${connected ? 'text-green-400' : 'text-red-400'}`}>
              {connected ? <Wifi className="w-6 h-6" /> : <WifiOff className="w-6 h-6" />}
              <span className="font-semibold">{connected ? 'Connected' : 'Disconnected'}</span>
            </div>
            {lastUpdate && (
              <p className="text-sm text-gray-400 mt-1">Last update: {lastUpdate.toLocaleTimeString()}</p>
            )}
          </div>
        </div>
      </div>

      <div className="max-w-7xl mx-auto">
        {!parkingData && connected && (
          <div className="bg-blue-900/30 border border-blue-500/50 rounded-lg p-6 mb-6 flex items-center gap-3">
            <Activity className="w-6 h-6 animate-pulse" />
            <p className="text-lg">Waiting for data from ESP32...</p>
          </div>
        )}

        {!connected && (
          <div className="bg-red-900/30 border border-red-500/50 rounded-lg p-6 mb-6 flex items-center gap-3">
            <AlertCircle className="w-6 h-6" />
            <p className="text-lg">Connecting to MQTT broker...</p>
          </div>
        )}

        {parkingData && (
          <>
            <div className="grid grid-cols-1 md:grid-cols-4 gap-6 mb-8">
              <div className="bg-gradient-to-br from-green-600 to-green-700 rounded-lg p-6 shadow-lg">
                <div className="text-green-100 text-sm font-semibold mb-2">FREE SPACES</div>
                <div className="text-4xl font-bold">{parkingData.free}</div>
                <div className="text-green-100 text-xs mt-2">Available now</div>
              </div>

              <div className="bg-gradient-to-br from-red-600 to-red-700 rounded-lg p-6 shadow-lg">
                <div className="text-red-100 text-sm font-semibold mb-2">OCCUPIED</div>
                <div className="text-4xl font-bold">{parkingData.occupied}</div>
                <div className="text-red-100 text-xs mt-2">Currently parked</div>
              </div>

              <div className="bg-gradient-to-br from-blue-600 to-blue-700 rounded-lg p-6 shadow-lg">
                <div className="text-blue-100 text-sm font-semibold mb-2">AVAILABILITY</div>
                <div className="text-4xl font-bold">{parkingData.availability}%</div>
                <div className="text-blue-100 text-xs mt-2">Current rate</div>
              </div>

              <div className="bg-gradient-to-br from-purple-600 to-purple-700 rounded-lg p-6 shadow-lg">
                <div className="text-purple-100 text-sm font-semibold mb-2">TOTAL SPACES</div>
                <div className="text-4xl font-bold">{parkingData.places.length}</div>
                <div className="text-purple-100 text-xs mt-2">Parking capacity</div>
              </div>
            </div>

            <div className="bg-slate-800/50 backdrop-blur rounded-lg p-8 shadow-xl mb-8">
              <h2 className="text-2xl font-bold mb-6 flex items-center gap-2">
                <Car className="w-6 h-6" />
                Parking Spots Status
              </h2>
              <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6">
                {parkingData.places.map((place) => (
                  <div key={place.place} className={`${getStatusColor(place.occupied)} rounded-lg p-6 shadow-lg transform transition-all hover:scale-105`}>
                    <div className="flex items-center justify-between mb-4">
                      <span className="text-2xl font-bold">Place {place.place}</span>
                      <Car className="w-8 h-8" />
                    </div>
                    <div className="space-y-2">
                      <div className="bg-black/20 rounded px-3 py-2">
                        <div className="text-xs opacity-80">Status</div>
                        <div className="text-lg font-bold">{getStatusText(place.occupied)}</div>
                      </div>
                      <div className="bg-black/20 rounded px-3 py-2">
                        <div className="text-xs opacity-80">Distance</div>
                        <div className="text-lg font-bold">{place.distance >= 999 ? 'No object' : `${place.distance} cm`}</div>
                      </div>
                    </div>
                  </div>
                ))}
              </div>
            </div>

            <div className="bg-slate-800/50 backdrop-blur rounded-lg p-6 shadow-xl mb-8">
              <h2 className="text-xl font-bold mb-4">Device Information</h2>
              <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                <div className="bg-slate-700/50 rounded p-4">
                  <div className="text-gray-400 text-sm mb-1">Device ID</div>
                  <div className="font-mono text-lg">{parkingData.device}</div>
                </div>
                <div className="bg-slate-700/50 rounded p-4">
                  <div className="text-gray-400 text-sm mb-1">MQTT Topic</div>
                  <div className="font-mono text-lg">{MQTT_CONFIG.topic}</div>
                </div>
              </div>
            </div>

            <div className="bg-slate-800/50 backdrop-blur rounded-lg p-6 shadow-xl">
              <h2 className="text-xl font-bold mb-4 flex items-center gap-2">
                <Activity className="w-5 h-5" />
                Recent Activity
              </h2>
              <div className="space-y-2 max-h-64 overflow-y-auto">
                {messages.map((msg, idx) => (
                  <div key={idx} className="bg-slate-700/50 rounded p-3 flex justify-between items-center">
                    <div className="flex items-center gap-4">
                      <span className="text-gray-400 text-sm font-mono">{msg.time}</span>
                      <span className="text-sm">Free: <span className="text-green-400 font-bold">{msg.data.free}</span> {' | '} Occupied: <span className="text-red-400 font-bold">{msg.data.occupied}</span></span>
                    </div>
                    <span className="text-blue-400 font-semibold">{msg.data.availability}%</span>
                  </div>
                ))}
                {messages.length === 0 && (
                  <div className="text-gray-400 text-center py-4">No recent activity</div>
                )}
              </div>
            </div>
          </>
        )}
      </div>

      <div className="max-w-7xl mx-auto mt-8 text-center text-gray-400 text-sm">
        <p>Smart Parking System v2.1 • Real-time MQTT Monitoring</p>
      </div>
    </div>
  );
};

export default ParkingDashboard;
