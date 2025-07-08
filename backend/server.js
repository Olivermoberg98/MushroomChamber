// vite-project/server.js (or wherever your backend file is)

import express from "express";
import cors from "cors";

const app = express();
const PORT = 3001;

// Phase config
const phaseConfigs = ["Incubation", "Primordia", "Fruiting"];
let currentPhase = phaseConfigs[0]; // Default: Incubation

app.use(cors());

app.get("/api/data", (req, res) => {
  res.json({
    humidity: 62.3,
    temperature: 21.7,
    pressure: 1009.2,
    timestamp: new Date().toISOString()
  });
});

app.listen(PORT, () => {
  console.log(`Server listening on http://localhost:${PORT}`);
});

// Get list of phases
app.get('/api/phases', (req, res) => {
  res.json(phaseConfigs);
});

// Get current phase
app.get('/api/phase', (req, res) => {
  res.json({ phase: currentPhase });
});

// Set current phase
app.post('/api/phase', express.json(), (req, res) => {
  const { phase } = req.body;
  if (!phaseConfigs.includes(phase)) {
    return res.status(400).json({ error: 'Invalid phase name' });
  }
  currentPhase = phase;
  res.json({ success: true });
});
