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

// ====== Middleware ======
app.use(cors({
  origin: 'http://localhost:5173', // Only needed during local dev
  credentials: true,
}));
app.use(express.json());

// Log all requests (handy for debugging)
app.use((req, res, next) => {
  console.log(`[${new Date().toISOString()}] ${req.method} ${req.url}`);
  next();
});

// ====== API Routes ======
app.get("/api/data", (req, res) => {
  res.json({
    humidity: 62.3,
    temperature: 21.7,
    pressure: 1009.2,
    timestamp: new Date().toISOString()
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
});
