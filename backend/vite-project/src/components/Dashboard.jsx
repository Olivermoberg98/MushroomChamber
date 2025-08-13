import { useEffect, useState } from "react";
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer, AreaChart, Area } from "recharts";
import { Thermometer, Droplets, Gauge, Activity, AlertTriangle, CheckCircle, Clock, Leaf } from "lucide-react";

export default function Dashboard() {
  const [data, setData] = useState({
    temperature: 21.7,
    humidity: 62.3,
    pressure: 1009.2,
    timestamp: new Date().toISOString()
  });
  const [history, setHistory] = useState([
    { temperature: 21.5, humidity: 61.8, pressure: 1008.9, time: "7:25:50 PM" },
    { temperature: 21.6, humidity: 62.1, pressure: 1009.0, time: "7:25:55 PM" },
    { temperature: 21.7, humidity: 62.3, pressure: 1009.2, time: "7:26:00 PM" }
  ]);
  const [phases, setPhases] = useState(['Incubation', 'Primordia', 'Fruiting']);
  const [currentPhase, setCurrentPhase] = useState("Incubation");
  const [loading, setLoading] = useState(false);

  // Simulate real-time updates
  useEffect(() => {
    // Fetch initial data
    fetchLatestData();
    fetchPhases();
    fetchCurrentPhase();
    fetchHistory();
    
    // Set up interval to fetch real data every 30 seconds
    const interval = setInterval(() => {
      fetchLatestData();
    }, 30000);

    return () => clearInterval(interval);
  }, []);

  // Add these fetch functions to your Dashboard component:
  const fetchLatestData = async () => {
    try {
      const response = await fetch('/api/data');
      if (response.ok) {
        const newData = await response.json();
        
        // Handle case where no real data is available yet
        if (newData.mock) {
          console.log('No sensor data available yet, using mock data');
          return;
        }
        
        setData({
          temperature: newData.temperature,
          humidity: newData.humidity,
          pressure: newData.pressure,
          timestamp: newData.timestamp
        });
        
        // Add to history when we get new data
        setHistory(prev => {
          const updated = [...prev, { 
            temperature: newData.temperature,
            humidity: newData.humidity,
            pressure: newData.pressure,
            time: new Date(newData.timestamp).toLocaleTimeString([], {
              hour: '2-digit', 
              minute:'2-digit', 
              second:'2-digit'
            })
          }];
          return updated.slice(-20); // Keep last 20 readings
        });
      }
    } catch (error) {
      console.error('Failed to fetch sensor data:', error);
    }
  };

  const fetchPhases = async () => {
    try {
      const response = await fetch('/api/phases');
      if (response.ok) {
        const phasesData = await response.json();
        setPhases(phasesData);
      }
    } catch (error) {
      console.error('Failed to fetch phases:', error);
    }
  };

  const fetchCurrentPhase = async () => {
    try {
      const response = await fetch('/api/phase');
      if (response.ok) {
        const phaseData = await response.json();
        setCurrentPhase(phaseData.phase);
      }
    } catch (error) {
      console.error('Failed to fetch current phase:', error);
    }
  };

  const fetchHistory = async () => {
    try {
      const response = await fetch('/api/history?limit=20');
      if (response.ok) {
        const historyData = await response.json();
        const formattedHistory = historyData.data.map(item => ({
          temperature: item.temperature,
          humidity: item.humidity,
          pressure: item.pressure,
          time: new Date(item.timestamp).toLocaleTimeString([], {
            hour: '2-digit', 
            minute:'2-digit', 
            second:'2-digit'
          })
        }));
        setHistory(formattedHistory);
      }
    } catch (error) {
      console.error('Failed to fetch history:', error);
    }
  };

  // Update your handlePhaseChange function:
  const handlePhaseChange = async (e) => {
    const newPhase = e.target.value;
    setLoading(true);
    
    try {
      const response = await fetch('/api/phase', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ phase: newPhase }),
      });
      
      if (response.ok) {
        setCurrentPhase(newPhase);
      } else {
        console.error('Failed to update phase');
      }
    } catch (error) {
      console.error('Failed to update phase:', error);
    } finally {
      setLoading(false);
    }
  };

  const getStatusColor = (value, type) => {
    if (type === 'temperature') {
      if (value >= 20 && value <= 24) return 'text-green-400';
      if (value >= 18 && value <= 26) return 'text-yellow-400';
      return 'text-red-400';
    }
    if (type === 'humidity') {
      if (value >= 80 && value <= 95) return 'text-green-400';
      if (value >= 70 && value <= 98) return 'text-yellow-400';
      return 'text-red-400';
    }
    if (type === 'pressure') {
      if (value >= 1005 && value <= 1015) return 'text-green-400';
      if (value >= 1000 && value <= 1020) return 'text-yellow-400';
      return 'text-red-400';
    }
    return 'text-gray-400';
  };

  const getStatusIcon = (value, type) => {
    const color = getStatusColor(value, type);
    if (color.includes('green')) return <CheckCircle className="w-4 h-4 text-green-400" />;
    if (color.includes('yellow')) return <AlertTriangle className="w-4 h-4 text-yellow-400" />;
    return <AlertTriangle className="w-4 h-4 text-red-400" />;
  };

  const phaseInfo = {
    'Incubation': { duration: '7-14 days', temp: '20-24°C', humidity: '85-95%', description: 'Initial spore germination' },
    'Primordia': { duration: '5-10 days', temp: '18-22°C', humidity: '90-95%', description: 'Pin formation begins' },
    'Fruiting': { duration: '7-14 days', temp: '16-20°C', humidity: '85-92%', description: 'Mushroom development' },
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-slate-800 to-slate-900 p-4 lg:p-8">
      <div className="max-w-7xl mx-auto space-y-6">
        
        {/* Header */}
        <div className="bg-white/10 backdrop-blur-md rounded-2xl border border-white/20 p-6">
          <div className="flex flex-col lg:flex-row lg:items-center lg:justify-between gap-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-emerald-500/20 rounded-lg">
                <Leaf className="w-8 h-8 text-emerald-400" />
              </div>
              <div>
                <h1 className="text-2xl lg:text-3xl font-bold text-white">Mushroom Chamber Control</h1>
                <p className="text-slate-300">Environmental monitoring and phase management</p>
              </div>
            </div>
            <div className="flex items-center gap-2 text-slate-300">
              <Clock className="w-4 h-4" />
              <span className="text-sm">Last update: {new Date(data.timestamp).toLocaleString()}</span>
            </div>
          </div>
        </div>

        {/* Key Metrics */}
        <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
          <div className="bg-white/10 backdrop-blur-md rounded-2xl border border-white/20 p-6">
            <div className="flex items-center justify-between mb-4">
              <div className="flex items-center gap-3">
                <div className="p-2 bg-orange-500/20 rounded-lg">
                  <Thermometer className="w-6 h-6 text-orange-400" />
                </div>
                <div>
                  <p className="text-slate-400 text-sm">Temperature</p>
                  <p className={`text-2xl font-bold ${getStatusColor(data.temperature, 'temperature')}`}>
                    {data.temperature.toFixed(1)}°C
                  </p>
                </div>
              </div>
              {getStatusIcon(data.temperature, 'temperature')}
            </div>
            <div className="text-xs text-slate-400">
              Optimal range: 20-24°C for {currentPhase}
            </div>
            <div className="mt-3 bg-slate-800/50 rounded-lg p-2">
              <div className="h-2 bg-slate-700 rounded-full overflow-hidden">
                <div 
                  className="h-full bg-gradient-to-r from-orange-500 to-orange-400 rounded-full transition-all"
                  style={{ width: `${Math.min(100, (data.temperature / 30) * 100)}%` }}
                />
              </div>
            </div>
          </div>

          <div className="bg-white/10 backdrop-blur-md rounded-2xl border border-white/20 p-6">
            <div className="flex items-center justify-between mb-4">
              <div className="flex items-center gap-3">
                <div className="p-2 bg-blue-500/20 rounded-lg">
                  <Droplets className="w-6 h-6 text-blue-400" />
                </div>
                <div>
                  <p className="text-slate-400 text-sm">Humidity</p>
                  <p className={`text-2xl font-bold ${getStatusColor(data.humidity, 'humidity')}`}>
                    {data.humidity.toFixed(1)}%
                  </p>
                </div>
              </div>
              {getStatusIcon(data.humidity, 'humidity')}
            </div>
            <div className="text-xs text-slate-400">
              Optimal range: 80-95% for {currentPhase}
            </div>
            <div className="mt-3 bg-slate-800/50 rounded-lg p-2">
              <div className="h-2 bg-slate-700 rounded-full overflow-hidden">
                <div 
                  className="h-full bg-gradient-to-r from-blue-500 to-blue-400 rounded-full transition-all"
                  style={{ width: `${Math.min(100, data.humidity)}%` }}
                />
              </div>
            </div>
          </div>

          <div className="bg-white/10 backdrop-blur-md rounded-2xl border border-white/20 p-6">
            <div className="flex items-center justify-between mb-4">
              <div className="flex items-center gap-3">
                <div className="p-2 bg-purple-500/20 rounded-lg">
                  <Gauge className="w-6 h-6 text-purple-400" />
                </div>
                <div>
                  <p className="text-slate-400 text-sm">Pressure</p>
                  <p className={`text-2xl font-bold ${getStatusColor(data.pressure, 'pressure')}`}>
                    {data.pressure.toFixed(1)} hPa
                  </p>
                </div>
              </div>
              {getStatusIcon(data.pressure, 'pressure')}
            </div>
            <div className="text-xs text-slate-400">
              Standard range: 1005-1015 hPa
            </div>
            <div className="mt-3 bg-slate-800/50 rounded-lg p-2">
              <div className="h-2 bg-slate-700 rounded-full overflow-hidden">
                <div 
                  className="h-full bg-gradient-to-r from-purple-500 to-purple-400 rounded-full transition-all"
                  style={{ width: `${Math.min(100, ((data.pressure - 990) / 30) * 100)}%` }}
                />
              </div>
            </div>
          </div>
        </div>

        <div className="grid grid-cols-1 xl:grid-cols-3 gap-6">
          {/* Environmental Trends Chart */}
          <div className="xl:col-span-2 bg-white/10 backdrop-blur-md rounded-2xl border border-white/20 p-6">
            <div className="flex items-center gap-3 mb-6">
              <Activity className="w-5 h-5 text-emerald-400" />
              <h2 className="text-xl font-semibold text-white">Environmental Trends</h2>
            </div>
            
            {history.length > 1 && (
              <ResponsiveContainer width="100%" height={400}>
                <AreaChart data={history}>
                  <defs>
                    <linearGradient id="tempGradient" x1="0" y1="0" x2="0" y2="1">
                      <stop offset="5%" stopColor="#f97316" stopOpacity={0.3}/>
                      <stop offset="95%" stopColor="#f97316" stopOpacity={0}/>
                    </linearGradient>
                    <linearGradient id="humidityGradient" x1="0" y1="0" x2="0" y2="1">
                      <stop offset="5%" stopColor="#3b82f6" stopOpacity={0.3}/>
                      <stop offset="95%" stopColor="#3b82f6" stopOpacity={0}/>
                    </linearGradient>
                  </defs>
                  <CartesianGrid strokeDasharray="3 3" stroke="#475569" strokeOpacity={0.3} />
                  <XAxis 
                    dataKey="time" 
                    stroke="#94a3b8" 
                    fontSize={12}
                    tick={{ fill: '#94a3b8' }}
                  />
                  <YAxis 
                    stroke="#94a3b8" 
                    fontSize={12}
                    tick={{ fill: '#94a3b8' }}
                  />
                  <Tooltip 
                    contentStyle={{ 
                      backgroundColor: '#1e293b', 
                      border: '1px solid #334155',
                      borderRadius: '8px',
                      color: '#f8fafc'
                    }}
                  />
                  <Legend />
                  <Area
                    type="monotone"
                    dataKey="temperature"
                    stroke="#f97316"
                    fillOpacity={1}
                    fill="url(#tempGradient)"
                    name="Temperature (°C)"
                    strokeWidth={2}
                  />
                  <Area
                    type="monotone"
                    dataKey="humidity"
                    stroke="#3b82f6"
                    fillOpacity={1}
                    fill="url(#humidityGradient)"
                    name="Humidity (%)"
                    strokeWidth={2}
                  />
                </AreaChart>
              </ResponsiveContainer>
            )}
          </div>

          {/* Growth Phase Control */}
          <div className="space-y-6">
            <div className="bg-white/10 backdrop-blur-md rounded-2xl border border-white/20 p-6">
              <h3 className="text-lg font-semibold text-white mb-4">Growth Phase Control</h3>
              
              <div className="mb-4">
                <label htmlFor="phase" className="block text-sm text-slate-300 mb-2">
                  Current Phase
                </label>
                <select
                  id="phase"
                  value={currentPhase}
                  onChange={handlePhaseChange}
                  className="w-full bg-slate-800/70 border border-slate-600 rounded-lg px-3 py-2 text-white focus:outline-none focus:ring-2 focus:ring-emerald-500 focus:border-transparent"
                >
                  {phases.map((phase) => (
                    <option key={phase} value={phase} className="bg-slate-800">
                      {phase}
                    </option>
                  ))}
                </select>
              </div>

              <div className="bg-slate-800/50 rounded-lg p-4 space-y-2">
                <div className="flex justify-between text-sm">
                  <span className="text-slate-400">Duration:</span>
                  <span className="text-white">{phaseInfo[currentPhase]?.duration}</span>
                </div>
                <div className="flex justify-between text-sm">
                  <span className="text-slate-400">Temperature:</span>
                  <span className="text-white">{phaseInfo[currentPhase]?.temp}</span>
                </div>
                <div className="flex justify-between text-sm">
                  <span className="text-slate-400">Humidity:</span>
                  <span className="text-white">{phaseInfo[currentPhase]?.humidity}</span>
                </div>
                <div className="pt-2 border-t border-slate-600">
                  <p className="text-xs text-slate-400">{phaseInfo[currentPhase]?.description}</p>
                </div>
              </div>
            </div>

            {/* System Status */}
            <div className="bg-white/10 backdrop-blur-md rounded-2xl border border-white/20 p-6">
              <h3 className="text-lg font-semibold text-white mb-4">System Status</h3>
              
              <div className="space-y-3">
                <div className="flex items-center justify-between p-3 bg-slate-800/50 rounded-lg">
                  <div className="flex items-center gap-2">
                    <div className="w-2 h-2 bg-green-400 rounded-full"></div>
                    <span className="text-sm text-slate-300">Environmental Control</span>
                  </div>
                  <span className="text-xs text-green-400">Online</span>
                </div>
                
                <div className="flex items-center justify-between p-3 bg-slate-800/50 rounded-lg">
                  <div className="flex items-center gap-2">
                    <div className="w-2 h-2 bg-green-400 rounded-full"></div>
                    <span className="text-sm text-slate-300">Data Collection</span>
                  </div>
                  <span className="text-xs text-green-400">Active</span>
                </div>
                
                <div className="flex items-center justify-between p-3 bg-slate-800/50 rounded-lg">
                  <div className="flex items-center gap-2">
                    <div className="w-2 h-2 bg-yellow-400 rounded-full"></div>
                    <span className="text-sm text-slate-300">Ventilation</span>
                  </div>
                  <span className="text-xs text-yellow-400">Standby</span>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}