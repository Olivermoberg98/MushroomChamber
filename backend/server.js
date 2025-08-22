import express from "express";
import cors from "cors";
import path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
const PORT = 3001;

// ====== Config ======
const phaseConfigs = ["Incubation", "Primordia", "Fruiting"];
let currentPhase = phaseConfigs[0]; // Default phase

// ====== Data Storage ======
// In-memory storage for sensor data (consider using a database for production)
let latestSensorData = {
  humidity: null,
  temperature: null,
  pressure: null,
  timestamp: null,
  device_id: null,
  wifi_rssi: null
};

// Store historical data (last 100 readings)
let sensorHistory = [];
const MAX_HISTORY_SIZE = 100;

// ====== Middleware ======
app.use(cors({
  origin: 'http://localhost:5173', // Only needed during local dev
  credentials: true,
}));
app.use(express.json());

// Log all requests (handy for debugging)
app.use((req, res, next) => {
  console.log(`[${new Date().toISOString()}] ${req.method} ${req.url}`);
  if (req.method === 'POST') {
    console.log('Request body:', JSON.stringify(req.body, null, 2));
  }
  next();
});

// ====== API Routes ======

// POST endpoint to receive sensor data from ESP32
app.post("/api/sensor-data", (req, res) => {
  try {
    const { timestamp, device_id, humidity, temperature, pressure, wifi_rssi } = req.body;
    
    // Validate required fields
    if (humidity === undefined || temperature === undefined || pressure === undefined) {
      return res.status(400).json({ 
        error: 'Missing required sensor data (humidity, temperature, pressure)' 
      });
    }
    
    // Update latest sensor data
    latestSensorData = {
      humidity: parseFloat(humidity),
      temperature: parseFloat(temperature),
      pressure: parseFloat(pressure),
      timestamp: new Date().toISOString(),
      device_id: device_id || 'unknown',
      wifi_rssi: wifi_rssi || null
    };
    
    // Add to history
    sensorHistory.push({
      ...latestSensorData,
      received_at: new Date().toISOString()
    });
    
    // Keep only the last MAX_HISTORY_SIZE entries
    if (sensorHistory.length > MAX_HISTORY_SIZE) {
      sensorHistory = sensorHistory.slice(-MAX_HISTORY_SIZE);
    }
    
    console.log(`📊 Received sensor data from ${device_id}:`, {
      humidity: `${humidity}%`,
      temperature: `${temperature}°C`,
      pressure: `${pressure} hPa`,
      rssi: `${wifi_rssi} dBm`
    });
    
    res.json({ 
      success: true, 
      message: 'Sensor data received successfully',
      timestamp: latestSensorData.timestamp
    });
    
  } catch (error) {
    console.error('Error processing sensor data:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// GET endpoint to retrieve latest sensor data for frontend
app.get("/api/data", (req, res) => {
  if (!latestSensorData.timestamp) {
    // No sensor data received yet, return mock data or error
    return res.status(404).json({ 
      error: 'No sensor data available yet',
      mock: true,
      humidity: 0,
      temperature: 0,
      pressure: 0,
      timestamp: new Date().toISOString()
    });
  }
  
  // Convert timestamp to ISO string if it's a number
  const responseData = {
    ...latestSensorData,
    timestamp: typeof latestSensorData.timestamp === 'number' 
      ? new Date(latestSensorData.timestamp).toISOString()
      : latestSensorData.timestamp
  };
  
  res.json(responseData);
});

// GET endpoint to retrieve sensor data history
app.get("/api/history", (req, res) => {
  const limit = parseInt(req.query.limit) || 50;
  const limitedHistory = sensorHistory.slice(-limit);
  
  res.json({
    data: limitedHistory,
    total: sensorHistory.length,
    latest: latestSensorData.timestamp
  });
});

// GET endpoint to check if we're receiving data from ESP32
app.get("/api/status", (req, res) => {
  const now = Date.now();
  const lastDataTime = latestSensorData.timestamp;
  const isDataRecent = lastDataTime && (now - lastDataTime < 60000); // within last minute
  
  res.json({
    esp32_connected: isDataRecent,
    last_data_received: lastDataTime ? new Date(lastDataTime).toISOString() : null,
    device_id: latestSensorData.device_id,
    wifi_rssi: latestSensorData.wifi_rssi,
    data_points_stored: sensorHistory.length
  });
});

app.get('/api/phases', (req, res) => {
  res.json(phaseConfigs);
});

app.get('/api/phase', (req, res) => {
  res.json({ phase: currentPhase });
});

app.post('/api/phase', (req, res) => {
  const { phase } = req.body;
  if (!phaseConfigs.includes(phase)) {
    return res.status(400).json({ error: 'Invalid phase name' });
  }
  currentPhase = phase;
  console.log(`🔄 Phase changed to: ${currentPhase}`);
  res.json({ success: true });
});

// ====== Frontend Serving ======
// Serve static frontend files for non-API routes
const frontendPath = path.join(__dirname, "vite-project", "dist");
app.use(express.static(frontendPath));

// Any route that doesn't start with /api should return index.html
app.get(/^(?!\/api).*/, (req, res) => {
  res.sendFile(path.join(frontendPath, "index.html"));
});

// ====== Start Server ======
app.listen(PORT, () => {
  console.log(`✅ Server running at http://localhost:${PORT}`);
  console.log(`📡 ESP32 should POST sensor data to: http://localhost:${PORT}/api/sensor-data`);
  console.log(`🌐 Frontend available at: http://localhost:${PORT}`);
});