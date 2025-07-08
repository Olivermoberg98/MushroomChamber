import { useEffect, useState } from "react";

export default function Dashboard() {
  const [data, setData] = useState(null);
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
    <div className="p-6 text-white max-w-lg mx-auto">
      <h1 className="text-3xl font-bold mb-4">Mushroom Chamber Dashboard</h1>

      {data ? (
        <div className="space-y-2 mb-6">
          <p><strong>Temperature:</strong> {data.temperature} °C</p>
          <p><strong>Humidity:</strong> {data.humidity} %</p>
          <p><strong>Pressure:</strong> {data.pressure} hPa</p>
          <p className="text-sm text-gray-300">
            Last updated: {new Date(data.timestamp).toLocaleString()}
          </p>
        </div>
      ) : (
        <p>Loading data...</p>
      )}

      <div className="mt-6">
        <label htmlFor="phase" className="block mb-2 font-semibold">Select Growth Phase:</label>
        {loading ? (
          <p>Loading phase data...</p>
        ) : (
          <select
            id="phase"
            value={currentPhase}
            onChange={handlePhaseChange}
            className="bg-zinc-800 text-white p-2 rounded"
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
  );
}
