import { useEffect, useState } from "react";
import { XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, AreaChart, Area, ReferenceArea } from "recharts";
import { Thermometer, Droplets, Gauge, Activity, AlertTriangle, CheckCircle, Clock, Leaf, Wifi, WifiOff, Download, RefreshCw, Timer, Fan, Wind, CloudDrizzle } from "lucide-react";

// Dark-surface steps from the shared viz palette. Each chart carries a single
// series, so its title names it and no legend is needed.
const C_TEMP = "#d95926";
const C_HUMIDITY = "#3987e5";
const C_FAN = "#199e70";
const AXIS_INK = "#94a3b8";
const GRID_INK = "#334155";
const RANGES = ["1h", "6h", "24h"];

const isNum = (v) => typeof v === "number" && Number.isFinite(v);
const fmtVal = (v, digits, suffix) => (isNum(v) ? v.toFixed(digits) + suffix : "\u2014");

const fmtDuration = (secs) => {
  if (!isNum(secs)) return "\u2014";
  const h = Math.floor(secs / 3600);
  const m = Math.round((secs % 3600) / 60);
  return h > 0 ? h + "h " + m + "m" : m + "m";
};

// The axis is scaled to the data, never to the target band. A +/-10 band is far
// wider than the chamber drifts, so folding it into the domain would flatten the
// trace again - the exact problem these charts replace.
const domainFor = (points, key, minPad) => {
  const vals = points.map((p) => p[key]).filter(isNum);
  if (!vals.length) return ["auto", "auto"];
  const min = Math.min(...vals);
  const max = Math.max(...vals);
  const pad = Math.max((max - min) * 0.2, minPad);
  return [Number((min - pad).toFixed(1)), Number((max + pad).toFixed(1))];
};

function ChartTooltip({ active, payload, label, unit, digits }) {
  if (!active || !payload || !payload.length) return null;
  const v = payload[0].value;
  return (
    <div className="rounded-lg border border-slate-700 bg-slate-900/95 px-3 py-2 shadow-lg">
      <div className="text-xs text-slate-400 tabular-nums">
        {new Date(label).toLocaleString("sv-SE", { month: "short", day: "numeric", hour: "2-digit", minute: "2-digit" })}
      </div>
      <div className="text-base font-semibold text-white tabular-nums">
        {isNum(v) ? v.toFixed(digits) + unit : "no reading"}
      </div>
    </div>
  );
}

function TrendChart({ points, dataKey, color, unit, digits, minPad, target, gradientId }) {
  const hasData = points.some((p) => isNum(p[dataKey]));
  if (!hasData) {
    return (
      <div className="flex h-60 items-center justify-center text-sm text-slate-400">
        No readings in this window
      </div>
    );
  }

  return (
    <ResponsiveContainer width="100%" height={240}>
      <AreaChart data={points} margin={{ top: 8, right: 12, bottom: 4, left: 0 }}>
        <defs>
          <linearGradient id={gradientId} x1="0" y1="0" x2="0" y2="1">
            <stop offset="0%" stopColor={color} stopOpacity={0.28} />
            <stop offset="100%" stopColor={color} stopOpacity={0} />
          </linearGradient>
        </defs>
        <CartesianGrid stroke={GRID_INK} strokeOpacity={0.5} vertical={false} />
        {isNum(target) && (
          <ReferenceArea
            y1={target - 10}
            y2={target + 10}
            fill={color}
            fillOpacity={0.07}
            stroke="none"
            ifOverflow="hidden"
          />
        )}
        <XAxis
          dataKey="t"
          type="number"
          scale="time"
          domain={["dataMin", "dataMax"]}
          tickFormatter={(t) => new Date(t).toLocaleTimeString("sv-SE", { hour: "2-digit", minute: "2-digit" })}
          stroke={AXIS_INK}
          tick={{ fill: AXIS_INK, fontSize: 11 }}
          tickLine={false}
          minTickGap={48}
        />
        <YAxis
          domain={domainFor(points, dataKey, minPad)}
          stroke={AXIS_INK}
          tick={{ fill: AXIS_INK, fontSize: 11 }}
          tickLine={false}
          width={52}
          tickFormatter={(v) => v.toFixed(digits)}
        />
        <Tooltip
          content={<ChartTooltip unit={unit} digits={digits} />}
          cursor={{ stroke: AXIS_INK, strokeOpacity: 0.5 }}
        />
        <Area
          type="monotone"
          dataKey={dataKey}
          stroke={color}
          strokeWidth={2}
          fill={"url(#" + gradientId + ")"}
          connectNulls={false}
          dot={false}
          isAnimationActive={false}
          activeDot={{ r: 4, strokeWidth: 2, stroke: "#0f172a" }}
        />
      </AreaChart>
    </ResponsiveContainer>
  );
}

