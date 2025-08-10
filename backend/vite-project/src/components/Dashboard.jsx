import { useEffect, useState } from "react";
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from "recharts";

export default function Dashboard() {
  const [data, setData] = useState(null);
  const [history, setHistory] = useState([]);
  const [phases, setPhases] = useState([]);
  const [currentPhase, setCurrentPhase] = useState("");
  const [loading, setLoading] = useState(true);

  // Fetch sensor data
  useEffect(() => {
    const fetchSensorData = async () => {
      try {
        const res = await fetch("http://localhost:3001/api/data");
        const json = await res.json();
        setData(json);

        // Append to history (keep last 20)
        setHistory((prev) => {
          const updated = [...prev, { ...json, time: new Date(json.timestamp).toLocaleTimeString() }];
          return updated.slice(-20);
        });
      } catch (err) {
        console.error("Failed to fetch sensor data:", err);
      }
    };

    fetchSensorData();
    const interval = setInterval(fetchSensorData, 5000);
    return () => clearInterval(interval);
  }, []);

  // Fetch phase data (once on mount)
  useEffect(() => {
    const fetchPhaseInfo = async () => {
      try {
        const [phasesRes, currentPhaseRes] = await Promise.all([
          fetch("http://localhost:3001/api/phases"),
          fetch("http://localhost:3001/api/phase")
        ]);

        const phasesJson = await phasesRes.json();
        const currentPhaseJson = await currentPhaseRes.json();

        setPhases(phasesJson);
        setCurrentPhase(currentPhaseJson.phase);
        setLoading(false);
      } catch (err) {
        console.error("Failed to fetch phase info:", err);
      }
    };

    fetchPhaseInfo();
  }, []);

  // Handle phase change
  const handlePhaseChange = async (e) => {
    const newPhase = e.target.value;
    try {
      const res = await fetch("http://localhost:3001/api/phase", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ phase: newPhase }),
      });

      if (res.ok) {
        setCurrentPhase(newPhase);
      } else {
        console.error("Failed to update phase");
      }
    } catch (err) {
      console.error("Error updating phase:", err);
    }
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-green-900 via-zinc-900 to-black p-8 text-white font-sans">
      <div className="max-w-4xl mx-auto bg-zinc-800 bg-opacity-80 rounded-2xl shadow-lg p-6 backdrop-blur-md">
        
        {/* Header */}
        <h1 className="text-4xl font-bold mb-6 text-green-300 text-center">
          🍄 Mushroom Chamber Dashboard
        </h1>

        {/* Sensor cards */}
        {data ? (
          <div className="grid grid-cols-1 sm:grid-cols-3 gap-4 mb-8">
            <div className="bg-zinc-900 p-4 rounded-xl shadow-md text-center">
              <p className="text-sm text-gray-400">Temperature</p>
              <p className="text-2xl font-bold text-orange-300">{data.temperature} °C</p>
            </div>
            <div className="bg-zinc-900 p-4 rounded-xl shadow-md text-center">
              <p className="text-sm text-gray-400">Humidity</p>
              <p className="text-2xl font-bold text-blue-300">{data.humidity} %</p>
            </div>
            <div className="bg-zinc-900 p-4 rounded-xl shadow-md text-center">
              <p className="text-sm text-gray-400">Pressure</p>
              <p className="text-2xl font-bold text-purple-300">{data.pressure} hPa</p>
            </div>
          </div>
        ) : (
          <p className="text-center">Loading data...</p>
        )}

        {/* Last updated */}
        {data && (
          <p className="text-xs text-gray-400 mb-6 text-center">
            Last updated: {new Date(data.timestamp).toLocaleString()}
          </p>
        )}

        {/* Chart */}
        {history.length > 1 && (
          <div className="bg-zinc-900 p-4 rounded-xl shadow-md mb-8">
            <h2 className="text-lg font-semibold mb-4 text-green-200">Environmental Trends</h2>
            <ResponsiveContainer width="100%" height={300}>
              <LineChart data={history}>
                <CartesianGrid strokeDasharray="3 3" stroke="#444" />
                <XAxis dataKey="time" stroke="#aaa" />
                <YAxis stroke="#aaa" />
                <Tooltip contentStyle={{ backgroundColor: "#222", border: "none" }} />
                <Legend />
                <Line type="monotone" dataKey="temperature" stroke="#fca311" name="Temp (°C)" />
                <Line type="monotone" dataKey="humidity" stroke="#219ebc" name="Humidity (%)" />
              </LineChart>
            </ResponsiveContainer>
          </div>
        )}

        {/* Phase selector */}
        <div className="bg-zinc-900 p-4 rounded-xl shadow-md">
          <label htmlFor="phase" className="block mb-2 font-semibold text-green-200">
            Select Growth Phase:
          </label>
          {loading ? (
            <p>Loading phase data...</p>
          ) : (
            <select
              id="phase"
              value={currentPhase}
              onChange={handlePhaseChange}
              className="bg-zinc-800 text-white p-2 rounded w-full"
            >
              {phases.map((phase) => (
                <option key={phase} value={phase}>
                  {phase}
                </option>
              ))}
            </select>
          )}
        </div>
      </div>
    </div>
  );
}
