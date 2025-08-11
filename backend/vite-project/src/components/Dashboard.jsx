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
  const [phases, setPhases] = useState(['Incubation', 'Colonization', 'Pinning', 'Fruiting', 'Harvesting']);
  const [currentPhase, setCurrentPhase] = useState("Incubation");
  const [loading, setLoading] = useState(false);

  // Simulate real-time updates
  useEffect(() => {
    const interval = setInterval(() => {
      const newData = {
        temperature: 21.7 + (Math.random() - 0.5) * 0.4,
        humidity: 85.3 + (Math.random() - 0.5) * 2,
        pressure: 1009.2 + (Math.random() - 0.5) * 1,
        timestamp: new Date().toISOString()
      };
      setData(newData);
      
      setHistory(prev => {
        const updated = [...prev, { 
          ...newData, 
          time: new Date().toLocaleTimeString([], {hour: '2-digit', minute:'2-digit', second:'2-digit'})
        }];
        return updated.slice(-20);
      });
    }, 5000);

    return () => clearInterval(interval);
  }, []);

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

  const handlePhaseChange = (e) => {
    setCurrentPhase(e.target.value);
  };

  const phaseInfo = {
    'Incubation': { duration: '7-14 days', temp: '20-24°C', humidity: '85-95%', description: 'Initial spore germination' },
    'Colonization': { duration: '2-4 weeks', temp: '22-25°C', humidity: '80-90%', description: 'Mycelium growth phase' },
    'Pinning': { duration: '5-10 days', temp: '18-22°C', humidity: '90-95%', description: 'Pin formation begins' },
    'Fruiting': { duration: '7-14 days', temp: '16-20°C', humidity: '85-92%', description: 'Mushroom development' },
    'Harvesting': { duration: '3-7 days', temp: '16-18°C', humidity: '80-85%', description: 'Ready for harvest' }
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