function DutyStrip({ points, dataKey, color, gradientId }) {
  return (
    <ResponsiveContainer width="100%" height={72}>
      <AreaChart data={points} margin={{ top: 4, right: 12, bottom: 0, left: 0 }}>
        <defs>
          <linearGradient id={gradientId} x1="0" y1="0" x2="0" y2="1">
            <stop offset="0%" stopColor={color} stopOpacity={0.55} />
            <stop offset="100%" stopColor={color} stopOpacity={0.05} />
          </linearGradient>
        </defs>
        <XAxis dataKey="t" type="number" scale="time" domain={["dataMin", "dataMax"]} hide />
        <YAxis domain={[0, 1]} hide />
        <Tooltip
          cursor={{ stroke: AXIS_INK, strokeOpacity: 0.5 }}
          content={({ active, payload, label }) => {
            if (!active || !payload || !payload.length) return null;
            const v = payload[0].value;
            return (
              <div className="rounded-lg border border-slate-700 bg-slate-900/95 px-3 py-2">
                <div className="text-xs text-slate-400 tabular-nums">
                  {new Date(label).toLocaleTimeString("sv-SE", { hour: "2-digit", minute: "2-digit" })}
                </div>
                <div className="text-sm font-semibold text-white tabular-nums">
                  {isNum(v) ? Math.round(v * 100) + "% on" : "no data"}
                </div>
              </div>
            );
          }}
        />
        <Area
          type="stepAfter"
          dataKey={dataKey}
          stroke={color}
          strokeWidth={1.5}
          fill={"url(#" + gradientId + ")"}
          connectNulls={false}
          dot={false}
          isAnimationActive={false}
        />
      </AreaChart>
    </ResponsiveContainer>
  );
}

function ActuatorTile(props) {
  const { label, on, available, duty, range, spin } = props;
  const Icon = props.Icon;
  const running = on === true;
  const stateText = !available ? "Unknown" : running ? "Running" : "Idle";
  return (
    <div className="bg-white/10 backdrop-blur-md rounded-2xl border border-white/20 p-5">
      <div className="flex items-center gap-3">
        <div className={"p-2 rounded-lg " + (running ? "bg-emerald-500/20" : "bg-slate-600/20")}>
          <Icon
            className={
              "w-5 h-5 " +
              (running ? "text-emerald-400" : "text-slate-400") +
              (running && spin ? " animate-spin" : running ? " animate-pulse" : "")
            }
          />
        </div>
        <div className="min-w-0">
          <p className="text-sm text-slate-400">{label}</p>
          {/* spelled out, so state never rests on colour alone */}
          <p className="text-lg font-semibold text-white">{stateText}</p>
        </div>
      </div>
      <div className="mt-3 border-t border-slate-700/60 pt-2 text-xs text-slate-400 tabular-nums">
        {duty
          ? "Last " + range + ": " + duty.dutyPct + "% on \u00b7 " + duty.cycles + " cycles \u00b7 " + fmtDuration(duty.onSeconds)
          : "No history for the last " + range}
      </div>
    </div>
  );
}

