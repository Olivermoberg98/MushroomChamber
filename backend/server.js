import express from "express";
import cors from "cors";
import fs from "fs";
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

// Treat the ESP32 as connected only if we heard from it this recently.
// timestamp is always an ISO string (or null), so it must be parsed before comparing.
const DATA_FRESHNESS_MS = 60000;

function isDataFresh() {
  if (!latestSensorData.timestamp) return false;
  return Date.now() - new Date(latestSensorData.timestamp).getTime() < DATA_FRESHNESS_MS;
}

// ====== Grow Log ======
// Newline-delimited JSON on disk. sensorHistory above only feeds the dashboard
// and is lost on restart; this is the durable record for later analysis.
// Samples are downsampled - logging every POST would be ~1.3M rows a month.
const LOG_DIR = path.join(__dirname, "data");
const LOG_FILE = path.join(LOG_DIR, "grow-log.ndjson");
const LOG_INTERVAL_MS = 30000;
let lastLogTime = 0;

fs.mkdirSync(LOG_DIR, { recursive: true });

function appendLog(entry) {
  fs.appendFile(LOG_FILE, JSON.stringify(entry) + "\n", (err) => {
    if (err) console.error("Failed to write grow log:", err.message);
  });
}

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
    const {
      timestamp, device_id, humidity, temperature, pressure, wifi_rssi,
      state, humidifier_on, fans_on, vent_duration_ms
    } = req.body;
    
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
      wifi_rssi: wifi_rssi || null,
      state: state || null,
      humidifier_on: humidifier_on ?? null,
      fans_on: fans_on ?? null,
      vent_duration_ms: vent_duration_ms ?? null
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

    if (Date.now() - lastLogTime >= LOG_INTERVAL_MS) {
      lastLogTime = Date.now();
      appendLog({ event: "sample", phase: currentPhase, ...latestSensorData });
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

  res.json({
    ...latestSensorData,
    esp32_connected: isDataFresh()
  });
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
  res.json({
    esp32_connected: isDataFresh(),
    last_data_received: latestSensorData.timestamp,
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
  appendLog({ event: "phase_change", timestamp: new Date().toISOString(), phase: currentPhase });
  res.json({ success: true });
});

// ====== Git Update Endpoint ======
import { exec } from 'child_process';
import { promisify } from 'util';

const execAsync = promisify(exec);

// This host is a deploy target: it should mirror origin/main exactly and never
// carry local commits. Resetting to the fetched ref rather than merging means
// the update still works after the remote history has been rewritten or
// force-pushed, which `git pull` cannot survive (it fails with divergent
// history and leaves the box stuck until someone fixes it by hand).
// Trade-off: any local edits made directly on this machine are discarded.
app.post('/api/update-system', async (req, res) => {
  const updateSteps = [];
  let responded = false;

  // The timeout and the work below race; whichever finishes first answers.
  const respond = (status, body) => {
    if (responded) return false;
    responded = true;
    res.status(status).json(body);
    return true;
  };

  const updateTimeout = setTimeout(() => {
    respond(408, {
      success: false,
      error: 'Update process timed out',
      stage: 'timeout',
      steps: updateSteps
    });
  }, 120000); // 2 minute timeout

  try {
    console.log('🔄 Starting system update process...');

    updateSteps.push('Fetching latest changes...');
    console.log('📥 Fetching from git...');
    await execAsync('git fetch origin', { cwd: __dirname });

    const { stdout: localSha } = await execAsync('git rev-parse HEAD', { cwd: __dirname });
    const { stdout: remoteSha } = await execAsync('git rev-parse origin/main', { cwd: __dirname });
    const local = localSha.trim();
    const remote = remoteSha.trim();

    if (local === remote) {
      clearTimeout(updateTimeout);
      return respond(200, {
        success: true,
        message: 'System is already up to date',
        changes: false,
        revision: remote,
        steps: updateSteps
      });
    }

    updateSteps.push('Applying latest changes...');
    console.log(`📥 Updating ${local.slice(0, 7)} → ${remote.slice(0, 7)}`);
    await execAsync('git reset --hard origin/main', { cwd: __dirname });

    // Dependencies are no longer committed, so this step is load-bearing:
    // it needs working access to the npm registry.
    updateSteps.push('Installing dependencies...');
    console.log('📦 Installing dependencies...');
    await execAsync('npm install', { cwd: __dirname });

    updateSteps.push('Building frontend...');
    console.log('🏗️ Building frontend...');
    await execAsync('npm run build --prefix vite-project', { cwd: __dirname });

    updateSteps.push('Update completed successfully!');
    clearTimeout(updateTimeout);

    const delivered = respond(200, {
      success: true,
      message: 'System updated successfully. Restarting server...',
      changes: true,
      revision: remote,
      steps: updateSteps
    });

    // Only restart if the client actually received the success response. If the
    // timeout already answered, an exit here would read as an unexplained crash.
    if (delivered) {
      console.log('🔄 Restarting server in 3 seconds...');
      setTimeout(() => {
        process.exit(0); // PM2 will restart automatically
      }, 3000);
    }

  } catch (error) {
    clearTimeout(updateTimeout);
    console.error('❌ Update failed:', error);
    respond(500, {
      success: false,
      error: error.message,
      stage: 'execution',
      steps: updateSteps
    });
  }
});

const serverStartTime = Date.now();

// Add this new endpoint with your other API routes
app.get('/api/uptime', (req, res) => {
  const uptimeMs = Date.now() - serverStartTime;
  const uptimeSeconds = Math.floor(uptimeMs / 1000);
  const uptimeMinutes = Math.floor(uptimeSeconds / 60);
  const uptimeHours = Math.floor(uptimeMinutes / 60);
  const uptimeDays = Math.floor(uptimeHours / 24);
  
  res.json({
    uptimeMs,
    uptimeSeconds,
    uptimeMinutes, 
    uptimeHours,
    uptimeDays,
    startTime: new Date(serverStartTime).toISOString(),
    formatted: {
      days: uptimeDays,
      hours: uptimeHours % 24,
      minutes: uptimeMinutes % 60,
      seconds: uptimeSeconds % 60
    }
  });
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
app.listen(PORT, "0.0.0.0", () => {
  console.log(`✅ Server running at http://localhost:${PORT}`);
  console.log(`📡 ESP32 should POST sensor data to: http://localhost:${PORT}/api/sensor-data`);
  console.log(`🌐 Frontend available at: http://localhost:${PORT}`);
});