export default function Dashboard() {
  const [data, setData] = useState({
    temperature: null,
    humidity: null,
    pressure: null,
    timestamp: null
  });
  const [phases, setPhases] = useState(['Incubation', 'Primordia', 'Fruiting']);
  const [currentPhase, setCurrentPhase] = useState("Incubation");
  const [loading, setLoading] = useState(false);
  const [isConnected, setIsConnected] = useState(false);
  const [lastUpdate, setLastUpdate] = useState(null);
  const [isUpdating, setIsUpdating] = useState(false);
  const [updateStatus, setUpdateStatus] = useState('');
  const [uptime, setUptime] = useState(null);
  const [range, setRange] = useState('1h');
  const [trends, setTrends] = useState(null);

  // Fetch functions with better error handling and real-time updates
  const fetchLatestData = async () => {
    try {
      console.log('Fetching latest sensor data...');
      const response = await fetch('/api/data');
      
      if (response.ok) {
        const newData = await response.json();
        console.log('Received data:', newData);
        
        // The server always answers, so a 200 only means the Pi is alive.
        // esp32_connected is the server's freshness check on the last POST.
        setIsConnected(newData.esp32_connected === true);
        
        // Always update state with new data (remove mock check)
        // A null reading means the sensor is not answering. Falling back to the
        // previous value here made a dead BME280 look like live data.
        setData({
          temperature: isNum(parseFloat(newData.temperature)) ? parseFloat(newData.temperature) : null,
          humidity: isNum(parseFloat(newData.humidity)) ? parseFloat(newData.humidity) : null,
          pressure: isNum(parseFloat(newData.pressure)) ? parseFloat(newData.pressure) : null,
          timestamp: newData.timestamp || new Date().toISOString(),
          state: newData.state ?? null,
          humidifier_on: newData.humidifier_on ?? null,
          fans_on: newData.fans_on ?? null,
          inlet_fan_on: newData.inlet_fan_on ?? null,
          exhaust_fan_on: newData.exhaust_fan_on ?? null,
          target_temperature: newData.target_temperature ?? null,
          target_humidity: newData.target_humidity ?? null
        });
        
        setLastUpdate(new Date(newData.timestamp || Date.now()));
        
      } else {
        console.error('Failed to fetch data:', response.status);
        setIsConnected(false);
      }
    } catch (error) {
      console.error('Error fetching sensor data:', error);
      setIsConnected(false);
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

  const fetchTrends = async (r) => {
    try {
      const response = await fetch(`/api/trends?range=${r}`);
      if (response.ok) setTrends(await response.json());
    } catch (error) {
      console.error('Failed to fetch trends:', error);
    }
  };

  const fetchUptime = async () => {
    try {
      const response = await fetch('/api/uptime');
      if (response.ok) {
        const uptimeData = await response.json();
        setUptime(uptimeData);
      }
    } catch (error) {
      console.error('Failed to fetch uptime:', error);
    }
  };

  const formatUptime = (uptimeData) => {
    if (!uptimeData) return 'Unknown';
    
    const { days, hours, minutes, seconds } = uptimeData.formatted;
    
    if (days > 0) {
      return `${days}d ${hours}h ${minutes}m`;
    } else if (hours > 0) {
      return `${hours}h ${minutes}m`;
    } else if (minutes > 0) {
      return `${minutes}m ${seconds}s`;
    } else {
      return `${seconds}s`;
    }
  };

  // System update function
  const handleSystemUpdate = async () => {
    if (isUpdating) return;
    
    const confirmed = window.confirm(
      'This will fetch the latest changes from git and restart the system. The dashboard will be unavailable for about 1-2 minutes. Continue?'
    );
    
    if (!confirmed) return;
    
    setIsUpdating(true);
    setUpdateStatus('Starting update...');
    
    try {
      const response = await fetch('/api/update-system', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
      });
      
      const result = await response.json();
      
      if (result.success) {
        if (result.changes) {
          setUpdateStatus('Update completed! Restarting...');
          // Show success message for a few seconds before the server restarts
          setTimeout(() => {
            setUpdateStatus('Server is restarting...');
          }, 2000);
          
          // After 10 seconds, start checking if server is back online
          setTimeout(() => {
            window.location.reload();
          }, 10000);
        } else {
          setUpdateStatus('System is already up to date');
          setIsUpdating(false);
          setTimeout(() => setUpdateStatus(''), 3000);
        }
      } else {
        setUpdateStatus(`Update failed: ${result.error}`);
        setIsUpdating(false);
        setTimeout(() => setUpdateStatus(''), 5000);
      }
    } catch (error) {
      setUpdateStatus('Update failed: Network error');
      setIsUpdating(false);
      setTimeout(() => setUpdateStatus(''), 5000);
    }
  };

  // Initial setup and intervals
  useEffect(() => {
    console.log('Dashboard mounted, setting up data fetching...');
    
    // Fetch initial data
    fetchLatestData();
    fetchPhases();
    fetchCurrentPhase();
    fetchUptime();
    
    // Set up more frequent updates for real-time feel
    const dataInterval = setInterval(() => {
      console.log('Interval: fetching latest data...');
      fetchLatestData();
    }, 5000); // Check every 5 seconds for new data

    // Less frequent phase check
    const phaseInterval = setInterval(() => {
      fetchCurrentPhase();
    }, 30000); // Check phase every 30 seconds

    // Update uptime every 10 seconds
    const uptimeInterval = setInterval(() => {
      fetchUptime();
    }, 10000);

    return () => {
      clearInterval(dataInterval);
      clearInterval(phaseInterval);
      clearInterval(uptimeInterval);
    };
  }, []);

  // The grow log only gains a row every 30s, so trends poll far more slowly
  // than the live reading does.
  useEffect(() => {
    fetchTrends(range);
    const id = setInterval(() => fetchTrends(range), 30000);
    return () => clearInterval(id);
  }, [range]);

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
        console.log('Phase updated to:', newPhase);
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
    if (!isNum(value)) return 'text-slate-400';
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
    if (!isNum(value)) return <AlertTriangle className="w-4 h-4 text-slate-400" />;
    const color = getStatusColor(value, type);
    if (color.includes('green')) return <CheckCircle className="w-4 h-4 text-green-400" />;
    if (color.includes('yellow')) return <AlertTriangle className="w-4 h-4 text-yellow-400" />;
    return <AlertTriangle className="w-4 h-4 text-red-400" />;
  };

  const trendPoints = trends?.points ?? [];
  // Older firmware reports only the combined flag; fall back so the tiles still
  // read correctly on a node that has not been reflashed yet.
  const inletOn = data.inlet_fan_on ?? data.fans_on ?? null;
  const exhaustOn = data.exhaust_fan_on ?? data.fans_on ?? null;

  const phaseInfo = {
    'Incubation': { duration: '7-14 days', temp: '20-24°C', humidity: '85-95%', description: 'Initial spore germination' },
    'Primordia': { duration: '5-10 days', temp: '18-22°C', humidity: '90-95%', description: 'Pin formation begins' },
    'Fruiting': { duration: '7-14 days', temp: '16-20°C', humidity: '85-92%', description: 'Mushroom development' },
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-slate-800 to-slate-900 p-4 lg:p-8">
      <div className="max-w-7xl mx-auto space-y-6">
        
        {/* Header with connection status */}
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
            <div className="flex flex-col lg:flex-row lg:items-center lg:justify-between gap-4">
              {/* Connection status indicator */}
              <div className="flex items-center gap-2">
                {isConnected ? (
                  <Wifi className="w-4 h-4 text-green-400" />
                ) : (
                  <WifiOff className="w-4 h-4 text-red-400" />
                )}
                <span className={`text-sm ${isConnected ? 'text-green-400' : 'text-red-400'}`}>
                  {isConnected ? 'Connected' : 'Disconnected'}
                </span>
              </div>
              <div className="flex flex-col sm:flex-row gap-4 text-slate-300 text-sm">
                <div className="flex items-center gap-2">
                  <Clock className="w-4 h-4" />
                  <span>Last update: {lastUpdate ? lastUpdate.toLocaleTimeString('sv-SE') : 'Never'}</span>
                </div>
                <div className="flex items-center gap-2">
                  <Timer className="w-4 h-4" />
                  <span>Uptime: {formatUptime(uptime)}</span>
                </div>
              </div>
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
                    {fmtVal(data.temperature, 1, '°C')}
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
                  className="h-full bg-gradient-to-r from-orange-500 to-orange-400 rounded-full transition-all duration-500"
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
                    {fmtVal(data.humidity, 1, '%')}
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
                  className="h-full bg-gradient-to-r from-blue-500 to-blue-400 rounded-full transition-all duration-500"
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
                    {fmtVal(data.pressure, 1, ' hPa')}
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
                  className="h-full bg-gradient-to-r from-purple-500 to-purple-400 rounded-full transition-all duration-500"
                  style={{ width: `${Math.min(100, ((data.pressure - 990) / 30) * 100)}%` }}
                />
              </div>
            </div>
          </div>
        </div>

        {/* Actuator status - live state plus how hard each one has worked */}
        <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
          <ActuatorTile
            Icon={Wind}
            label="Inlet fan"
            spin
            on={inletOn}
            available={isConnected && inletOn !== null}
            duty={trends?.actuators?.fans}
            range={range}
          />
          <ActuatorTile
            Icon={Fan}
            label="Exhaust fan"
            spin
            on={exhaustOn}
            available={isConnected && exhaustOn !== null}
            duty={trends?.actuators?.fans}
            range={range}
          />
          <ActuatorTile
            Icon={CloudDrizzle}
            label="Humidifier"
            on={data.humidifier_on ?? null}
            available={isConnected && (data.humidifier_on ?? null) !== null}
            duty={trends?.actuators?.humidifier}
            range={range}
          />
        </div>

        <div className="grid grid-cols-1 xl:grid-cols-3 gap-6">
          {/* Environmental trends - one chart per measure, each on its own scale */}
          <div className="xl:col-span-2 space-y-6">
            <div className="flex flex-wrap items-center justify-between gap-3">
              <div className="flex items-center gap-3">
                <Activity className="w-5 h-5 text-emerald-400" />
                <h2 className="text-xl font-semibold text-white">Environmental Trends</h2>
                {trends && (
                  <span className="text-xs text-slate-400 tabular-nums">
                    {trends.sampleCount} samples &middot; {trends.coveragePct}% coverage
                  </span>
                )}
              </div>
              <div className="inline-flex rounded-lg border border-white/20 bg-white/5 p-1">
                {RANGES.map((r) => (
                  <button
                    key={r}
                    onClick={() => setRange(r)}
                    aria-pressed={range === r}
                    className={
                      "px-3 py-1.5 text-sm rounded-md transition-colors " +
                      (range === r
                        ? "bg-emerald-600 text-white"
                        : "text-slate-300 hover:text-white hover:bg-white/10")
                    }
                  >
                    {r}
                  </button>
                ))}
              </div>
            </div>

            <div className="bg-white/10 backdrop-blur-md rounded-2xl border border-white/20 p-6">
              <div className="flex items-baseline justify-between mb-4">
                <h3 className="text-base font-semibold text-white">Temperature</h3>
                <span className="text-sm text-slate-400 tabular-nums">
                  now {fmtVal(data.temperature, 1, "\u00b0C")}
                  {isNum(data.target_temperature) && " \u00b7 target " + data.target_temperature.toFixed(1) + "\u00b0C \u00b110"}
                </span>
              </div>
              <TrendChart
                points={trendPoints}
                dataKey="temperature"
                color={C_TEMP}
                unit="\u00b0C"
                digits={1}
                minPad={0.5}
                target={data.target_temperature}
                gradientId="tempFill"
              />
            </div>

            <div className="bg-white/10 backdrop-blur-md rounded-2xl border border-white/20 p-6">
              <div className="flex items-baseline justify-between mb-4">
                <h3 className="text-base font-semibold text-white">Humidity</h3>
                <span className="text-sm text-slate-400 tabular-nums">
                  now {fmtVal(data.humidity, 1, "%")}
                  {isNum(data.target_humidity) && " \u00b7 target " + data.target_humidity.toFixed(0) + "% \u00b110"}
                </span>
              </div>
              <TrendChart
                points={trendPoints}
                dataKey="humidity"
                color={C_HUMIDITY}
                unit="%"
                digits={1}
                minPad={2}
                target={data.target_humidity}
                gradientId="humidityFill"
              />
            </div>

            <div className="bg-white/10 backdrop-blur-md rounded-2xl border border-white/20 p-6">
              <h3 className="text-base font-semibold text-white mb-1">Actuator usage</h3>
              <p className="text-xs text-slate-400 mb-4">
                Share of each interval the actuator was on, over the last {range}.
              </p>

              {trendPoints.length ? (
                <div className="space-y-4">
                  <div>
                    <div className="flex items-baseline justify-between mb-1">
                      <span className="text-sm text-slate-300">Humidifier</span>
                      <span className="text-xs text-slate-400 tabular-nums">
                        {trends?.actuators?.humidifier
                          ? trends.actuators.humidifier.dutyPct + "% on"
                          : "\u2014"}
                      </span>
                    </div>
                    <DutyStrip
                      points={trendPoints}
                      dataKey="humidifier_duty"
                      color={C_HUMIDITY}
                      gradientId="humDutyFill"
                    />
                  </div>

                  <div>
                    <div className="flex items-baseline justify-between mb-1">
                      <span className="text-sm text-slate-300">Fans (inlet + exhaust)</span>
                      <span className="text-xs text-slate-400 tabular-nums">
                        {trends?.actuators?.fans ? trends.actuators.fans.dutyPct + "% on" : "\u2014"}
                      </span>
                    </div>
                    <DutyStrip
                      points={trendPoints}
                      dataKey="fan_duty"
                      color={C_FAN}
                      gradientId="fanDutyFill"
                    />
                    {/* Both fans are switched by one call, so the log cannot separate
                        them; the live tiles above report each pin individually. */}
                    <p className="mt-1 text-xs text-slate-500">
                      Driven together, so both fans share one history.
                    </p>
                  </div>
                </div>
              ) : (
                <div className="flex h-24 items-center justify-center text-sm text-slate-400">
                  No actuator history for this window
                </div>
              )}
            </div>
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
                  disabled={loading}
                  className="w-full bg-slate-800/70 border border-slate-600 rounded-lg px-3 py-2 text-white focus:outline-none focus:ring-2 focus:ring-emerald-500 focus:border-transparent disabled:opacity-50"
                >
                  {phases.map((phase) => (
                    <option key={phase} value={phase} className="bg-slate-800">
                      {phase}
                    </option>
                  ))}
                </select>
                {loading && <p className="text-xs text-yellow-400 mt-1">Updating phase...</p>}
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
              <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-3 mb-4">
                <h3 className="text-lg font-semibold text-white">System Status</h3>
                <button
                  onClick={handleSystemUpdate}
                  disabled={isUpdating}
                  className="flex items-center justify-center gap-2 px-3 py-2 bg-emerald-600 hover:bg-emerald-700 disabled:bg-slate-600 text-white text-sm rounded-lg transition-colors duration-200 disabled:cursor-not-allowed whitespace-nowrap"
                >
                  {isUpdating ? (
                    <RefreshCw className="w-4 h-4 animate-spin" />
                  ) : (
                    <Download className="w-4 h-4" />
                  )}
                  {isUpdating ? 'Updating...' : 'Update System'}
                </button>
              </div>
              
              {updateStatus && (
                <div className={`mb-4 p-3 rounded-lg text-sm ${
                  updateStatus.includes('failed') || updateStatus.includes('error') 
                    ? 'bg-red-500/20 text-red-300' 
                    : updateStatus.includes('completed') || updateStatus.includes('up to date')
                    ? 'bg-green-500/20 text-green-300'
                    : 'bg-blue-500/20 text-blue-300'
                }`}>
                  {updateStatus}
                </div>
              )}
              
              <div className="space-y-3">
                <div className="flex items-center justify-between p-3 bg-slate-800/50 rounded-lg">
                  <div className="flex items-center gap-2">
                    <div className={`w-2 h-2 rounded-full ${isConnected ? 'bg-green-400' : 'bg-red-400'}`}></div>
                    <span className="text-sm text-slate-300">ESP32 Connection</span>
                  </div>
                  <span className={`text-xs ${isConnected ? 'text-green-400' : 'text-red-400'}`}>
                    {isConnected ? 'Online' : 'Offline'}
                  </span>
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
                    <span className="text-sm text-slate-300">Auto Updates</span>
                  </div>
                  <span className="text-xs text-yellow-400">Available</span>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}